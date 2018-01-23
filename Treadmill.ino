#include <M5Stack.h>
#include "Free_Fonts.h"
//#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <StopWatch.h>

const char *OTAName = "treadmill";         // A name and a password for the OTA service
const char *OTAPassword = "m5stack";

int half_revolutions = 0;
int debounce = 500;
unsigned int rpm = 0;
unsigned long lastmillis = 0;
unsigned long totalTime = 0;
int16_t old_dist = 0;
int16_t intDist;
float totalDist, kmDist;
float v, vkmt;
float r = 0.075;  // radius i meter
unsigned long timeold, timeDuration;
bool going = false;

StopWatch MySW;
StopWatch SWarray[5];
StopWatch sw_secs(StopWatch::SECONDS);


void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
}

void printTemp( float temp) {
  M5.Lcd.setCursor(235, 160);
//  M5.Lcd.print(temp);
}

void printSpeed( float spd) {
  float pace;
  pace = (60.0 / spd);
  int minutes = pace;
  if (minutes > 59)
  {
    minutes = 0;
  }
//  Serial.println(minutes);
  float seconds = pace * 100 - minutes * 100;
//  Serial.println(seconds);
  int intSeconds = (seconds /100) *60;
   if (intSeconds > 59)
  {
    intSeconds = 0;
  }
//  Serial.println(intSeconds);
  M5.Lcd.setFreeFont(FF44);
  M5.Lcd.fillRect(10, 30, 160, 70, BLUE);
  M5.Lcd.setCursor(50, 80);
  M5.Lcd.printf("%02d:%02d", minutes, intSeconds);

  M5.Lcd.fillRect(200, 30, 110, 70, BLUE);
  M5.Lcd.setCursor(200, 80);
  M5.Lcd.print(spd);

}

void printDist(float dist) {
  M5.Lcd.setFreeFont(FF44);
  M5.Lcd.fillRect(10, 135, 160, 50, BLUE);
  M5.Lcd.setCursor(25, 180);
  M5.Lcd.print(dist);
//  intDist = (int) dist;
//  if (dist = old_dist+1){
//    M5.Speaker.beep();
//    dist = old_dist
//  }

}

void printTime() {
  M5.Lcd.setFreeFont(FF38);
  M5.Lcd.fillRect(5, 191, 200, 25, BLUE);
  M5.Lcd.setCursor(10, 210);
  timeDuration = MySW.value() / 1000;
  
  int runHours = timeDuration / 3600;
  int secsRemaining = timeDuration % 3600;
  int runMinutes = secsRemaining / 60;
  int runSeconds = secsRemaining % 60;
  Serial.printf("%02d:%02d:%02d", runHours, runMinutes, runSeconds);
  M5.Lcd.printf("%02d:%02d:%02d", runHours, runMinutes, runSeconds);
}

void setup() {
  M5.begin();
  Serial.begin(115200);
  Serial.println("setup...");
   M5.Speaker.setVolume(5);
  pinMode(22, INPUT);
  pinMode(21, INPUT);
  attachInterrupt(21, rpm_fan, RISING);
  M5.Lcd.fillScreen(BLUE);
  M5.Lcd.drawRect(0, 0, 320, 190, WHITE);
  M5.Lcd.drawRect(1, 1, 318, 188, WHITE);
  M5.Lcd.drawRect(2, 2, 316, 186, WHITE);
  M5.Lcd.drawLine(190, 0, 190, 189, WHITE);
  M5.Lcd.drawLine(191, 0, 191, 189, WHITE);
  M5.Lcd.drawLine(192, 0, 192, 189, WHITE);
  M5.Lcd.drawLine(0, 109, 320, 109, WHITE);
  M5.Lcd.drawLine(0, 110, 320, 110, WHITE);
  M5.Lcd.drawLine(0, 111, 320, 111, WHITE);
  M5.Lcd.setFreeFont(FF38);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 25);
  M5.Lcd.printf("min/km");
  M5.Lcd.setCursor(205, 25);
  M5.Lcd.printf("km/h");
  M5.Lcd.setCursor(140, 160);
  M5.Lcd.print("km");
  M5.Lcd.setCursor(10, 130);
  M5.Lcd.printf("Distanse");
//  M5.Lcd.setCursor(205, 130);
//  M5.Lcd.printf("  ");
  M5.Lcd.setCursor(50, 239);
  M5.Lcd.printf("Start");
  M5.Lcd.setCursor(130, 239);
  M5.Lcd.printf("Stopp");
  M5.Lcd.setCursor(210, 239);
  M5.Lcd.printf("Reset");

  startOTA();

//  printTemp(15);
  printSpeed(14);
  printDist(1.5);
  

}

void rpm_fan()
{
  if (micros() - debounce > 500) { //500 microseconds since last interrupt
    debounce = micros();
    half_revolutions++;
  }
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    MySW.start();
    going = true;
  }
  if (M5.BtnB.wasPressed()) {
    MySW.stop();
    going = false;
  }
  if (M5.BtnC.wasPressed()) {
    MySW.reset();
    kmDist = 0;
    old_dist = 0;
  }

  if (millis() - lastmillis == 1000) { //Uptade every one second, this will be equal to reading frecuency (Hz).
    //    detachInterrupt(0);//Disable interrupt when calculating
    detachInterrupt(21);
    //rpm = half_revolutions * 60; // Convert frecuency to RPM, note: this works for one interruption per full rotation. For two interrups per full rotation use half_revolutions * 30.
    rpm = (60 * 1000 / (millis() - timeold) * half_revolutions) / 36;
    timeold = millis();
    totalDist = half_revolutions/36;
    half_revolutions = 0; // Restart the RPM counter
    lastmillis = millis(); // Uptade lasmillis
    attachInterrupt(21, rpm_fan, RISING);
    rpm = (rpm *3.4)/10; // driven gear / drive gear = 10/3,4 = 2,941176470588235
    v = (r * rpm * 0.10472) ;
    vkmt = v * 3.6;
    printSpeed(vkmt);
    Serial.print("RPM =\t"); //print the word "RPM" and tab.
    Serial.print(rpm); // print the rpm value.
    Serial.print("\t Hz=\t"); //print the word "Hz".
    Serial.println(totalDist); //print revolutions per second or Hz. And print new line or enter.
    Serial.print("M/S =\t");
    Serial.print(v);
    Serial.print("\tkm/t= \t");
    Serial.println(vkmt);
    if(going){
    kmDist += (totalDist * (2 * r * 3.14)) / 1000;
    }
    Serial.print(totalDist);
    Serial.print('\t');
    Serial.println(kmDist);
    printDist(kmDist);
    printTime();

  }



}

