# Issues

- Sending large (300 ish) bytes over usb back to hose seems to create some issues. Doing this with a dht via pulse io (which sends 81 pulses or 324 bytes) is a good way to eventually reproduce the error

# Pinout

![pinout](https://github.com/ray33ee/not232/blob/master/doc/pinout.png)

# Todo

- Get SPI working
- Get FLASH working
- Get CAN working
- Get Touch key working
- Get op amps working
- Get F0-F3 (TX, RX, Sw and LED) working as regular GPIO
- Get F0 and F1 working as PWM and ADC
- Disable interrupts in critical sections
- Add extra information to the device id (i.e. the information received by an 'identity' request) such as
	- git commit id for firmware