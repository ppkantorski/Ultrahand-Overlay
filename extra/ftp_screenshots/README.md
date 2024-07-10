# Switch FTP Screenshots Logger

## Overview

**Switch FTP Screenshots Logger** is a Python script that monitors an FTP server for new Nintendo Switch screenshots saved and downloads them to a local directory. The script is designed to work across Windows, macOS, and Linux.

## Features

- Connects to an FTP server and checks for new files in specified directories.
- Downloads new screenshots to a local directory.
- Logs all actions with timestamps.
- Clears terminal lines for a clean and readable output.

## Requirements

- Python 3.x
- FTP server accessible with the necessary credentials
    - Requires `sys-ftpd` or a similar background FTP module on the Switch.

## Configuration

The script reads configuration details from a `config.ini` file located in the same directory as the script. Below is an example `config.ini` file:

```ini
[FTP]
FTP_SERVER = <SWITCH_SERVER_IP>
FTP_PORT = 5000
FTP_USER = root
FTP_PASS = 
FTP_PATH = /emuMMC/RAW1/Nintendo/Album/

[LOCAL]
OUTPUT_PATH = <SCREENSHOT_OUTPUT_FOLDER>
CHECK_RATE = 20
```

- `FTP_SERVER`: IP address of the FTP server.
- `FTP_PORT`: Port number of the FTP server.
- `FTP_USER`: Username for FTP login.
- `FTP_PASS`: Password for FTP login (leave empty if no password).
- `FTP_PATH`: Path on the FTP server to monitor for new files.
- `OUTPUT_PATH`: Local directory where files will be saved.
- `CHECK_RATE`: Time interval (in seconds) to wait between checks.

## Usage

1. Clone or download the repository.
2. Install required packages:
    - Ensure you have Python 3 installed.
    - Install necessary packages using pip if not already installed:
      ```pip install configparser```
3. Run the script:
    - `python3 switch_screenshots.py`
