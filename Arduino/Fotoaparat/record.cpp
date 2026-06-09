#include "record.h"

// Konstruktor – inicializace button a switch
record::record(TFT_eSPI& tft, xpt& touch, ov7670& cam, SDkarta& sd)
    : window(tft, touch), // 320 / 2 = 160 240/2 = 120
      stop(tft, touch, 200, 120, TFT_RED, 80, 60, "STOP", 4),
      sd(sd),
      cam(cam),
      tft(tft)
{
    bgColor = TFT_DARKGREY;
}
int sekundy;

// Zavolá se při vstupu do okna
void record::onEnter()
{
    // Místo pro inicializaci logiky, callbacků, odkazů na jiné objekty
    tft.fillScreen(TFT_BLACK);
    sd.AVIHeaderYUV(XRES,YRES, String(String(millis()) + "video.avi").c_str());
    sekundy = sd.readSetting("casosber",60);
}

// Vykreslení ovládacích prvků
void record::onDraw()
{
    stop.draw();
    cam.yuv();
}

unsigned long lastFrame = 0;

void record::onUpdate()
{
    if (stop.isPressed())
    {
        sd.close();
        escape();
        return;
    }

    if (millis() - lastFrame >= sekundy*1000)
    {
        lastFrame = millis();

        cam.YUVToSD(XRES, YRES, 2, sd);
        sd.DataFrame(XRES, YRES);
        sd.flush();
    }
}

// Při opuštění okna
void record::onExit()
{
    tft.fillScreen(TFT_BLACK);
    cam.rgb();
    delay(2000);
}
