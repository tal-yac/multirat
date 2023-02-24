#pragma once

#include "winapi.h"

DWORD WINAPI accept_clients(LPVOID lpParameter);

DWORD WINAPI client_input_handler(LPVOID lpParameter);