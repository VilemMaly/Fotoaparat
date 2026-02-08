#include "SDkarta.h"
#include "spiCS.h"
#include "piny.h"
#include <ArduinoJson.h>


uint8_t SDkarta::rowBuffer[SDkarta::MAX_X * 8];

SDkarta::SDkarta(TFT_eSPI& tftref)
    : tft(tftref), spi(nullptr)
{
}

void SDkarta::dbg(const char* msg, uint16_t y)
{
    spiTFT();
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(2, y);
    tft.fillRect(0, y, 240, 10, TFT_BLACK);
    tft.print(msg);
}


bool SDkarta::writeSetting(const char* name, int value)
{
    spiSD();
    StaticJsonDocument<1024> doc;

    if (sd.exists(SETTINGS_FILE)) {
        FsFile rf = sd.open(SETTINGS_FILE, O_READ);
        if (rf) {
            deserializeJson(doc, rf);
            rf.close();
        }
    }

    doc[name] = value;

    FsFile wf = sd.open(
        SETTINGS_FILE,
        O_WRITE | O_CREAT | O_TRUNC
    );
    if (!wf) {
        spiAllHigh();
        return false;
    }

    if (serializeJson(doc, wf) == 0) {
        wf.close();
        spiAllHigh();
        return false;
    }

    wf.sync();
    wf.close();
    spiAllHigh();
    return true;
}


int SDkarta::readSetting(const char* name, int defaultValue)
{
    spiSD();

    if (!sd.exists(SETTINGS_FILE)) {
        spiAllHigh();
        return defaultValue;
    }

    FsFile f = sd.open(SETTINGS_FILE, O_READ);
    if (!f) {
        spiAllHigh();
        return defaultValue;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err || !doc.containsKey(name)) {
        spiAllHigh();
        return defaultValue;
    }

    int v = doc[name].as<int>();
    spiAllHigh();
    return v;
}

void SDkarta::AVIHeaderYUV(int XRES, int YRES, const char* name) {
    const char* filename = name; // můžeš měnit podle potřeby
    if (!openWrite(filename)) {
        dbg("Chyba: nelze otevrit soubor", 0);
        return;
    }

    // ====================
    // Výpočty základních hodnot
    // ====================
    uint32_t frames = 0; // zatím 0, bude se doplňovat při přidávání snímků
    uint32_t microSecPerFrame = 500000; // 1 fps
    uint32_t maxBytesPerSec = XRES * YRES * 2; // YUV422: 2 bajty na pixel
    uint32_t bufferSize = XRES * YRES * 2;

    // ====================
    // Chunk ID "RIFF" a velikost
    // ====================
    file.write("RIFF", 4);
    uint32_t riffSize = 4 + (8 + 56) + (8 + 56) + 8; // do "movi" + header, do budoucna se doplní
    file.write((uint8_t*)&riffSize, 4);
    file.write("AVI ", 4);

    // ====================
    // LIST hdrl
    // ====================
    file.write("LIST", 4);
    uint32_t listHdrlSize = 4 + (8 + 56) + (8 + 56); // hlavička + stream header + stream format
    file.write((uint8_t*)&listHdrlSize, 4);
    file.write("hdrl", 4);

    // ====================
    // Main AVI header
    // ====================
    AVI_MainHeader mainHeader = {};
    mainHeader.microSecPerFrame = microSecPerFrame;
    mainHeader.maxBytesPerSec   = maxBytesPerSec;
    mainHeader.padding          = 0;
    mainHeader.flags            = 0x10; // AVIF_HASINDEX
    mainHeader.totalFrames      = frames;
    mainHeader.initialFrames    = 0;
    mainHeader.streams          = 1;
    mainHeader.bufferSize       = bufferSize;
    mainHeader.width            = XRES;
    mainHeader.height           = YRES;

    file.write("avih", 4);
    uint32_t mainHeaderSize = sizeof(mainHeader);
    file.write((uint8_t*)&mainHeaderSize, 4);
    file.write((uint8_t*)&mainHeader, sizeof(mainHeader));

    // ====================
    // Stream header
    // ====================
    AVI_StreamHeader streamHeader = {};
    memcpy(streamHeader.fccType, "vids", 4);
    memcpy(streamHeader.fccHandler, "DIB ", 4); // žádný kompresor
    streamHeader.scale      = 1;
    streamHeader.rate       = 1; // 1 fps
    streamHeader.length     = frames;
    streamHeader.bufferSize = bufferSize;
    streamHeader.sampleSize = 0;

    file.write("LIST", 4);
    uint32_t listStreamSize = 4 + 8 + sizeof(streamHeader) + 8 + sizeof(BITMAPINFOHEADER_AVI);
    file.write((uint8_t*)&listStreamSize, 4);
    file.write("strl", 4);

    file.write("strh", 4);
    uint32_t strhSize = sizeof(streamHeader);
    file.write((uint8_t*)&strhSize, 4);
    file.write((uint8_t*)&streamHeader, sizeof(streamHeader));

    // ====================
    // Stream format (BITMAPINFOHEADER)
    // ====================
    BITMAPINFOHEADER_AVI bmpInfo = {};
    bmpInfo.size      = sizeof(BITMAPINFOHEADER_AVI);
    bmpInfo.width     = XRES;
    bmpInfo.height    = YRES;
    bmpInfo.planes    = 1;
    bmpInfo.bitCount  = 16;         // YUYV = 16 bit na pixel
    bmpInfo.compression = 0x32595559; // 'YUY2'
    bmpInfo.imageSize = XRES * YRES * 2;
    bmpInfo.xppm      = 0;
    bmpInfo.yppm      = 0;

    file.write("strf", 4);
    uint32_t strfSize = sizeof(BITMAPINFOHEADER_AVI);
    file.write((uint8_t*)&strfSize, 4);
    file.write((uint8_t*)&bmpInfo, sizeof(bmpInfo));

    // ====================
    // LIST movi (začátek dat)
    // ====================
    file.write("LIST", 4);
    uint32_t listMoviSize = 0xFFFFFFFF; // neznáme velikost, doplníme později
    file.write((uint8_t*)&listMoviSize, 4);
    file.write("movi", 4);
    DataFrame(XRES,YRES);

    // Nyní je soubor připraven pro zápis snímků YUV422 za hlavičku
}

