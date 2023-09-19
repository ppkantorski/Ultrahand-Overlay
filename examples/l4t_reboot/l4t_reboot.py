#!/usr/bin/python3
"""
File: l4t_reboot.py
Author: ppkantorski
Description:
    This script is part of the Ultrahand-Overlay project and is responsible for
    handling the reboot functionality for the project. It interacts with the
    bootloader and configuration files to perform the necessary reboot actions.

    Key Features:
    - Rebooting the system.
    - Backup and restoration of bootloader configurations.
    - Integration with the Ultrahand-Overlay project.

    Note: Please refer to the project documentation and README.md for detailed
    information on how to use and configure this script within the Ultrahand-Overlay.

    For the latest updates and contributions, visit the project's GitHub repository.
    (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)

Copyright (c) 2023 ppkantorski
All rights reserved.
"""
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
