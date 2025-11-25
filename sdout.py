##################################################################################
# File: sdout.py — SDOut Package Builder
#
# Description:
#   This script automates the creation of `sdout.zip`, a complete deployment
#   package for Ultrahand Overlay. It downloads, organizes, and assembles
#   Ultrahand Overlay components, nx-ovlloader, and any additional files into
#   a proper SD card structure ready for use.
#
#   The script automatically creates the required folder structure, downloads
#   and extracts dependencies, organizes their contents, and packages everything
#   into `sdout.zip` — which can be extracted directly to the root of an SD card.
#
# Related Projects:
#   - Ultrahand Overlay: https://github.com/ppkantorski/Ultrahand-Overlay
#   - nx-ovlloader: https://github.com/ppkantorski/nx-ovlloader
#
#   For the latest updates or to contribute, visit the GitHub repository:
#   https://github.com/ppkantorski/Ultrahand-Overlay
#
# Note:
#   This notice is part of the official project documentation and must not
#   be altered or removed.
#
# Requirements:
#   - Python 3.6+
#   - requests library (`pip install requests`)
#   - `ovlmenu.ovl` file in the script directory
#
# Licensed under GPLv2
# Copyright (c) 2025 ppkantorski
##################################################################################

import os
import shutil
import zipfile
import requests
from pathlib import Path
import tempfile

def download_file(url, destination):
    """Download a file from URL to destination"""
    print(f"Downloading {url}...")
    response = requests.get(url, stream=True)
    response.raise_for_status()
    
    with open(destination, 'wb') as f:
        for chunk in response.iter_content(chunk_size=8192):
            f.write(chunk)
    print(f"Downloaded to {destination}")

def extract_zip(zip_path, extract_to, exclude_metadata=True):
    """Extract zip file, optionally excluding metadata files"""
    print(f"Extracting {zip_path}...")
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        for member in zip_ref.namelist():
            # Skip metadata files like ._ files and __MACOSX
            if exclude_metadata:
                if member.startswith('__MACOSX') or '._' in member:
                    continue
            zip_ref.extract(member, extract_to)
    print(f"Extracted to {extract_to}")

