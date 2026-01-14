#include "SDkarta.h"
#include "spiCS.h"
#include "piny.h"

uint8_t SDkarta::rowBuffer[SDkarta::MAX_X * 8];

SDkarta::SDkarta(TFT_eSPI& tftref)
    : tft(tftref), spi(nullptr)
{
}

// Budoucí potenciální implementace natáčení videa
bool SDkarta::binRGB565toAVI(
    const char* binName,
    const char* aviName,
    uint16_t xres,
    uint16_t yres,
    uint16_t fps
)
{
    spiSD();

    FsFile bin = sd.open(binName, O_READ);
    if (!bin) return false;

    uint32_t fileSize = bin.size();
    uint32_t frameSize565 = xres * yres * 2;

    if (fileSize % frameSize565 != 0) {
        bin.close();
        return false;
    }

    uint32_t frameCount = fileSize / frameSize565;
    uint32_t frameSize24 = xres * yres * 3;

    FsFile avi = sd.open(aviName, O_WRITE | O_CREAT | O_TRUNC);
    if (!avi) {
        bin.close();
        return false;
    }

    uint32_t nothing = 0;
    /* ===== RIFF AVI ===== */
    avi.write("RIFF", 4);
    uint32_t riffSizePos = avi.curPosition();
    avi.write(nothing);     // doplní se později
    avi.write("AVI ", 4);

    /* ===== hdrl ===== */
    avi.write("LIST", 4);
    uint32_t hdrlSizePos = avi.curPosition();
    avi.write(nothing);
    avi.write("hdrl", 4);

    avi.write("avih", 4);
    avi.write((uint32_t)sizeof(AVI_MainHeader));

    AVI_MainHeader avih{};
    avih.microSecPerFrame = 1000000UL / fps;
    avih.maxBytesPerSec = frameSize24 * fps;
    avih.flags = 0x10;
    avih.totalFrames = frameCount;
    avih.streams = 1;
    avih.bufferSize = frameSize24;
    avih.width = xres;
    avih.height = yres;
    avi.write(&avih, sizeof(avih));

    /* ===== stream ===== */
    avi.write("LIST", 4);
    uint32_t strlSizePos = avi.curPosition();
    avi.write(nothing);
    avi.write("strl", 4);

    avi.write("strh", 4);
    avi.write((uint32_t)sizeof(AVI_StreamHeader));

    AVI_StreamHeader strh{};
    memcpy(strh.fccType, "vids", 4);
    memcpy(strh.fccHandler, "DIB ", 4);
    strh.scale = 1;
    strh.rate = fps;
    strh.length = frameCount;
    strh.bufferSize = frameSize24;
    strh.right = xres;
    strh.bottom = yres;
    avi.write(&strh, sizeof(strh));

    avi.write("strf", 4);
    avi.write((uint32_t)sizeof(BITMAPINFOHEADER_AVI));

    BITMAPINFOHEADER_AVI bih{};
    bih.size = sizeof(BITMAPINFOHEADER_AVI);
    bih.width = xres;
    bih.height = yres;
    bih.planes = 1;
    bih.bitCount = 24;
    bih.compression = 0;
    bih.imageSize = frameSize24;
    avi.write(&bih, sizeof(bih));

    /* === dopočty LIST velikostí === */
    uint32_t cur = avi.curPosition();
    avi.seekSet(strlSizePos);
    avi.write(cur - strlSizePos - 4);
    avi.seekSet(cur);

    avi.seekSet(hdrlSizePos);
    avi.write(cur - hdrlSizePos - 4);
    avi.seekSet(cur);

    /* ===== movi ===== */
    avi.write("LIST", 4);
    uint32_t moviSizePos = avi.curPosition();
    avi.write(nothing);
    avi.write("movi", 4);

    uint8_t in[2];
    uint8_t out[3];

    for (uint32_t f = 0; f < frameCount; f++) {
        avi.write("00db", 4);
        avi.write(frameSize24);

        for (uint32_t i = 0; i < xres * yres; i++) {
            bin.read(in, 2);
            uint16_t rgb565 = (in[0] << 8) | in[1];

            out[0] = ((rgb565 >> 11) & 0x1F) << 3;
            out[1] = ((rgb565 >> 5) & 0x3F) << 2;
            out[2] = (rgb565 & 0x1F) << 3;

            avi.write(out, 3);
        }
    }

    /* ===== finální velikosti ===== */
    cur = avi.curPosition();
    avi.seekSet(moviSizePos);
    avi.write(cur - moviSizePos - 4);
    avi.seekSet(riffSizePos);
    avi.write(cur - 8);

    bin.close();
    avi.close();
    spiAllHigh();
    return true;
}


