#ifndef LoRa_h
#define LoRa_h

#include <avr/pgmspace.h>

#include <Arduino.h>

class LoRa
{


  private:
    //    int red = A3; //this sets the red led pin
    //    int green = A2; //this sets the green led pin
    //    int blue = A0; //this sets the blue led pin


    void LoRaBlink()
    {
      pinMode(13, OUTPUT);
      digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
      delay(100);
      digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);              // wait for a second


      //      analogWrite(red, 0);
      //      analogWrite(blue, 255);
      //      analogWrite(green, 255);
      //      delay(50);
      //      analogWrite(red, 255);
      //      analogWrite(blue, 255);
      //      analogWrite(green, 255);
      //      delay(50);
    }

    void LoRaBlinkOff()
    {
      //      analogWrite(red, 255);
      //      analogWrite(blue, 0);
      //      analogWrite(green, 0);
      digitalWrite(13, LOW);
    }



  public:
    void LoRaConfig();
    void LoRaSendAndReceive(String message);
};
#endif
