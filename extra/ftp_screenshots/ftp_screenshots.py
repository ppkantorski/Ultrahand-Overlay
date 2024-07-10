import ftplib
import os
import time
import configparser
from datetime import datetime

TITLE = "Switch FTP Screenshots"
VERSION = "0.1.2"
AUTHOR = "ppkantorski"

# Determine the directory where the script is located
script_dir = os.path.dirname(os.path.abspath(__file__))

# Path to the config.ini file
config_path = os.path.join(script_dir, 'config.ini')

# Read configuration from config.ini
config = configparser.ConfigParser()
config.read(config_path)

# FTP server details
FTP_SERVER = config.get('FTP', 'FTP_SERVER')
FTP_PORT = config.getint('FTP', 'FTP_PORT')
FTP_USER = config.get('FTP', 'FTP_USER')
FTP_PASS = config.get('FTP', 'FTP_PASS')
FTP_PATH = config.get('FTP', 'FTP_PATH')

# Local directory to save files
OUTPUT_PATH = config.get('LOCAL', 'OUTPUT_PATH')
CHECK_RATE = int(config.get('SETTINGS', 'CHECK_RATE'))
DT_FORMAT = config.get('SETTINGS', 'DT_FORMAT')

def log_message(message):
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] {message}")

def connect_ftp():
    ftp = ftplib.FTP()
    ftp.connect(FTP_SERVER, FTP_PORT)
    ftp.login(FTP_USER, FTP_PASS)
    return ftp

def list_files(ftp, path):
    file_list = []
    try:
        ftp.cwd(path)
        files = ftp.nlst()
        for file in files:
            full_path = os.path.join(path, file)
            try:
                ftp.cwd(full_path)
                file_list.extend(list_files(ftp, full_path))
                ftp.cwd('..')
            except ftplib.error_perm:
                file_list.append(full_path)
    except ftplib.error_perm:
        pass
    return file_list

def download_file(ftp, remote_file, local_file):
    with open(local_file, 'wb') as f:
        ftp.retrbinary(f'RETR {remote_file}', f.write)

def format_filename(file_name, dt_format):
    # Extract timestamp part from the filename (without extension)
    base_name, extension = os.path.splitext(file_name)
    try:
        timestamp_str = base_name.split('-')[0]
        # Parse timestamp into a datetime object
        timestamp_dt = datetime.strptime(timestamp_str, '%Y%m%d%H%M%S%f')
        # Format datetime object into the desired string format
        formatted_str = timestamp_dt.strftime(dt_format)
        return formatted_str
    except ValueError:
        # If timestamp parsing fails, return the original filename without modification
        return base_name  # Return the base name without the extension

def clear_screen():
    if os.name == 'nt':  # For Windows
        os.system('cls')
    else:  # For macOS and Linux
        os.system('clear')

def delete_line():
    print("\033[F\033[K", end='')  # Move cursor up one line and clear the line

def main():
    clear_screen()
    print(f"{TITLE} v{VERSION} by {AUTHOR}\n")
    time.sleep(1)
    log_message("Screenshots mirroring service has been initialized.")
    time.sleep(1)
    connection_success = False
    initial_loop = True
    was_last_message = False
    while True:
        start_time = time.time()
        try:
            ftp = connect_ftp()
            current_files = list_files(ftp, FTP_PATH)
            if connection_success and was_last_message:
                delete_line()
            log_message(f"FTP Connection to {FTP_SERVER} successful.")
            connection_success = True
            was_last_message = True
            for file in current_files:
                file_name = os.path.basename(file)
                if DT_FORMAT:
                    formatted_name = format_filename(file_name, DT_FORMAT)
                    local_file_path = os.path.join(OUTPUT_PATH, formatted_name + os.path.splitext(file_name)[1])
                else:
                    local_file_path = os.path.join(OUTPUT_PATH, file_name)
                if not os.path.exists(local_file_path):
                    try:
                        download_file(ftp, file, local_file_path)
                        log_message(f"Downloaded: {file}")
                        was_last_message = False
                    except Exception as e:
                        log_message(f"Error downloading {file}: {e}")
                        was_last_message = False
            
            ftp.quit()
        except Exception as e:
            if not connection_success and not initial_loop:
                delete_line()
            log_message(f"Error connecting to FTP server: {e}")
            connection_success = False
        
        initial_loop = False
        elapsed_time = time.time() - start_time
        sleep_time = max(0, CHECK_RATE - elapsed_time)
        time.sleep(sleep_time)

if __name__ == "__main__":
    main()