/* ======================= INIT ======================= */

bool SDkarta::begin()
{
    spi = &tft.getSPIinstance();
    spiAllHigh();

    SdSpiConfig cfg(
        SD_CS,
        SHARED_SPI,
        SD_SCK_MHZ(35),
        spi
    );

    while (true) {
        spiSD();
        bool ok = sd.begin(cfg);

        if (ok) {
            spiAllHigh();
            break;
        }

        // SD selhala → vypis na TFT
        spiTFT();
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(30, 60);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(2);
        tft.setTextFont(1);
        tft.println("Neni pripojena");
        tft.println("SD karta!");

        if (debugMode) {
            Serial3.print("SD init error: ");
            Serial3.println(sd.sdErrorCode());
        }

        delay(1000);
    }

    if (debugMode) {
        Serial3.println("SD init OK");
    }

    spiAllHigh();
    return true;
}


/* ======================= FILE CORE ======================= */


void SDkarta::write(const uint8_t* data, size_t len)
{
    file.write(data, len);
}

bool SDkarta::openWrite(const char* name)
{
    spiSD();
    file = sd.open(name, O_WRITE | O_CREAT | O_TRUNC);
    return file;
}

bool SDkarta::openRW(const char* name)
{
    spiSD();
    file = sd.open(name, O_RDWR);
    return file;
}

void SDkarta::close()
{
    if (file) {
        file.sync();
        file.close();
    }
    spiAllHigh();
}

void SDkarta::write512(const uint8_t* data)
{
    file.write(data, 512);
}

void SDkarta::writeb(uint8_t byte)
{
    file.write(&byte, 1);
}

/* ======================= RGB565 SWAP ======================= */

void SDkarta::swapRGB565Bytes(const char* filename, size_t headerSkipBytes)
{
    if (!openRW(filename)) {
        Serial3.println("Open RW failed");
        spiAllHigh();
        return;
    }

    file.seekSet(headerSkipBytes);

    uint32_t dataSize = file.size() - headerSkipBytes;
    uint8_t buffer[512];
    uint32_t processed = 0;

    while (processed < dataSize) {
        uint16_t n = min<uint32_t>(512, dataSize - processed);
        int r = file.read(buffer, n);
        if (r <= 0) break;

        for (int i = 0; i < r - 1; i += 2) {
            uint8_t t = buffer[i];
            buffer[i] = buffer[i + 1];
            buffer[i + 1] = t;
        }

        file.seekSet(headerSkipBytes + processed);
        file.write(buffer, r);
        processed += r;
    }

    close();
}

/* ======================= BMP HEADER ======================= */

void SDkarta::BMPHeaderRGB565(int XRES, int YRES)
{
    spiSD();

    BMPHeader hdr{};
    hdr.signature = 0x4D42;
    hdr.headerSize = 40;
    hdr.width = XRES;
    hdr.height = -YRES;
    hdr.planes = 1;
    hdr.bitsPerPixel = 16;
    hdr.compression = 3;
    hdr.imageSize = XRES * YRES * 2;
    hdr.redMask   = 0xF800;
    hdr.greenMask = 0x07E0;
    hdr.blueMask  = 0x001F;
    hdr.dataOffset = sizeof(BMPHeader);
    hdr.fileSize = hdr.dataOffset + hdr.imageSize;

    file.write(&hdr, sizeof(hdr));
}

/* ======================= TFT → BMP ======================= */

void SDkarta::BMPToSD(const String& name, int XRES, int YRES)
{
    if (!openWrite(name.c_str())) {
        Serial3.println("File open failed");
        return;
    }

    BMPHeaderRGB565(XRES, YRES);

    for (int y = YRES - 1; y >= 0; y--) {
        uint16_t i = 0;

        spiTFT();
        for (int x = 0; x < XRES; x++) {
            uint16_t c = tft.readPixel(x, y);
            rowBuffer[i++] = c >> 8;
            rowBuffer[i++] = c & 0xFF;
        }

        spiSD();
        file.write(rowBuffer, XRES * 2);
    }

    close();
}



/* ======================= AUDIO ======================= */

void SDkarta::prehrajZvuk(const char* filename)
{
    if (!openRW(filename)) {
        Serial.println("Audio open failed");
        return;
    }

    pinMode(ZVUK, OUTPUT);
    const uint32_t interval = 125;
    uint32_t next = micros();

    while (file.available()) {
        while (micros() < next) { __asm__("nop"); }
        analogWrite(ZVUK, file.read());
        next += interval;
    }

    analogWrite(ZVUK, 0);
    close();
}
