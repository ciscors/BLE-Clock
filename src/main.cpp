#include <TM1637.h>
#include <Wire.h>
#include "RTClib.h"
#include <FastLED.h> 
#include "OneButton.h"
#include <SoftwareSerial.h>


RTC_DS1307 rtc;
TM1637 tm(2, 3);

#define NUM_LEDS 9// number of led present in your strip
#define DATA_PIN 4
CRGB ledsArray[NUM_LEDS];

struct LedState {
  bool isOn = false;
   CRGB color = CRGB::Black;
  unsigned long onTime = 0;
  unsigned long duration = 0;
};

LedState leds[NUM_LEDS];

char timeStr[9]; // "HH:MM:ss" + null

uint8_t hour;
uint8_t currHour=0;
uint8_t currMinute=0;
uint8_t minute;
uint8_t second;
uint8_t hourVal,minuteVal,secondVal;
uint16_t yearVal;
uint8_t mountVal,dayVal;

uint8_t startcolor=40,endcolor=60;

unsigned long lastMill;
unsigned long lastClick=0;

SoftwareSerial mySerial(10, 11); // RX, TX

String serialBuffer = "";
bool commandReady = false;
String lastCommand = "";

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000;

unsigned long lastSerialReceive = 0;
const unsigned long serialTimeout = 500;

#define BTN1 7
#define BTN2 8
#define BTN3 9

#define WAIT_CLICK 15000
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB 

unsigned long lastActivation = 0;
unsigned long nextActivationDelay = 0;
unsigned long effectDuration=0;
unsigned long effectLong = 5000; //how long effect will be displayed in ms.
uint16_t effectStep=0;

//uint8_t totalOn=0;

bool setTime=false;
//bool showTime=true;
bool showLed=true;
uint8_t effect=1;



OneButton button1(BTN1, true);
OneButton button2(BTN2, true);
OneButton button3(BTN3, true);

uint8_t menuItem=0;
//int subMenuItem=0;
//int curMenu=0;


void longPressButtonBtn1()
{
  //Serial.println("LPB1dd");
  if(menuItem==0) {menuItem=1;
    //Serial.println("M#1");
     DateTime now = rtc.now();
    currHour = now.hour();currMinute = now.minute();

    }
  else if(menuItem==1 || menuItem==2 ) {
    //Serial.println("BM#0,SC");
     menuItem=0;setTime=true;}

  
 
 // showTime=true;
  lastClick=millis();

}

void singleClkButtonBtn1()
{ //Serial.println("SCB1d");
  if(menuItem==0) {
  //Serial.println("HCLS");
  showLed=!showLed;
  FastLED.clear();
  FastLED.show();} 
  else if(menuItem==1) {
    //Serial.println("sM");
    menuItem=2;}
  else if(menuItem==2) {
    //Serial.println("sH");
    menuItem=1;}
  
}
 


void doubleClkButtonBtn1()
{ 
  //Serial.print("DCB1d"); Serial.println(menuItem);
  if(menuItem==0) 
  {menuItem=10;}
  else if(menuItem==10) {
     menuItem=0;
     effectLong=effectStep*60000;
    //  Serial.println(effectLong);
      effectDuration = millis();
     showLed=true;
     menuItem=0;
  } else menuItem=0;
//Serial.print("DCB1o"); Serial.println(menuItem);
lastClick=millis();
}


void singleClkButtonBtn2()
{
  //Serial.println("SCB2d");
  if(menuItem==0) { effect++;
  if(effect>3) effect=1;
  effectDuration = millis();
  showLed=true;
  //Serial.print("CE: ");Serial.println(effect);
  } else if(menuItem==1) { //Serial.println("iHs"); 
    currHour++;}
    else if(menuItem==2)  { //Serial.println("iMs");
      currMinute++;}
    else if(menuItem==10) {
      effectStep+=5; }
  lastClick=millis();
}


void singleClkButtonBtn3()
{
  //Serial.println("SCB3");
   if(menuItem==0) { effect--;
  if(effect<1) effect=3;
  effectDuration = millis();
  showLed=true;
  //Serial.print("CE: ");Serial.println(effect);
  } else if(menuItem==1) { //Serial.println("dHs");
    currHour--;}
    else if(menuItem==2)  { //Serial.println("dMs");
      currMinute--;}
    else if(menuItem==10) {//Serial.println("dEt");
      effectStep-=5; }
    lastClick=millis();
}


