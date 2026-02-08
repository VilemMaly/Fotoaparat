#include "xpt.h"

xpt::xpt(XPT2046_Touchscreen& touch, SDkarta& sd)
    : _touch(touch), sd(sd),
      min_x(0), max_x(4095),
      min_y(0), max_y(4095)
{
}

bool xpt::touched()
{
    spiTouch();
    return _touch.touched();
}

xpt::Point xpt::touchedWhere()
{
    spiTouch();
    Point p{ -1, -1 };

    if (!_touch.touched())
        return p;

    TS_Point raw = _touch.getPoint();

    p.x = ::map(raw.x, min_x, max_x, 0, SCREEN_W);
    p.y = ::map(raw.y, min_y, max_y, 0, SCREEN_H);

    if (p.x < 0) p.x = 0;
    if (p.x > SCREEN_W) p.x = SCREEN_W;
    if (p.y < 0) p.y = 0;
    if (p.y > SCREEN_H) p.y = SCREEN_H;

    return p;
}

xpt::Point xpt::calibrate(uint8_t corner)
{
    if (!_touch.touched())
        return Point{0,0};

    TS_Point raw = _touch.getPoint();

    switch (corner)
    {
        case 0: // levý horní
            min_x = raw.x;
            min_y = raw.y;
            break;
        case 1: // pravý horní
            max_x = raw.x;
            min_y = min(min_y, raw.y);
            break;
        case 2: // pravý dolní
            max_x = raw.x;
            max_y = raw.y; 
            break;
        case 3: // levý dolní
            min_x = raw.x;
            max_y = max(max_y, raw.y);
            break;
    }

    delay(1000);

    return Point{raw.x, raw.y};
}

void xpt::autoCalibrate()
{
    spiTouch();
    // levý horní
    while (!_touch.touched()) delay(10);
    calibrate(0);
    tone(ZVUK, 2000, 100);
    while (_touch.touched()) delay(10);

    // pravý horní
    while (!_touch.touched()) delay(10);
    calibrate(1);
    tone(ZVUK, 2000, 100);
    while (_touch.touched()) delay(10);

    // pravý dolní
    while (!_touch.touched()) delay(10);
    calibrate(2);
    tone(ZVUK, 2000, 100);
    while (_touch.touched()) delay(10);

    // levý dolní
    while (!_touch.touched()) delay(10);
    calibrate(3);
    tone(ZVUK, 2000, 100);
    while (_touch.touched()) delay(10);

    int y1 = min_y;
    min_y = max_y;
    max_y = y1;

    sd.writeSetting("xmin", min_x);
    sd.writeSetting("xmax", max_x);
    sd.writeSetting("ymin", min_y);
    sd.writeSetting("ymax", max_y);
}

void xpt::setCalibration()
{
    min_x = sd.readSetting("xmin",1000);
    max_x = sd.readSetting("xmax");
    min_y = sd.readSetting("ymin");
    max_y = sd.readSetting("ymax");
}

int xpt::touchedMs(uint32_t limitMs)
{
    spiTouch();
    // čekáme krátce, jestli vůbec dojde ke stisku
    uint32_t waitStart = millis();
    if (!_touch.touched())
    {
        return -1;
    }

    // stisk začal
    uint32_t start = millis();

    // měření držení
    while (_touch.touched())
    {
        uint32_t elapsed = millis() - start;

        if (elapsed >= limitMs)
        {
            // limit dosažen – počkej na uvolnění
            while (_touch.touched())
                delay(1);

            return limitMs;
        }

        delay(1);
    }

    spiAllHigh();

    // tlačítko puštěno dřív než limit
    return millis() - start;
}

