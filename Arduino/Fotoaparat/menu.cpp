#include "menu.h"

// Konstruktor – inicializace button a switch
menu::menu(TFT_eSPI& tft, xpt& touch, SDkarta& sd, ov7670& cam)
    : window(tft, touch), // 320 / 2 = 160 240/2 = 120
      nastaveni(tft, touch, 160, 120, TFT_BLUE, 0, 120, "Nastaveni", 2),
      mediaTlacitko(tft, touch, 160, 120, TFT_BLUE, 160, 0, "Fotky", 2),
      zpet(tft, touch, 160, 120, TFT_BLUE, 0, 0, "<", 20),
      natacet(tft, touch, 160, 120, TFT_BLUE, 160, 120, "Video", 2),
      cam(cam),
      sd(sd)
{
    bgColor = TFT_DARKGREY;
}

// Zavolá se při vstupu do okna
void menu::onEnter()
{
    // Místo pro inicializaci logiky, callbacků, odkazů na jiné objekty
    tft.fillScreen(TFT_BLACK);
}

// Vykreslení ovládacích prvků
void menu::onDraw()
{
    nastaveni.draw();
    mediaTlacitko.draw();
    zpet.draw();
    natacet.draw();
}

// Aktualizace logiky
void menu::onUpdate()
{
    if (zpet.isPressed())
    {
        // Například přepnutí okna / změna stavu aplikace
        escape();
    }

    if (nastaveni.isPressed())
    {
      settings Nastaveni = settings(tft, _touch, sd);
      Nastaveni.enter();
      onDraw();
    }

    if(mediaTlacitko.isPressed())
    {

      media Media = media(tft, _touch, sd);
      Media.enter();
      onDraw();

    }

    if(natacet.isPressed())
    {

      record Record = record(tft, _touch, cam, sd);
      Record.enter();
      onDraw();

    }
}

// Při opuštění okna
void menu::onExit()
{
    tft.fillScreen(TFT_BLACK);
}