void readSoftSerial() {
  while (mySerial.available()) {
    char c = mySerial.read();
   // Serial.println(c);
    lastSerialReceive = millis();

    if (c == '\n') {
      lastCommand = serialBuffer;
      serialBuffer = "";
      commandReady = true;
      return;
    }

    serialBuffer += c;
  }
}


void processCommandIfReady() {
  if (!commandReady) return;

  commandReady = false;
  lastCommand.trim();

  Serial.print("Команда: ");
  Serial.println(lastCommand);

  if (lastCommand == "ping") {
    Serial.println("pong");
    mySerial.println("pong");
  } 
   /* else if (lastCommand == "time") {
    Serial.println("Time: " + String(millis() / 1000));
    mySerial.println("Time: " + String(millis() / 1000));
  } else if (lastCommand.startsWith("msg ")) {
    
    Serial.println("msg shown");
    mySerial.println(lastCommand.substring(4));
   
  } */ 
   else if (lastCommand.startsWith("settime:")) {
    String timePart = lastCommand.substring(8);
      int comma1 = timePart.indexOf(',');
      int comma2 = timePart.indexOf(',', comma1 + 1);

      if (comma1 > 0 && comma2 > comma1) {
        hourVal = timePart.substring(0, comma1).toInt();
        minuteVal = timePart.substring(comma1 + 1, comma2).toInt();
        secondVal = timePart.substring(comma2 + 1).toInt();
        rtc.adjust(DateTime(yearVal, mountVal, dayVal, hourVal, minuteVal, secondVal));
       tm.clearScreen();
      
      }


  }  else if(lastCommand.startsWith("setcolor:")) {
     String timePart = lastCommand.substring(8);
      int comma1 = timePart.indexOf(',');
      startcolor  = timePart.substring(0, comma1).toInt();
        endcolor = timePart.substring(comma1 + 1).toInt();
        FastLED.clear();

  }
  
  else if(lastCommand.startsWith("setefct:")) {
     String timePart = lastCommand.substring(8);
      int comma1 = timePart.indexOf(',');
      effect  = timePart.substring(0, comma1).toInt();
      effectLong = timePart.substring(comma1 + 1).toInt()*1000;
      showLed=true;
       effectDuration = millis();

      FastLED.clear();

  }
    else if(lastCommand.startsWith("setdate:")) {
     //Serial.println("setdate");
     String timePart = lastCommand.substring(8);
      int comma1 = timePart.indexOf(',');
      int comma2 = timePart.indexOf(',', comma1 + 1);
      

      if (comma1 > 0 && comma2 > comma1) {
        yearVal = timePart.substring(0, comma1).toInt();
        mountVal = timePart.substring(comma1 + 1, comma2).toInt();
        dayVal = timePart.substring(comma2 + 1).toInt();
        
        
      
  }
}

 /* else if(lastCommand=="gettime"){
    DateTime now = rtc.now();
     hour = now.hour();
     minute = now.minute();
     second = now.second();
     sprintf(timeStr, "%02d:%02d:%02d", hour, minute,second);
     mySerial.println(timeStr);
  } */  
  else if(lastCommand=="ledon"){
    showLed=true;
  }
  else if(lastCommand=="ledoff"){
    showLed=false;
    FastLED.clear();
    FastLED.show();
  }
  else {
    Serial.println("ERR: Unknown cmd");
  }
}


void discardBrokenCommandIfStale() {
  if (serialBuffer.length() > 0 && millis() - lastSerialReceive > serialTimeout) {
    serialBuffer = ""; // очистка при таймауті
  }
}


void handleLeds() {
  static unsigned long lastLedCheck = 0;
  unsigned long now_lt = millis();

  if (now_lt - lastLedCheck < 50) return; // максимум 20 fps
  lastLedCheck = now_lt;

  // погасити старі
  for (int i = 0; i < NUM_LEDS; i++) {
    if (leds[i].isOn && (now_lt - leds[i].onTime > leds[i].duration)) {
      leds[i].isOn = false;
      ledsArray[i] = CRGB::Black;
      //totalOn--;
    }
  }

  // активувати нові
  if (now_lt - lastActivation >= nextActivationDelay) {
    int newCount = random(1, 5);
    for (int j = 0; j < newCount; j++) {
      int idx = random(NUM_LEDS);
      leds[idx].isOn = true;
      leds[idx].onTime = now_lt;
      leds[idx].duration = random(5000, 15000);
      leds[idx].color = CRGB(random(255), random(255), random(255));
      ledsArray[idx] = leds[idx].color;
      //totalOn++;
    }
    lastActivation = now_lt;
    nextActivationDelay = random(1500, 5000);
  }

  FastLED.show();  // тільки після змін
}

