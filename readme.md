# Issues

- Sending large (300 ish) bytes over usb back to hose seems to create some issues. Doing this with a dht via pulse io (which sends 81 pulses or 324 bytes) is a good way to eventually reproduce the error.
	- I think this is fixed? TBD

# Pinout

![pinout](https://github.com/ray33ee/not232/blob/main/doc/pinout.png)
		  
# Todo

- Read the [Circuit Python](https://docs.circuitpython.org/en/latest/docs/index.html) API (NOT BLINKA) to get detailed explanations of the functions, and make sure our code conforms
- Turn the python code into a working python package
- Get UART working (use interrupts for RXNE, good luck)
- Get FLASH working
- Get CAN working
- Get op amps working
- Disable interrupts in critical sections
- Create a tool to download the not232 repo, modify the code to include the repo version, compile this, then upload the hex to a Not232 device
	- Add extra information to the device id (i.e. the information received by an 'identity' request) such as
		- git commit id for firmware
- Error handling
	- Operation timeouts
	- Buffer overflows
	- Check the right pins are used (host and device)