# Ultrahand Overlay (HOS 16.0.0+)
![Package Menu](https://gbatemp.net/attachments/img_4547-jpg.386207/)
![Broomstick Package](https://gbatemp.net/attachments/img_4544-jpg.386202/)

Create directories, merge files, and customize configurations effortlessly using simple config.ini files.

Ultrahand is a versatile tool that enables you to create and share command-based packages, providing enhanced functionality for managing files and directories on your SD card. It offers a range of features that allow you to efficiently interact with your system environment. With Ultrahand, you have the flexibility to customize and shape your file management system according to your needs, empowering you with greater control over your system configurations.


## Features

Ultrahand is a Tesla menu replacement that provides powerful C/C++ commands through the usage of its own custom interpretive programming language (similar to Shell/BASH).

It offers the following features:

- Create Directories:
  - Effortlessly create directories on your SD card by specifying the directory path. Ultrahand will handle the creation process for you.

- Copy Files or Directories:
  - Easily copy files or directories from one location to another on your SD card. Just provide the source and destination paths, and Ultrahand will seamlessly handle the copying process.

- Delete Files or Directories:
  - Simplify file and directory deletion on your SD card. By specifying the path of the file or directory you want to delete, Ultrahand promptly removes it, making the deletion process hassle-free.

- Move Files or Directories:
  - Seamlessly move files or directories between locations on your SD card. Provide the source path and the destination directory path, and Ultrahand takes care of the moving process, ensuring smooth relocation.

- Modify INI Files:
  - Edit INI files on your SD card with ease. Take full control over your configurations by updating existing key-value pairs, adding new entries, or creating new sections within the INI file using Ultrahand.

- Hex Edit Files:
  - Perform hexadecimal editing of files on your SD card. Edit the binary data directly, allowing for precise control over your data. Ultrahand's Hex Edit Files feature enables you to analyze, modify, and customize files in their raw form.

- Download Files:
  - Download files to your SD card with ease using. Efficiently retrieve files from repositories or URLS to your desired location. Whether you need to download / update projects or transfer files between locations, this feature simplifies the process, making repository management a breeze.

- Unzip Files:
  - Extract compressed zip files on your SD card by unzip archived files, preserving their original structure. Whether you have downloaded zip archives or received compressed files, this command simplifies the process of extracting them, making it effortless to access the contents within.

Ultrahand provides a convenient command-line interface to perform these operations, allowing you to efficiently manage your files, directories, and INI files on an SD card.


## Getting Started

### Usage

To use Ultrahand, follow these steps:

1. Download `ovlmenu.ovl` and place it within `/switch/.overlays/`. (Warning: Tesla Menu will be overwritten)
2. After installing Ultrahand Overlay, a new folder named `ultrahand` will be created within the config root folder on your SD card (`/config/ultrahand/`) along with a config.ini file containing various Ultrahand settings.
3. Launch `Ultrahand` similarly to `Tesla Menu` with your specified hotkey.  A new folder will be made `/switch/.packages/` containing your base commands in the root config.ini file.
4. Place your custom config.ini package in the ultrahand package directory (`/switch/.packages/YOUR_PACKAGE/`). This package file contains the commands for your Ultrahand package.
5. Your commands will show up on the packages menu within Ultrahand.  You can click A to execute any command as well as click X to view the indivicual command lines written in the package ini for execution.

### Nintendo Switch Compatibility
To run the Ultrahand File Management overlay on the Nintendo Switch, you need to have the necessary homebrew environment set up on your console. Once you have the homebrew environment set up, you can transfer the compiled executable file to your Switch and launch it using the Tesla Overlay.

Please note that running homebrew software on your Nintendo Switch may void your warranty and can carry certain risks. Ensure that you understand the implications and follow the appropriate guidelines and precautions when using homebrew software.

### Compilation Prerequisites

To compile and run the program, you need to have the following dependencies installed:

- [libtesla](https://github.com/WerWolv/libtesla)
- switch-curl
- switch-zziplib
- switch-mbedtls
- switch-jansson


## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please create an issue or submit a pull request.

## License

This project is licensed under the [CC-BY-NC-4.0 License](LICENSE).

Copyright (c) 2023 ppkantorski

All rights reserved.
