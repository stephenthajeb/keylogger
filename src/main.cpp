#include "hook.h"

int main()
{
    if (!installHook())
    {
        MessageBoxA(NULL, "Failed to install keyboard hook.", "Error", MB_ICONERROR);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    uninstallHook();
    return 0;
}

