# Pico Quad Audio Switch

This project is a four-channel audio input switch based on the Raspberry Pi Pico. It connects to a host via USB Type-C, allowing the host to switch audio input channels through serial commands.

## Features

- Connects to the host via USB Type-C
- Host can switch audio inputs via USB virtual serial port commands
- Supports switching between four audio input channels
- Supports querying the current audio input channel
- Supports switching channels by button

## Usage

1. Connect the device to the host using a USB Type-C cable.
2. Open a serial terminal (such as `putty`, `minicom`, or any serial debugging tool), select the corresponding serial port, and set the baud rate to 115200.
3. Send the following character commands to operate:

    | Command | Function                  |
    |---------|---------------------------|
    | 1       | Switch to audio input 1   |
    | 2       | Switch to audio input 2   |
    | 3       | Switch to audio input 3   |
    | 4       | Switch to audio input 4   |
    | 0       | Query current audio input |

4. The device will automatically switch the audio input or return the current channel number.

## PCB
Designed in KiCAD, gerber files included.

## License

This project is licensed under the MIT License. See the LICENSE file for details.
