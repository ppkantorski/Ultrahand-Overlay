# Overlay Detacher

The Overlay Detacher package allows you to offload and restore specific overlays on your device.

## Configuration

The `config.ini` file contains the following configurations:

- `[Offload Overlays]`: Moves selected overlays to a designated offload directory.
- `[Restore Overlays]`: Restores previously offloaded overlays back to their original location.

Each configuration has a set of commands that perform the offloading or restoration of overlays.

## Usage

To use Overlay Detacher:

1. Open the `config.ini` file located in the `examples/Overlay Detacher` directory.
2. Uncomment the desired configuration and its corresponding commands by removing the semicolon (`;`) at the beginning of each line.
3. Save the `config.ini` file after making the necessary changes.
4. Copy the `Overlay Detacher` directory to your device under the appropriate location.

Note: Offloading overlays can help improve performance and free up system resources, but it may also affect the functionality of certain features or applications that rely on those overlays. Use this feature with caution and ensure that you understand the potential implications.

## Additional Information

For more details on the Ultrahand Overlay project and its features, please refer to the [official GitHub repository](https://github.com/ppkantorski/Ultrahand-Overlay).
