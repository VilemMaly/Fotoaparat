#include "slider.h"

slider::slider(
    TFT_eSPI& tftref,
    xpt& touch,
    int width,
    int height,
    int color,
    int x,
    int y,
    String text,
    uint8_t textSize
)
    : tft(tftref),
      _touch(touch),
      w(width),
      h(height),
      bgColor(color),
      posX(x),
      posY(y),
      label(text),
      textSize(textSize),
      hodnota(0)
{
}

int slider::checkSlider()
{
    if (!_touch.touched())
        return hodnota;

    xpt::Point p = _touch.touchedWhere();

    // kontrola, zda je dotyk uvnitř slideru
    if (p.x >= posX && p.x <= (posX + w) &&
        p.y >= posY && p.y <= (posY + h))
    {
        // přepočet X → hodnota 0–100
        hodnota = map(p.x, posX, posX + w, 0, 100);

        // ochrana rozsahu
        if (hodnota < 0) hodnota = 0;
        if (hodnota > 100) hodnota = 100;

        draw();
    }

    return hodnota;
}

void slider::draw()
{
    // pozadí
    tft.fillRect(posX, posY, w, h, bgColor);

    // rámeček
    tft.drawRect(posX, posY, w, h, TFT_WHITE);

    // výplň podle hodnoty
    int fillWidth = map(hodnota, 0, 100, 0, w);
    tft.fillRect(posX, posY, fillWidth, h, TFT_GREEN);

    // text
    tft.setTextColor(TFT_WHITE, bgColor);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(textSize);
    tft.drawString(label + ": " + String(hodnota),
                   posX + w / 2,
                   posY + h / 2);
}
