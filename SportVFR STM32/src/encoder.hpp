// //                           _______         _______       
// //           A    Pin1 ______|       |_______|       |______ Pin1
// // negative <---         _______         _______         __      --> positive
// //           B    Pin2 __|       |_______|       |_______|   Pin2


// byte a=1,b=1;
// volatile int encoder = 0;

// #define EncoderPinA PA8   // 0 on the PCB
// #define EncoderPinB PA9   // 1 on the PCB

// void A_Change()
// {
//   a = digitalRead(EncoderPinA);
//   if (a ^ b)
//     encoder--;

//   if (!a ^ b)
//     encoder++;
// }
// void B_Change()
// {
//   b = digitalRead(EncoderPinB);
//   if (a ^ !b)
//     encoder--;
//   if (a ^ b)
//     encoder++;
// }

// void InitializeEncoder()
// {
//     pinMode(EncoderPinA, INPUT_PULLUP);
//     pinMode(EncoderPinB, INPUT_PULLUP);
//     attachInterrupt(digitalPinToInterrupt(EncoderPinA),A_Change,CHANGE);
//     attachInterrupt(digitalPinToInterrupt(EncoderPinB),B_Change,CHANGE);
// }
#pragma once

#include <Arduino.h>

#define ENCODER_PIN_A PA8
#define ENCODER_PIN_B PA9


volatile int32_t encoder = 0;
volatile bool lastA = 0;

void handleEncoder() {

  bool A = digitalRead(ENCODER_PIN_A);
  bool B = digitalRead(ENCODER_PIN_B);

  if (A != lastA) {
    if (A == B) {
      encoder++;
    } else {
      encoder--;
    }
    lastA = A;
  }
}

#define BUTTON_PIN PA10  // Replace with your button pin
#define DEBOUNCE_MS 50  // Debounce time in milliseconds

volatile bool buttonPressed = false;
volatile uint32_t lastInterruptTime = 0;

bool isButtonPressed()
{
  return digitalRead(BUTTON_PIN) == LOW;
}

bool buttonReleased() {
  auto buttSav = buttonPressed;
  buttonPressed = false;
  return buttSav;
}

void handleButtonInterrupt() {
  uint32_t currentTime = millis();
  if (currentTime - lastInterruptTime > DEBOUNCE_MS) {
    buttonPressed = isButtonPressed();
    lastInterruptTime = currentTime;
  }
}

void setupEncoders()
{
  pinMode(ENCODER_PIN_A, INPUT_PULLUP); // Channel A (CLK)
  pinMode(ENCODER_PIN_B, INPUT_PULLUP); // Channel B (DT)

  // Enable AFIO clock and remap EXTI for PA8
  AFIO->EXTICR[2] &= ~(AFIO_EXTICR3_EXTI8); // Clear EXTI8 bits
  AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI8_PA; // Map EXTI8 to PA8

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), handleEncoder, CHANGE);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Use pull-up resistor
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
}


