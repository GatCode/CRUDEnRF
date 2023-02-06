<p align="center">
  <img width="500" src="assets/logo.png">
  <p align="center">
  <img alt="nrf52840" src="https://img.shields.io/badge/Nordic-nrf52840-blue">
  <img alt="zephyr" src="https://img.shields.io/badge/RTOS-Zephyr-blueviolet">
  <img alt="license" src="https://img.shields.io/badge/License-Apache-blue">
  </p>
</p>

## About

This is a very crude implementation of a BLE packet sniffer, based on a [Nordic nRF52840DK](https://www.nordicsemi.com/Products/Development-hardware/nRF52840-DK). This project allows users to select a PHY (1M or 2M) as well as an access address and receive a 255 byte long PDU in hex.

## Basic Principle

- **Step 1**: Compile and flash the DK (recommended: [how to build your Zephyr app](https://zephyrproject.org/how-to-build-your-zephyr-app-in-a-standalone-folder/))
- **Step 2**: Connect to the DK (e.g., with [tio](https://github.com/tio/tio))
- **Step 3**: Select the PHY (`1M` / `2M`)
- **Step 4**: Enter the access address (e.g., `0x8e89bed6` for advertisements)
- **Step 5**: Enter the RF channel (e.g., `5`)

## Example Output

```
*** Booting Zephyr OS build zephyr-v3.2.0-3907-gbad5c921cd17 ***
Select PHY: [1M, 2M]
2M
Enter RF Channel: (between 0-39)
5
Enter Access Address: (e.g., 0xdeadbeef)
0x17a463bc
Starting Sniffer...
DEVMATCH: 0 | DEVMISS: 1 | RXMATCH: 0 | CRCOK: 0 | Packet: 07 06 00 04 ff ad de be 03 03 03 fe 0f 09 4c 45 5f 57 48 2d 31 30 30 30 58 4d 35 00 fd fd 22 00 00 4d 1d 66 23 da 78 24 aa d5 58 01 c7 de 49 5e c1 33 62 52 d3 94 bd f3 cc b8 00 00 76 00 00 00 00 20 00 40 a1 b7 00 00 0a 00 0b 15 25 6c 00 00 44 b5 00 00 93 6c 00 00 09 6c 00 00 0a 00 00 00 40 00 00 00 40 2d 00 20 6f c3 00 00 43 a8 00 00 24 2d 00 20 a1 0a 00 00 98 07 00 20 98 07 00 20 ff ff ff ff 70 c3 00 00 00 00 00 00 53 82 00 00 98 07 00 20 00 00 00 00 00 00 00 00 ff ff ff ff 00 00 00 00 00 00 00 00 00 04 00 00 98 07 00 20 10 2d 00 20 ff ff ff ff ff ff ff ff f9 86 00 00 98 07 00 20 00 00 00 00 00 00 00 00 0d 80 00 00 40 2d 00 20 40 2d 00 20 0d 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 55 08 00 00 00 00 00 00 24 2d 00 20 00 00 00 20 3b 91 00
... 
```

## Limitations

This Software is provided as-is!

Please feel free to adapt it to your needs and contribute to the project. I would be very grateful to include your improvements. Thanks for your support!

Currently there is no support for device address matching via the command line, but a device address can be set in the `main.c` file (untested).