#include "media.h"

// Konstruktor – inicializace button a switch
media::media(TFT_eSPI& tft, xpt& touch, SDkarta& sd)
    : window(tft, touch),
      exit(tft, touch, 40, 240, TFT_BLUE, 0, 0, "<", 4),
      next(tft, touch, 40, 240, TFT_BLUE, 280, 0, ">", 4),
      sd(sd)
{
    bgColor = TFT_DARKGREY;
    index = 0;
}

// Zavolá se při vstupu do okna
void media::onEnter()
{
    // Místo pro inicializaci logiky, callbacků, odkazů na jiné objekty
}

// Vykreslení ovládacích prvků
void media::onDraw()
{
    index = 0;
    sd.SDtoImage(sd.getToken(sd.getImages(), index).c_str());
    exit.draw();
    next.draw();
}

// Aktualizace logiky
void media::onUpdate()
{
    if (exit.isPressed())
    {
      index--;
      if(index <= -1) escape();

      sd.SDtoImage(sd.getToken(sd.getImages(), index).c_str());
      exit.draw();
      next.draw();
    }

    if (next.isPressed())
    {
      index++;

      sd.SDtoImage(sd.getToken(sd.getImages(), index).c_str());
      exit.draw();
      next.draw();
    }
}

// Při opuštění okna
void media::onExit()
{
    tft.fillScreen(TFT_BLACK);
}
