#pragma once
#include <TFT_eSPI.h>
#include "xpt.h"

class window
{
public:
    window(TFT_eSPI& tftref, xpt& touch);
    virtual ~window() = default;

    void enter();     // vstup do okna (spustí loop)
    void escape();    // ukončí okno

protected:
    TFT_eSPI& tft;
    xpt& _touch;

    uint16_t bgColor = TFT_BLACK;

    // životní cyklus okna – implementuje odvozená třída
    virtual void onEnter() = 0;
    virtual void onDraw() = 0;
    virtual void onUpdate() = 0;
    virtual void onExit() = 0;

    bool isActive() const;

private:
    bool active = false;
};
