#pragma once
#define INT_DIGITS 5

char *my_itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;  /* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {      /* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

// to_hex12(0xABCDE)   // "CDE"
// to_hex12(0x25)      // "025"
// to_hex12(4095)      // "FFF"
const char *to_hex12(uint32_t value)
{
    static char buf[4];          // 3 hex chars + null
    uint32_t v = value & 0xFFF;  // keep only 12 bits

    for (int i = 2; i >= 0; i--) {
        uint32_t nib = v & 0xF;          // extract lowest nibble
        buf[i] = (nib < 10) ? ('0' + nib)
                            : ('A' + (nib - 10));
        v >>= 4;                         // shift to next nibble
    }

    buf[3] = '\0';
    return buf;
}

int atan2deg(int y, int x)
{
    int maxT=max(y, x);
    if (maxT > 60)   // sqrt(2^31/3667)... overflows an internal value
    {
        x = 60 * x / maxT;
        y = 60 * y / maxT; 
    }

    if (y == 0)
        return (x >= 0) ? 360 : 180;

    if (x == 0)
        return (y > 0) ? 90 : 270;

    /* inner atan function */
    int aX = abs(x);
    int aY = abs(y);
    
    int angle = (aY <= aX) ? (3667 * aX * aY) / (64 * aX * aX + 17 * aY * aY) :
        90 - (3667 * aX * aY) / (64 * aY * aY + 17 * aX * aX);

    angle = (x < 0) ? 
        (y < 0) ? 180 + angle : 180 - angle :
        (y < 0) ? 360 - angle : angle;

    if (0 == angle)
        angle = 360;

    return angle;
}


char *Format(int value, int width, char pad)
{
  char *val = my_itoa(value);
  for(short i = width - strlen(val); i > 0; i--)
    *--val = pad;
  return val;
}


// byte Level[8]{
//     B00000,
//     B00000,
//     B00000,
//     B11111,
//     B11111,
//     B00000,
//     B00000,
//     B00000};

// byte UpArrow1[8]{
//     B00000,
//     B00000,
//     B00100,
//     B01110,
//     B11011,
//     B10001,
//     B00000,
//     B00000};

// byte UpArrow2[8]{
//     0b00100,
//     0b01110,
//     0b11011,
//     0b10001,
//     0b00100,
//     0b01110,
//     0b11011,
//     0b10001};

// const byte DownArrow1[8]{
//     0b00000,
//     0b00000,
//     0b10001,
//     0b11011,
//     0b01110,
//     0b00100,
//     0b00000,
//     0b00000};

// const byte DownArrow2[8]{
//     0b10001,
//     0b11011,
//     0b01110,
//     0b00100,
//     0b10001,
//     0b11011,
//     0b01110,
//     0b00100};

// const byte Knots[8]{
//       0b10010,
//       0b10100,
//       0b11000,
//       0b10100,
//       0b10010,
//       0b00111,
//       0b00010,
//       0b00010};
    
// const byte GPH[8]{
//       0b01100,
//       0b10000,
//       0b10100,
//       0b01100,
//       0b00000,
//       0b00101,
//       0b00111,
//       0b00101};

#define ChCustomerChar0 0
#define ChDownArrow1 ChCustomerChar0
#define ChDownArrow2 ChCustomerChar0 + 1
#define ChLevel    ChCustomerChar0 + 2
#define ChUpArrow2 ChCustomerChar0 + 3
#define ChUpArrow1 ChCustomerChar0 + 4

void SetupLCDCustomCharacters()
{
  // lcd.createChar(ChLevel, (char *)Level);
  // lcd.createChar(ChUpArrow1, (char *)UpArrow1);
  // lcd.createChar(ChUpArrow2, (char *)UpArrow2);
  // lcd.createChar(ChDownArrow1, (char *)DownArrow1);
  // lcd.createChar(ChDownArrow2, (char *)DownArrow2);
}