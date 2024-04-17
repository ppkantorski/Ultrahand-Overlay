"""
File: pchtxt2ips.py
Author: ppkantorski
Description:
    This script converts a .pchtxt file to an .ips file. It reads the contents
    of the .pchtxt file, extracts the address-value pairs, and generates an .ips
    file in the specified output folder.

    For the latest updates and contributions, visit the project's GitHub repository.
    (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)

    Note: Please be aware that this notice cannot be altered or removed. It is a part
    of the project's documentation and must remain intact.
    
Licensed under CC-BY-NC-SA-4.0
Copyright (c) 2024 ppkantorski
"""

import os
import struct
import sys

IPS32_HEAD_MAGIC = bytearray("IPS32", 'ascii')
IPS32_FOOT_MAGIC = bytearray("EEOF", 'ascii')

PATCH_TEXT_EXT = ".pchtxt"
PATCH_TEXT_ENCODING = "ascii"

PATCH_TEXT_NSOBID_ATTRIBUTE_NAME = "@nsobid-"


"""
Converts a .pchtxt file to an .ips file.

This function reads the contents of a .pchtxt file, extracts the address-value pairs,
and generates an .ips file in the specified output folder.

:param pchtxt_path: The file path to the .pchtxt file.
:param output_folder: The folder path for the output .ips file.
"""
def convert_pchtxt_to_ips(pchtxt_path, output_folder):
    with open(pchtxt_path, 'r') as pchtxt_file:
        pchtxt_data = pchtxt_file.read()

    # Extract NSOBID or use file name without extension
    nsobid = None
    for line in pchtxt_data.split('\n'):
        if line.startswith('@nsobid-'):
            nsobid = line.split('-')[1]
            break
    if not nsobid:
        nsobid = os.path.splitext(os.path.basename(pchtxt_path))[0]

    patches = []
    for line_num, line in enumerate(pchtxt_data.split('\n'), start=1):
        if not line.strip() or line.strip().startswith('@'):  # Skip empty lines and lines starting with '@'
            continue
        parts = line.split()
        if len(parts) < 2:
            #print(f"Warning: Line {line_num} does not contain address and value")
            continue
        address_str, value_str = parts[:2]
        try:
            address = int(address_str, 16)
        except ValueError:
            #print(f"Warning: Line {line_num} contains an invalid address")
            continue
        value_bytes = bytes.fromhex(value_str)
        patches.append((address, value_bytes))

    ips_data = bytearray()
    for address, value in patches:
        ips_data.extend(struct.pack('>I', address))  # Pack address as 4-byte unsigned integer
        ips_data.extend(struct.pack('>H', len(value)))  # Pack length of value as 2-byte unsigned integer
        ips_data.extend(value)

    # Output file path
    out_ips_path = os.path.join(output_folder, f"{nsobid}.ips")

    with open(out_ips_path, 'wb') as ips_file:
        ips_file.write(IPS32_HEAD_MAGIC)  # Write IPS32 header magic bytes
        ips_file.write(ips_data)  # Write IPS data
        ips_file.write(IPS32_FOOT_MAGIC)  # Write IPS32 footer magic bytes


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python pchtxt2ips.py input_pchtxt_file [output_folder]")
        sys.exit(1)

    pchtxt_path = sys.argv[1]
    output_folder = sys.argv[2] if len(sys.argv) > 2 else os.path.dirname(pchtxt_path)
    convert_pchtxt_to_ips(pchtxt_path, output_folder)
