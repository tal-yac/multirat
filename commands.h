#pragma once

#include "winapi.h"
#include <winreg.h>

#define RESTART_STR "shutdown /r /t 0"
#define SHUTDOWN_STR "shutdown /s /t 0"

int add_to_registry(HKEY hkey, char *path, char *name, char *value);

int remove_from_registry(HKEY hkey, char *path, char *name);

int install_to_registry(char *name);

int uninstall_to_registry(char *name);

void popup(const char *title, const char *text);
