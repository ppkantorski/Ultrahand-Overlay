# Mod Master

The Mod Master package provides various modifications and enhancements for your device.

## Configuration

The `config.ini` file contains the following configurations:

- `[*Toggle exeFS Patches]`: Toggles exeFS patches by selection.
- `[*Toggle Content Mods]`: Toggles content modifications by selection.
    -  Mods are sorted accordingly within `/config/ultrahand/Mod Master/<GAME>/<MOD>/<ID>/...`
- `[Enable 21:9 Mods]`: Enables 21:9 aspect ratio modifications.
- `[Disable 21:9 Mods]`: Disables 21:9 aspect ratio modifications.

Each configuration has a set of commands that perform the corresponding actions. The commands include moving, deleting, creating directories, and copying files to specific locations on your device.

## Obtaining Mods

To use the mods provided in the Mod Master package, you will need to obtain them separately. The mods should be placed in the appropriate folders within the `Mod Master` directory.

Please refer to the shared documentation or additional resources to acquire the specific mods you are interested in. Once you have obtained the mods, place them in the corresponding folders within the `Mod Master` directory.

## Usage

To use Mod Master:

1. Open the `config.ini` file located in the `examples/Mod Master` directory.
2. Uncomment the desired configuration and its corresponding commands by removing the semicolon (`;`) at the beginning of each line.
3. Save the `config.ini` file after making the necessary changes.
4. Copy the `Mod Master` directory to your switch under `/config/ultrahand/` by placing it in that location.

Note: Make sure you have organized the mods properly within the subdirectories as mentioned in the "Obtaining Mods" section.

## Additional Information

For more details on the Ultrahand Overlay project and its features, please refer to the [official GitHub repository](https://github.com/ppkantorski/Ultrahand-Overlay).
