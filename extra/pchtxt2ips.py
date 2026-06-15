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

NSO_HEADER_LEN = 0x100  # Must match ips2pchtxt.py


def convert_pchtxt_to_ips(pchtxt_path, output_folder):
    """
    Converts a .pchtxt file to an .ips file.

    This function reads the contents of a .pchtxt file, extracts the address-value pairs,
    and generates an .ips file in the specified output folder.

    :param pchtxt_path: The file path to the .pchtxt file.
    :param output_folder: The folder path for the output .ips file.
    """
    try:
        with open(pchtxt_path, 'r', encoding=PATCH_TEXT_ENCODING) as pchtxt_file:
            pchtxt_data = pchtxt_file.read()
    except Exception as e:
        print(f"Error reading file {pchtxt_path}: {e}")
        return

    # Extract NSOBID and offset_shift
    nsobid = None
    offset_shift = 0
    
    for line in pchtxt_data.split('\n'):
        line_stripped = line.strip()
        
        # Extract NSOBID
        if line_stripped.startswith('@nsobid-'):
            # Use split with maxsplit=1 to handle hyphens in the ID
            nsobid = line_stripped[8:].strip()  # Skip "@nsobid-"
            
        # Extract offset_shift flag
        elif line_stripped.startswith('@flag offset_shift'):
            parts = line_stripped.split()
            if len(parts) >= 3:
                try:
                    # Support both hex (0x100) and decimal formats
                    offset_shift = int(parts[2], 0)
                except ValueError:
                    print(f"Warning: Invalid offset_shift value: {parts[2]}, using 0")
                    offset_shift = 0
    
    # Use filename if no NSOBID found
    if not nsobid:
        nsobid = os.path.splitext(os.path.basename(pchtxt_path))[0]

    # Parse patches
    patches = []
    is_enabled = False  # Start disabled by default
    stop_parsing = False
    
    for line_num, line in enumerate(pchtxt_data.split('\n'), start=1):
        line_stripped = line.strip()
        
        # Skip empty lines
        if not line_stripped:
            continue
        
        # Handle @stop flag - stop parsing patches after this point
        if line_stripped == '@stop':
            stop_parsing = True
            continue
        
        # Don't parse patches after @stop
        if stop_parsing:
            continue
        
        # Handle @enabled/@disabled flags
        if line_stripped == '@enabled':
            is_enabled = True
            continue
        elif line_stripped == '@disabled':
            is_enabled = False
            continue
        
        # Skip lines starting with '@', '//', or '#'
        if line_stripped.startswith('@') or line_stripped.startswith('//') or line_stripped.startswith('#'):
            continue
        
        # Only parse patches in enabled sections
        if not is_enabled:
            continue
        
        # Parse address and value
        parts = line_stripped.split()
        if len(parts) < 2:
            continue
        
        address_str = parts[0]
        value_str = parts[1]
        
        # Validate and parse address
        try:
            address = int(address_str, 16)
        except ValueError:
            print(f"Warning: Line {line_num} contains invalid address '{address_str}', skipping")
            continue
        
        # Validate and parse hex value
        try:
            value_bytes = bytes.fromhex(value_str)
        except ValueError:
            print(f"Warning: Line {line_num} contains invalid hex data '{value_str}', skipping")
            continue
        
        # CRITICAL: Add offset_shift back to address
        # ips2pchtxt subtracts NSO_HEADER_LEN, we need to add it back
        address_with_offset = address + offset_shift
        
        patches.append((address_with_offset, value_bytes))

    if not patches:
        print(f"Warning: No valid patches found in {pchtxt_path}")
        return

    # Keep original order from the file (don't sort)
    # patches.sort(key=lambda x: x[0])

    # Build IPS data
    ips_data = bytearray()
    for address, value in patches:
        ips_data.extend(struct.pack('>I', address))  # 4-byte address (big-endian)
        ips_data.extend(struct.pack('>H', len(value)))  # 2-byte length (big-endian)
        ips_data.extend(value)

    # Create output folder if it doesn't exist
    if output_folder and not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # Output file path
    out_ips_path = os.path.join(output_folder, f"{nsobid}.ips")

    # Write IPS file
    try:
        with open(out_ips_path, 'wb') as ips_file:
            ips_file.write(IPS32_HEAD_MAGIC)  # IPS32 header
            ips_file.write(ips_data)          # Patch data
            ips_file.write(IPS32_FOOT_MAGIC)  # IPS32 footer
        
        print(f"Successfully converted: {pchtxt_path}")
        print(f"Output file: {out_ips_path}")
        print(f"Total patches: {len(patches)}")
        if offset_shift:
            print(f"Offset shift applied: {hex(offset_shift)}")
            
    except Exception as e:
        print(f"Error writing IPS file {out_ips_path}: {e}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python pchtxt2ips.py input_pchtxt_file [output_folder]")
        sys.exit(1)

    pchtxt_path = sys.argv[1]
    
    # Handle output folder
    if len(sys.argv) > 2:
        output_folder = sys.argv[2]
    else:
        output_folder = os.path.dirname(pchtxt_path)
        if not output_folder:
            output_folder = "."
    
    convert_pchtxt_to_ips(pchtxt_path, output_folder)
