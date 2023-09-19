#!/bin/bash
################################################################################
# File: install.sh
# Author: ppkantorski
# Description:
#   This script is part of the Ultrahand-Overlay project and is responsible for
#   installing and configuring the necessary components of the project. It
#   automates the setup process for the Ultrahand-Overlay on your system.
#
#   Key Features:
#   - Installation of required dependencies.
#   - Configuration of system settings.
#   - Setup of the Ultrahand-Overlay project.
#
#   Note: Please refer to the project documentation and README.md for detailed
#   information on how to use and configure this script within the Ultrahand-Overlay.
#
#   For the latest updates and contributions, visit the project's GitHub repository.
#   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
#
# Copyright (c) 2023 ppkantorski
# All rights reserved.
################################################################################

# Set the path to your Python script
script_path="/usr/local/bin/l4t_reboot.py"

# Move the script to the desired location
mv "$(dirname "$0")/l4t_reboot.py" "$script_path"
chmod +x "$script_path"

# Create the .desktop file
echo "[Desktop Entry]
Type=Application
Exec=/usr/bin/python3 $script_path
Hidden=false
NoDisplay=false
X-GNOME-Autostart-enabled=true
Name[en_US]=L4T-Reboot
Name=L4T-Reboot
Comment[en_US]=Run L4T-Reboot on startup
Comment=Run L4T-Reboot on startup
Terminal=false" > ~/.config/autostart/L4T-Reboot.desktop

# Make the .desktop file executable
chmod +x ~/.config/autostart/L4T-Reboot.desktop
