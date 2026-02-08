#pragma once
#include "window.h"
#include "button.h"
#include "Switch.h"
#include "settings.h"
#include "media.h"
#include "camera.h"
#include "record.h"

class menu : public window
{
public:
    menu(TFT_eSPI& tft, xpt& touch, SDkarta& sd, ov7670& cam);

protected:
    void onEnter() override;
    void onDraw() override;
    void onUpdate() override;
    void onExit() override;

private:
    button nastaveni;
    button mediaTlacitko;
    button zpet;
    button natacet;
    SDkarta& sd;
    ov7670& cam;
};
