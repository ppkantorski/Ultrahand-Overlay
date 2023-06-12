# l4t_reboot - Ubuntu Helper Function

"l4t_reboot" is an Ubuntu helper function designed to streamline the handling of L4T (Linux for Tegra) reboots. This tool provides a convenient way to manage and execute reboot operations on your Ubuntu-based system.

## Purpose

The purpose of l4t_reboot is to simplify the process of handling L4T reboots in an Ubuntu environment. It offers a set of functions and utilities that assist in managing the reboot process smoothly and efficiently.

## Features

- **Easy Reboot Execution**: l4t_reboot provides a straightforward method to initiate L4T reboots from your Ubuntu system, eliminating the need for complex commands or manual intervention.

- **Ubuntu-Specific Setup**: This utility includes specific files, such as `config.ini`, that are tailored for Ubuntu setups. The `config.ini` file assumes a third entry in `hekate_ipl.ini` for your Linux setup, allowing for seamless integration with Ubuntu-based systems.

## Getting Started

To use l4t_reboot on your Ubuntu system, follow these steps:

1. Clone or download this repository to your local machine.

2. Ensure that you have the necessary prerequisites installed, including a working Ubuntu environment and the required dependencies.

3. Configure `config.ini` to match your specific Linux setup. Make sure to set the appropriate entries in `hekate_ipl.ini` for successful execution.

4. Execute the `l4t_reboot.py` script to initiate the L4T reboot. You can either run the script directly or utilize the provided helper functions for more convenient usage.

Note: It is essential to review and understand the implications of rebooting your system before executing any commands. Ensure that you have saved any necessary data or configurations to prevent data loss or system instability.

## Contributions

Contributions to l4t_reboot are welcome! If you have any improvements, bug fixes, or additional features to suggest, please feel free to submit a pull request.

## License

This utility is open source and licensed under the [MIT License](https://github.com/ppkantorski/Ultrahand-Overlay/blob/main/LICENSE). You are free to use, modify, and distribute this software in accordance with the terms of the license.
