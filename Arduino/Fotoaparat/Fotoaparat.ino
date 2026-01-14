#include <SPI.h>
#include <SdFat.h>
#include "SDkarta.h"
#include "Adafruit_GFX.h"         // Adafruit Graphics library
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "piny.h"
#include "I2C.h"


// Initialize ILI9341 display with custom SPI and defined control pins
TFT_eSPI tft = TFT_eSPI(); // Vytvoření objektu

XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

HardwareSerial Serial3(PA3, PA2); // RX, TX

#include "I2C.h"
#include "FifoCamera.h"

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

OV7670_I2C i2c(PB7, PB6);

FifoCamera<RRST, WRST, RCK, WR, DAT0, DAT1, DAT2, DAT3, DAT4, DAT5, DAT6, DAT7> camera(i2c);

SDkarta sdkarta(tft);

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
//unsigned char frame[frameSize];
unsigned char frame[1];
#include "stm32f1xx_hal.h"

void setup() 
{
  //digitalWrite(PA12, LOW);
  //digitalWrite(PA9, HIGH);
  Serial3.begin(500000);
  Serial3.println("Initialization...");

  __HAL_RCC_AFIO_CLK_ENABLE();

  // vypne JTAG, nechá SWD
  MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_1);

  pinMode(PB11, OUTPUT);
  digitalWrite(PB11, LOW);   // PWDN = 0
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);   // PWDN = 0
  digitalWrite(TFT_RST, HIGH);   // PWDN = 0
  digitalWrite(TFT_CS, HIGH);   // PWDN = 0
  digitalWrite(TOUCH_CS, HIGH);   // PWDN = 0
  digitalWrite(SD_CS, HIGH);   // PWDN = 0
  delay(50);
  // zbytek setupu


  i2c.init();

  //i2c.testOV7670();
  

  camera.init();
  #ifdef QVGA
  camera.QVGARGB565();  
  #endif

  #ifdef QQVGA
  camera.QQVGARGB565();
  #endif
  //camera.QVGARGB565();

  //camera.QQVGARGB565();
  //camera.QQQVGARGB565();
  //camera.QQVGAYUV();
  //camera.RGBRaw();
  //camera.testImage();

  Serial3.println( i2c.readRegister(0x12), HEX);
  Serial3.println( i2c.readRegister(0x71), HEX);
  
  pinMode(VSYNC, INPUT);
  pinMode(TFT_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);




  //Serial3.println("start");
  digitalWrite(TOUCH_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
  delay(10);
  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(3);
  tft.fillScreen(0);
  delay(10);
  digitalWrite(TFT_CS, HIGH);
  touch.begin(tft.getSPIinstance());
  delay(10);
  sdkarta.begin();

  initBattery();

}

void loop() 
{
  //frameToSerial3();
  digitalWrite(TOUCH_CS, LOW);
  digitalWrite(TFT_CS, LOW);
  delay(10);
  if (touch.touched()) {
    tone(ZVUK, 2000, 100);
    //sdkarta.prehrajZvuk("shutter.raw");

    // Časovač pro SD operaci
    spiAllHigh();
    unsigned long sdStart = millis();
    //sdkarta.BMPToSD(String(millis()) + ".bmp", XRES, YRES);
    String name = String(millis()) + "foto.bmp";
    camera.BinToSD(name.c_str(), XRES, YRES, 4, sdkarta);
    unsigned long sdDuration = millis() - sdStart;
    //sdkarta.convertRGB565Endian(name.c_str(), 40);
    //sdkarta.swapRGB565Bytes(name.c_str(), 40);
    Serial3.println("SD trvala ");
    Serial3.print(sdDuration);
    Serial3.println(" ms");
    spiTouch();
    
    //camera.FrameToSerial(Serial3, XRES, YRES);

  }
  else
  {
    showBattery();
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(TFT_CS, LOW);
    unsigned long lcdStart = millis();
    while(!digitalRead(VSYNC));
    while(digitalRead(VSYNC));
    camera.prepareCapture();
    camera.startCapture();
    while(!digitalRead(VSYNC));
    camera.stopCapture();
    camera.FrameToDisplay(tft, XRES, YRES);
    //camera.testFifo(XRES, YRES);
    unsigned long lcdDuration = millis() - lcdStart;
    Serial3.print("LCD trvala ");
    Serial3.print(lcdDuration);
    Serial3.println(" ms");

  }

  //camera.YuvToDisplay(&tft, XRES, YRES, &Serial3);
  //camera.readFrame(frame, XRES, YRES, BYTES_PER_PIXEL);
  //displayRGB565();
  
  //testTFT();
  
  //b/w 
  //camera.readFrameOnlySecondByte(frame, XRES, YRES);
  //displayRGB565();
  //displayY8();
  
}

// Rozměry a pozice baterie
#define BATTERY_X 250
#define BATTERY_Y 10
#define BATTERY_WIDTH 40
#define BATTERY_HEIGHT 20
#define BATTERY_PADDING 2

// Barvy
#define BATTERY_FRAME_COLOR TFT_WHITE
#define BATTERY_OK_COLOR    TFT_GREEN
#define BATTERY_BAD_COLOR   TFT_RED
#define BATTERY_BG_COLOR    TFT_BLACK

// ADC
#define ADC_MAX 4095
#define ADC_TOLERANCE 5

void showBattery()
{
    int adcValue = analogRead(BATTERY);

    // ±5 kroků kolem 3.3 V
    bool ok = (adcValue >= (ADC_MAX - ADC_TOLERANCE)) &&
              (adcValue <= ADC_MAX);

    uint16_t fillColor = ok ? BATTERY_OK_COLOR : BATTERY_BAD_COLOR;

    // vymazání oblasti
    tft.fillRect(BATTERY_X, BATTERY_Y,
                 BATTERY_WIDTH + 4, BATTERY_HEIGHT,
                 BATTERY_BG_COLOR);

    // rámeček
    tft.drawRect(BATTERY_X, BATTERY_Y,
                 BATTERY_WIDTH, BATTERY_HEIGHT,
                 BATTERY_FRAME_COLOR);

    // pól baterie
    tft.fillRect(BATTERY_X + BATTERY_WIDTH,
                 BATTERY_Y + BATTERY_HEIGHT / 4,
                 3, BATTERY_HEIGHT / 2,
                 BATTERY_FRAME_COLOR);

    // plná výplň – jen barva
    tft.fillRect(BATTERY_X + BATTERY_PADDING + 1,
                 BATTERY_Y + BATTERY_PADDING + 1,
                 BATTERY_WIDTH - 2 * BATTERY_PADDING - 2,
                 BATTERY_HEIGHT - 2 * BATTERY_PADDING - 2,
                 fillColor);
}

void initBattery()
{
    pinMode(BATTERY, INPUT_ANALOG);
    analogReadResolution(12);
    showBattery();
}



