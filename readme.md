# Issues

- EVentually, some of the write commands timeout and stop working (the known offenders are commented in the device code)

# Pinout

![pinout](https://github.com/ray33ee/not232/blob/main/doc/pinout.png)
		  
# Todo

- Implement return codes from the littlefs functions
- Add a bridge to allow user to select how the host talks - either over USB or via the TX and RX pins
	- Create C code which call two functions, send byte and receive byte and builds the entire N232 communication protocol.
	  This way users can use it in ANY C code that can talk over either USB or UART.
- Turn the python code into a working python package
- Get UART working (use interrupts for RXNE, good luck)
- Get FLASH working
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

