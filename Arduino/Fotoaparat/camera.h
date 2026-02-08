#pragma once
#include "FifoCamera.h"

#define VSYNCSTATUS() (GPIOB->IDR >> 5) & 1

const int VSYNC = PB5;   // vsync
const int SIOD  = PB7;   // siod (SDA)
const int SIOC  = PB6;   // sioc (SCL)


const int RCK  = PB14;   // read clock
const int RRST = PB15;   // read reset
const int WR   = PA10;   // write enable
const int WRST = PA9;    // write reset

//OE -> GND     (output enable always on since we control the read clock)
//PWDN not nonnected  
//HREF not connected
//STR not connected
//RST -> 3.3V 

const int DAT0 = PB13;  // D0
const int DAT1 = PA12;  // D1
const int DAT2 = PB12;  // D2
const int DAT3 = PA15;  // D3
const int DAT4 = PB9;   // D4
const int DAT5 = PB3;   // D5
const int DAT6 = PB8;   // D6
const int DAT7 = PB4;   // D7

//#define QQVGA
//#define QQQVGA
#define QVGA
#ifdef QVGA
const int XRES = 320;
const int YRES = 240;
#endif

#ifdef QQVGA
const int XRES = 160;
const int YRES = 120;
#endif
#ifdef QQQVGA
const int XRES = 80;
const int YRES = 60;
#endif

const int BYTES_PER_PIXEL = 1;
const int frameSize = XRES * YRES * BYTES_PER_PIXEL;

using ov7670 = FifoCamera<RRST, WRST, RCK, WR, DAT0, DAT1, DAT2, DAT3, DAT4, DAT5, DAT6, DAT7>;