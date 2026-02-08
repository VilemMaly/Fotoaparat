#pragma once
#include <TFT_eSPI.h>
#include "xpt.h"
#include "spiCS.h"

class button
{
public:
    button(
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

    bool isPressed();
    void draw();

private:
    TFT_eSPI& tft;
    xpt& _touch;

    String text;

    int width;
    int height;
    int color;
    int x;
    int y;

    uint8_t textSize;          // ← NOVÉ

    bool pressed = false;

    bool contains(int px, int py);
    uint16_t darker(uint16_t color);
};
