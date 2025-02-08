# ESP32Pass 🔐

A minimalist, secure, and offline password manager running on ESP32. Store your passwords locally with PIN protection and SHA-256 hashing.

![System Overview](https://s0.dtur.xyz/cover/esp32-password-manager-1.jpg)

## Features ✨

- 🔒 PIN-based authentication with SHA-256 hashing
- 💾 Local storage using SPIFFS (SPI Flash File System)
- 📝 JSON-based password management
- 🖥️ Simple Serial Terminal interface
- ⚡ Fast and efficient operation
- 🛡️ Offline security - no network connection required

## Requirements 🛠️

### Hardware
- ESP32 development board (tested on ESP32-C3-DevKitC-02)
- USB cable for power and communication

### Software
- PlatformIO
- Arduino framework
- Required libraries:
  - ArduinoJson (^7.3.0)
  - Built-in ESP32 libraries (SPIFFS, mbedtls)

## Installation 📥

1. Clone this repository:
```bash
git clone https://github.com/dongitran/Esp32Pass.git
```

2. Open the project in PlatformIO

3. Build and upload to your ESP32:
```bash
pio run -t upload
```

4. Open serial monitor at 115200 baud rate

## Usage 📖

### First Time Setup
1. Connect to your ESP32 using a serial terminal (115200 baud)
2. Create a PIN when prompted
3. Use the PIN to authenticate and access the password manager

### Available Commands
- `create` : Add a new password
- `get`    : Retrieve a stored password
- `delete` : Remove a password
- `list`   : Show all stored passwords
- `info`   : Display system information

### Example Usage
```
Enter new PIN > ****
✓ PIN created successfully!

> create
Enter password name > github
Enter password for 'github' > super_secret_456
✓ Password saved successfully!
```

## Security Features 🛡️

- PIN is hashed using SHA-256 before storage
- Passwords are stored in protected SPIFFS storage
- Offline operation prevents network-based attacks
- System limitations to prevent memory attacks:
  - Maximum 32 characters for password names
  - Maximum 8000 characters per password
  - Maximum 1000 stored passwords
  - Maximum 100KB total storage

## Contributing 🤝

Contributions are welcome! Please feel free to submit a Pull Request.

## License 📄

This project is licensed under the MIT License - see the LICENSE file for details.
