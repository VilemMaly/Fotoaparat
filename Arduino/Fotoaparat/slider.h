#pragma once
#include <TFT_eSPI.h>
#include "xpt.h"

class slider
{
public:
    slider(
        TFT_eSPI& tftref,
        xpt& touch,
        int width,
        int height,
        int color,
        int x,
        int y,
        String text,
        uint8_t textSize        // ← NOVÉ
    );

    int checkSlider();   // kontrola dotyku + změna hodnoty
    void draw();         // vykreslení slideru

private:
    TFT_eSPI& tft;
    xpt& _touch;

    int w;
    int h;
    int bgColor;
    int posX;
    int posY;
    String label;
    uint8_t textSize;

    int hodnota;         // 0–100
};
