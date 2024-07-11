import os
import platform

# Determine the operating system
current_platform = platform.system()

# Delete dist and build folders if they exist
if os.path.exists('dist'):
    os.system('rm -rf dist')
if os.path.exists('build'):
    os.system('rm -rf build')

os.system('pyinstaller ftp_screenshots.spec')

#if current_platform == 'Windows':
#    os.system('pyinstaller ftp_screenshots.spec')
#elif current_platform == 'Darwin':
#    os.system('pyinstaller ftp_screenshots.spec')
#else:
#    print(f"Unsupported platform: {current_platform}")
