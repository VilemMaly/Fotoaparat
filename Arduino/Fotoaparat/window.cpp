#include "window.h"

window::window(TFT_eSPI& tftref, xpt& touch)
    : tft(tftref), _touch(touch)
{
}

void window::enter()
{
    active = true;

    tft.fillScreen(bgColor);
    onEnter();
    onDraw();

    while (active)
    {
        onUpdate();
        delay(1); // jednoduchý debounce / CPU relief
    }

    onExit();
}

void window::escape()
{
    active = false;
}

bool window::isActive() const
{
    return active;
}
