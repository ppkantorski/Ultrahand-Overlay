# Ultrahand Overlay (HOS 16.0.0+)
![Alt Text](https://www.pcinvasion.com/wp-content/uploads/2023/05/How-to-Get-and-Use-Ultrahand-Ability-in-Tears-of-the-Kingdom.jpg)
![Alt Text](https://gbatemp.net/attachments/img_4417-jpg.376189/)
![Alt Text](https://gbatemp.net/attachments/img_4418-jpg.376190/)

Unleash Your Creative Potential with Ultrahand: Craft, Share, and Customize Your Commands!

Ultrahand empowers you to craft and share your own packages, revolutionizing the way you interact with files and directories on your SD card. Inspired by the new Zelda feature, Ultrahand lets you unleash your creativity and shape your digital experiences like never before.

Create directories, merge files, and customize configurations effortlessly using simple config.ini files. Whether you're a gamer, a creative enthusiast, or a tech-savvy individual, Ultrahand invites you to take advantage of premade packages as well as create your own.

Join a vibrant community of like-minded creators as you share your crafted packages, collaborate, and explore new possibilities. Ready to step into the realm of automation crafting and discover the endless potential of Ultrahand? Start crafting, sharing, and customizing today!

## Features



Ultrahand is a Tesla overlay that provides powerful functionality for managing files and directories on an SD card. It offers the following features:

- Create Directories: Ultrahand allows you to effortlessly create directories on your SD card. Simply specify the directory path, and Ultrahand will create it for you.

- Copy Files or Directories: With Ultrahand, you can easily copy files or directories from one location to another on your SD card. Just provide the source file or directory path and the destination file or directory path, and Ultrahand will handle the copying process seamlessly.

- Delete Files or Directories: Ultrahand simplifies file and directory deletion on your SD card. By specifying the file or directory path, Ultrahand will promptly remove it for you, making the deletion process hassle-free.

- Move Files or Directories: Ultrahand enables you to move files or directories between locations on your SD card effortlessly. Provide the source file or directory path and the destination directory path, and Ultrahand will take care of moving them, ensuring their seamless relocation.

- Modify INI Files: Ultrahand empowers you to edit INI files on your SD card. You can update existing key-value pairs, add new entries, or even create new sections within the INI file. Ultrahand makes it easy to customize and tailor the configurations of your INI files.

Ultrahand provides a convenient command-line interface to perform these operations, allowing you to efficiently manage your files, directories, and INI files on an SD card.


## Getting Started

### Nintendo Switch Compatibility
To run the Ultrahand File Management overlay on the Nintendo Switch, you need to have the necessary homebrew environment set up on your console. Once you have the homebrew environment set up, you can transfer the compiled executable file to your Switch and launch it using the Tesla Overlay.

Please note that running homebrew software on your Nintendo Switch may void your warranty and can carry certain risks. Ensure that you understand the implications and follow the appropriate guidelines and precautions when using homebrew software.

### Compilation Prerequisites

To compile and run the program, you need to have the following dependencies installed:

- TESLA library
- Standard C++ libraries


### Usage

To use Ultrahand, follow these steps:

1. Create a directory named ultrahand in the config root folder on your SD card.
2. Place the config.ini package file in the ultrahand directory (or sub-directory). This package file contains the configuration options for Ultrahand.
3. Your commands will show up on the Tesla menu within the Ultrahand overlay.  You can click A to execute any command as well as click X to view the indivicual command lines written in the ini for execution.

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
move /config/ultrahand/example1/config.ini /config/ultrahand/example1/configX.ini
move /config/ultrahand/example2/config.ini /config/ultrahand/example2/configX.ini

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

[modify ini file]
copy /bootloader/hekate_ipl.ini /config/ultrahand/
set-ini-val /config/ultrahand/hekate_ipl.ini 'Atmosphere' fss0 gonnawritesomethingelse
new-ini-entry /config/ultrahand/hekate_ipl.ini 'Atmosphere' booty true
```
You can add your own sections and commands to customize the actions performed by Ultrahand.

Note: The paths specified in the commands should be relative to the SD card root directory and should start with /.

## Command Reference

Ultrahand supports the following commands:

- make or mkdir: Creates a directory.
  - Usage: `mkdir <directory_path>`

- copy or cp: Copies a file or diectory.
  - Usage: `copy <source_file_path> <destination_file_path>`

- delete or del: Deletes a file or directory.
  - Usage: `delete <file_path>`

- move or mv: Moves/renames a file/directory to a new location/label.
  - Usage: `move <file_path> <destination_directory_path>`

- set-ini-val or set-ini-value: Edits an INI file by updating a section with a desired key-value pair.
  - Usage: `set-ini-val <file_to_edit> <desired_section> <desired_key> <desired_value>`

- set-ini-key: Edits an INI file by updating a section with a new key.
  - Usage: `set-ini-key <file_to_edit> <desired_section> <desired_key> <desired_new_key>`

- new-ini-entry: Edits an INI file by adding a new entry to a section.
  - Usage: `new-ini-entry <file_to_edit> <desired_section> <desired_key> <desired_value>`

- reboot: Restarts the system.
  - Usage: `reboot`

- shutdown: Shuts down the system.
  - Usage: `shutdown`


Make sure to follow the correct syntax and provide the required arguments for each command.

You can configure these commands in the `config.ini` file by specifying them under the corresponding options. Make sure to provide the necessary arguments as described for each command.


## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please create an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

