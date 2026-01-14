#include "I2C.h"

void OV7670_I2C::init(uint32_t clock)
{
    _wire.begin();
    _wire.setClock(clock);
}

void OV7670_I2C::scan(Stream& out)
{
    out.println("=== I2C SCAN ===");
    uint8_t found = 0;
    
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        _wire.beginTransmission(addr);
        if (_wire.endTransmission() == 0)
        {
            out.print("Found device at 0x");
            out.println(addr, HEX);
            found++;
        }
    }
    
    if (!found)
        out.println("NO I2C DEVICES FOUND");
    
    out.println("================");
}

bool OV7670_I2C::writeRegister(uint8_t address, uint8_t reg, uint8_t val)
{
    _wire.beginTransmission(address);
    _wire.write(reg);
    _wire.write(val);
    uint8_t result = _wire.endTransmission();
    delay(1);  // Malá pauza pro OV7670
    return (result == 0);
}

uint8_t OV7670_I2C::readRegister(uint8_t reg)
{
    _wire.beginTransmission(_addr);
    _wire.write(reg);
    
    if (_wire.endTransmission() != 0)
        return 0xFF;
    
    delayMicroseconds(50);
    
    _wire.requestFrom(_addr, (uint8_t)1);
    if (_wire.available())
        return _wire.read();
    
    return 0xFF;
}