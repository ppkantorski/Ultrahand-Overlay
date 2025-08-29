import json
import time
import os
import platform

from ftplib import FTP
import sys
from prompt_toolkit import PromptSession
from prompt_toolkit.history import FileHistory
from prompt_toolkit.formatted_text import HTML
from prompt_toolkit.completion import WordCompleter

# --- Global FTP credentials ---
FTP_HOST = "192.168.1.7"
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
        "font_size": font_size
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

def clear_screen():
    """
    Clear the terminal screen in a cross-platform way.
    Works on macOS, Linux, Windows, and a-Shell (iOS).
    """
    system_name = platform.system()
    if system_name in ("Linux", "Darwin"):  # Darwin = macOS / a-Shell
        os.system("clear")
    elif system_name == "Windows":
        os.system("cls")
    else:
        # fallback
        print("\n" * 100)

def print_banner():
    banner = """
  \033[37multra\033[31mhand\033[0m \033[35mnotify\033[0m \033[37m¯\\_(ツ)_/¯ \033[0m
      \033[93m***\033[0m \033[93m*\033[0m    \033[93m**\033[0m    R    R   
   \033[93m* \033[93m*\033[0m   \033[93m*  **\033[0m       X    X   
  \033[93m* \033[93m*\033[0m         \033[93m*  \033[93m*  \033[93m*0\033[0m    0\033[93m*\033[0m  
      \033[93m* \033[93m*\033[0m            1    1\033[93m*\033[0m  
                    *2    2   
   \033[93m**\033[0m         \033[93m* \033[0m     3    3   
  \033[93m* \033[0m       \033[93m* \033[0m        4    4   
         \033[93m*  \033[93m*\033[0m        5\033[93m*\033[0m   5   
             \033[93m*  \033[93m*\033[0m    6   \033[93m*6\033[0m   
                     7    7   
       \033[93m* \033[0m         \033[93m*  \033[93m*8\033[0m  \033[93m*8\033[0m   
     \033[93m* \033[0m         \033[93m* \033[0m   9    9   
"""
    print(banner)


# --- Interactive mode ---
def interactive_mode():
    clear_screen()
    print_banner()

    print("Entering interactive notification mode.\nType '/quit' to exit.")

    # Store history in a file next to script
    history_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".notify_history")

    # Built-in commands
    commands = ["/quit", "/exit", "/clear"]
    command_completer = WordCompleter(commands, ignore_case=True, match_middle=True)

    # Create a prompt session with persistent history and autocomplete
    session = PromptSession(history=FileHistory(history_file), completer=command_completer, complete_while_typing=True)

    while True:
        try:
            # "notify" in purple + underlined, ">" plain
            text = session.prompt(HTML('<ansimagenta><u>notify</u> > </ansimagenta>'))
        except (EOFError, KeyboardInterrupt):
            print("\nExiting interactive mode.")
            break

        cmd = text.strip().lower()
        if cmd in ["/quit", "/exit"]:
            break
        elif cmd == "/clear":
            clear_screen()
            continue

        generate_and_upload_notify(text)
        print("Notification sent!")

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
