***(UNDER CONSTRUCTIONS)***
Requires some I/O Error handling. Otherwise, TCA9648A should function well with the PCA9685.
-20/07/2025

Added functionality (TCA9548A)
Most users won’t limit themselves to a single pair of servos. Yet the PCA9685 bundled with the Pan Tilt Camera from The Pi Hut** defaults to the I²C address 0x40. Meaning, if you connect multiple Pan Tilt Cameras**, they will all share the same I²C bus address — creating buffer conflicts and limiting control (they will move identical to one another and there is no way to differentiate between your products). While the PCA9685 comes in various configurations (some supporting more than two servos adding more degrees of servo control), the Pan Tilt Camera module is restricted to just two servos. To scale beyond this particular limitation, the most practical solution is to use an I²C multiplexer (TCA9548A) , like the SparkFun Qwiic Mux Breakout (https://thepihut.com/products/sparkfun-qwiic-mux-breakout-8-channel-tca9548a). 
-15/07/2025


Working C code for the Upgraded Pan Tilt Platform for Official Raspberry Pi Camera Module** 

This a free "software": you are free to change and redistribute it. There is NO WARRANTY, to the extent permitted by law.
This "software" is intended as an example on how to use use operate with the PCA9686 chip.
WARNING Before executing this "software" ensure you understand what the code does.

Simply compile as (assuming gcc compiler):

gcc main.c utils.c -l gpiod
**https://thepihut.com/products/pan-tilt-platform-for-official-raspberry-pi-camera-module

This is still work in progress as I aim to make it beginner friendly - So I want to add some explanations about the necessary concepts to work with the PCA9685 chip.
-28/05/2025
