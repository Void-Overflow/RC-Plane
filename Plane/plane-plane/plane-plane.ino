#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <Servo.h>

RF24 radio(8, 7); // CE, CSN

Servo ESC; 
Servo rudderServo;
Servo elevatorServo;
Servo aileron1Servo;
Servo aileron2Servo;

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

void resetData() {
  data.throttleYValue = 0;
  data.rudderXValue = 122;

  data.throttleState = false;

  data.AileronXValue = 124;
  byte ElevatorYValue = 127;

  data.trimValue = 255;
  data.responsivenessValue = 0;
}

int responsiveness;
int rudderValue;

unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  
  const byte address[6] = "00001";
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  ESC.attach(10,1000,2000);

  rudderServo.attach(3);
  elevatorServo.attach(5);
  aileron1Servo.attach(6);
  aileron2Servo.attach(9);
}

void loop() {
  currentTime = millis();
  if ( currentTime - lastReceiveTime > 1000 ) 
    resetData();
  
  if (radio.available()) {
    radio.read(&data, sizeof(dataPackage));
    Serial.println("1|X" + String(data.rudderXValue) + ":Y" + String(data.throttleYValue) + ":T" + String(data.trimValue));
    Serial.println("2|X" + String(data.AileronXValue) + ":Y" + String(data.ElevatorYValue) + ":R" + String(data.responsivenessValue));
    Serial.println("throttleState:" + String(data.throttleState) + '\n');

    lastReceiveTime = millis();
  }

  if(data.throttleState == true){
    if (data.throttleYValue > 151)
      data.throttleYValue = 151;
    int throttleValue = constrain(data.throttleYValue, 109, 255);
    ESC.writeMicroseconds(map(throttleValue, 109, 255, 1000, 2000));
  } else{ ESC.writeMicroseconds(0);}

   
  responsiveness = map(data.responsivenessValue, 0, 255, 0, 25);

  elevatorServo.write(map(data.ElevatorYValue, 0, 255, (85 - responsiveness), (35 + responsiveness)));

  aileron1Servo.write(map(data.AileronXValue, 0, 255, (10 + responsiveness), (80 - responsiveness)));
  aileron2Servo.write(map(data.AileronXValue, 0, 255, (10 + responsiveness), (80 - responsiveness)));

  if (data.trimValue > 127) {
    rudderValue = data.rudderXValue + (data.trimValue - 127);
  }
  if (data.trimValue < 127) {
    rudderValue = data.rudderXValue - (127 - data.trimValue);
  }

  rudderValue = map(rudderValue, 0, 255, (10 + responsiveness), (90 - responsiveness));
  rudderServo.write(rudderValue);
  delay(4);
}
