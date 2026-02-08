#include "Arduino.h"

#define RCK_HIGH() (GPIOB->BSRR = (1 << 14))
#define RCK_LOW()  (GPIOB->BSRR = (1 << (14 + 16)))


template<int RRST, int WRST, int RCK, int WR, int D0, int D1, int D2, int D3, int D4, int D5, int D6, int D7>
class Fifo
{
  public:

  
  
  static void inline init()
  {
    pinMode(RRST, OUTPUT);
    pinMode(RCK, OUTPUT);
    pinMode(WRST, OUTPUT);
    pinMode(WR, OUTPUT);
    digitalWrite(RRST, 1);
    digitalWrite(WRST, 1);
    digitalWrite(WR, 1);
    pinMode(D0, INPUT);
    pinMode(D1, INPUT);
    pinMode(D2, INPUT);
    pinMode(D3, INPUT);
    pinMode(D4, INPUT);
    pinMode(D5, INPUT);
    pinMode(D6, INPUT);
    pinMode(D7, INPUT);
  }

  static void inline readReset()
  {
      digitalWrite(RRST, 0);
      delayMicroseconds(1);
      digitalWrite(RCK, 0);
      delayMicroseconds(1);
      digitalWrite(RCK, 1);
      delayMicroseconds(1);
      digitalWrite(RRST, 1);  
  }

  static void inline writeEnable()
  {
      //Set A2, A12, A13 (HIGH)
      GPIOA->BRR = 0b0000000000000000 | (1 << WR); //lower 16 bits
      //digitalWrite(WR, 0);
      delayMicroseconds(5);
  }
  
  static void inline writeDisable()
  {
      GPIOA->BSRR = 0b0000000000000000 | (1 << WR); //lower 16 bits
      //digitalWrite(WR, 1);
      delayMicroseconds(5);
  }

  static void inline writeReset()
  {
      GPIOA->BRR = 0b0000000000000000 | (1 << 9); //lower 16 bits
      //digitalWrite(WRST, 0);
      delayMicroseconds(1);
      GPIOA->BSRR = 0b0000000000000000 | (1 << 9); //lower 16 bits
      
      //digitalWrite(WRST, 1);
  }

static inline uint8_t readByte()
{
  RCK_HIGH();

  uint32_t a = GPIOA->IDR;
  uint32_t b = GPIOB->IDR;

  uint8_t v =
    ((b >> 13) & 1) |       // D0 PB13
    ((a >> 12) & 1) << 1 |  // D1 PA12
    ((b >> 12) & 1) << 2 |  // D2 PB12
    ((a >> 15) & 1) << 3 |  // D3 PA15
    ((b >>  9) & 1) << 4 |  // D4 PB9
    ((b >>  3) & 1) << 5 |  // D5 PB3
    ((b >>  8) & 1) << 6 |  // D6 PB8
    ((b >>  4) & 1) << 7;   // D7 PB4

  RCK_LOW();
  return v;
}



  static void inline skipByte()
  {
      digitalWrite(RCK, 1);
      digitalWrite(RCK, 1); //just for the delay
      digitalWrite(RCK, 0);
      digitalWrite(RCK, 0); //needed if more skips in sequence
  }
  static void inline readBytes(unsigned char *buffer, int count)
  {
    for(int i = 0; i < count; i++)
      buffer[i] = readByte();
  }
};

