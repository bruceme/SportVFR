#ifdef AoA // needs an accelleration source (not on the GY-652)

const float vs = 42;        // stall speed
const float va = 116;       // maneuvering speed
const float gmax_10 = 6.0f; // Maximum G's in tenths (ie.. 3.8 is 38)

#define aFactor 8       // this helps to keep a reasonable integer number
#define bFactor 256     // same for b
#define scaleFactor 256 // scales the output so we can use faster integer math... this correlates to the duration of the beep

const long a = (int)((aFactor * vs * va * va - vs * vs * va) / (gmax_10 * vs - va));
const long b = (int)(bFactor / vs - bFactor * a * vs);
const int stallMarginStartBeep = scaleFactor * 2 / 3;

#define BeepFrequency 2794
#define AUDIO_OUT_PIN 10

void AoABeep()
{
  int vel = 60;

  int accellOfStall =  (scaleFactor * aFactor * vel * vel / a) + (scaleFactor * bFactor / vel));

  int stallMargin = accellOfStall / accelleration; // (in range 0-scaleFactor)

  if (stallMargin > stallMarginStartBeep)
    tone(AUDIO_OUT_PIN, BeepFrequency, stallMargin);
}
#endif // AoA