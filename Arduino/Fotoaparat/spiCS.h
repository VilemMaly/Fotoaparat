#ifndef SPI_CS_CONTROL_H
#define SPI_CS_CONTROL_H

#include "piny.h"

#define tftHIGH() GPIOB->BSRR = 0b1 << 10;
#define tftLOW() GPIOB->BRR = 0b1 << 10;

#define sdHIGH() GPIOA->BSRR = 0b1 << 11;
#define sdLOW() GPIOA->BRR = 0b1 << 11;

#define touchHIGH() GPIOB->BSRR = 0b1;
#define touchLOW() GPIOB->BRR = 0b1;

// Funkce pro kontrolu CS pinů pomocí Arduino API

inline void spiAllHigh() {
    // Nastaví všechny CS piny do HIGH (neaktivní)
    tftHIGH();
    sdHIGH();
    touchHIGH();
    /*
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    digitalWrite(TOUCH_CS, HIGH);*/
}

inline void spiTFT() {
    // Nastaví TFT_CS na LOW (aktivní), ostatní HIGH
    tftLOW();
    sdHIGH();
    touchHIGH();
    /*
    digitalWrite(SD_CS, HIGH);
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(TFT_CS, LOW);
    */
}

inline void spiSD() {
    // Nastaví SD_CS na LOW (aktivní), ostatní HIGH
    tftHIGH();
    sdLOW();
    touchHIGH();
    /*
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(SD_CS, LOW);*/
}

inline void spiTouch() {
    // Nastaví TOUCH_CS na LOW (aktivní), ostatní HIGH
    tftHIGH();
    sdHIGH();
    touchLOW();
    /*
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    digitalWrite(TOUCH_CS, LOW);*/
}

// Optimovaná verze s předpokladem, že piny jsou na stejném portu
// (rychlejší, ale závislá na konkrétní desce)
#ifdef USE_FAST_CS_CONTROL
inline void spiAllHighFast() {
    // Atomické nastavení pinů - vyžaduje, aby všechny CS piny byly na stejném portu
    uint8_t mask = digitalPinToBitMask(TFT_CS) | digitalPinToBitMask(SD_CS) | digitalPinToBitMask(TOUCH_CS);
    *portOutputRegister(digitalPinToPort(TFT_CS)) |= mask;
}

inline void spiTFTFast() {
    uint8_t tft_mask = digitalPinToBitMask(TFT_CS);
    uint8_t other_mask = digitalPinToBitMask(SD_CS) | digitalPinToBitMask(TOUCH_CS);
    volatile uint8_t* port = portOutputRegister(digitalPinToPort(TFT_CS));
    
    *port &= ~tft_mask;  // TFT_CS LOW
    *port |= other_mask; // Ostatní HIGH
}

inline void spiSDFast() {
    uint8_t sd_mask = digitalPinToBitMask(SD_CS);
    uint8_t other_mask = digitalPinToBitMask(TFT_CS) | digitalPinToBitMask(TOUCH_CS);
    volatile uint8_t* port = portOutputRegister(digitalPinToPort(SD_CS));
    
    *port &= ~sd_mask;   // SD_CS LOW
    *port |= other_mask; // Ostatní HIGH
}

inline void spiTouchFast() {
    uint8_t touch_mask = digitalPinToBitMask(TOUCH_CS);
    uint8_t other_mask = digitalPinToBitMask(TFT_CS) | digitalPinToBitMask(SD_CS);
    volatile uint8_t* port = portOutputRegister(digitalPinToPort(TOUCH_CS));
    
    *port &= ~touch_mask; // TOUCH_CS LOW
    *port |= other_mask;  // Ostatní HIGH
}
#endif

#endif // SPI_CS_CONTROL_H