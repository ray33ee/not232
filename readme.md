# Issues

- EVentually, some of the write commands timeout and stop working (the known offenders are commented in the device code)
- Why does the device stop working when we connect an SD card (using the zif board) while the device is running?

# Pinout

![pinout](https://github.com/ray33ee/not232/blob/main/doc/pinout.png)
		  
# Todo

- I2C stuff is looking good - now implement the sytem that allows the host to poll until the device is ready (turn peripheral off when working, then back on when ready).
	 - At the start of a write, we wait for the address byte then ack it. If the host issues a stop after, we know this is just polling to see if the device is busy, to we loop back and wait for another address byte. If the next event isnt a stop and it asks for data, this is the actual write command.
	 - Implement a system for selecting either I2C or USB. I suggest a 3 terminal solder bridge. This can be used in three settings - connected to GND (via strong pullup 2.2k), VCC (via strong pullup 2.2k) or open. Connected to GND or VCC selects either I2C or USB. Left floating then fallsback to the saved setting in the FLASH. (We can determine whether the pin is floating or not by connecting to internal pullups and pulldowns and reading the results)
- Implement the trylock/unlock 
- Turn the python code into a working python package
- Get UART working (use interrupts for RXNE, good luck)
- Get CAN working
- Get op amps working
- Get RNG working
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
- Check PA13 and PA14 - CH32 datasheet says they are 1-wire debug on startup - do they need to be setup as AFPP/OD to work?
- Allow the N232 to be controlled over I2C as a slave device - This works in a similar way to EEPROM I2C - read a command fully, then execute it. Disable I2C while commands are being executed, then reenable when complete.
  The host must poll the device until it gets an ack, then it knows the device is free. nanoCH32V203 repo contains I2C example code in device mode
- I dont like how littlefs lets you read past EOF
- Add a bridge to allow user to select how the host talks - either over USB or via the TX and RX pins
	- Create C code which call two functions, send byte and receive byte and builds the entire N232 communication protocol.
	  This way users can use it in ANY C code that can talk over either USB or UART.