Working C code for the LASER SENSOR module from WAVESHARE  

Uses libgpiod v2.3-devel 
by Copyright (C) 2017-2023 Bartosz Golaszewski License: GPL-2.0-or-later

Code here references the WAVESHARE website (i.e harware information and without using their software): 
https://www.waveshare.com/wiki/Laser_Sensor

This a free "software": you are free to change and redistribute it. 
There is NO WARRANTY, to the extent permitted by law.

This "software" is intended as an example on how to use libgpiod v2.3-devel.

WARNING
Before executing this "software", bear in mind WAVESHARE's note: 

"Note: Try to avoid touching the resistors and capacitors on the board when the power is on, so as to avoid burning out the infrared emitting tube. As the laser is harmful to the eyes, DO NOT irradiate the eyes directly while using."

Additionally, I have encountered my WAVESHARE LASER SENSOR would overheat and produce possibly toxic smoke when it is operated for prolonged periods of time. Please be mindful about #define DETECT set to very high number as the software will persist until either user forcefully stops the program or reaches the set DETECT value.

Some WAVESHARE LASER SENSOR appear to operate only by using more than 3.3v. Users are reminded, when operating with 5v GPIO this software will not terminate power from this line.

-18/05/2025
