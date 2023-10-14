# Broomstick (now deprecated on v1.3.9+)

The Broomstick package allows you to offload and restore specific overlays on your device.

## Configuration

The `package.ini` file contains the following configurations:

- `[Offload Overlays]`: Moves selected overlays to a designated offload directory.
- `[Restore Overlays]`: Restores previously offloaded overlays back to their original location.

Each configuration has a set of commands that perform the offloading or restoration of overlays.

## Usage

To use Broomstick:

1. Open the `package.ini` file located in the `examples/Broomstick` directory.
2. Uncomment the desired configuration and its corresponding commands by removing the semicolon (`;`) at the beginning of each line.
3. Save the `package.ini` file after making the necessary changes.
4. Copy the `Broomstick` directory to your device under the appropriate location.

Note: Offloading overlays can help improve performance and free up system resources, but it may also affect the functionality of certain features or applications that rely on those overlays. Use this feature with caution and ensure that you understand the potential implications.

## Additional Information

For more details on the Ultrahand Overlay project and its features, please refer to the [official GitHub repository](https://github.com/ppkantorski/Ultrahand-Overlay).
