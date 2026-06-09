#include "settings.h"
#include "settings2.h"

// Konstruktor – inicializace button a switch
settings::settings(TFT_eSPI& tft, xpt& touch, SDkarta& sd)
    : window(tft, touch),
      exit(tft, touch, 40, 240, TFT_BLUE, 0, 0, "<", 4),
      next(tft, touch, 40, 240, TFT_BLUE, 280, 0, ">", 4),
      grbg(tft, touch, 80, 40, TFT_GREEN, 60, 0, "GRBG", 2),
      rgb(tft, touch, 80, 40, TFT_BLUE, 60, 60, "rgb565", 2),
      jas(tft, touch, 80, 40, TFT_BLACK, 60, 120, "Exp", 2),
      clk(tft, touch, 80, 40, TFT_BLACK, 160, 120, "clk", 2),
      sum(tft, touch, 80, 40, TFT_BLACK, 60, 170, "Sum", 2),
      gain(tft, touch, 120, 40, TFT_BLACK, 150, 170, "Gain", 2),
      aec(tft, touch, 80, 40, TFT_BLUE, 160, 0, "AEC", 2),
      postProcess(tft, touch, 80, 40, TFT_BLUE, 160, 60, "Process", 2),
      sd(sd),
      touch(touch)
{
    bgColor = TFT_BLACK;
}

// Zavolá se při vstupu do okna
void settings::onEnter()
{
    // Místo pro inicializaci logiky, callbacků, odkazů na jiné objekty
}

// Vykreslení ovládacích prvků
void settings::onDraw()
{
    //sd.SDtoImage("wallpaper.bmp");
    exit.draw();
    grbg.draw();
    next.draw();
    rgb.draw();
    jas.draw();
    sum.draw();
    clk.draw();
    postProcess.draw();
    aec.draw();
    gain.draw();
}

// Aktualizace logiky
void settings::onUpdate()
{
    if (exit.isPressed())
    {
        // Například přepnutí okna / změna stavu aplikace
        escape();
    }

    if (next.isPressed())
    {
    settings2 Nastaveni = settings2(tft, touch, sd);
    Nastaveni.enter();
    onDraw();
    }

    rgb.checkSwitch();
    jas.checkSlider();
    sum.checkSlider();
    clk.checkSlider();
    aec.checkSwitch();
    gain.checkSlider();
    postProcess.checkSwitch();
}

// Při opuštění okna
void settings::onExit()
{
    if(grbg.checkSwitch())
        sd.writeSetting("grbg", 1);
    else
        sd.writeSetting("grbg", 0);
    if(rgb.checkSwitch())
        sd.writeSetting("rgb565", 1);
    else
        sd.writeSetting("rgb565", 0);
    if(aec.checkSwitch())
        sd.writeSetting("aec", 1);
    else
        sd.writeSetting("aec", 0);
    if(postProcess.checkSwitch())
        sd.writeSetting("postProcessYUV", 1);
    else
        sd.writeSetting("postProcessYUV", 0);
    sd.writeSetting("denoise", sum.checkSlider());
    sd.writeSetting("expozice", jas.checkSlider());
    sd.writeSetting("gain", gain.checkSlider());
    sd.writeSetting("clk", clk.checkSlider());
    tft.fillScreen(TFT_BLACK);
}
