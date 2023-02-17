#include "commands.h"
#include <Windows.h>
#include <minwindef.h>
#include <winreg.h>
#include <winuser.h>

#define RUN_AT_STARTUP_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Run"

int add_to_registry(HKEY hkey, char *path, char *name, char *value) {
  HKEY hk;
  int status = 0;
  if (RegOpenKeyExA(hkey, path, 0, KEY_ALL_ACCESS, &hk) != ERROR_SUCCESS) {
    return 1;
  }
  if (RegSetValueExA(hk, name, 0, REG_SZ, (BYTE *)value, strlen(value)) !=
      ERROR_SUCCESS) {
    status = 1;
  }
  RegCloseKey(hk);
  return status;
}

int remove_from_registry(HKEY hkey, char *path, char *name) {
  HKEY hk;
  int status = 0;
  if (RegOpenKeyExA(hkey, path, 0, KEY_ALL_ACCESS, &hk) != ERROR_SUCCESS) {
    return 1;
  }
  if (RegDeleteValueA(hk, name) != ERROR_SUCCESS) {
    status = 1;
  }
  RegCloseKey(hk);
  return status;
}

int install_to_registry(char *name, char *value) {
  return add_to_registry(HKEY_CURRENT_USER, RUN_AT_STARTUP_PATH, name, value);
}

int uninstall_to_registry(char *name) {
  return remove_from_registry(HKEY_CURRENT_USER, RUN_AT_STARTUP_PATH, name);
}

void popup(const char *title, const char *text) {
  MessageBoxA(NULL, text, title, MB_ICONWARNING);
}
