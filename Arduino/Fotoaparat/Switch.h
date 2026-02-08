#pragma once
#include <TFT_eSPI.h>
#include "xpt.h"

class Switch
{
public:
    Switch(
        TFT_eSPI& tftref,
        xpt& touch,
        int width,
        int height,
        int color,
        int x,
        int y,
        String text,
        uint8_t textSize
    );

    bool checkSwitch();   // kontrola dotyku + přepnutí stavu
    void draw();          // vykreslení spínače

private:
    TFT_eSPI& tft;
    xpt& _touch;

    String text;

    int width;
    int height;
    int color;
    int x;
    int y;
    int delayMs = 100;

    bool state = false;   // false = OFF, true = ON
    bool pressed = false;

    bool contains(int px, int py);
    uint16_t darker(uint16_t color);
    uint8_t textSize;
};
