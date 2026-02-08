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

// Zavolá se při vstupu do okna
void record::onEnter()
{
    // Místo pro inicializaci logiky, callbacků, odkazů na jiné objekty
    tft.fillScreen(TFT_BLACK);
    sd.AVIHeaderYUV(XRES,YRES, String(String(millis()) + "video.avi").c_str());
}

// Vykreslení ovládacích prvků
void record::onDraw()
{
    stop.draw();
    cam.yuv();
}

// Aktualizace logiky
void record::onUpdate()
{
    
    if (stop.isPressed())
    {
        // Například přepnutí okna / změna stavu aplikace
        sd.close();
        escape();
    }
    cam.YUVToSD(XRES, YRES, 2, sd);
    sd.DataFrame(XRES,YRES);
}

// Při opuštění okna
void record::onExit()
{
    tft.fillScreen(TFT_BLACK);
    cam.rgb();
    delay(1000);
}
