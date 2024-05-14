#include <Arduino.h>

// Resistors: 510 = 51R per row
// negative side: (columns), PB1 | PD0 | PD1 | PD6 | PD3 | PB0 | PC0 | PC1
// positive side: (rows),    PB7 | PB2 | PB6 | PD5 | PC2 | PD4 | PC3 | PD2

// refresh rate set: 120Hz
// line rate: 120Hz * 8 = 960Hz
// pwm dimming rate? 1/10 of line time to 1/100 of line time
// => on time: 960Hz * 10 = 9600Hz - 96000Hz
//
// max pwr per pin: 40mA
// max continous 1 led cur: 33mA
// max continous 8 led cur: 264mA => dimming of at least 6.6 to not exceed current!
//
// clock is 8MHz
// each tick is 0.125us

// five | ten | a quarter | twenty | one | two | three | four | five | six | seven | eight | nine | ten | eleven | twelve
uint16_t currentTime = 0x0008;
// half | 0 | past | to | a | led top | led side top | led side bot
uint8_t signal_leds = B10100000;

/*
ONE:
R8: B01000011
TWO:
R7: B11000000
R8: B01000000
THREE:
R6: B00011111
FOUR:
R8: B11110000
FIVE:
R5: B11110000
SIX:
R6: B11100000
SEVEN:
R6: B10000000
R7: B00001111
EIGHT:
R5: B00011111
NINE:
R8: B00001111
TEN:
R5: B00000001
R6: B00000001
R7: B00000001
ELEVEN:
R7: B00111111
TWELVE:
R7: B11110110

-- minutes
FIVE:
R2: B11110000
TEN:
R1: B00101100
QUARTER:
R1: B10000000
R3: B11111110
TWENTY:
R1: B00111111
HALF:
R2: B00001111
A:
R1: B10000000
PAST:
R4: B11110000
TO:
R4: B00000110
*/

/*const byte lineMatrix[] = {
    B00000010,
    B00000001,
    B00000010,
    B01000000,
    B00001000,
    B00000001,
    B00000001,
    B00000010,
};
const uint8_t linePorts[] = {
    _SFR_IO8(0x05),
};*/
/*PORTB |= B00000001; // col 6 PB0
  PORTB |= B00000010; // col 1 PB1
  PORTD |= B00000001; // col 2 PD0
  PORTD |= B00000010; // col 3 PD1
  PORTD |= B00001000; // col 5 PD3
  PORTD |= B01000000; // col 4 PD6
  PORTC |= B00000001; // col 7 PC0
  PORTC |= B00000010; // col 8 PC1*/

/*
 */
void setup()
{
  // put your setup code here, to run once:
  DDRB = B11000111; // PD0,1,2,6,7
  DDRC = B00001111; // PD0,1,2,3
  DDRD = B01111111; // PD0,1,2,3,4,5,6

  // set all cols high!
  PORTB |= B00000011;
  PORTC |= B00000011;
  PORTD |= B01001011;
  // PORTB &= ~B00000010; // set first col enabled.
}

inline void turnOnLine(byte rowNr)
{
  switch (rowNr)
  {
  case 0:
    PORTB |= B10000000; // row 1 PB7
    return;
  case 1:
    PORTB |= B00000100; // row 2 PB2
    return;
  case 2:
    PORTB |= B01000000; // row 3 PB6
    return;
  case 3:
    PORTD |= B00100000; // row 4 PD5
    return;
  case 4:
    PORTC |= B00000100; // row 5 PC2
    return;
  case 5:
    PORTD |= B00010000; // row 6 PD4
    return;
  case 6:
    PORTC |= B00001000; // row 7 PC3
    return;
  case 7:
    PORTD |= B10000100; // row 8 PD2
    return;
  default:
    return;
  }
}

inline void turnOffLine(byte rowNr)
{
  switch (rowNr)
  {
  case 0:
    PORTB &= ~B10000000; // row 1 PB7
    return;
  case 1:
    PORTB &= ~B00000100; // row 2 PB2
    return;
  case 2:
    PORTB &= ~B01000000; // row 3 PB6
    return;
  case 3:
    PORTD &= ~B00100000; // row 4 PD5
    return;
  case 4:
    PORTC &= ~B00000100; // row 5 PC2
    return;
  case 5:
    PORTD &= ~B00010000; // row 6 PD4
    return;
  case 6:
    PORTC &= ~B00001000; // row 7 PC3
    return;
  case 7:
    PORTD &= ~B10000100; // row 8 PD2
    return;
  default:
    return;
  }
}

