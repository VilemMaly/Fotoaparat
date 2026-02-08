#pragma once
#include "window.h"
#include "button.h"
#include "Switch.h"
#include "settings.h"
#include "media.h"
#include "camera.h"

class record : public window
{
public:
    record(TFT_eSPI& tft, xpt& touch, ov7670& cam, SDkarta& sd);

protected:
    void onEnter() override;
    void onDraw() override;
    void onUpdate() override;
    void onExit() override;

private:
    button stop;
    TFT_eSPI& tft;
    ov7670& cam;
    SDkarta& sd;
};
