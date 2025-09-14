import json
import time
import os
import platform

from ftplib import FTP
import sys
from prompt_toolkit import PromptSession
from prompt_toolkit.history import FileHistory
from prompt_toolkit.formatted_text import HTML
from prompt_toolkit.completion import Completer, Completion, WordCompleter

# --- Global FTP credentials ---
FTP_HOST = "192.168.3.3"
FTP_PORT = 5000
FTP_USERNAME = "root"
FTP_PASSWORD = ""
REMOTE_PATH = "/config/ultrahand/notifications/"

def generate_and_upload_notify(text, font_size=28):
    """
    Generates a notification JSON file with the given text and font size, then uploads it via FTP.
    """
    font_size = max(1, min(34, font_size))  # Clamp font size

    program_folder = os.path.dirname(os.path.abspath(__file__))
    timestamp = int(time.time())
    filename = f"ultrahand_notify-{timestamp}.notify"
    local_file = os.path.join(program_folder, filename)

    notify_data = {
        "text": text,
        "font_size": font_size,
        "priority": 10
    }

    with open(local_file, "w") as f:
        json.dump(notify_data, f, indent=4)

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
        os.system("clear; clear")
    elif system_name == "Windows":
        os.system("cls; cls")
    else:
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

def print_help():
    print("""
Available commands:
  /help                Show this help message
  /quit or /exit       Exit interactive mode
  /clear               Clear the screen
  /font_size N         Set font size (1–34)

Just type any other text to send it as a notification.
""")


# --- Interactive mode ---
def interactive_mode():
    clear_screen()
    print_banner()
    print_help()

    history_file = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".notify_history")

    commands = ["/quit", "/exit", "/clear", "/font_size", "/help"]
    command_completer = WordCompleter(
        commands,
        ignore_case=True,
        sentence=True,     # allow matching anywhere
        #match_middle=True  # allow substring matching
    )

    session = PromptSession(
        history=FileHistory(history_file),
        completer=command_completer,
        complete_while_typing=True
    )

    current_font_size = 28

    while True:
        try:
            text = session.prompt(HTML('<ansimagenta><u>notify</u> > </ansimagenta>'))
        except (EOFError, KeyboardInterrupt):
            break

        cmd = text.strip()

        if cmd.lower() in ["/quit", "/exit"]:
            break
        elif cmd.lower() == "/clear":
            clear_screen()
            continue
        elif cmd.lower().startswith("/font_size"):
            parts = cmd.split()
            if len(parts) == 2 and parts[1].isdigit():
                new_size = int(parts[1])
                if 1 <= new_size <= 34:
                    current_font_size = new_size
                    print(f"Font size set to {current_font_size}")
                else:
                    print("⚠ Font size must be between 1 and 34")
            else:
                print("Usage: /font_size N")
            continue
        elif cmd.lower() == "/help":
            print_help()
            continue

        # Otherwise, treat input as notification text
        generate_and_upload_notify(text, font_size=current_font_size)
        print(f"> Notification sent! (font size {current_font_size})")

# --- CLI entry ---
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
