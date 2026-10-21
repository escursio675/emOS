#projects 

P3 owns the hardware drivers of MiniOS, and two of the three, GPIO and UART, are built and verified on a Nucleo-F401RE board. ADC is the next driver.

## Details

**Flash tool**
st-flash from the stlink package, run by make flash , writing at address 0x8000000
**Serial terminal**
picocom at 115200 baud, 8 data bits, no parity, 1 stop bit

## Building and Flashing
1. Build with `make clean` and then `make` . A successful build prints nothing.
2. Plug in the board and flash with `make flash` . Success ends with `Flash written and verified! jolly good!` .
3. For UART tests, find the port with `ls /dev/tty.usb*` for Macs and `/dev/typACM0` for Linux. Open it with `picocom -b 115200 /dev/tty.usbmodemXXXX` or `picocom -b 115200 /dev/ttyACM0`.

**Useful details:**

1. Exit picocom with Ctrl+A, then Ctrl+X.
2. The black RESET button restarts the program. Use it when a test prints a message only once at startup, so you can open the terminal first and then catch it.
3. The port number can change after unplugging or reflashing, so run ls /dev/tty.usb* again each time. On Linux the port usually appears as /dev/ttyACM0 , and on Windows as a COM port in Device Manager.
4. Close every other serial program before opening picocom. Two programs on one port can split the incoming data.
5. Run make clean after switching branches or machines. Stale .d files can cause a `No rule to make target ... stdint.h error`.