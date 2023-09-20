# Ultrahand Overlay (HOS 16.0.0+)
Create directories, manage files, and customize configurations effortlessly using simple package.ini files.

![Package Menu](https://gbatemp.net/attachments/img_4547-jpg.386207/)
![Broomstick Package](https://gbatemp.net/attachments/img_4544-jpg.386202/)

Ultrahand Overlay is a [Tesla Menu](https://github.com/WerWolv/Tesla-Menu) replacement built from the ground up off of [libtesla](https://github.com/WerWolv/libtesla) that provides powerful C/C++ commands through the usage of its own custom interpretive programming language (similar to Shell/BASH).  It is a versatile tool that enables you to create and share custom command-based packages, providing enhanced functionality for managing files and directories on your SD card.

With Ultrahand, you have the flexibility to customize and shape your file management system according to your needs, empowering you with greater control over your system configurations.


## Features

Ultrahand Overlay currently offers the following features:

- Create Directories:
  - Effortlessly create directories on your SD card by specifying the directory path. Ultrahand will handle the creation process for you.

- Copy Files or Directories:
  - Easily copy files or directories from one location to another on your SD card. Just provide the source and destination paths, and Ultrahand will seamlessly handle the copying process.

- Delete Files or Directories:
  - Simplify file and directory deletion on your SD card. By specifying the path of the file or directory you want to delete, Ultrahand promptly removes it, making the deletion process hassle-free.

- Move Files or Directories:
  - Seamlessly move files or directories between locations on your SD card. Provide the source path and the destination directory path, and Ultrahand takes care of the moving process, ensuring smooth relocation.

- Download Files:
  - Download files to your SD card with ease. Efficiently retrieve files from repositories or URLs to your desired location. Whether you need to download/update homebrew or transfer files between locations, this feature simplifies the process, making repository management a breeze.

- Unzip Files:
  - Extract compressed zip files on your SD card by unzipping archived files, preserving their original structure. Whether you have downloaded zip archives or received compressed files, this command simplifies the process of extracting them, making it effortless to access the contents within.

- Modify INI Files:
  - Edit INI files on your SD card with ease. Take full control over your configurations by updating existing key-value pairs, adding new entries, or creating new sections within the INI file using Ultrahand.

- Hex Edit Files:
  - Perform hexadecimal editing of files on your SD card. Edit the binary data directly, allowing for precise control over your data. Ultrahand's Hex Edit Files feature enables you to analyze, modify, and customize files in their raw form.


## Getting Started

### Usage

To use Ultrahand, follow these steps:

1. Download and install the latest [nxovloader](https://github.com/WerWolv/nx-ovlloader).
2. Download the latest Ultrahand [ovlmenu.ovl](https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest/download/ovlmenu.ovl) and place it within `/switch/.overlays/`.
    - WARNING: This will overwrite `Tesla Menu` if already installed.
3. After installing Ultrahand Overlay, a new folder named `ultrahand` will be created within the root config folder on your SD card (`/config/ultrahand/`) along with a `config.ini` file containing various Ultrahand settings.
4. Launch Ultrahand similarly to `Tesla Menu` with your specified hotkey.  A new folder will be made (`/switch/.packages/`) with a preset `package.ini` file for your base menu commands.

5. Place your custom `package.ini` package file in the Ultrahand package directory (`/switch/.packages/YOUR_PACKAGE/`). This file will contains the commands for your custom Ultrahand package.
6. Your commands will now show up on the packages menu within Ultrahand.  You can click A to execute any command as well as click X to view/execute the individual command lines written in the ini for execution.

For additional assistance with custom packages, feel free to checkout the [Ultrahand Overlay Wiki](https://github.com/ppkantorski/Ultrahand-Overlay/wiki).

### Nintendo Switch Compatibility
To run Ultrahand Overlay on the Nintendo Switch, you need to have the necessary [homebrew environment](https://github.com/Atmosphere-NX/Atmosphere) set up on your console running HOS 16.0.0+. Once you have the homebrew environment set up, you can transfer the compiled .ovl to your Switch and launch it using your old `Tesla Menu` hotkeys.

Please note that running homebrew software on your Nintendo Switch may void your warranty and can carry certain risks. Ensure that you understand the implications and follow the appropriate guidelines and precautions when using homebrew software.

### Compilation Prerequisites

To compile and run the software, you need to have the following C/C++ dependencies installed:

- [custom libtesla fork](https://github.com/ppkantorski/Ultrahand-Overlay/tree/main/lib/libtesla)
- switch-curl
- switch-zziplib
- switch-mbedtls
- switch-jansson


## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please raise an [issue](https://github.com/ppkantorski/Ultrahand-Overlay/issues/new/choose), submit a [pull request](https://github.com/ppkantorski/Ultrahand-Overlay/compare) or reach out to me directly on [GBATemp](https://gbatemp.net/threads/ultrahand-overlay-the-fully-craft-able-overlay-executor.633560/).

## License

This project is licensed under the [CC-BY-NC-4.0 License](LICENSE).

Copyright (c) 2023 ppkantorski

All rights reserved.
