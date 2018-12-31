/*
  Stroboscope for the Arduino
  
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

*/

#define VERSION "Stroboscope V0.2"

#define DEBUGGING true

#include <LiquidCrystal.h>

#include "TimerOne.h"

#define LED_PIN 13

#define TIMER_US 1000                         // 1mS set timer duration in microseconds 
#define TICK_COUNTS 1000                      // one second worth of timer ticks
#define ON_TICKS 2

/*
 * Cycles per Second
 * 
 *  1   1000
 *  2   500
 *  3   333
 *  4   250
 *  5   200
 *  
 *  10  100
 *  20  50
 *  50  20
 *  100 10
 */

volatile long tick_count = TICK_COUNTS;         // Counter for one second
volatile bool led_on = false;
volatile long on_tick = 0;
volatile long current_tick_count = TICK_COUNTS;

// Rotary Encoder

#define encoder0PinA 6
#define encoder0PinB 7
#define encoder0Btn  8

float counter = 1.0; 
float encoderIncrement = 1.0;
int aState;
int aLastState;
int btnState;
int btnLastState; 
bool oddEncoder = false;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print(VERSION);

  Serial.begin(115200);
  delay(500);
#if DEBUGGING
  Serial.println("Stroboscope V0.1");
  Serial.println("Enter cycles per second: ");
  Serial.print("cps=");
  Serial.println((float)(TIMER_US/tick_count));
#endif

  pinMode(LED_PIN, OUTPUT);
  Timer1.initialize(TIMER_US); // initialize timer1, and set a 10ms period
  Timer1.attachInterrupt(Blink); // attaches Blink() as a timer interrupt function

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print("cps = ");
  lcd.print(TIMER_US/tick_count);  

  pinMode (encoder0PinA,INPUT_PULLUP);
  pinMode (encoder0PinB,INPUT_PULLUP);
  pinMode (encoder0Btn, INPUT_PULLUP);
  digitalWrite(encoder0Btn, HIGH); 
  aLastState = digitalRead(encoder0PinA); 
  btnLastState = HIGH;    
}

void Blink()
{
  if (tick_count-- == 0)
  {
    tick_count = current_tick_count;
    led_on = true;
    on_tick = ON_TICKS;
    digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  else
  {
    if (led_on == true)
    {
      if (on_tick-- == 0)
      {
        led_on = false;
        digitalWrite(LED_PIN, LOW);   // turn the LED on (HIGH is the voltage level)            
      }
    }
  }
}

void loop()
{
#if DEBUGGING
  String cps;
  
  if (Serial.available()) 
  { // If data is available to read,
    cps = Serial.readString();
    current_tick_count = (int)((float)TIMER_US/cps.toFloat());
    Serial.print("cps=");
    Serial.println(cps);

    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print("cps = ");
    lcd.print(cps);  
    lcd.print("  ");
  }
#endif

  btnState = digitalRead(encoder0Btn);
  if(btnState == LOW)//if button pull down
  {
    if (btnLastState == HIGH)
    {
      btnLastState = LOW;
      if (encoderIncrement == 0.1)
      {
        encoderIncrement = 1.0;
        counter = (float)((int)counter);
        lcd.setCursor(0, 1);
        // print the number of seconds since reset:
        lcd.print("cps = ");
        lcd.print(counter);  
        lcd.print("    ");    
      }
      else
        encoderIncrement = 0.1;     
    }
  }
   else
  {
    btnLastState = HIGH;
  }
  
  aState = digitalRead(encoder0PinA); // Reads the "current" state of the encoder0PinA
  // If the previous and the current state of the encoder0PinA are different, that means a Pulse has occured
  if (aState != aLastState)
  {
    if (oddEncoder == false)  // This rotary encode seems to give two "clicks" per physical click of the knob, this divides clicks by two
    {
      oddEncoder = true; 
      aLastState = aState;
      return;      
    }
    oddEncoder = false;
    
    // If the encoder0PinB state is different to the encoder0PinA state, that means the encoder is rotating clockwise
    if (digitalRead(encoder0PinB) != aState)
    { 
      counter -= encoderIncrement;
      if (counter < 0.1)
      {
        if (encoderIncrement == 0.1)
          counter = 0.1;
        else
          counter = 1.0;
      }
    }
    else
    {
      counter += encoderIncrement;
    }
    current_tick_count = (int)((float)TIMER_US/counter);

#if DEBUGGING
    Serial.print("Position: ");
    Serial.println(counter);

    Serial.print("cps=");
    Serial.println(counter);
#endif

    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print("cps = ");
    lcd.print(counter);  
    lcd.print("    ");    
  } 
  aLastState = aState; // Updates the previous state of the encoder0PinA with the current state  
}
