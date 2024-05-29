#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200);
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
  //delay(1000);
}

// #include <Wire.h>
// #include "MAX30105.h"
// #include "heartRate.h"

// MAX30105 particleSensor;

// #define MAX_BRIGHTNESS 255

// void setup() {
//   Serial.begin(115200); // 打开串口通信

//   // 初始化传感器
//   if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
//     Serial.println("MAX30102 was not found. Please check wiring/power.");
//     while (1);
//   }

//   particleSensor.setup(); // 使用默认配置
//   particleSensor.setPulseAmplitudeRed(MAX_BRIGHTNESS); // 设置红光亮度
//   particleSensor.setPulseAmplitudeGreen(0); // 关闭绿光以省电
// }

// void loop() {
//   long irValue = particleSensor.getIR(); // 获取红外数据

//   if (checkForBeat(irValue) == true) {
//     static uint32_t lastBeat = 0; // 记录上次心跳的时间
//     uint32_t delta = millis() - lastBeat; // 计算两次心跳之间的时间差
//     lastBeat = millis();

//     float beatsPerMinute = 60.0 / (delta / 1000.0); // 计算心率
//     Serial.print("Heart Rate: ");
//     Serial.print(beatsPerMinute);
//     Serial.println(" BPM");
//   }

//   delay(100); // 延时100毫秒
// }
