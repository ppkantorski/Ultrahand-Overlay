# 4IFIR-Explorer

The 4IFIR overclock package includes the base version as well as versions for Stage 7, Stage 8, and Stage 7+. You can choose the desired overclock level based on your preference and device requirements.

## Installation

To obtain the 4IFIR overclock packages and the OC Suite, as well as the Atmosphere and stock files, users can follow the instructions below:

1. 4IFIR Overclock Package: Users can go to the [4IFIR GitHub repository](https://github.com/rashevskyv/4IFIR/blob/main/README_ENG.md) to obtain the 4IFIR overclock packages. The repository's README provides instructions on how to download and use the packages. Users should follow those instructions to acquire the desired version of the 4IFIR overclock package.
2. OC Suite: Users can visit the [OC Suite GitHub repository](https://github.com/hanai3Bi/Switch-OC-Suite) to obtain the OC Suite package. The repository contains information on how to download and utilize the OC Suite. Users should refer to the repository's documentation for instructions on acquiring and installing the OC Suite on their devices.
3. Atmosphere: Users can obtain the Atmosphere files from the [Atmosphere GitHub repository](https://github.com/Atmosphere-NX/Atmosphere). The repository provides detailed instructions on downloading and setting up Atmosphere. Users should follow the repository's guidelines to download and install Atmosphere according to their device's requirements.
4. Stock: The stock files can be acquired from the [sys-clk GitHub repository](https://github.com/retronx-team/sys-clk). Users should refer to the repository's documentation to obtain the stock files and follow the provided instructions to install them on their devices.

Note: It's important to review the documentation and guidelines provided in each repository to ensure the correct installation and usage of the respective packages.

## Usage

1. Open the `config.ini` file located in the `examples/4IFIR Explorer` directory.
2. Edit the configuration based on your requirements. You can enable or disable specific overclock packages by commenting or uncommenting the relevant sections in the file.
3. Save the `config.ini` file after making the necessary changes.

## Swapping Overclock Packages

To swap between different overclock packages:

1. Make sure your device is powered off.
2. Boot your device into the desired custom firmware or bootloader.
3. Locate the `atmosphere` directory and backup the existing `loader.kip` file if necessary.
4. Copy the appropriate `loader.kip` file from the desired overclock package directory (`4IFIR 1.5`, `Stage 7`, `Stage 8+`, or `Stage 7+`) to the `atmosphere/kips/` directory on your device.
5. Delete the existing `sys-clk` and `sys-clk-oc` directories in the `config` directory (if present).
6. Copy the contents of the corresponding `sys-clk` directory from the desired overclock package directory to the `config` directory on your device.
7. Delete any existing overlay files in the `/switch/.overlays/` directory.
8. Copy the overlay files (`ReverseNX-RT-ovl.ovl` and other required overlays) from the desired overclock package directory to the `/switch/.overlays/` directory on your device.
9. Save all changes and safely eject your device from the computer.
10. Power on your device and enjoy the selected overclock package.

## Additional Information

For more details on the Ultrahand-Overlay project and its features, please refer to the [official GitHub repository](https://github.com/ppkantorski/Ultrahand-Overlay).

## Credits

This project was created by [ppkantorski](https://github.com/ppkantorski) and contributors. Thank you for your hard work and dedication!