void SDkarta::DataFrame(int XRES, int YRES)
{
    file.write("00db", 4);                      // chunk ID
    uint32_t frameSize = XRES * YRES * 2;
    file.write((uint8_t*)&frameSize, 4);        // velikost snímku

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
    spiSD();
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

void SDkarta::remove(const char* filename)
{
    openRW(filename);
    file.remove();
}
uint8_t SDkarta::bit_reverse(uint8_t x)
{
    x = (x >> 4) | (x << 4);
    x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);
    x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);
    return x;
}


void SDkarta::yuvToRGB(const char* filename, int xres, int yres)
{
    String outName = String(filename) + ".bmp";

    if (!openWrite(outName.c_str())) {
        spiAllHigh();
        return;
    }

    BMPHeaderRGB24(xres, yres); // nejdřív zapíšu bitmap hlavičku
    close();

    uint8_t YUV[512];                 // YUV422
    uint8_t RGB[(512 / 2) * 3];       // RGB24

    uint8_t Y = 0;
    uint8_t U = 0;
    uint8_t V = 0;
    uint8_t Y2 = 0;

    int16_t R = 0;
    int16_t G = 0;
    int16_t B = 0;

    for (int i = 0; i < (xres * yres * 2) / 512; i++)
    {
        FsFile yuv = sd.open(filename, O_RDONLY);
        yuv.seek(i * 512);
        yuv.read(YUV, 512);
        yuv.close();

        int yuv_i = 0;

        for (int b = 0; b < 768; b += 6)
        {
            Y  = YUV[yuv_i++];
            U  = YUV[yuv_i++];
            Y2 = YUV[yuv_i++];
            V  = YUV[yuv_i++];

            R = Y + ((359 * (V - 128)) >> 8);
            G = Y - (( 88 * (U - 128) + 183 * (V - 128)) >> 8);
            B = Y + ((454 * (U - 128)) >> 8);

            RGB[b+2]   = constrain(R, 0, 255);
            RGB[b+1] = constrain(G, 0, 255);
            RGB[b] = constrain(B, 0, 255);

            R = Y2 + ((359 * (V - 128)) >> 8);
            G = Y2 - (( 88 * (U - 128) + 183 * (V - 128)) >> 8);
            B = Y2 + ((454 * (U - 128)) >> 8);

            RGB[b+5] = constrain(R, 0, 255);
            RGB[b+4] = constrain(G, 0, 255);
            RGB[b+3] = constrain(B, 0, 255);
        }

        FsFile rgb = sd.open(outName.c_str(), O_RDWR);
        rgb.seek(54 + i * 768); // 54 = BMP hlavička
        rgb.write(RGB, 768);
        rgb.close();
    }
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

    /* ======================= BMP HEADER ======================= */

void SDkarta::BMPHeaderRGB24(int XRES, int YRES)
{
    spiSD();

    BMPHeader hdr{};
    hdr.signature     = 0x4D42;              // "BM"
    hdr.fileSize      = 54 + (XRES * YRES * 3);
    hdr.reserved      = 0;
    hdr.dataOffset    = 54;

    hdr.headerSize    = 40;                  // BITMAPINFOHEADER
    hdr.width         = XRES;
    hdr.height        = -YRES;               // top-down
    hdr.planes        = 1;
    hdr.bitsPerPixel  = 24;
    hdr.compression   = 0;                   // BI_RGB !!!
    hdr.imageSize     = XRES * YRES * 3;

    hdr.xPixelsPerMeter = 0;
    hdr.yPixelsPerMeter = 0;
    hdr.colorsUsed    = 0;
    hdr.colorsImportant = 0;

    file.write(&hdr, 54);
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


void SDkarta::SDtoImage(const char* name)
{
    spiSD();
    BMPHeader hdr;

    // ---- hlavička ----
    {
        spiSD();
        FsFile f = sd.open(name, O_RDONLY);
        if (!f) {
            dbg("BMP open fail", 0);
            return;
        }
        f.read(&hdr, sizeof(BMPHeader));
        f.close();
    }

    if (hdr.signature != 0x4D42 || hdr.bitsPerPixel != 24) {
        dbg("Bad BMP", 10);
        return;
    }

    const int width  = hdr.width;
    const int height = abs(hdr.height);

    const uint32_t rowSize   = (width * 3 + 3) & ~3;
    const uint32_t dataStart = hdr.dataOffset;

    uint8_t  sdBuf[SD_BLOCK];
    uint16_t lineBuf[320];     // 320 px → RGB565

    uint32_t filePos = 0;
    int y = 0;

    while (y < height)
    {
        // ---- SD: otevřít → seek → číst → zavřít ----
        spiSD();
        FsFile f = sd.open(name, O_RDONLY);
        if (!f) break;

        f.seek(dataStart + filePos);
        int rd = f.read(sdBuf, sizeof(sdBuf));
        f.close();
        if (rd <= 0) break;

        int i = 0;

        while (i + rowSize <= rd && y < height)
        {
            // ---- konverze jednoho řádku ----
            int p = 0;
            for (int x = 0; x < width; x++)
            {
                uint8_t B = sdBuf[i++];
                uint8_t G = sdBuf[i++];
                uint8_t R = sdBuf[i++];

                lineBuf[p++] =
                    ((R & 0xF8) << 8) |
                    ((G & 0xFC) << 3) |
                    ( B >> 3);
            }

            // přeskoč padding
            i += (rowSize - width * 3);

            // ---- TFT: celý řádek ----
            spiTFT();
            tft.startWrite();
            tft.setAddrWindow(0, y, width, 1);
            tft.pushColors(lineBuf, width, true);
            tft.endWrite();

            y++;
            filePos += rowSize;
        }
    }
}




void SDkarta::imageToSD(const char* name)
{
    spiSD();
    delay(10);

    FsFile bmp = sd.open(name, O_RDONLY);
    if (!bmp) {
        dbg("BMP open fail", 0);
        return;
    }

    BMPHeader hdr;
    bmp.read(&hdr, sizeof(BMPHeader));

    if (hdr.signature != 0x4D42) { // 'BM'
        dbg("Not BMP", 10);
        bmp.close();
        return;
    }

    if (hdr.bitsPerPixel != 16 || hdr.compression != 3) { // RGB565
        dbg("Unsupported BMP", 20);
        bmp.close();
        return;
    }

    const int width  = hdr.width;
    const int height = abs(hdr.height);

    if (width > MAX_X || height > 240) {
        dbg("Too big", 30);
        bmp.close();
        return;
    }

    // Přeskočíme hlavičku + masky
    //bmp.seek(hdr.dataOffset);

    constexpr int LINES = 20;
    const int bytesPerLine = width * 2;
    uint16_t lineBuf[width * LINES];

    spiTFT();
    delay(10);
    tft.startWrite();
    tft.setAddrWindow(0, 0, width, height);

    int y = 0;
    bool flipVertically = (hdr.height > 0); // BMP ukládá zdola nahoru, pokud height > 0

    while (y < height) {
        int linesToRead = min(LINES, height - y);
        int bytesToRead = linesToRead * bytesPerLine;
        uint8_t* dst = (uint8_t*)lineBuf;
        int remaining = bytesToRead;

        spiSD();
        delay(1);
        while (remaining > 0) {
            int n = bmp.read(dst, remaining);
            if (n <= 0) {
                dbg("SD read error", 99);
                break;
            }
            dst += n;
            remaining -= n;
        }

        spiTFT();
        if (flipVertically) {
            // flip každou řadu, protože BMP je zdola nahoru
            for (int l = 0; l < linesToRead; l++) {
                tft.setAddrWindow(0, height - (y + l) - 1, width, 1);
                tft.pushColors(&lineBuf[l * width], width, true);
            }
        } else {
            tft.pushColors(lineBuf, width * linesToRead, true);
        }

        y += linesToRead;
    }
    spiTFT();

    tft.endWrite();
    spiSD();
    bmp.close();
    spiAllHigh();
}






String SDkarta::getImages()
{
    spiSD();
    delay(10);

    FsFile dir = sd.open("/");
    if (!dir) return "";

    String result;
    FsFile file;

    char fname[64];   // FAT max 8.3 = 13, LFN typicky do 255, ale 64 stačí

    while ((file = dir.openNextFile())) {
        if (!file.isDirectory()) {
            if (file.getName(fname, sizeof(fname))) {
                String name = fname;
                name.toLowerCase();

                if (name.endsWith(".bmp")) {
                    if (result.length()) result += " ";
                    result += fname;   // původní jméno, ne lower-case
                }
            }
        }
        file.close();
    }

    dir.close();
    return result;
}



String SDkarta::getToken(const String& src, uint16_t index)
{
    uint16_t count = 0;
    int start = 0;

    for (uint16_t i = 0; i <= src.length(); i++) {
        if (i == src.length() || src[i] == ' ') {
            if (count == index) {
                return src.substring(start, i);
            }
            count++;
            start = i + 1;
        }
    }
    return "";
}

