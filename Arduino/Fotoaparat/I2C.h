#pragma once
#include <Arduino.h>
#include <Wire.h>

class OV7670_I2C
{
public:
    /* ======== Datové typy ======== */
    struct RegVal {
        uint8_t reg;
        uint8_t val;
    };

    /* ======== Konstruktor - vytvoří vlastní TwoWire ======== */
    OV7670_I2C(uint8_t sda_pin, uint8_t scl_pin, uint8_t addr7 = 0x21)
        : _wire(sda_pin, scl_pin), _addr(addr7)
    {}

    /* ======== Inicializace ======== */
    void init(uint32_t clock = 100000);

    /* ======== I2C scan ======== */
    void scan(Stream& out);

    /* ======== Zápis registru ======== */
    bool writeRegister(uint8_t address, uint8_t reg, uint8_t val);
    
    /* ======== Čtení registru - s výchozí adresou ======== */
    uint8_t readRegister(uint8_t reg);
    
private:
    TwoWire _wire;   // Vlastní objekt (ne reference!)
    uint8_t _addr;
};