#include <SPI.h>
#include <SdFat.h>
#include "SDkarta.h"
#include "Adafruit_GFX.h"         // Adafruit Graphics library
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "piny.h"
#include "I2C.h"
#include "xpt.h"
#include "menu.h"
#include "I2C.h"
#include "camera.h"
#include "stm32f1xx_hal.h"

TFT_eSPI tft = TFT_eSPI(); // Vytvoření objektu

XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

HardwareSerial Serial3(PA3, PA2); // RX, TX

OV7670_I2C i2c(PB7, PB6);

SDkarta sdkarta(tft);

ov7670 camera(i2c, sdkarta);

xpt dotek = xpt(touch, sdkarta);

void setup() 
{
  Serial3.begin(500000);
  Serial3.println("Initialization...");

  __HAL_RCC_AFIO_CLK_ENABLE();

  // vypne JTAG, nechá SWD
  MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_1);

  pinMode(PB11, OUTPUT);
  pinMode(VSYNC, INPUT);
  pinMode(TFT_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);

  digitalWrite(PB11, LOW);   // PWDN = 0
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  digitalWrite(TFT_RST, HIGH);
  spiAllHigh();
  delay(10);

  // zbytek setupu
  i2c.init();
  //i2c.scan();
  

  camera.init();
  #ifdef QVGA
  camera.QVGARGB565();  
  #endif

  #ifdef QQVGA
  camera.QQVGARGB565();
  #endif

  //camera.RGBRaw();
  //camera.testImage();

  Serial3.println( i2c.readRegister(0x12), HEX);
  Serial3.println( i2c.readRegister(0x71), HEX);
  Serial3.println("start");

  spiTFT();
  delay(10);
  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(3);
  tft.fillScreen(0);

  delay(10);
  spiTouch();
  touch.begin(tft.getSPIinstance());
  delay(10);

  spiSD();
  sdkarta.begin();
  delay(10);
  if(sdkarta.readSetting("calibrate", 0) == 1)
    {
    dotek.autoCalibrate();
    }
  dotek.setCalibration();

  initBattery();

  camera.loadSettings();

}

void loop() 
{
  //frameToSerial3();
  spiTouch();
  delay(10);
  int timePress = dotek.touchedMs(500);
  if (timePress > 0 && timePress < 500) {
    camera.yuv();
    tone(ZVUK, 2000, 100);
    //sdkarta.prehrajZvuk("shutter.raw");

    // Časovač pro SD operaci
    unsigned long sdStart = millis();
    String name = String(millis()) + "yuv";

    if(sdkarta.readSetting("rgb565", 0) == 1)
    {
      camera.BinToSD(String(name + "rgb565.bmp").c_str(), XRES, YRES, 2, sdkarta);
    }
    
    camera.YUVtoSD(name.c_str(), XRES, YRES, 2, sdkarta);
    unsigned long sdDuration = millis() - sdStart;
    if(sdkarta.readSetting("postProcessYUV", 0) == 1)
    {
      sdkarta.yuvToRGB(name.c_str(), XRES, YRES);
      sdkarta.remove(name.c_str());
    }
    //sdkarta.convertRGB565Endian(name.c_str(), 40);
    //sdkarta.swapRGB565Bytes(name.c_str(), 40);
    Serial3.println("SD trvala ");
    Serial3.print(sdDuration);
    Serial3.println(" ms");
    camera.rgb();
    
    //camera.FrameToSerial(Serial3, XRES, YRES);

  }
  else if( timePress >= 500)
  {

    menu Menu = menu(tft, dotek, sdkarta, camera);
    Menu.enter();
    camera.loadSettings();

  }
    spiTFT();
    showBattery();
    unsigned long lcdStart = millis();
    // čekej na konec předchozího frame
    while(VSYNCSTATUS());

    // čekej na začátek frame
    while(!VSYNCSTATUS());

    // připrav FIFO (RST)
    camera.prepareCapture();

    // začni zapisovat
    camera.startCapture();

    // čekej na konec frame
    while(VSYNCSTATUS());

    // zastav zápis
    camera.stopCapture();

    // teď čti FIFO
    camera.FrameToDisplay(tft, XRES, YRES);

    //camera.testFifo(XRES, YRES);
    unsigned long lcdDuration = millis() - lcdStart;
    Serial3.print("LCD trvala ");
    Serial3.print(lcdDuration);
    Serial3.println(" ms");
  
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

    tft.setTextColor(TFT_WHITE, fillColor);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);     // ← použití velikosti textu

    tft.drawString(
        String(adcValue),
        BATTERY_WIDTH - 2 * BATTERY_PADDING - 2,
        BATTERY_HEIGHT - 2 * BATTERY_PADDING - 2
    );
}

void initBattery()
{
    pinMode(BATTERY, INPUT_ANALOG);
    analogReadResolution(12);
    showBattery();
}



