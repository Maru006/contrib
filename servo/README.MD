*UNDER CONSTRUCTION*
Most users won’t limit themselves to a single pair of servos, yet the PCA9685 bundled with the Pan Tilt Camera from The Pi Hut defaults to the I²C address 0x40. If you connect multiple Pan Tilt Cameras, they’ll all share the same I²C bus — creating address conflicts and limiting control. While the PCA9685 comes in various configurations (some supporting more than two servos), the Pan Tilt Camera module is restricted to just two. To scale beyond this limitation, the most practical solution is to use an I²C multiplexer, like the SparkFun Qwiic Mux Breakout (TCA9548A).
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
