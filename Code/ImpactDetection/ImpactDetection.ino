#include <WiFiS3.h>
#include <Wire.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const char* ssid = "mate40pro";
const char* password = "88888888";

// Firebase configuration
const char* host = "firestore.googleapis.com";
const char* apiKey = "AIzaSyDQLBZquq18ScMUz-CusPv1LZtTwV4dJxA";
const char* projectId = "fir-flutter-codelab-a80ae";
const char* collection = "Helmet-data-3in1"; // Collection name

// HTTP client
WiFiSSLClient wifiClient;
HttpClient client = HttpClient(wifiClient, host, 443);

unsigned long previousMillis = 0;  // 上次上传数据的时间
const long interval = 10000;       // 10秒钟的间隔

const int xPin = A0;
const int yPin = A1;
const int zPin = A2;

const int sampleSize = 10; // 每次取样的数量
const float alpha = 0.5;   // 低通滤波的权重

float xFiltered = 0;
float yFiltered = 0;
float zFiltered = 0;

float magnitude = 0;
bool Beinghit= false;

void setup() {
  Serial.begin(115200);
  xFiltered = analogRead(xPin);
  yFiltered = analogRead(yPin);
  zFiltered = analogRead(zPin);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  // Initialize NTP client
  timeClient.begin();
  // Initialize sensor
  xFiltered = analogRead(xPin);
  yFiltered = analogRead(yPin);
  zFiltered = analogRead(zPin);
}

void loop() {
  impact();
      //获取当前时间
  unsigned long currentMillis = millis();
  // 检查是否到达上传数据的时间
  if (currentMillis - previousMillis >= interval) {
    // 保存当前时间
    previousMillis = currentMillis;
    // 上传数据到Firebase
    timeClient.update();
    senddata();
  }
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
    Beinghit=true;
    // 可以在此处添加其他撞击检测后的处理逻辑
  }else{
    Beinghit=false;
  }
}

void senddata() {
  if (WiFi.status() == WL_CONNECTED) {
    // Get current timestamp
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    setTime(epochTime);
    // 检查是否为夏令时期间
    int timeOffset = 0;
    if (isDST(day(), month(), weekday())) {
      timeOffset = 0;  // UTC +1 小时
    }
    timeClient.setTimeOffset(timeOffset);

    char dateTimeStr[25];
    sprintf(dateTimeStr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year(), month(), day(), hour(), minute(), second());

    // Building a REST API URL for Firestore
    String url = "https://firestore.googleapis.com/v1/projects/" + String(projectId) + "/databases/(default)/documents/" + String(collection) + "?key=" + apiKey;

    // Build JSON data
    StaticJsonDocument<200> doc;
    doc["fields"]["timestamp"]["timestampValue"] = dateTimeStr;  // 当前时间戳
    doc["fields"]["impactmagnitude"]["doubleValue"] = magnitude;  // 
    doc["fields"]["impact"]["booleanValue"] = Beinghit;  // 
    String jsonData;
    serializeJson(doc, jsonData);

    client.beginRequest();
    client.post(url);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", jsonData.length());
    client.beginBody();
    client.print(jsonData);
    client.endRequest();

    // Get the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    if (statusCode == 200) {
      Serial.println("Data sent successfully to Firestore");
    } else {
      Serial.print("Failed to send data, status code: ");
      Serial.println(statusCode);
      Serial.println("Response: " + response);
    }
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