/// @brief
/// @param rowNr make sure, range is only between 0 and 7!
/// @param line bitmask for line. highest bit is most left, lowest bit is most right
void writeLine(byte rowNr, byte line)
{
  // turn off all previous columns
  PORTB |= B00000011;
  PORTC |= B00000011;
  PORTD |= B01001011;

  // turn off previous row
  if (rowNr == 0) // hacky workaround. stop using if statement here.
    turnOffLine(7);
  else
    turnOffLine((rowNr - 1));

  //  turn on columns according to byte line
  if (line & B10000000)
    PORTB &= ~B00000010; // col 1 PB1
  if (line & B01000000)
    PORTD &= ~B00000001; // col 2 PD0
  if (line & B00100000)
    PORTD &= ~B00000010; // col 3 PD1
  if (line & B00010000)
    PORTD &= ~B01000000; // col 4 PD6
  if (line & B00001000)
    PORTD &= ~B00001000; // col 5 PD3
  if (line & B00000100)
    PORTB &= ~B00000001; // col 6 PB0
  if (line & B00000010)
    PORTC &= ~B00000001; // col 7 PC0
  if (line & B00000001)
    PORTC &= ~B00000010; // col 8 PC1

  // turn on current row
  turnOnLine(rowNr);
}

/// @brief Abstract
/// @param time
/// @param linemask
/// @return
byte timeToMask(byte time, byte &linemask)
{
  return 1;
}

