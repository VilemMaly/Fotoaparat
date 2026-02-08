#pragma once
#include "window.h"
#include "button.h"
#include "Switch.h"
#include "slider.h"
#include "SDkarta.h"

class settings : public window
{
public:
    settings(TFT_eSPI& tft, xpt& touch, SDkarta& sd);

protected:
    void onEnter() override;
    void onDraw() override;
    void onUpdate() override;
    void onExit() override;

private:
    button exit;
    button next;
    Switch rgb;
    Switch postProcess;
    Switch aec;
    Switch grbg;
    slider jas;
    slider clk;
    slider sum;
    slider gain;
    SDkarta& sd;
    xpt& touch;
};
