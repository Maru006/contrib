#pragma once

/* https://en.wikipedia.org/wiki/Servo_control */
#define UPDATE_RATE 50
#define PWM 35
#define PCA9685_OSC_CLOCK 25000000

/* PCA9685 DATASHEET REGISTERS*/
#define MODE1 0x00
#define PCA 0x40
#define HEAD 0x06 //PCA LED0_ON_H
#define TAIL 0x0A //PCA LED1_ON_L
#define PRESCALE 0xFE

/* TCA9548A DATASHEET REGISTERS*/
#define MUX 0x70

/* Master definitions for communication*/
#define I2C_DEV_PATH "/dev/i2c-1"

/* Servo Limitations */
#define MAXLEFT 400
#define MAXDOWN 400
#define MAXRIGHT 2200
#define MAXUP 2100

/* Control Parameters */
#define SENSITIVITY 100
#define HEADDEFAULT 1000
#define TAILDEFAULT 1000

/* Controls */
#define TOGGLE 'y'
#define TURNUP 'w'
#define TURNLEFT 'a'
#define TURNDOWN 's'
#define TURNRIGHT 'd'
#define RESET 'r'
#define CHANGECHANNEL 'c'
#define TERMINATE 27 // keyboard 'ESC' key

/* Function Definitions for utils.c*/
int command2tca(int fd, uint8_t channel);
int command2pca(int fd, uint8_t reg, uint8_t data); 
int setangle(int fd, int *runtime, uint8_t channel, int data, uint8_t prescale);

int raw_command2pca(int fd, uint8_t reg, uint8_t data);
int raw_set_angle(int fd, int channel, int data, uint8_t prescale);
