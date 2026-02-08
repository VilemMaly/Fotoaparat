#pragma once
#include <XPT2046_Touchscreen.h>
#include "SDkarta.h"
#include "piny.h"
#include "spiCS.h"
#include <Arduino.h>

class xpt
{
public:
    struct Point {
        int16_t x;
        int16_t y;
    };

    explicit xpt(XPT2046_Touchscreen& touch, SDkarta& sd);

    bool touched();
    Point touchedWhere();          // vrací pixelovou pozici
    Point calibrate(uint8_t corner); // kalibrace jednoho rohu
    void autoCalibrate();          // kompletní kalibrace všech 4 rohů
    void setCalibration();

    int touchedMs(uint32_t limitMs);


private:
    XPT2046_Touchscreen& _touch;
    SDkarta& sd;

    int16_t min_x;
    int16_t max_x;
    int16_t min_y;
    int16_t max_y;

    static constexpr int16_t SCREEN_W = 320;
    static constexpr int16_t SCREEN_H = 240;
};
