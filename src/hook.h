#ifndef HOOK_H
#define HOOK_H

#include <windows.h>

BOOL installHook();
void uninstallHook();
const char *getLogFilePath();

#endif