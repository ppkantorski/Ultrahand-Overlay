# Host Guard

The Host Guard package provides blocking and unblocking functionality for specific hosts on your device.

## Configuration

The `config.ini` file contains the following configurations:

- `[Enable Blocking]`: Enables blocking of specific hosts.
- `[Disable Blocking]`: Disables blocking of specific hosts.
- `[Enable Blocking (emummc)]`: Enables blocking for hosts on emummc.
- `[Disable Blocking (emummc)]`: Disables blocking for hosts on emummc.
- `[Enable Blocking (sysmmc)]`: Enables blocking for hosts on sysmmc.
- `[Disable Blocking (sysmmc)]`: Disables blocking for hosts on sysmmc.

Each configuration has a set of commands that perform the corresponding actions. The commands include deleting existing host files and copying new host files from the `/config/ultrahand/Host Guard/` directory to the `/atmosphere/hosts/` directory.

## Installation

To install Host Guard:

1. Copy the `Host Guard` directory to your switch under `/config/ultrahand/` by placing it in that location.

## Usage

To use Host Guard:

1. Navigate to the Ultrahand package on the Tesla menu.
2. Click "Host Guard".
3. Click the command you desire and effects will be applied immediately.

Note: Please refer to the documentation provided in the repository for more detailed instructions on using Host Guard and configuring the host blocking functionality.

## Additional Information

For more details on the Ultrahand Overlay project and its features, please refer to the [official GitHub repository](https://github.com/ppkantorski/Ultrahand-Overlay).
