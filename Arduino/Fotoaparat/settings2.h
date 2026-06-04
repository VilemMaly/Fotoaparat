#pragma once
#include "window.h"
#include "button.h"
#include "Switch.h"
#include "slider.h"
#include "SDkarta.h"

class settings2 : public window
{
public:
    settings2(TFT_eSPI& tft, xpt& touch, SDkarta& sd);

protected:
    void onEnter() override;
    void onDraw() override;
    void onUpdate() override;
    void onExit() override;

private:
    button exit;
    button next;
    button calibrate;
    Switch agc;
    Switch awb;
    slider Lens;
    slider Glitch;
    SDkarta& sd;
    xpt& touch;
};