void colorHue(uint8_t c1,uint8_t c2)
{

  static unsigned long lastLedCheck = 0;
  unsigned long now_lt = millis();

  if (now_lt - lastLedCheck < 50) return; // максимум 20 fps
  lastLedCheck = now_lt;

  // погасити старі
  for (int i = 0; i < NUM_LEDS; i++) {
    if (leds[i].isOn && (now_lt - leds[i].onTime > leds[i].duration)) {
      leds[i].isOn = false;
      ledsArray[i] = CRGB::Black;
      //totalOn--;
    }
  }

  // активувати нові
  if (now_lt - lastActivation >= nextActivationDelay) {
    int newCount = random(1, 5);
    for (int j = 0; j < newCount; j++) {
      int idx = random(NUM_LEDS);
      leds[idx].isOn = true;
      leds[idx].onTime = now_lt;
      leds[idx].duration = random(5000, 15000);
      byte yellowHue = random8(c1, c2); // Відтінки жовтого
      leds[idx].color = CHSV(yellowHue, 255,random8(40, 70)); // Макс. насиченість і яскравість
      
      ledsArray[idx] = leds[idx].color;
      //totalOn++;
    }
    lastActivation = now_lt;
    nextActivationDelay = random(1500, 5000);
  }

  FastLED.show();  // тільки після змін
}
 

void updateBreathingRainbow() {
  static uint8_t baseHue = 0;
  static unsigned long lastUpdate = 0;
  static const unsigned long updateInterval = 20; // мс

  // Перевірка таймера — оновлюємо лише кожні 20 мс
  if (millis() - lastUpdate < updateInterval) return;
  lastUpdate = millis();

  // Обрахунок яскравості
  uint8_t breath = beatsin8(4, 10, 70); // плавна хвиля

  for (int i = 0; i < NUM_LEDS; i++) {
    CHSV hsv = CHSV(baseHue + i * 8, 255, 30);
    CRGB rgb = hsv;

    rgb.nscale8_video(breath);
    ledsArray[i].fadeToBlackBy(10);
    ledsArray[i] += rgb;
  }

  baseHue++;
  FastLED.show();
}



void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
   button1.attachClick(singleClkButtonBtn1);
   button1.attachLongPressStop(longPressButtonBtn1);
   button1.attachDoubleClick(doubleClkButtonBtn1);
    button2.attachClick(singleClkButtonBtn2);
    button3.attachClick(singleClkButtonBtn3);

  if (!rtc.begin()) {
    Serial.println("RTC не знайдено");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC не працює, встановлюємо час");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  tm.begin();
  tm.setBrightness(5);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(ledsArray, NUM_LEDS);
FastLED.setBrightness(40);
Serial.println("LED Clock v0.2");

}

void loop() {
 

  button1.tick();
  button2.tick();
  button3.tick();
  
  readSoftSerial();
  processCommandIfReady();
  discardBrokenCommandIfStale();

 if(millis()-lastClick>WAIT_CLICK) {
  setTime=false;
  menuItem=0;
}

 if(setTime) {
  rtc.adjust(DateTime(2025, 6, 19, currHour, currMinute, 0));
  setTime=false;
 }
 
 if(menuItem==0) {  //Main process
  
  if(millis() - lastMill > 1000) {
     DateTime now = rtc.now();
     hour = now.hour();
     minute = now.minute();
     sprintf(timeStr, "%02d%02d", hour, minute);
     //Serial.println(timeStr);
     tm.display(String(timeStr));
     tm.switchColon();
     
     lastMill=millis();
  }


  if (showLed) {
  if (millis() - effectDuration > effectLong) {
    showLed = false;
    FastLED.clear(true);  // гасимо світлодіоди
  } else {
    switch (effect) {
      case 1:
        FastLED.setBrightness(255);
        updateBreathingRainbow();
        break;
      case 2:
        FastLED.setBrightness(255);
        colorHue(startcolor, endcolor);
        break;
      case 3:
        FastLED.setBrightness(5);
        handleLeds();
        break;
    }
  }
}
} else if (menuItem==1) {
  sprintf(timeStr, "%02d  ", currHour);
  tm.display(String(timeStr));}
else if (menuItem==2) {
  sprintf(timeStr, "  %02d", currMinute);
  tm.display(String(timeStr));}
else if (menuItem==10){
  if(effectStep>60) effectStep=60;
  if(effectStep<0) effectStep=0;
  sprintf(timeStr, "ED%02d", effectStep);
  
  tm.display(String(timeStr));
}

}