def create_zip_without_metadata(source_dir, output_zip):
    """Create a zip file excluding metadata files"""
    print(f"Creating {output_zip}...")
    with zipfile.ZipFile(output_zip, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(source_dir):
            for file in files:
                # Skip metadata files
                if file.startswith('._') or file == '.DS_Store':
                    continue
                
                file_path = os.path.join(root, file)
                arcname = os.path.relpath(file_path, source_dir)
                zipf.write(file_path, arcname)
    print(f"Created {output_zip}")

def main():
    script_dir = Path.cwd()
    sdout_dir = script_dir / "sdout"
    sdout_zip = script_dir / "sdout.zip"
    temp_dir = tempfile.mkdtemp()
    
    try:
        # Clean up any existing sdout folder and zip file
        print("Cleaning up previous builds...")
        if sdout_dir.exists():
            shutil.rmtree(sdout_dir)
            print("Deleted existing sdout folder")
        if sdout_zip.exists():
            sdout_zip.unlink()
            print("Deleted existing sdout.zip")
        
        # Step 1: Create sdout folder structure
        print("Creating folder structure...")
        folders = [
            "config/ultrahand",
            "config/ultrahand/downloads",
            "config/ultrahand/flags",
            "config/ultrahand/lang",
            "config/ultrahand/notifications",
            "config/ultrahand/payloads",
            "config/ultrahand/sounds",
            "config/ultrahand/themes",
            "config/ultrahand/wallpapers",
            "switch/.overlays",
            "switch/.packages"
        ]
        
        for folder in folders:
            folder_path = sdout_dir / folder
            folder_path.mkdir(parents=True, exist_ok=True)
            print(f"Created {folder_path}")
        
        # Step 2: Download and extract nx-ovlloader.zip
        ovlloader_zip = Path(temp_dir) / "nx-ovlloader.zip"
        download_file(
            "https://github.com/ppkantorski/nx-ovlloader/releases/latest/download/nx-ovlloader.zip",
            ovlloader_zip
        )
        extract_zip(ovlloader_zip, sdout_dir)
        
        # Step 3: Download and process Ultrahand-Overlay
        ultrahand_zip = Path(temp_dir) / "ultrahand-main.zip"
        ultrahand_temp = Path(temp_dir) / "ultrahand_temp"
        
        download_file(
            "https://github.com/ppkantorski/Ultrahand-Overlay/archive/refs/heads/main.zip",
            ultrahand_zip
        )
        extract_zip(ultrahand_zip, ultrahand_temp)
        
        # Find the extracted folder (it will be Ultrahand-Overlay-main)
        extracted_folders = [f for f in ultrahand_temp.iterdir() if f.is_dir()]
        if not extracted_folders:
            raise Exception("Could not find extracted Ultrahand folder")
        
        ultrahand_root = extracted_folders[0]
        
        # Step 4: Copy lang files
        lang_source = ultrahand_root / "lang"
        lang_dest = sdout_dir / "config/ultrahand/lang"
        
        if lang_source.exists():
            print("Copying language files...")
            for json_file in lang_source.glob("*.json"):
                shutil.copy2(json_file, lang_dest)
                print(f"Copied {json_file.name}")
        
        # Step 5: Copy ultrahand_updater.bin
        payload_source = ultrahand_root / "payloads/ultrahand_updater.bin"
        payload_dest = sdout_dir / "config/ultrahand/payloads"
        
        if payload_source.exists():
            print("Copying payload file...")
            shutil.copy2(payload_source, payload_dest)
            print(f"Copied ultrahand_updater.bin")
        
        # Step 6: Copy theme files from the downloaded repository
        print("Copying theme files...")
        theme_source = ultrahand_root / "themes"
        theme_dest = sdout_dir / "config/ultrahand/themes"
        theme_files = ["ultra.ini", "ultra-blue.ini"]
        
        if theme_source.exists():
            for theme_file in theme_files:
                theme_file_path = theme_source / theme_file
                if theme_file_path.exists():
                    shutil.copy2(theme_file_path, theme_dest)
                    print(f"Copied {theme_file}")
                else:
                    print(f"Warning: {theme_file} not found in themes folder")
        else:
            print("Warning: themes folder not found in Ultrahand repository")
        
        # Step 7: Copy sound files
        print("Copying sound files...")
        sounds_source = ultrahand_root / "sounds"
        sounds_dest = sdout_dir / "config/ultrahand/sounds"
        
        if sounds_source.exists():
            for wav_file in sounds_source.glob("*.wav"):
                shutil.copy2(wav_file, sounds_dest)
                print(f"Copied {wav_file.name}")
        else:
            print("Warning: sounds folder not found in Ultrahand repository")
        
        # Step 8: Copy ovlmenu.ovl
        print("Copying ovlmenu.ovl...")
        ovlmenu_source = script_dir / "ovlmenu.ovl"
        ovlmenu_dest = sdout_dir / "switch/.overlays"
        
        if ovlmenu_source.exists():
            shutil.copy2(ovlmenu_source, ovlmenu_dest)
            print(f"Copied ovlmenu.ovl")
        else:
            print("Warning: ovlmenu.ovl not found in script directory")
        
        # Step 9: Clean up temporary files
        print("Cleaning up temporary files...")
        shutil.rmtree(temp_dir)
        print("Temporary files deleted")
        
        # Step 10: Create final zip
        output_zip = script_dir / "sdout.zip"
        create_zip_without_metadata(sdout_dir, output_zip)
        
        print("\n✓ Successfully created sdout.zip!")
        print(f"Location: {output_zip}")
        
    except Exception as e:
        print(f"\n✗ Error: {e}")
        # Clean up on error
        if Path(temp_dir).exists():
            shutil.rmtree(temp_dir)
        raise
    
if __name__ == "__main__":
    main()
