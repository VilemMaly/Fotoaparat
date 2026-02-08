#include "Switch.h"

Switch::Switch(
    TFT_eSPI& tftref,
    xpt& touch,
    int w,
    int h,
    int c,
    int xpos,
    int ypos,
    String txt,
    uint8_t textSize
)
    : tft(tftref),
      _touch(touch),
      width(w),
      height(h),
      color(c),
      x(xpos),
      y(ypos),
      text(txt),
      textSize(textSize)
{
}

/**
 * Kontrola dotyku a přepnutí stavu
 */
bool Switch::checkSwitch()
{
    if (_touch.touched())
    {
        auto p = _touch.touchedWhere();

        if (contains(p.x, p.y) && !pressed)
        {
            pressed = true;
            state = !state;   // toggle
            draw();

            delay(delayMs);

            // počkej na puštění
            while (_touch.touched()) {
                delay(5);
            }
            pressed = false;
        }
    }

    return state;
}

/**
 * Vykreslení přepínače
 */
void Switch::draw()
{
    uint16_t bg = state ? color : darker(color);
    uint16_t knobColor = TFT_WHITE;

    // tělo spínače
    tft.fillRoundRect(x, y, width, height, height / 2, bg);
    tft.drawRoundRect(x, y, width, height, height / 2, TFT_BLACK);

    // posuvný obdélník
    int knobW = width / 2;
    int knobX = state ? (x + width - knobW) : x;

    tft.fillRoundRect(
        knobX,
        y,
        knobW,
        height,
        height / 2,
        knobColor
    );

    // popisek pod spínačem (volitelné)
    if (text.length())
    {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(textSize);
        tft.drawString(text, x + width / 2, y + height + 10);
    }
}

/**
 * Detekce, zda bod leží uvnitř spínače
 */
bool Switch::contains(int px, int py)
{
    return (px >= x && px <= x + width &&
            py >= y && py <= y + height);
}

/**
 * Ztmavení barvy (RGB565)
 */
uint16_t Switch::darker(uint16_t c)
{
    uint8_t r = (c >> 11) & 0x1F;
    uint8_t g = (c >> 5) & 0x3F;
    uint8_t b = c & 0x1F;

    r = r * 2 / 3;
    g = g * 2 / 3;
    b = b * 2 / 3;

    return (r << 11) | (g << 5) | b;
}
