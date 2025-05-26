Working C code for DHT11 Humidity and Temperature

Uses libgpiod v2.3-devel 
by Copyright (C) 2017-2023 Bartosz Golaszewski License: GPL-2.0-or-later

Code here references the data sheet provided in the following link:
https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf

This a free "software": you are free to change and redistribute it. 
There is NO WARRANTY, to the extent permitted by law.

This "software" is intended as an example on how to use libgpiod v2.3-devel.

-13/05/2025

Instructions:

**Bearing in mind the version of libpgiod this is using and assuming you have cloned this repository as is with all the libraries in the same directory:

Simply compile as (assuming gcc compiler):

gcc main.c utils.c -l gpiod**
