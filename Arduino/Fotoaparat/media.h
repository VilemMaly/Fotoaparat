#pragma once
#include "window.h"
#include "button.h"
#include "Switch.h"
#include "slider.h"
#include "SDkarta.h"

class media : public window
{
public:
    media(TFT_eSPI& tft, xpt& touch, SDkarta& sd);

protected:
    void onEnter() override;
    void onDraw() override;
    void onUpdate() override;
    void onExit() override;

private:
    button exit;
    button next;
    SDkarta& sd;

    int index;
};
