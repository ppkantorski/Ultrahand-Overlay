# Overclock Buster (experimental)

The Overclock Buster package provides various options for adjusting the clock speeds and voltages of your device.

## Configuration

The `config.ini` file contains the following configurations:

- `[1996 to 2400 (RAM)]`: Adjusts the clock speed range from 1996MHz to 2400MHz for RAM.
- `[2400 to 1996 (RAM)]`: Adjusts the clock speed range from 2400MHz to 1996MHz for RAM.

Each configuration has a set of commands that modify the clock speeds, voltages, and other settings of your device.

## Usage

To use Overclock Buster:

1. Open the `config.ini` file located in the `examples/Overclock Buster` directory.
2. Uncomment the desired configuration and its corresponding commands by removing the semicolon (`;`) at the beginning of each line.
3. Save the `config.ini` file after making the necessary changes.
4. Copy the `Overclock Buster` directory to your device under the appropriate location.

Note: Modifying clock speeds and voltages can affect the stability and performance of your device. Use these configurations with caution and make sure you understand the potential risks involved. It is recommended to have a backup of your system before applying any changes.

## Additional Information

For more details on the Ultrahand Overlay project and its features, please refer to the [official GitHub repository](https://github.com/ppkantorski/Ultrahand-Overlay).

For additional help with OC Suite kip values, you can visit [this link](https://github.com/hanai3Bi/Switch-OC-Suite/blob/master/Source/Atmosphere/stratosphere/loader/source/oc/customize.cpp). This resource provides helpful information specifically for OC Suite kip values.

This package is compatible with v1.1.0 or later versions.
