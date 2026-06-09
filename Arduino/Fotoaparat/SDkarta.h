#pragma once

#include <ArduinoJson.h>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SdFat.h>

class SDkarta
{
    #define SD_BLOCK 10240   // 10 kB
#pragma pack(push, 1)
typedef struct {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
    uint32_t headerSize;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t  xPixelsPerMeter;
    int32_t  yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
} BMPHeader;
#pragma pack(pop)

#pragma pack(push, 1)

struct AVI_MainHeader {
    uint32_t microSecPerFrame;
    uint32_t maxBytesPerSec;
    uint32_t padding;
    uint32_t flags;
    uint32_t totalFrames;
    uint32_t initialFrames;
    uint32_t streams;
    uint32_t bufferSize;
    uint32_t width;
    uint32_t height;
    uint32_t reserved[4];
};

struct AVI_StreamHeader {
    char     fccType[4];   // "vids"
    char     fccHandler[4];// "DIB "
    uint32_t flags;
    uint16_t priority;
    uint16_t language;
    uint32_t initialFrames;
    uint32_t scale;
    uint32_t rate;
    uint32_t start;
    uint32_t length;
    uint32_t bufferSize;
    uint32_t quality;
    uint32_t sampleSize;
    int16_t  left, top, right, bottom;
};

struct BITMAPINFOHEADER_AVI {
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t imageSize;
    int32_t  xppm;
    int32_t  yppm;
    uint32_t clrUsed;
    uint32_t clrImportant;
};

#pragma pack(pop)


public:
    SDkarta(TFT_eSPI& tftref);

        // JSON settings
    bool writeSetting(const char* name, int value);
    int  readSetting(const char* name, int defaultValue = -1);

    void SDtoImage(const char* name);
    void imageToSD(const char* name);
    String getImages();
    String getToken(const String& src, uint16_t index);

    bool begin();

    bool openWrite(const char* name);
    bool openRW(const char* name);
    void close();
    void remove(const char* filename);
    uint8_t bit_reverse(uint8_t x);
    void flush();

    void write512(const uint8_t* data);
    void writeb(uint8_t byte);

    void DataFrame(int XRES, int YRES);
    void AVIHeaderYUV(int XRES, int YRES, const char* name);
    void BMPHeaderRGB565(int XRES, int YRES);
    void BMPToSD(const String& name, int XRES, int YRES);
    void BMPHeaderRGB24(int XRES, int YRES);
    

    void yuvToRGB(const char* filename, int xres, int yres);
    void swapRGB565Bytes(const char* filename, size_t headerSkipBytes);
    void prehrajZvuk(const char* filename);
    void write(const uint8_t* data, size_t len);

    void dbg(const char* msg, uint16_t y);

    bool binRGB565toAVI(
    const char* binName,
    const char* aviName,
    uint16_t xres,
    uint16_t yres,
    uint16_t fps
    );

    


private:
    SdFat sd;
    FsFile file;
    TFT_eSPI& tft;
    SPIClass* spi;

    bool debugMode = true;

    static constexpr uint16_t MAX_X = 320;
    static uint8_t rowBuffer[MAX_X * 8];

    static constexpr const char* SETTINGS_FILE = "settings.json";
};
