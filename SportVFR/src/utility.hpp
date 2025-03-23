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
