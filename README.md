# Ultrahand Overlay (HOS 16.0.0+)
![Alt Text](https://www.pcinvasion.com/wp-content/uploads/2023/05/How-to-Get-and-Use-Ultrahand-Ability-in-Tears-of-the-Kingdom.jpg)

Ultrahand Overlay is a C++ program that provides a user-friendly main menu overlay for performing various file operations. It is designed to run on various platforms, including the Nintendo Switch, and utilizes the TESLA library to create an interactive menu interface. It allows you to perform actions such as creating directories, copying files, renaming files, moving directories, deleting files, and parsing INI files.

## Features

Ultrahand is a command-line tool that provides functionality for managing files and directories on an SD card. It offers the following features:

- Create directories: You can use Ultrahand to create directories on your SD card. Simply specify the directory path, and Ultrahand will create it for you.
- Copy files: Ultrahand allows you to copy files from one location to another on your SD card. Just provide the source file path and the destination file path, and Ultrahand will handle the file copying process.
- Delete files: With Ultrahand, you can easily delete files from your SD card. Just specify the file path, and Ultrahand will remove the file for you.
- Move files: Ultrahand enables you to move files from one directory to another on your SD card. Provide the file path and the destination directory path, and Ultrahand will handle the file movement for you.



## Nintendo Switch Compatibility
The Ultrahand File Management System is compatible with the Nintendo Switch, allowing users to manage files and perform file operations directly on their Switch console.

To run the Ultrahand File Management System on the Nintendo Switch, you need to have the necessary homebrew environment set up on your console. Once you have the homebrew environment set up, you can transfer the compiled executable file to your Switch and launch it using the Tesla Overlay.

Please note that running homebrew software on your Nintendo Switch may void your warranty and can carry certain risks. Ensure that you understand the implications and follow the appropriate guidelines and precautions when using homebrew software.



## Getting Started

### Prerequisites

To compile and run the program, you need to have the following dependencies installed:

- TESLA library
- Standard C++ libraries


### Usage

To use Ultrahand, follow these steps:

1. Create a directory named ultrahand in the config folder on your SD card.
2. Place the config.ini file in the ultrahand directory. This file contains the configuration options for Ultrahand.
3. Launch Ultrahand by executing the compiled binary.

## Configuration Options

The config.ini file contains multiple sections, each defining a set of commands that can be executed. The sections are enclosed in square brackets [ ], and the commands are listed below each section.

Here's an example of the config.ini file:
```
[make directories]
mkdir /config/ultrahand/example1/
mkdir /config/ultrahand/example2/

[copy files]
copy /config/ultrahand/config.ini /config/ultrahand/example1/
copy /config/ultrahand/config.ini /config/ultrahand/example2/

[rename files]
rename /config/ultrahand/example1/config.ini /config/ultrahand/example1/configX.ini
rename /config/ultrahand/example2/config.ini /config/ultrahand/example2/configX.ini

[move directories]
move /config/ultrahand/example1/ /config/ultrahand/example3/
move /config/ultrahand/example2/ /config/ultrahand/example4/

[delete files]
delete /config/ultrahand/example1/config.ini
delete /config/ultrahand/example2/config.ini

[delete directories]
delete /config/ultrahand/example1/
delete /config/ultrahand/example2/
delete /config/ultrahand/example3/
delete /config/ultrahand/example4/

[parse ini file]
copy /bootloader/hekate_ipl.ini /config/ultrahand/
parse-ini /config/ultrahand/hekate_ipl.ini 'L4T Ubuntu Bionic' r2p_action working
```
You can add your own sections and commands to customize the actions performed by Ultrahand.

Note: The paths specified in the commands should be relative to the SD card root directory and should start with /.

### Command Reference

Ultrahand supports the following commands:

- make or mkdir: Creates a directory.
  - Usage: `make <directory_path>`

- copy or cp: Copies a file.
  - Usage: `copy <source_file_path> <destination_file_path>`

- delete or del: Deletes a file.
  - Usage: `delete <file_path>`

- move: Moves a file to a directory.
  - Usage: `move <file_path> <destination_directory_path>`

- rename or mv: Renames a file.
  - Usage: `rename <file_path> <new_file_name>`

- set-ini-val or set-ini-value: Edits an INI file by updating a section with a desired key-value pair.
  - Usage: `set-ini-val <file_to_edit> <desired_section> <desired_key> <desired_value>`

- set-ini-key: Edits an INI file by updating a section with a new key.
  - Usage: `set-ini-key <file_to_edit> <desired_section> <desired_key> <desired_new_key>`

- new-ini-entry: Edits an INI file by adding a new entry to a section.
  - Usage: `new-ini-entry <file_to_edit> <desired_section> <desired_key> <desired_value>`

Please note that `<directory_path>`, `<source_file_path>`, `<destination_file_path>`, `<file_path>`, `<destination_directory_path>`, `<new_file_name>`, `<file_to_edit>`, `<desired_section>`, `<desired_key>`, and `<desired_value>` should be replaced with the actual paths, names, sections, and values relevant to your use case.

Make sure to follow the correct syntax and provide the required arguments for each command.


You can configure these commands in the `config.ini` file by specifying them under the corresponding options. Make sure to provide the necessary arguments as described for each command.


## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please create an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