void writeScreen()
{
  for (byte i = 0; i < 8; i++)
  {
    byte line = 0x0;
    switch (i)
    {
    case 0:
      /*
      TEN:
  R1: B00101100
  QUARTER:
  R1: B10000000
  R3: B11111110
  TWENTY:
  R1: B00111111
  A:
  R1: B10000000
  */
      // five | ten | a quarter | twenty | one | two | three | four | five | six | seven | eight | nine | ten | eleven | twelve
      // currentTime&B0100 0000 0000 0000 0x4000 ten
      // currentTime&B0010 0000 0000 0000 0x2000 a quarter (first part)
      // currentTime&B0001 0000 0000 0000 0x1000 twenty
      // half | 0 | past | to | a | led top | led side top | led side bot
      // signal_leds&B00001000 0x08 a
      // signal_leds&B00000100 0x04 led top
      if ((currentTime & 0x4000) == 0x4000) // ten
        line |= B00101100;
      if ((currentTime & 0x2000) == 0x2000) // a (quarter)
        line |= B10000000;
      if ((currentTime & 0x1000) == 0x1000) // twenty
        line |= B00111111;
      if ((signal_leds & 0x08) == 0x08) // a
        line |= B10000000;
      if ((signal_leds & 0x04) == 0x04) // led top
        line |= B01000000;
      break;
    case 1:
      /*
      FIVE:
  R2: B11110000
  HALF:
R2: B00001111
      */
      // currentTime&B1000 0000 0000 0000 0x8000 five
      // signal_leds&B10000000 0x80 half
      if ((currentTime & 0x8000) == 0x8000) // five
        line |= B11110000;
      if ((signal_leds & 0x80) == 0x80) // half
        line |= B00001111;
      break;
    case 2:
      /*
      QUARTER:
R1: B10000000
R3: B11111110
       */
      // currenTime&B0010 0000 0000 0000 0x2000 a quarter (second part)
      // signal_leds&B00000010 0x02 led side top
      if ((currentTime & 0x2000) == 0x2000) // (a) quarter
        line |= B11111110;
      // if ((signal_leds & 0x02) == 0x02) // led side top
      //   line |= B00000001;
      break;
    case 3:
      /*
      PAST:
      R4: B11110000
      TO:
      R4: B00000110
      */
      // signal_leds&B00100000 0x20 past
      // signal_leds&B00010000 0x10 to
      // signal_leds&B00000001 0x01 led side bot
      if ((signal_leds & 0x20) == 0x20) // past
        line |= B11110000;
      if ((signal_leds & 0x10) == 0x10) // to
        line |= B00000110;
      // if ((signal_leds & 0x01) == 0x01) // led side bot
      //   line |= B00000001;
      break;
    case 4:
      /*FIVE:
  R5: B11110000
  EIGHT:
  R5: B00011111
  TEN:
  R5: B00000001
  R6: B00000001
  R7: B00000001*/
      // five | ten | a quarter | twenty | one | two | three | four | five | six | seven | eight | nine | ten | eleven | twelve
      // currentTime&B0000 0000 1000 0000 0x0080 five
      // currentTime&B0000 0000 0001 0000 0x0010 eight
      // currentTime&B0000 0000 0000 0100 0x0004 ten (first part)
      if ((currentTime & 0x0080) == 0x0080) // five
        line |= B11110000;
      if ((currentTime & 0x0010) == 0x0010) // eight
        line |= B00011111;
      if ((currentTime & 0x0004) == 0x0004) // t(en)
        line |= B00000001;
      break;
    case 5:
      /*
      THREE:
R6: B00011111
SIX:
R6: B11100000
SEVEN:
R6: B10000000
        TEN:
    R5: B00000001
    R6: B00000001
    R7: B00000001
      */
      // currentTime&B0000 0010 0000 0000 0x0200 three
      // currentTime&B0000 0000 0100 0000 0x0040 six
      // currentTime&B0000 0000 0010 0000 0x0020 seven (first part)
      // currentTime&B0000 0000 0000 0100 0x0004 ten (second part)
      if ((currentTime & 0x0200) == 0x0200) // five
        line |= B00011111;
      if ((currentTime & 0x0040) == 0x0040) // six
        line |= B11100000;
      if ((currentTime & 0x0020) == 0x0020) // s(even)
        line |= B10000000;
      if ((currentTime & 0x0004) == 0x0004) // (t)e(n)
        line |= B00000001;
      break;
    case 6:
      /*
      TWO:
R7: B11000000
R8: B01000000
SEVEN:
R6: B10000000
R7: B00001111
TEN:
R5: B00000001
R6: B00000001
R7: B00000001
ELEVEN:
R7: B00111111
TWELVE:
R7: B11110110
       */
      // five | ten | a quarter | twenty | one | two | three | four | five | six | seven | eight | nine | ten | eleven | twelve
      // currentTime&B0000 0100 0000 0000 0x0400 two (first part)
      // currentTime&B0000 0000 0010 0000 0x0020 seven (second part)
      // currentTime&B0000 0000 0000 0100 0x0004 ten (thrid part)
      // currentTime&B0000 0000 0000 0010 0x0002 eleven
      // currentTime&B0000 0000 0000 0001 0x0001 twelve
      if ((currentTime & 0x0400) == 0x0400) // tw(o)
        line |= B11000000;
      if ((currentTime & 0x0020) == 0x0020) // (s)even
        line |= B00001111;
      if ((currentTime & 0x0004) == 0x0004) // (te)n
        line |= B00000001;
      if ((currentTime & 0x0002) == 0x0002) // eleven
        line |= B00111111;
      if ((currentTime & 0x0001) == 0x0001) // twelve
        line |= B11110110;
      break;
    case 7:
      /*
ONE:
R8: B01000011
TWO:
R7: B11000000
R8: B01000000
FOUR:
R8: B11110000
NINE:
R8: B00001111
      */
      // currentTime&B0000 1000 0000 0000 0x0800 one
      // currentTime&B0000 0100 0000 0000 0x0400 two (second part)
      // currentTime&B0000 0001 0000 0000 0x0100 four
      // currentTime&B0000 0000 0000 1000 0x0008 nine
      if ((currentTime & 0x0800) == 0x0800) // one
        line |= B01000011;
      if ((currentTime & 0x0400) == 0x0400) // (tw)o
        line |= B01000000;
      if ((currentTime & 0x0100) == 0x0100) // four
        line |= B11110000;
      if ((currentTime & 0x0008) == 0x0008) // nine
        line |= B00001111;
      break;
    default:
      return;
    }
    writeLine(i, line);
    delayMicroseconds(20);
  }
}
void off()
{
  // turn off (on) all cols
  PORTB |= B00000011;
  PORTC |= B00000011;
  PORTD |= B01001011;

  PORTB &= ~B11000100; // PB7, PB6, PB2
  PORTC &= ~B00001100; // PC3, PC2
  PORTD &= ~B00110100; // PD5, PD4, PD2
}

// #define CHECK_LIGHT

byte row = 0;
unsigned long lastUpdate = 0;
void loop()
{
#ifdef CHECK_LIGHT
  writeLine(row, 1 << row);
  row++;
  row %= 8;
  delay(1);
#else
  writeScreen();
  off();
  delay(2);
  if (lastUpdate + 2000 < millis())
  {
    lastUpdate = millis();
    signal_leds ^= B00000100;
  }
#endif /*CHECK_LIGHT*/
}