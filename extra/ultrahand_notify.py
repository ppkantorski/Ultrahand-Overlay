import json
import time
import os
from ftplib import FTP
import sys

# --- Global FTP credentials ---
FTP_HOST = "192.168.1.101"
FTP_PORT = 5000
FTP_USERNAME = "root"
FTP_PASSWORD = ""
REMOTE_PATH = "/config/ultrahand/notifications/"

def generate_and_upload_notify(text, font_size=28):
    """
    Generates a notification JSON file with the given text and font size, then uploads it via FTP.

    :param text: Notification text to display
    :param font_size: Font size to use (default: 28)
    """

    # Clamp font size to 1–34
    font_size = max(1, min(34, font_size))

    # --- Generate local JSON file ---
    program_folder = os.path.dirname(os.path.abspath(__file__))
    timestamp = int(time.time())
    filename = f"ftp-{timestamp}.notify"
    local_file = os.path.join(program_folder, filename)

    notify_data = {
        "text": text,
        "fontSize": font_size
    }

    with open(local_file, "w") as f:
        json.dump(notify_data, f, indent=4)

    # --- Upload via FTP ---
    ftp = FTP()
    ftp.connect(FTP_HOST, FTP_PORT)
    ftp.login(FTP_USERNAME, FTP_PASSWORD)

    remote_dir = REMOTE_PATH
    if not remote_dir.endswith("/"):
        remote_dir += "/"
    ftp.cwd(remote_dir)

    with open(local_file, "rb") as f:
        ftp.storbinary(f"STOR {filename}", f)

    ftp.quit()
    os.remove(local_file)

# --- Interactive mode ---
def interactive_mode():
    print("Entering interactive notification mode. Type 'quit' to exit.")
    while True:
        text = input("Enter notification text: ")
        if text.lower() in ["quit", "exit"]:
            break

        #font_size_input = input("Enter font size (1–34, default 28): ").strip()
        #try:
        #    font_size = int(font_size_input) if font_size_input else 28
        #except ValueError:
        #    print("Invalid font size. Using default 28.")
        #    font_size = 28

        generate_and_upload_notify(text)
        print("Notification sent!\n")

# --- Command line interface ---
if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1] == "-i":
        interactive_mode()
    else:
        if len(sys.argv) < 2:
            print("Usage: python3 ultrahand_notify.py 'Your notification text here' [font_size]")
            print("   or: python3 ultrahand_notify.py -i   (interactive mode)")
            sys.exit(1)

        notification_text = sys.argv[1]

        if len(sys.argv) >= 3:
            try:
                font_size = int(sys.argv[2])
            except ValueError:
                print("Invalid font size. Using default of 28.")
                font_size = 28
        else:
            font_size = 28

        generate_and_upload_notify(notification_text, font_size)