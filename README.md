# Keylogger Project

A Windows keylogger application with a Python server for remote log collection and management.

## Project Structure

```
keylogger/
├── src/                    # C++ source code for Windows keylogger
│   ├── main.cpp           # Main keylogger application
│   ├── hook.cpp           # Windows hook implementation
│   ├── hook.h             # Hook header file
│   ├── config.h           # Configuration settings
│   └── build.bat          # Windows build script
├── src/keylogger-server/  # Python server for log collection
│   ├── server.py          # Flask web server
│   └── requirements.txt   # Python dependencies
├── logs/                  # Log storage directory
└── render.yaml           # Render deployment configuration
```

## Components

### 1. Windows Keylogger (C++)

The Windows keylogger is a C++ application that:
- Captures keystrokes using Windows hooks
- Saves logs locally with timestamps
- Sends log files to the remote server via HTTP POST
- Runs as a background service

**Features:**
- Silent operation (no visible UI)
- Automatic log file generation
- Remote log upload capability
- Configurable capture settings

**Build Instructions:**
```bash
cd src
build.bat
```

### 2. Keylogger Server (Python/Flask)

A Python Flask server that:
- Receives log files from Windows clients
- Stores logs with timestamps
- Provides API endpoints for log access
- Runs on Render cloud platform

**Features:**
- RESTful API for log management
- File upload endpoint (`/upload`)
- Download latest n logs endpoint (`/download-latest-log?n=`)


**Deployment:**
- Configured for Render cloud deployment
- Uses `render.yaml` for automated deployment
- Root directory: `src/keylogger-server`

## API Endpoints

### Upload Log File
```http
POST /upload
Content-Type: multipart/form-data

file: [log file]
```

###  Download latest n logs endpoint 
```http
GET /download-latest-log?n=
```
Downloads the specified log file.

###  list logs
```http
GET /logs
```
List existing logs

## Local Development

### Prerequisites
- Python 3.9+
- Windows SDK (for C++ build)
- Visual Studio Build Tools

### Setup Python Server
```bash
cd src/keylogger-server
python3 -m venv keylogger-server
source keylogger-server/bin/activate  # On Windows: keylogger-server\Scripts\activate
pip install -r requirements.txt
python server.py
```

### Build Windows Keylogger
```bash
cd src
build.bat
```

## Deployment

The server is configured for deployment on Render:

1. **Repository**: `https://github.com/stephenthajeb/keylogger`
2. **Branch**: `main`
3. **Root Directory**: `src/keylogger-server`
4. **Build Command**: `pip install -r requirements.txt`
5. **Start Command**: `python server.py`

## Security Considerations

⚠️ **Important**: This project is for educational purposes only.

- The Windows keylogger operates silently and captures sensitive data
- Log files contain potentially sensitive information
- Server endpoints are currently public (consider adding authentication)
- Use responsibly and only on systems you own

## Usage

1. **Deploy the server** to Render or your preferred platform
2. **Build the Windows keylogger** using the provided build script
3. **Configure the client** to send logs to your server URL
4. **Access logs** via the web interface or API endpoints

## File Structure Details

### Windows Keylogger Files
- `main.cpp`: Entry point and main application logic
- `hook.cpp/hook.h`: Windows keyboard hook implementation
- `config.h`: Configuration constants and settings
- `build.bat`: Windows compilation script

### Server Files
- `server.py`: Flask web server with upload/download endpoints
- `requirements.txt`: Python dependencies (Flask)
- `logs/`: Directory where uploaded log files are stored


## License

This project is for educational purposes. Use responsibly and in accordance with applicable laws and regulations.


## Project Deliverables
- [Slide](https://unsw-my.sharepoint.com/:p:/g/personal/z5631971_ad_unsw_edu_au/EchOVUjmZThNj1qlMIXYSLwBrOsPpQcvJOH7J9-1Q9NGig?e=QJ3w0S)
- [Project Summary](https://docs.google.com/document/d/15o5bh60u5ztfRfXZL6g6S7dgO_Obo0qOFTUKlc8ogHQ/edit?usp=sharing)
- [Source Code](https://github.com/stephenthajeb/keylogger)

If the link is not clickable, you can find the slide and project summary document in this repo as alternative.