#!/bin/bash

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
