#include <WiFiS3.h>
#include <Wire.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
// Define gas sensor
#include "DFRobot_MICS.h"
#define CALIBRATION_TIME   3
#define ADC_PIN   A0
#define POWER_PIN 2
DFRobot_MICS_ADC mics(/*adcPin*/ADC_PIN, /*powerPin*/POWER_PIN);
float gasdataC2H5OH = 0;
bool C2H5OHalarm= false;
float gasdataCO = 0;
bool CH4alarm= false;
float gasdataCH4 = 0;
bool COalarm= false;

#define FORCE_SENSOR_PIN A4 
float magnitude = 0;
bool Beinghit= false;
bool Wear= false;

// Define accelerometer sensor
const int xPin = A1;
const int yPin = A2;
const int zPin = A3;
int prevX = 0, prevY = 0, prevZ = 0;
int threshold = 200;  // Threshold

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const char* ssid = "mate40pro";
const char* password = "88888888";

// Firebase configuration
const char* host = "firestore.googleapis.com";
const char* apiKey = "AIzaSyDQLBZquq18ScMUz-CusPv1LZtTwV4dJxA";
const char* projectId = "fir-flutter-codelab-a80ae";
const char* collection1 = "Helmet-data-3in1"; // Collection name
const char* collection2 = "Helmet-data-impact"; 

// HTTP client
WiFiSSLClient wifiClient;
HttpClient client = HttpClient(wifiClient, host, 443);

//Defining time intervals
unsigned long previousMillis = 0;  
const long interval = 15000;       
unsigned long gaspreviousMillis = 0;    
const unsigned long gasinterval = 10000; 


String receivedData = "";
bool readingAvg = false;
int avgbpm;

int redPin = 9;
int greenPin = 10;
int bluePin = 11;
int buzzerPin = 13;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200); // Initialize the second serial port for communication with other Arduino

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to Wi-Fi");

  // Initialize NTP client
  timeClient.begin();

  // Initialize the gas sensor
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
    setColor(255, 255, 0);
    delay(1000);
  }

  // Initialize accelerometer sensor
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(zPin, INPUT);
  prevX = analogRead(xPin);
  prevY = analogRead(yPin);
  prevZ = analogRead(zPin);
}

void loop() {

  fsr(); // Detect helmet wearing status
  gas(); // Detect gas concentration
  impact(); // Detect impact events

  // Process the heart rate data received from the serial port
  while (Serial1.available() > 0) {
    char incomingByte = Serial1.read();
    receivedData += incomingByte;

    if (receivedData.endsWith(" AVG_END")) {
      int avgStartIndex = receivedData.indexOf("AVG_START ");
      if (avgStartIndex != -1) {
        String avgValueStr = receivedData.substring(avgStartIndex + 10, receivedData.length() - 8);
        avgbpm = avgValueStr.toInt();
        Serial.print("Received Avg BPM: ");
        Serial.println(avgbpm);
      }
      receivedData = ""; 
    }
  }
    // Check if data needs to be uploaded
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    timeClient.update();
    senddata();
  }
}

void gas(){
  // Read gas sensor data
  gasdataC2H5OH = mics.getGasData(C2H5OH);
  gasdataCO = mics.getGasData(CO);
  gasdataCH4 = mics.getGasData(CH4);
  // Print gas concentration information
  Serial.print("C2H5OH: ");
  Serial.print(gasdataC2H5OH,1);
  Serial.println(" PPM");

  Serial.print("CO: ");
  Serial.print(gasdataCO,1);
  Serial.println(" PPM");

  Serial.print("CH4: ");
  Serial.print(gasdataCH4,1);
  Serial.println(" PPM");
  if (gasdataC2H5OH >= 400 || C2H5OHalarm >= 50 || gasdataCH4 >= 5000) {
    setColor(255, 0, 0);
    tone(buzzerPin, 1000);
    unsigned long currentMillis = millis();
    if (currentMillis - gaspreviousMillis >= gasinterval) {
      gaspreviousMillis = currentMillis; 
      timeClient.update();
      senddata();
    }
  } else {
    setColor(0, 255, 0);
    noTone(buzzerPin);
  }

  if(gasdataC2H5OH >= 500 ){
    C2H5OHalarm= true;
  }else{
    C2H5OHalarm= false;
  }
  if(gasdataCO >= 50 ){
    CH4alarm= true;
  }else{
    CH4alarm= false;
  }
  if(gasdataCH4 >= 5000 ){
    COalarm= true;
  }else{
    COalarm= false;
  }
}

