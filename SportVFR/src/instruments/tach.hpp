SimpleKalmanFilter tachFilter(2, 2, 0.01);

void tachInt()
{
  revolutions++;
}

#define TachPin 7       // 7 on the PCB
#define PulsesPerRev 2 // set by magneto mfg
void attachTachInt()
{
    pinMode(TachPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TachPin), tachInt, FALLING);
}

int ReadTachometer()
{
  //detachInterrupt(tachIntNum); // Disable interrupt when calculating
  int tachT = (int)tachFilter.updateEstimate(revolutions * 60 / (PulsesPerRev * (now - lastmillis)));
  revolutions = 0;
  attachTachInt();
  return tachT ;
}
