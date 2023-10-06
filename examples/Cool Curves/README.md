# Cool Curves

The "Cool Curves" package is a part of the Ultrahand Overlay project, which provides a fully craft-able overlay executor. This example demonstrates how to configure and set temperature-related settings in the system_settings.ini file for different temperature ranges.

## Configuration

To configure the temperature-related settings yourself, follow these steps:

1. Make sure you have the necessary tools and files for the Ultrahand-Overlay project.
2. Locate the `package.ini` file in the `examples/Cool Curves` directory.
3. Open the `package.ini` file in a text editor.

## Temperature Settings

The `package.ini` file contains temperature settings for different temperature ranges. Each temperature range is associated with specific configurations for the console and handheld devices. Here are the available temperature ranges and their configurations:

### Max Temp 54°C

- Console: `tskin_rate_table_console_on_fwdbg`: [[-1000000, 40000, 0, 0], [36000, 43000, 51, 51], [43000, 49000, 51, 128], [49000, 54000, 128, 255], [54000, 1000000, 255, 255]]
- Handheld: `tskin_rate_table_handheld_on_fwdbg`: [[-1000000, 40000, 0, 0], [36000, 43000, 51, 51], [43000, 49000, 51, 128], [49000, 54000, 128, 255], [54000, 1000000, 255, 255]]
- `holdable_tskin`: 60000 (hex value: 0xEA60)
- `touchable_tskin`: 60000 (hex value: 0xEA60)

### Max Temp 56°C

- Console: `tskin_rate_table_console_on_fwdbg`: [[-1000000, 40000, 0, 0], [36000, 43000, 51, 51], [43000, 49000, 51, 128], [49000, 56000, 128, 255], [56000, 1000000, 255, 255]]
- Handheld: `tskin_rate_table_handheld_on_fwdbg`: [[-1000000, 40000, 0, 0], [36000, 43000, 51, 51], [43000, 49000, 51, 128], [49000, 56000, 128, 255], [56000, 1000000, 255, 255]]
- `holdable_tskin`: 60000 (hex value: 0xEA60)
- `touchable_tskin`: 60000 (hex value: 0xEA60)

### Max Temp 58°C

- Console: `tskin_rate_table_console_on_fwdbg`: [[-1000000, 40000, 0, 0], [36000, 43000, 51, 51], [43000, 49000, 51, 128], [49000, 58000, 128, 255], [58000, 1000000, 255, 255]]
- Handheld: `tskin_rate_table_handheld_on_fwdbg`: [[-1000000, 40000, 0, 0], [36000, 43000, 51, 51], [43000, 49000, 51, 128], [49000, 58000, 128, 255], [58000, 1000000, 255, 255]]
- `holdable_tskin`: 60000 (hex value: 0xEA60)
- `touchable_tskin`: 60000 (hex value: 0xEA60)

### Max Temp 60°C

- Console: `tskin_rate_table_console_on_fwdbg`: [[-1000000, 40000, 0, 0], [36000, 43000, 51, 51], [43000, 49000, 51, 128], [49000, 60000, 128, 255], [60000, 1000000, 255, 255]]
- Handheld: `tskin_rate_table_handheld_on_fwdbg`: [[-1000000, 40000, 0, 0], [36000, 43000, 51, 51], [43000, 49000, 51, 128], [49000, 60000, 128, 255], [60000, 1000000, 255, 255]]
- `holdable_tskin`: 60000 (hex value: 0xEA60)
- `touchable_tskin`: 60000 (hex value: 0xEA60)

## Important Notes

- Before modifying any settings, it is recommended to create a backup of the original `system_settings.ini` file.
- Ensure that you understand the consequences of changing the temperature settings and their impact on your system's performance and stability.


## Acknowledgments

Special thanks to B3711.