void impact(){
  // Read the current value of the acceleration sensor
  int x = analogRead(xPin);
  int y = analogRead(yPin);
  int z = analogRead(zPin);

  // Calculate the difference between the current value and the previous value
  int deltaX = abs(x - prevX);
  int deltaY = abs(y - prevY);
  int deltaZ = abs(z - prevZ);

  // Update the previous value of the accelerometer
  prevX = x;
  prevY = y;
  prevZ = z;

  Serial.print("Delta X: "); Serial.print(deltaX);
  Serial.print(" Delta Y: "); Serial.print(deltaY);
  Serial.print(" Delta Z: "); Serial.println(deltaZ);
  
  // Check whether the acceleration change exceeds the threshold
  if (deltaX > threshold || deltaY > threshold || deltaZ > threshold) {
    Serial.println("Impact detected!");
    Beinghit=true;
    timeClient.update();
    impactsenddata();
  }else{
    Beinghit=false;
  }
}

void fsr(){
  int analogReading = analogRead(FORCE_SENSOR_PIN);
  Serial.print("Force sensor reading = ");
  Serial.print(analogReading); 
  if (analogReading < 50){
    Serial.println(" -> no pressure");
    Wear = false;
  }else{
    Serial.println("Wearing detected");
    Wear = true;
  }      
    
    
}

void senddata() {
  if (WiFi.status() == WL_CONNECTED) {
    // Get current timestamp
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    setTime(epochTime);
    int timeOffset = 0;
    if (isDST(day(), month(), weekday())) {
      timeOffset = 0;  
    }
    timeClient.setTimeOffset(timeOffset);

    //Format timestamp
    char dateTimeStr[25];
    sprintf(dateTimeStr, "%04d-%02d-%02dT%02d:%02d:%02dZ", year(), month(), day(), hour(), minute(), second());

    // Build the REST API URL to send to Firebase
    String url = "https://firestore.googleapis.com/v1/projects/" + String(projectId) + "/databases/(default)/documents/" + String(collection1) + "?key=" + apiKey;

    // Build JSON data
    StaticJsonDocument<200> doc;
    doc["fields"]["timestamp"]["timestampValue"] = dateTimeStr;  
    doc["fields"]["fsr"]["booleanValue"] = Wear;  
    doc["fields"]["C2H5OH"]["doubleValue"] = gasdataC2H5OH;  
    doc["fields"]["C2H5OHalarm"]["booleanValue"] = C2H5OHalarm;  
    doc["fields"]["CH4"]["doubleValue"] = gasdataCH4;  /
    doc["fields"]["CH4alarm"]["booleanValue"] = CH4alarm;
    doc["fields"]["CO"]["doubleValue"] = gasdataCO;   
    doc["fields"]["COalarm"]["booleanValue"] = COalarm;
    doc["fields"]["avgbpm"]["integerValue"] = avgbpm;  
    String jsonData;
    serializeJson(doc, jsonData);

    // Send HTTP POST request
    client.beginRequest();
    client.post(url);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", jsonData.length());
    client.beginBody();
    client.print(jsonData);
    client.endRequest();

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

void impactsenddata() {
  if (WiFi.status() == WL_CONNECTED) {
    
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

    
    String url = "https://firestore.googleapis.com/v1/projects/" + String(projectId) + "/databases/(default)/documents/" + String(collection2) + "?key=" + apiKey;

   
    StaticJsonDocument<200> doc;
    doc["fields"]["timestamp"]["timestampValue"] = dateTimeStr;  
    doc["fields"]["impact"]["booleanValue"] = Beinghit;   
    String jsonData;
    serializeJson(doc, jsonData);

    client.beginRequest();
    client.post(url);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", jsonData.length());
    client.beginBody();
    client.print(jsonData);
    client.endRequest();

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
    if (month < 3 || month > 10) return false; 
    if (month > 3 && month < 10) return true; 
    int previousSunday = day - weekday;
    if (month == 3) return previousSunday >= 25;
    if (month == 10) return previousSunday < 25;
    return false; 
}


void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, 255 - redValue);
  analogWrite(greenPin, 255 - greenValue);
  analogWrite(bluePin, 255 - blueValue);
}

