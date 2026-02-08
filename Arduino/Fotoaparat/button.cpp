#include "button.h"

button::button(
    TFT_eSPI& tftref,
    xpt& touch,
    int w,
    int h,
    int c,
    int xpos,
    int ypos,
    String txt,
    uint8_t txtSize
)
    : tft(tftref),
      _touch(touch),    
      width(w),
      height(h),
      color(c),
      x(xpos),
      y(ypos),
      text(txt),
      textSize(txtSize)
{
}

bool button::contains(int px, int py)
{
    return (px >= x &&
            px <= x + width &&
            py >= y &&
            py <= y + height);
}

uint16_t button::darker(uint16_t c)
{
    uint8_t r = (c >> 11) & 0x1F;
    uint8_t g = (c >> 5)  & 0x3F;
    uint8_t b =  c        & 0x1F;

    r = r * 3 / 4;
    g = g * 3 / 4;
    b = b * 3 / 4;

    return (r << 11) | (g << 5) | b;
}

bool button::isPressed()
{
    spiTouch();
    if (!_touch.touched())
    {
        pressed = false;
        return false;
    }

    xpt::Point p = _touch.touchedWhere();
    pressed = contains(p.x, p.y);
    return pressed;
}

void button::draw()
{
    uint16_t bg = pressed ? darker(color) : color;

    tft.fillRoundRect(x, y, width, height, 6, bg);
    tft.drawRoundRect(x, y, width, height, 6, TFT_WHITE);

    tft.setTextColor(TFT_WHITE, bg);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(textSize);     // ← použití velikosti textu

    tft.drawString(
        text,
        x + width / 2,
        y + height / 2
    );
}
