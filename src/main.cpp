#include "hook.h"
#include <cstdio>

int main()
{
    if (!installHook())
    {
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }

    uninstallHook();
    return 0;
}

