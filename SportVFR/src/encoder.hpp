//                           _______         _______       
//           A    Pin1 ______|       |_______|       |______ Pin1
// negative <---         _______         _______         __      --> positive
//           B    Pin2 __|       |_______|       |_______|   Pin2


byte a=1,b=1;
int encoder;

#define EncoderPinA 7   // 0 on the PCB
#define EncoderPinB 0   // 1 on the PCB

void A_Change()
{
  a = digitalRead(EncoderPinA);
  if (a ^ b)
    encoder++;

  if (!a ^ b)
    encoder--;
}
void B_Change()
{
  b = digitalRead(EncoderPinB);
  if (a ^ !b)
    encoder++;
  if (a ^ b)
    encoder--;
}

void InitializeEncoder()
{
    pinMode(EncoderPinA, INPUT_PULLUP);
    pinMode(EncoderPinB, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(EncoderPinA),A_Change,CHANGE);
    attachInterrupt(digitalPinToInterrupt(EncoderPinB),B_Change,CHANGE);
}