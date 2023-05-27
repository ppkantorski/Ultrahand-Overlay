#!/usr/bin/python3
import os, shutil, time

username = os.getlogin()
bootloader_path = f"/media/{username}/SWITCH SD1/bootloader/"
backup_path = bootloader_path+"hekate_ipl.ini.bak"
config_path = bootloader_path+"hekate_ipl.ini"

print(bootloader_path)
while True:
    if os.path.exists(bootloader_path):
        # Copy and replace the file
        if os.path.exists(backup_path) and os.path.exists(config_path):
            os.remove(config_path)  # Remove the existing destination file
            shutil.copy2(backup_path, config_path)  # Copy the source file to the destination location
            os.remove(backup_path)  # Remove the existing destination file
            break
        else:
            break
    else:
        time.sleep(1)
