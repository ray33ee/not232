# N232 Features Summary

## Device Overview

The Not232 is a USB-CDC controlled GPIO interface and protocol engine designed as a low-cost alternative to other USB-GPIO devices like the FT232H or the MCP2221. 

With a rich set of hardware features like GPIO, ADC, PWM, Touchkey, CAN and UART, combined with software features such as SPI, I2C, One-Wire, Pulse capturing and neopixel support which can be used on any combination of up to 16 pins. For IO functions that require precise timing, the Not232 also ships with a special VMIOX language that allows hosts to send commands that can be executed at near bare metal speeds. 

COMING SOON:

While primarily intended as a USB controlled device, the Not232 can also be setup to accept commands over I2C instead, turning it into the mother of all I2C GPIO devices. Extra GPIO, ADC, PWM, I2C expander or its own IO core, this device can replace many I2C devices in one. Defaulting to address xx which can be configured to accept any 7 bit address via flash memory.

Selecting between USB or I2C mode can be done via the pins of a solder jumper, or if the jumper is left open then the device will look to an entry in FLASH to decide the mode.

## Pinout

![pinout](https://github.com/ray33ee/not232/blob/main/doc/pinout.png)

## Host Interfaces

- USB device interface
  - Full-speed USB device with CDC-style serial/data presentation
- I2C device interface
  - Protocol layer also supports operation as an I2C peripheral
  - Coming Soon: Selectable I2C address via FLASH
- Device identification
  - All identities end with the same 8 bytes which can be used to identify it as a Not232 device
  - They also contain 12 bytes which are unique to each Not232 device

## GPIO Resources

- 20 logical user GPIO pins configurable as
  - Push-pull output
  - Open-drain output
  - Floating input
  - Input with pull-up
  - Input with pull-down

## Analog and Touch Functions

- ADC measurement
  - 12-bit style ADC conversion
  - 10 Channels
- Capacitive touch sensing on the ADC pins

## PWM and Timed Output

- Hardware PWM output
  - 8-bit PWM duty control
  - Approximately `37.5 kHz` PWM base frequency
  - 12 PWM channels
- Pulse train generation
  - Host can stream a sequence of pulse durations compatible with IR

## Pulse Measurement and Timing

- Capture up to 3000 pulse lengths accurate to 1uS
- COMING SOON: Pulse counter up to u32 pulses
- COMING SOON: Frequency counter (count pulses and record time to calculate frequenxy)

## Bit-Banged Serial Bus Support

- Software I2C master up to 600KHz on any combination of 16 pins
  - Write, read, combined write read, start, stop and repeated stop supported
  - Fully adjustable frequenxy up to 600KHz
  - Up to 8 separate interfaces
- Software SPI master up to 3MHz on any combination of 16 pins
  - Supports all SPI modes
  - Read, write, and full-duplex write-read transactions
  - Fully adjustable frequenxy up to 3MHz
  - Up to 4 separate interfaces
  - COMING SOON: Bit width configurable up to 32 bits per word
- Software 1-Wire interfaces
  - Up to 16 separate interfaces

## UART

- COMING SOON: One hardware UART peripheral

## LED Support

- External WS2812/ NeoPixel
  - WS2812, WS2812B, WS2811 and SK6812 supported
  - Up to 16 separate interfaces
- On Board WS2812B status LED drive
- On Board yellow LED

## Scriptable Protocol Engine

- Very fast IO engine
- read, write, wait and serial shift/stack instructions
- Simulate a wide variety of protocols
- VM resources
  - 32 general-purpose 8-bit registers
  - 256-byte transmit pile
  - 256-byte receive pile
  - 16-level call stack
- Available on 16 pins


## Embedded Flash Filesystem

- On-chip 160 KiB littlefs storage
- File operations over host protocol
  - Remove file
  - Move/rename file
  - Open file
  - Close file
  - Read file
  - Write file
  - Seek
  - Truncate
  - Create directory
- Directory operations over host protocol
  - Open directory
  - Close directory
  - Read directory entries
- Filesystem runtime behavior
  - Automatically formats and remounts if mount fails
  - Supports up to 10 open file handles
  - Supports up to 10 open directory handles

## Practical Limits and Caveats

- Feature coverage is not uniform across all 20 logical pins
  - General GPIO functions support the full 20-pin logical map
  - Several timing-sensitive bit-banged features operate only on `GPIOA` in the current implementation
- USB is the effective primary interface in this firmware build
  - Some pulse I/O paths write directly through USB serial helpers rather than the generic comms abstraction
- USB descriptors are generic
  - Host software should rely on protocol identity rather than descriptor strings for device identification

# Issues

- EVentually, some of the write commands timeout and stop working (the known offenders are commented in the device code)
- Why does the device stop working when we connect an SD card (using the zif board) while the device is running?

# Todo

- Create a basic python gui that lets the user read, write, setup adc pins and visualise it.
- I2C is kind of working - for short messages we get consistent responses. For longer messages,, occasionally we get 0xFFs instead. This seems to happen with the built in peripheral and our bit banging. This could imply that the issue is with polling which both techniques use. However during testing the clock rate was slowed right down (to 10KHz) and still we had issues.
   - At the start of a write, we wait for the address byte then ack it. If the host issues a stop after, we know this is just polling to see if the device is busy, to we loop back and wait for another address byte. If the next event isnt a stop and it asks for data, this is the actual write command.
   - Implement a system for selecting either I2C or USB. I suggest a 3 terminal solder bridge. This can be used in three settings - connected to GND (via strong pullup 2.2k), VCC (via strong pullup 2.2k) or open. Connected to GND or VCC selects either I2C or USB. Left floating then fallsback to the saved setting in the FLASH. (We can determine whether the pin is floating or not by connecting to internal pullups and pulldowns and reading the results)
- Implement the trylock/unlock for the python code
- Get UART working (use interrupts for RXNE, good luck)
- Get CAN working
- Get op amps working
- Implement Soft I2S
- Disable interrupts in critical sections
- Modify SPI code to allow an arbitrary number of bits per word - up to 32
- Create a tool to download the not232 repo, modify the code to include the repo version, compile this, then upload the hex to a Not232 device
  - Add extra information to the device id (i.e. the information received by an 'identity' request) such as
    - git commit id for firmware
- Error handling
  - Buffer overflows
  - Check the right pins are used (host and device)
  - Device timeouts
  - Host timeouts
- Check PA13 and PA14 - CH32 datasheet says they are 2-wire debug on startup - do they need to be remapped to work?
- I dont like how littlefs lets you read past EOF
- Add a bridge to allow user to select how the host talks - either over USB or via the TX and RX pins
  - Create C code which call two functions, send byte and receive byte and builds the entire N232 communication protocol.
    This way users can use it in ANY C code that can talk over either USB or UART.