#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "images.h"

#include <OneWire.h>
#include <DallasTemperature.h>

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D2, D1);

#define DEMO_DURATION 3000
typedef void (*Demo)(void);

int demoMode = 0;
int counter = 1;

// encoder pins D5 D6 ; GPiO 14, GPIO 12
// button pins D3, GPIO 0
#define encoder0PinA  12
#define encoder0PinB  14
#define button0Pin  0
#define HeatElPin  D7


volatile float encoder0Pos = 50;
volatile char Button0_state = 0;
volatile char blink_timestamp = 0;

unsigned long encoder_timestamp = 0;
unsigned long button_timestamp = 0;
unsigned long thermometer_timestamp = 0;
unsigned long OneS_timestamp = 0;

float temp_set = 0;
float temp_act = 0;
boolean heating = 0;


#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  sensors.begin();
  pinMode(digitalPinToInterrupt(encoder0PinA), INPUT_PULLUP);
  pinMode(digitalPinToInterrupt(encoder0PinB), INPUT_PULLUP);
  pinMode(digitalPinToInterrupt(button0Pin), INPUT_PULLUP);
  pinMode(HeatElPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(button0Pin), doButton0, FALLING);
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoderA, FALLING);
  attachInterrupt(digitalPinToInterrupt(encoder0PinB), doEncoderB, FALLING);

  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("Setup");
}

void drawTemp(float temp_set, float temp_act) {
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16); // 24
  unsigned long temp = millis() / 1000;
  display.drawString(5,  14, "Time: " + String(((temp/(60)) % 60)) + " m  "  + String(temp % 60 ) + " s" );
  display.drawString(5,  30, "Set: " + String(temp_set, 1) + " °C" );
  display.drawString(5, 46, "Act: " + String(temp_act, 1) + " °C" );

}

void drawRectDemo() {
  display.drawHorizontalLine(0, 0, 128);
  display.drawHorizontalLine(0, 63, 128);
  display.drawVerticalLine(0, 0, 64);
  display.drawVerticalLine(127, 0, 64);
}

void drawCircleDemo() {
  for (int i = 1; i < 8; i++) {
    display.setColor(WHITE);
    display.drawCircle(32, 32, i * 3);
    if (i % 2 == 0) {
      display.setColor(BLACK);
    }
    display.fillCircle(96, 32, 32 - i * 3);
  }
}

void drawProgressBarDemo() {
  int progress = (counter / 5) % 100;
  // draw the progress bar
  display.drawProgressBar(0, 32, 120, 10, progress);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(progress) + "%");
}


void loop() {
  display.clear();

  if (  millis() > ( thermometer_timestamp + 1000 ) )
  {
    sensors.requestTemperatures();
    thermometer_timestamp = millis();
    temp_act = sensors.getTempCByIndex(0);
    if (temp_set > temp_act)
    { // Heat ON
      heating = true;
    }
    else
    { // Heat OFF
      heating = false;
    }
  }
  temp_set = encoder0Pos;


  //Serial.println("Loop");

  if ( blink_timestamp == 0 )
  {
    // drawRectDemo();
    display.drawVerticalLine( 0, 0, 1);
    blink_timestamp = 1;
  }
  else
  {
    blink_timestamp = 0;
  }

  if ( (millis() - OneS_timestamp) > 1000 )
  {
    OneS_timestamp = millis();
  }

  drawTemp(temp_set, temp_act);
  if ( heating )
  {
    // display.drawHorizontalLine(0, 63, 128);
    digitalWrite(HeatElPin, HIGH);
  //  display.drawString(5,  0, "Heat ON" );
  }
  else
  {
    digitalWrite(HeatElPin, LOW);
   // display.drawString(5,  0, "Heat OFF" );
  }

  display.display();

  delay(50);
}



ICACHE_RAM_ATTR void doEncoderA()
{
  if ( digitalRead(encoder0PinB) == HIGH and ( millis() - encoder_timestamp ) > 10 and encoder0Pos > 10)
  {
    encoder0Pos = encoder0Pos - 1;
    encoder_timestamp = millis();
    thermometer_timestamp = millis() + 700;
  }
}

ICACHE_RAM_ATTR void doEncoderB()
{
  if ( digitalRead(encoder0PinA) == HIGH and ( millis() - encoder_timestamp ) > 10 and encoder0Pos < 120)
  {
    encoder0Pos = encoder0Pos + 1;
    encoder_timestamp = millis();
    thermometer_timestamp = millis() + 700;
  }
}

ICACHE_RAM_ATTR void doButton0()
{
  if ( Button0_state == 0 and ( millis() - encoder_timestamp ) > 10 )
  {
    Button0_state = 1;
    encoder_timestamp = millis();
    thermometer_timestamp = millis() + 700;
  }
}
