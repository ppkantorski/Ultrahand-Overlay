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
server = X.X.X.X
port = 5000
user = root
pass = 

[LOCAL]
output_path = /your/desired/pics/folder/

[SETTINGS]
auto_start = False
check_rate = 20
dt_format = %Y-%m-%d_%H-%M-%S
```

- `ftp_server`: IP address of the FTP server.
- `ftp_port`: Port number of the FTP server.
- `ftp_user`: Username for FTP login.
- `ftp_pass`: Password for FTP login (leave empty if no password).
- `output_path`: Local directory where files will be saved.
- `auto_start`: Variable for auto start (`True`/`False`)
- `check_rate`: Time interval (in seconds) to wait between checks.
- `dt_format`: Format of image file name.

## Usage

1. Clone or download the repository.
2. Install required packages:
    - Ensure you have Python 3 installed.
    - Install necessary packages using pip if not already installed:
      ```pip install configparser```
3. Run the script:
    - `python3 make.py`
