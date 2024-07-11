import os
import platform
import subprocess

# Determine the operating system
current_platform = platform.system()

# Function to install required modules
def install_requirements():
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "-r", "requirements.txt"])
    except subprocess.CalledProcessError as e:
        print(f"Error installing requirements: {e}")
        sys.exit(1)

# Delete dist and build folders if they exist
if os.path.exists('dist'):
    os.system('rm -rf dist')
if os.path.exists('build'):
    os.system('rm -rf build')

# Install required modules
install_requirements()

# Run PyInstaller with the .spec file
os.system('pyinstaller ftp_screenshots.spec')
