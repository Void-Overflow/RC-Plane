#include <LiquidCrystal.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

#define rudderX A0
#define throttleY A1

#define AileronX A3
#define ElevatorY A2

bool throttle_current = false;

#define throttle_start_pin 9
#define lcd_mode_pin A4

LiquidCrystal lcd(8, 7, 6, 4, 3, 2);
RF24 radio(10, A5); // CE, CSN

unsigned long previousRefresh = 0; 
int refreshRate = 200;

unsigned long throttle_previous = 0; 
int throttle_interval = 1000;

unsigned long lcd_previous = 0;
int lcd_interval = 1000;

bool lcd_current = false;

void setup(){ 
  lcd.begin(16, 2);

  pinMode(throttle_start_pin, INPUT_PULLUP);
  pinMode(lcd_mode_pin, INPUT_PULLUP);

  pinMode(0, OUTPUT);
  pinMode(A5, OUTPUT);

  digitalWrite(0, LOW);

  const byte address[6] = "00001";
  radio.begin();
  radio.openWritingPipe(address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
}

struct dataPackage{
  byte throttleYValue;
  byte rudderXValue;

  byte throttleState;

  byte AileronXValue;
  byte ElevatorYValue;

  byte trimValue;
  byte responsivenessValue;
};

dataPackage data;

void loop() {
  data.throttleYValue = map(analogRead(throttleY), 0, 1023, 0, 255);
  data.rudderXValue = map(analogRead(rudderX), 0, 1023, 0, 255);

  bool throttleState = digitalRead(throttle_start_pin);
  data.throttleState = throttle_current;
  bool lcdState = digitalRead(lcd_mode_pin);

  data.AileronXValue = map(analogRead(AileronX), 0, 1023, 0, 255);
  data.ElevatorYValue = map(analogRead(ElevatorY), 0, 1023, 0, 255);

  data.trimValue = map(analogRead(A6), 0, 1023, 0, 255);
  data.responsivenessValue = map(analogRead(A7), 0, 1023, 0, 255);

  radio.write(&data, sizeof(dataPackage));

  int currentMillis = millis();
  if(currentMillis - lcd_previous >= lcd_interval){
    if(lcdState == LOW){
      if(lcd_current == false)
        lcd_current = true;
      else
        lcd_current = false;
    }
      
    lcd_previous = currentMillis;
  }

  if(lcd_current == false){
    lcd.setCursor(0,0);
    lcd.print("1|X" + String(data.rudderXValue) + ":Y" + String(data.throttleYValue) + ":T" + String(data.trimValue));
    lcd.setCursor(0,1);
    lcd.print("2|X" + String(data.AileronXValue) + ":Y" + String(data.ElevatorYValue) + ":R" + String(data.responsivenessValue));
  }
  else{
    lcd.setCursor(0,0);
    lcd.print("Recent GPS Data:");
    lcd.setCursor(0,1);
    lcd.print(" 4124122 210265");
  }

  //delay(75);
  currentMillis = millis();

  if(currentMillis - throttle_previous >= throttle_interval){
    if(throttleState == LOW){
      if(throttle_current == false){
        throttle_current = true;
        digitalWrite(0, HIGH);
      }
      else{
        throttle_current = false;
        digitalWrite(0, LOW);
      }
    }

    throttle_previous = currentMillis;
  }

  if(currentMillis - previousRefresh >= refreshRate){
    lcd.clear();
    previousRefresh = currentMillis;
  }
}
