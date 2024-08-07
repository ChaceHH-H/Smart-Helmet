#include "DFRobot_MICS.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <TimeLib.h>
#define CALIBRATION_TIME   3
#define ADC_PIN   A0
#define POWER_PIN 2
#define FORCE_SENSOR_PIN A7 

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");



DFRobot_MICS_ADC mics(/*adcPin*/ADC_PIN, /*powerPin*/POWER_PIN);

const int xPin = A1;
const int yPin = A2;
const int zPin = A3;

const int sampleSize = 10; // 每次取样的数量
const float alpha = 0.5;   // 低通滤波的权重

float xFiltered = 0;
float yFiltered = 0;
float zFiltered = 0;

const char* ssid = "mate40pro";     
const char* password = "88888888"; 

// Firestore API 
const char* projectID = "fir-flutter-codelab-a80ae"; //Project ID
const char* collection = "Helmet-data-3in1"; // Collection name

unsigned long previousMillis = 0;  // 上次上传数据的时间
const long interval = 10000;       // 10秒钟的间隔

float gasdataC2H5OH = 0;
float gasdataCO = 0;
float gasdataCH4 = 0;

float magnitude = 0;
bool Wear= false;


void setup() 
{
  Serial.begin(115200);

  //while(!Serial);
  while(!mics.begin()){
    Serial.println("NO Deivces !");
    delay(1000);
  } Serial.println("Device connected successfully !");
  uint8_t mode = mics.getPowerState();
  if(mode == SLEEP_MODE){
    mics.wakeUpMode();
    Serial.println("wake up sensor success!");
  }else{
    Serial.println("The sensor is wake up mode");
  }
  while(!mics.warmUpTime(CALIBRATION_TIME)){
    Serial.println("Please wait until the warm-up time is over!");
    delay(1000);
  }

    // Initialize sensor
  xFiltered = analogRead(xPin);
  yFiltered = analogRead(yPin);
  zFiltered = analogRead(zPin);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  timeClient.begin();

}

void loop() 
{
  gas();
  impact();
  fsr();
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
  delay(100);
}

void gas(){
  gasdataC2H5OH = mics.getGasData(C2H5OH);
  gasdataCO = mics.getGasData(CO);
  gasdataCH4 = mics.getGasData(CH4);
  //float gasdataC3H8 = mics.getGasData(C3H8);
  Serial.print("C2H5OH: ");
  Serial.print(gasdataC2H5OH,1);
  Serial.println(" PPM");

  Serial.print("CO: ");
  Serial.print(gasdataCO,1);
  Serial.println(" PPM");

  Serial.print("CH4: ");
  Serial.print(gasdataCH4,1);
  Serial.println(" PPM");

}

void impact(){
  float xSum = 0, ySum = 0, zSum = 0;
  for (int i = 0; i < sampleSize; i++) {
    xSum += analogRead(xPin);
    ySum += analogRead(yPin);
    zSum += analogRead(zPin);
    delay(10); // 每次取样间隔10ms
  }

  // 计算取样平均值
  float x = xSum / sampleSize;
  float y = ySum / sampleSize;
  float z = zSum / sampleSize;

  // 低通滤波
  xFiltered = alpha * x + (1 - alpha) * xFiltered;
  yFiltered = alpha * y + (1 - alpha) * yFiltered;
  zFiltered = alpha * z + (1 - alpha) * zFiltered;

  // 打印当前滤波后的加速度值
  Serial.print("X: "); Serial.print(xFiltered);
  Serial.print(" Y: "); Serial.print(yFiltered);
  Serial.print(" Z: "); Serial.println(zFiltered);

  // 计算加速度矢量的模
  magnitude = sqrt(sq(xFiltered - 512) + sq(yFiltered - 512) + sq(zFiltered - 512));
  Serial.println(magnitude);
  // 检测撞击
  if (magnitude > 3000) { // 设定一个合理的阈值
    Serial.println("Impact detected!");
    // 可以在此处添加其他撞击检测后的处理逻辑
  }
}

void fsr(){
  int analogReading = analogRead(FORCE_SENSOR_PIN);
  Serial.print("Force sensor reading = ");
  Serial.print(analogReading); 
  if (analogReading < 30){
    Serial.println(" -> no pressure");
    Wear = false;
  }else{
    Serial.println("Wearing detected");
    Wear = true;
  }       // from 0 to 9
    
    
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
    timeClient.setTimeOffset(timeOffset);
    char dateTimeStr[25];
    sprintf(dateTimeStr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year(), month(), day(), hour(), minute(), second());


    // Building a REST API URL for Firestore
    String url = "https://firestore.googleapis.com/v1/projects/" + String(projectID) + "/databases/(default)/documents/" + String(collection) + "?key=YOUR_API_KEY";
    // Build JSON data
    // String jsonData = "{\"fields\": {
    //   \"timestamp\": {\"timestampValue\": \"" + String(dateTimeStr) + "\"}, 
    //   \"Heartrate\": {\"integerValue\": \"" + String(beatAvg) + "\"},
    //   \"fsr\": {\"booleanValue\": \"" + String(Wear ? "true" : "false") + "\"},
    //   \"impact\": {\"doubleValue\": \"" + String(magnitude) + "\"},
    //   \"C2H5OH\": {\"doubleValue\": \"" + String(gasdataC2H5OH) + "\"},
    //   \"CO\": {\"doubleValue\": \"" + String(gasdataCO) + "\"},
    //   \"CH4\": {\"doubleValue\": \"" + String(gasdataCH4) + "\"}}}";
      String jsonData = "{\"fields\": {\"timestamp\": {\"timestampValue\": \"" + String(dateTimeStr) + "\"}, \"fsr\": {\"booleanValue\": \"" + String(Wear ? "true" : "false") + "\"}, \"impact\": {\"doubleValue\": \"" + String(magnitude) + "\"}, \"C2H5OH\": {\"doubleValue\": \"" + String(gasdataC2H5OH) + "\"}, \"CO\": {\"doubleValue\": \"" + String(gasdataCO) + "\"}, \"CH4\": {\"doubleValue\": \"" + String(gasdataCH4) + "\"}}}";

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
