# IoT-Laundry-Alert
IoT device that sends text message alert when laundry cycle is complete. 

The program is currently using TM4C123 in place of the CC3220SF board and the 
blink_green function in place of send_sms while I'm working out a 
compatibility issue with the Temboo/Twilio SMS API.

This program is timed to my washer/dryer express cycle. You should adjust MAX_WAIT
and VIBRATION_SENSITIVITY as applicable for your own machine. 

## Installation and Usage Instructions

### Set Up Development Environment
- Download Energia IDE: https://energia.nu/download/
- Open Energia. Click Tools, Board, Boards Manager. 
- Find Energia Tiva C Boards. Click Install.

- Plug in TI TM4C123 Launchpad to USB Port.

- Open Device Manager (Windows)
- Expand “Ports” (COM & LPT).
  - Make note of the port number for Stellaris Virtual Serial Port (On mine, it is COM6).

- Return to the Energia Window
- Click Tools, Board.
- Select Launchpad (TIVA C) w/ tm4c123 (80mhz).
- Ensure the correct port is selected under Tools/Port (Found in previous step).

### Save Source Code File
- Within Energia Folder, save source code file (.ino) in a directory with the same name as the file. For example, my file path is C:…\Documents\Energia\Laundry_Alert\Laundry_Alert.ino

### Build the Circuit and Connect to the Launchpad
- Ensure that the Launchpad is not connected to power.
- Insert the SW-420 Vibration Sensor into the breadboard, connect DO to PB_0, VCC to 3.3V, and ground to ground on the breadboard.
- Insert LEDs with 470 Ohm resistors connected to the anodes, and cathodes to ground on the breadboard. Connect red LED to PB_1 and green LED to PB_2.
- Connect ground rail on breadboard to GND on Launchpad.
- Insert USB cable from Launchpad to computer. 
 
### Open, Compile, and Flash the Program
- Open the source code file.
- Click on the “verify” check mark in the upper left corner of the Energia window, to compile the code.
- Click on the “upload” arrow next to the check mark to flash the code to the Launchpad.

### Run the Program
- After flashing the program to the board, it is now running on your Launchpad.
- Remove USB from the COM Port on the computer. 
- When ready to use, simply place the Launchpad and circuit on top of the washer or dryer and connect to the 5V portable USB battery pack.
- The program is now running and will flash green to signify the end of the wash/dry cycle. 

[Installation_and_Usage_Instructions_git.pdf](https://github.com/joycemaferko/IoT-Laundry-Alert/files/7114052/Installation_and_Usage_Instructions_git.pdf)

