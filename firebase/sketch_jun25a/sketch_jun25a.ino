#include <WiFiNINA.h>
#include <WiFiSSLClient.h>
#include <Arduino_JSON.h>   // Arduino JSON 库

char ssid[] = "mate40pro";
char pass[] = "88888888";

// Firebase 数据库参数
#define FIREBASE_HOST "https://fir-flutter-codelab-a80ae.firestore.app" // 替换为你的 Firestore 数据库 URL
#define API_KEY "AIzaSyDQLBZquq18ScMUz-CusPv1LZtTwV4dJxA"                           // 替换为你的 Firebase Web API key

WiFiSSLClient client;


void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  sendFirebaseData();
}

void loop() {
  // 这里可以添加其他逻辑
}

void sendFirebaseData() {
  bool booleanValue = true;
  int intValue = 123;

  // 构建要发送的数据
  JSONVar data;
  data["booleanValue"] = booleanValue;
  data["intValue"] = intValue;

  // 将数据转换为 JSON 字符串
  String jsonString = JSON.stringify(data);

  // 创建 HTTP 请求
  String path = "//Helmet-data-uno.json"; // 替换为你的集合路径
  String url = "https://" + String(FIREBASE_HOST) + path + "?auth=" + API_KEY;

  Serial.print("Sending data to: ");
  Serial.println(url);

  // 发送 POST 请求到 Firebase
  if (client.connect(FIREBASE_HOST, 443)) {
    client.println("POST " + path + " HTTP/1.1");
    client.println("Host: " + String(FIREBASE_HOST));
    client.println("Content-Type: application/json");
    client.print("Authorization: Bearer ");
    client.println(API_KEY);
    client.print("Content-Length: ");
    client.println(jsonString.length());
    client.println("Connection: close");
    client.println();
    client.println(jsonString);
  } else {
    Serial.println("Connection failed");
    return;
  }

  // 等待响应
  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.println(line);
    }
  }

  client.stop();
}