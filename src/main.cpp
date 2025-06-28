#include <windows.h>
#include "hook.h"

int main()
{
    if (!installHook())
    {
        MessageBox(NULL, "Failed to install keyboard hook.", "Error", MB_ICONERROR);
        return 1;
    }

    // Message loop — keeps the program alive
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    uninstallHook(); // clean up before exit
    return 0;
}