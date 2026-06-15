# OC Toolkit (mariko only) - **DEPRECATED: EOS is recommended**

The OC Toolkit package provides various options for adjusting the clock speeds, voltages and timings of your device.

**WARNING: DO NOT USE YET IF YOU DO NOT KNOW WHAT YOU ARE DOING**

## Configurations

The `package.ini` file contains the following configurations:

## CPU

### Undervolt Mode (CPU)
- Configuration: `[*Undervolt Mode?CPU]`
- Options: `(0, 1, 2, 3, 4, 5, 6)`
- KIP File Modification: `/atmosphere/kips/loader.kip CUST 40 0{list_source(*)}`

### GPU UV3 Configuration
- Configuration: `[GPU UV3 Configuration]`
- JSON File Source: `/switch/.packages/OC Toolkit/mariko/gpu_uv3.json`
- KIP File Modification: `/atmosphere/kips/loader.kip CUST 148 {json_file_source(*,hex)}`

## RAM

### RAM Frequency
- Configuration: `[*Frequency?RAM]`
- JSON File Source: `/switch/.packages/OC Toolkit/mariko/ram_freqs.json`
- KIP File Modification: `/atmosphere/kips/loader.kip CUST 32 {json_file_source(*,hex)}`

### RAM Vddq Configuration
- Configuration: `[*Vddq]`
- JSON File Source: `/switch/.packages/OC Toolkit/mariko/ram_vddq.json`
- KIP File Modification: `/atmosphere/kips/loader.kip CUST 36 {json_file_source(*,hex)}`

### RAM Vdd2 Configuration
- Configuration: `[*Vdd2]`
- JSON File Source: `/switch/.packages/OC Toolkit/mariko/ram_vdd2.json`
- KIP File Modification: `/atmosphere/kips/loader.kip CUST 16 {json_file_source(*,hex)}`

### RAM Timings (DRAM)
- Configuration: `[*Timings][*DRAM]`
- JSON File Source: `/switch/.packages/OC Toolkit/mariko/ram_dram_timing.json`
- KIP File Modification: `/atmosphere/kips/loader.kip CUST 8 {json_file_source(*,hex)}`

### EMC DVB Table
- Configuration: `[*EMC DVB Table]`
- Options: `(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)`
- KIP File Modification: `/atmosphere/kips/loader.kip CUST 48 0{list_source(*)}`

### Core 1 Configuration
- Configuration: `[*Core 1]`
- Options: `(0, 1, 2, 3, 4, 5, 6)`
- KIP File Modification: `/atmosphere/kips/loader.kip CUST 52 0{list_source(*)}`

## Commands

### Profile
- Configuration: `[*Profile]`
- Current Slot Options: `(0, 1, 2, 3, 4, 5, 6)`

### Backup Configuration
- Configuration: `[Backup]`
- INI File: `/switch/.packages/OC Toolkit/config.ini/`
- Create Directory: `/switch/.packages/OC Toolkit/kips/slot_{ini_file("*Current Slot",footer)}/`
- Copy KIP File: `/atmosphere/kips/loader.kip '/switch/.packages/OC Toolkit/kips/slot_{ini_file("*Current Slot",footer)}/loader.kip'`

### Restore Configuration
- Configuration: `[Restore]`
- INI File: `/switch/.packages/OC Toolkit/config.ini/`



## Usage

To use `OC Toolkit`:

1. Install [OC Suite](https://github.com/hanai3Bi/Switch-OC-Suite) and clone this directory.
2. Copy the `OC Toolkit` directory into `/switch/.packages/`.
3. Use your menu hotkeys, go to the "Packages" section, then launch `OC Toolkit`.
  - A reboot will be requred for modifications to be applied.
  - **NOTICE:** Wrong values can lead to crashes or potential damage to your console.  Use and modify with caution and assistance from others if possible.

Note: Modifying clock speeds and voltages can affect the stability and performance of your device. Use these configurations with caution and make sure you understand the potential risks involved. It is recommended to have a backup of your system before applying any changes.

## Additional Information

For more details on the Ultrahand Overlay project and its features, please refer to the [official GitHub repository](https://github.com/ppkantorski/Ultrahand-Overlay).

For additional help with OC Suite kip values, you can visit [this link](https://github.com/hanai3Bi/Switch-OC-Suite/blob/master/Source/Atmosphere/stratosphere/loader/source/oc/customize.cpp). This resource provides helpful information specifically for OC Suite kip values.

This package is compatible with v1.3.6 or later versions.
