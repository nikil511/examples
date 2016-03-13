#include <rn2483.h>

/*
 * Author: JP Meijers
 * Date: 2016-02-07
 * 
 * Transmit a one byte packet via TTN. This happens as fast as possible, while still keeping to 
 * the 1% duty cycle rules enforced by the RN2483's built in LoRaWAN stack. Even though this is 
 * allowed by the radio regulations of the 868MHz band, the fair use policy of TTN may prohibit this.
 * 
 * CHECK THE RULES BEFORE USING THIS PROGRAM!
 * 
 * CHANGE ADDRESS!
 * Change the address this device should use to either a testing address or one from a range you "own".
 * The appropriate line is "myLora.initTTN("XXXXXX");
 * 
 * Connect the RN2483 as follows:
 * RN2483 -- Arduino
 * Uart TX -- 10
 * Uart RX -- 11
 * Reset -- 12
 * Vcc -- 3.3V
 * Gnd -- Gnd
 * 
 * If you use an Arduino with a free hardware serial port, you can replace 
 * the line "rn2483 myLora(mySerial);"
 * with     "rn2483 myLora(SerialX);"
 * where the parameter is the serial port the RN2483 is connected to.
 * Remember that the serial port should be initialised before calling initTTN().
 * For best performance the serial port should be set to 57600 baud, which is impossible with a software serial port.
 * If you use 57600 baud, you can remove the line "myLora.autobaud();".
 * 
 */
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

String str;

unsigned long time = 0;

//create an instance of the rn2483 library, 
//giving the software UART as stream to use,
//and using LoRa WAN
rn2483 myLora(mySerial);

// the setup routine runs once when you press reset:
void setup() {
  //output LED pin
  pinMode(13, OUTPUT);
  led_on();
  
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  mySerial.begin(9600);
  Serial.println("Startup");

  //reset rn2483
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  //initialise the rn2483 module
  myLora.autobaud();
  myLora.initTTN("XXXXXX");

  //transmit a startup message
  myLora.txUncnf("Fast TXer (TTN)");

  led_off();
  delay(2000);
}

// the loop routine runs over and over again forever:
void loop() {
    
    led_on();

    Serial.println("TXing");
    myLora.txUncnf("!"); //one byte, blocking function
    
    led_off();
    delay(200);
}

void led_on()
{
  digitalWrite(13, 1);
}

void led_off()
{
  digitalWrite(13, 0);
}
