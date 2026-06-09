#include "settings2.h"

// Konstruktor – inicializace button a switch
settings2::settings2(TFT_eSPI& tft, xpt& touch, SDkarta& sd)
    : window(tft, touch),
      exit(tft, touch, 40, 240, TFT_BLUE, 0, 0, "<", 4),
      next(tft, touch, 40, 240, TFT_BLUE, 280, 0, ">", 4),
      calibrate(tft, touch, 80, 40, TFT_BLUE, 120, 0, "KalDispl", 2),
      agc(tft, touch, 80, 40, TFT_GREEN, 60, 0, "AGC", 2),
      awb(tft, touch, 80, 40, TFT_BLUE, 60, 60, "AWB", 2),
      Lens(tft, touch, 80, 40, TFT_BLACK, 60, 120, "Lens", 2),
      Glitch(tft, touch, 80, 40, TFT_BLACK, 160, 120, "Glitch", 2),
      Casosber(tft, touch, 80, 40, TFT_BLACK, 160, 60, "Casosber", 2),
      sd(sd),
      touch(touch)
{
    bgColor = TFT_DARKGREY;
}

// Zavolá se při vstupu do okna
void settings2::onEnter()
{
    // Místo pro inicializaci logiky, callbacků, odkazů na jiné objekty
}

// Vykreslení ovládacích prvků
void settings2::onDraw()
{
    exit.draw();
    next.draw();
    agc.draw();
    awb.draw();
    Lens.draw();
    calibrate.draw();
    Glitch.draw();
    Casosber.draw();
}

// Aktualizace logiky
void settings2::onUpdate()
{
    if (exit.isPressed())
    {
        // Například přepnutí okna / změna stavu aplikace
        escape();
    }
    if (calibrate.isPressed())
    {
        tft.fillRoundRect(0, 0, 40, 40, 6, TFT_RED);
        tft.fillRoundRect(320-40, 0, 40, 40, 6, TFT_RED);
        tft.fillRoundRect(320-40, 240-40, 40, 40, 6, TFT_RED);
        tft.fillRoundRect(0, 240-40, 40, 40, 6, TFT_RED);
        touch.autoCalibrate();
        onDraw();
    }
    agc.checkSwitch();
    Lens.checkSlider();
    Glitch.checkSlider();
    Casosber.checkSlider();
    awb.checkSwitch();
}

// Při opuštění okna
void settings2::onExit()
{
    if(agc.checkSwitch())
        sd.writeSetting("agc", 0b100);
    else
        sd.writeSetting("agc", 0);
    if(awb.checkSwitch())
        sd.writeSetting("awb", 0b10);
    else
        sd.writeSetting("awb", 0);

    sd.writeSetting("correction", Lens.checkSlider());
    sd.writeSetting("mode", Glitch.checkSlider() / 10);
    sd.writeSetting("casosber", Casosber.checkSlider());
    tft.fillScreen(TFT_BLACK);
    delay(200);
}
