"""
File: rar2zip.py
Author: ppkantorski
Description:
    This script converts .rar files to .zip files. It recursively searches a specified
    directory for .rar files, extracts their contents, and then compresses them into .zip
    files. After compression, it deletes the extracted folder to avoid leaving behind
    leftover folders from the extraction process.

    For the latest updates and contributions, visit the project's GitHub repository.
    (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)

    Note: Please be aware that this notice cannot be altered or removed. It is a part
    of the project's documentation and must remain intact.
    
Licensed under CC-BY-NC-SA-4.0
Copyright (c) 2024 ppkantorski
"""

import os
import shutil


"""
Checks for and installs necessary dependencies based on the current operating system.

This function checks the operating system and installs the required dependencies
if they are not already installed. For Windows, it checks if pyunpack is installed
and if WinRAR or 7-Zip is available. For macOS and Linux, it verifies the presence
of p7zip and pyunpack.

Note: This function relies on platform-specific package managers like pip, Homebrew,
and apt-get to install dependencies.

Raises:
    NotImplementedError: If the operating system is not supported.
"""
def install_dependencies():
    import platform
    import subprocess

    system = platform.system()
    if system == "Windows":
        # Check if pyunpack is installed
        try:
            import pyunpack
        except ImportError:
            subprocess.run("pip install pyunpack", shell=True)
            print("Please make sure you have WinRAR or 7-Zip installed on your system.")
    elif system == "Darwin":
        # Check if p7zip is installed
        try:
            subprocess.run(["7z"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        except FileNotFoundError:
            subprocess.run("brew install p7zip", shell=True)
        # Check if pyunpack is installed
        try:
            import pyunpack
        except ImportError:
            subprocess.run("pip install pyunpack", shell=True)
    elif system == "Linux":
        # Check if p7zip is installed
        try:
            subprocess.run(["7z"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        except FileNotFoundError:
            subprocess.run("sudo apt-get install p7zip-full", shell=True)
        # Check if pyunpack is installed
        try:
            import pyunpack
        except ImportError:
            subprocess.run("pip install pyunpack", shell=True)
    else:
        print("Unsupported platform.")


"""
Converts .rar files to .zip files within a specified directory.

This function recursively searches the specified directory for .rar files,
extracts their contents, and compresses them into .zip files. After compression,
it deletes the extracted folder to avoid leaving behind leftover folders from
the extraction process.

Args:
    directory (str): The path to the directory containing .rar files.

Note:
    This function relies on the pyunpack module to extract .rar files and requires
    the installation of p7zip on macOS and Linux platforms.

Raises:
    FileNotFoundError: If the specified directory does not exist.
"""
def convert_rar_to_zip(directory):
    from pyunpack import Archive
    
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".rar"):
                rar_file = os.path.join(root, file)
                try:
                    # Extract the .rar file
                    Archive(rar_file).extractall(root)
                    
                    # Get the list of extracted folders
                    extracted_folders = [name for name in os.listdir(root) if os.path.isdir(os.path.join(root, name))]
                    
                    # Process each extracted folder
                    for extracted_folder in extracted_folders:
                        # Create the .zip file for this extracted folder
                        zip_file = os.path.join(root, extracted_folder + ".zip")
                        shutil.make_archive(os.path.join(root, extracted_folder), 'zip', root, extracted_folder)
                        
                        # Remove the extracted folder
                        shutil.rmtree(os.path.join(root, extracted_folder))
                        
                    # Remove the .rar file
                    os.remove(rar_file)
                    
                except Exception as e:
                    print(f"Failed to convert: {rar_file}. Error: {e}")


"""
Removes files named "..." from all directories within the specified directory.

Args:
    directory (str): The path to the directory to clean up.
"""
def cleanup_directory(directory):
    for root, _, files in os.walk(directory):
        for file in files:
            if file == "...":
                os.remove(os.path.join(root, file))


if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        print("Usage: python3 rar2zip.py /path/to/directory")
        sys.exit(1)

    install_dependencies()
    directory = sys.argv[1]
    convert_rar_to_zip(directory)
    cleanup_directory(directory)
