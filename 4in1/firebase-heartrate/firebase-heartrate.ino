#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <TimeLib.h>

// NTP
WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org");
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
/////heartRate
#include "MAX30105.h"
#include "heartRate.h"
MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
/////heartRate

const char* ssid = "mate40pro";     
const char* password = "88888888"; 

// Firestore API 
const char* projectID = "fir-flutter-codelab-a80ae"; //Project ID
const char* collection = "Helmet-data"; // Collection name

unsigned long previousMillis = 0;  // 上次上传数据的时间
const long interval = 10000;       // 10秒钟的间隔

void setup() {
  Serial.begin(115200);
  //Connect to wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  timeClient.begin();
  Serial.println("Initializing...");
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1);
  }
  

  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  
}

void loop() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();


  // 获取当前时间
  unsigned long currentMillis = millis();
  // 检查是否到达上传数据的时间
  if (currentMillis - previousMillis >= interval) {
    // 保存当前时间
    previousMillis = currentMillis;
    // 上传数据到Firebase
    senddata();
    Serial.println("senddata");
  }
  
}



void senddata(){
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    setTime(epochTime);
    int timeOffset = 0;
    if (isDST(day(), month(), weekday())) {
      timeOffset = 0;  // UTC +1 小时
    }
    //timeClient.setTimeOffset(timeOffset);
    adjustTime(timeOffset);
    char dateTimeStr[25];
    sprintf(dateTimeStr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year(), month(), day(), hour(), minute(), second());


    // Building a REST API URL for Firestore
    String url = "https://firestore.googleapis.com/v1/projects/" + String(projectID) + "/databases/(default)/documents/" + String(collection) + "?key=YOUR_API_KEY";
    // Build JSON data
    //String jsonData = "{\"fields\": {\"Heartrate\": {\"integerValue\": \"" + String(beatAvg) + "\"}}}";
    String jsonData = "{\"fields\": {\"timestamp\": {\"timestampValue\": \"" + String(dateTimeStr) + "\"}, \"Heartrate\": {\"integerValue\": \"" + String(beatAvg) + "\"}}}";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

bool isDST(int day, int month, int weekday) {
    // 英国夏令时从三月最后一个周日开始到十月最后一个周日结束
    if (month < 3 || month > 10) return false; // 一月, 二月, 十一月和十二月
    if (month > 3 && month < 10) return true; // 四月到九月

    int previousSunday = day - weekday;

    // 在三月，我们在最后一个周日过后是夏令时
    if (month == 3) return previousSunday >= 25;
    // 在十月，我们在最后一个周日之前仍然是夏令时
    if (month == 10) return previousSunday < 25;

    return false; // 应该不会发生
}