/*
  Components Used:-
  ESP32
  Arduino uno
  HC-SR04
  LED
  Photo Resistor
  16x2 LCD
  10k ohm potentiometer
  DHT11
  10k/220 Ohm resistor
  Breadboard
  Jumper wires

*/

/* ****************************** Header Files ************************** */
#include "config.h"
#include<SoftwareSerial.h>
#include "dht.h"
/* ****************************** PINS ******************************** */
#define dthpin 4
#define ledpin 23
#define trigPin 19
#define echoPin 18
#define ldrpin A0
#define RXpin 13
#define TXpin 12
/* ****************************** Feeds ******************************** */
AdafruitIO_Feed *MODE = io.feed("Mode");
AdafruitIO_Feed *LDR = io.feed("LDR");
AdafruitIO_Feed *LDRTHRESHOLD = io.feed("LdrThres");
AdafruitIO_Feed *KEY = io.feed("Key");
AdafruitIO_Feed *TIMER = io.feed("Timer");
AdafruitIO_Feed *HUMIDITY = io.feed("Humidity");
AdafruitIO_Feed *TEMPERATURE = io.feed("Temperature");
dht DHT;
SoftwareSerial s(RXpin, TXpin);
2021 - 22 / ENTH53 / TA5 - 6
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
const long interval = 30000;
int timer = 0;
int SelectedMode = 0;
long duration;
float distance;
int ldrThreshold = 50
                   int ldrValue;
unsigned long prevMillis = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis;
bool StartTimer = false;
bool output = false;
bool manualbtn = false;
bool ldrstatus = false;
/* ****************************** SETUP ******************************** */
void setup() {
  s.begin(9600);
  Serial.begin(115200);
  while (! Serial);
  Serial.print("Connecting to Adafruit IO");
  io.connect();
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println(io.statusText());
  MODE->onMessage(CallBackOnMode);
  LDRTHRESHOLD->onMessage(CallBackOnLdrThreshold);
  KEY->onMessage(CallBackOnKey);
  TIMER->onMessage(CallBackOnTimer);
  MODE->get();
  LDRTHRESHOLD->get();
  KEY->get();
  TIMER->get();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output

  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(ledpin, OUTPUT); // Sets the ledpin as an Input
  pinMode(dhtpin, INPUT); // Sets the dhtpin as an Input
  pinMode(ldrpin, INPUT); // Sets the ldrpin as an Input
}
/* ****************************** LOOP ******************************** */
void loop() {
  io.run();
  switch (SelectedMode) { // func calling for selected mode
    case 1:
      CallManual();
      break;
    case 2:
      CallAutoLdr();
      break;
    case 3:
      CallAutoPir();
      break;
    case 4:
      CallAutoLdrAndPir();
      break;
    default:
      Serial.println("OFF");
  }
  unsigned long currMillis = millis();
  if (currMillis - prevMillis >= interval) { //Updating feed every 30secs
    prevMillis = currMillis;
    updateValues();
  }
}
/* ****************************** Mode Selection *************************** */
void CallBackOnMode(AdafruitIO_Data *data)
{
  Serial.print("received <- ");
  SelectedMode = MODE->data->toInt();
  switch (SelectedMode) {
    case 1:
      s.write("Manual Mode");
      break;
    case 2:
      s.write("Auto LDR Mode");

      break;
    case 3:
      s.write("Auto PIR Mode is Selected");
      break;
    case 4:
      s.write("Auto PIR and LDR Mode");
      break;
    default:
      digitalWrite(light, LOW);
      s.write("OFF");
  }
}
/* ****************************** Publish *************************** */
void updateValues() {
  LDR->save(ldrValue); //Sending LDR value to cloud
  DHT.read11(dht_apin);
  HUMIDITY->save(DHT.humidity); //Sending humidity value to cloud
  TEMPERATURE->save(DHT.temperature); //Sending temperature value to cloud
}
/* ****************************** Subscribe *************************** */
void CallBackOnLdrThreshold(AdafruitIO_Data *data) {
  ldrThreshold = data->toInt(); //Updates LdrThreshold Value
}
void CallBackOnKey(AdafruitIO_Data *data) {
  manualbtn = data->toInt(); //Updates Key Value
}
void CallBackOnTimer(AdafruitIO_Data *data) {
  timer = data->toInt() * 1000; //Updates Timer Value
}
/* ****************************** OutputON **************************** */
void OutputOn() {
  digitalWrite(ledpin, HIGH); //Turn ON LED
  output = true;
}

/* ****************************** OutputOff **************************** */
void OutputOff() {
  digitalWrite(ledpin, LOW); //Turn OFF LED
  output = false;
}
/* ****************************** CallAutoLdr **************************** */
void DetectIntensity() {
  ldrValue = analogRead(A0); //Reading Analog ldr Value
  ldrValue = map(ldrValue, 0, 4095, 0, 100); //Mapping LDR value between 0 to 100
  if (ldrValue < ldrThreshold) {
    ldrstatus = true; //If Light intensity is less than threshold value
  }
  else {
    ldrstatus = false; //If Light intensity is More than threshold value
  }
}
void CallAutoLdr() {
  DetectIntensity();
  if (ldrstatus == true) //If Light intensity is less than threshold value
    OutputOn();
  else
    OutputOff(); //If Light intensity is More than threshold value
}
/* ****************************** CallManual **************************** */
void CallManual() {
  if (manualbtn == 1)
    OutputOn();
  else {
    OutputOff();
  }
}
/* ****************************** CallAutoPir **************************** */

void DetectMovement() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);// Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance
  distance = duration * SOUND_SPEED / 2;
  if (distance <= 50) {
    StartTimer = true; //Motion is detected
    previousMillis = millis();
  }
  else
    StartTimer = false;
}
void IsMovement() {
  currentMillis = millis();
  if ((currentMillis - previousMillis) > (timer)) {
    OutputOff();
    StartTimer = false;
  }
  else
    OutputOn();
}
void CallAutoPir() {
  if (StartTimer == true) {
    IsMovement(); //Motion is detected
  }
  else
    DetectMovement();
}
/* ************************** CallAutoLdrAndPir *********************** */
void CallAutoLdrAndPir() {
  CallAutoPir();
  DetectIntensity();
  if (StartTimer && ldrstatus) {
    IsMovement();
  }
}
