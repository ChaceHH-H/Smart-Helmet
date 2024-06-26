#include "DFRobot_MICS.h"
#define CALIBRATION_TIME   3
#define ADC_PIN   A0
#define POWER_PIN 2
#define FORCE_SENSOR_PIN A7 

#include <Wire.h>



DFRobot_MICS_ADC mics(/*adcPin*/ADC_PIN, /*powerPin*/POWER_PIN);

const int xPin = A1;
const int yPin = A2;
const int zPin = A3;

const int sampleSize = 10; // 每次取样的数量
const float alpha = 0.5;   // 低通滤波的权重

float xFiltered = 0;
float yFiltered = 0;
float zFiltered = 0;


void setup() 
{
  Serial.begin(115200);
  while(!Serial);
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

}

void loop() 
{
  gas();
  impact();
  fsr();
  delay(100);
}

void gas(){
  float gasdataC2H5OH = mics.getGasData(C2H5OH);
  float gasdataCO = mics.getGasData(CO);
  float gasdataCH4 = mics.getGasData(CH4);
  float gasdataC3H8 = mics.getGasData(C3H8);
  Serial.print("C2H5OH: ");
  Serial.print(gasdataC2H5OH,1);
  Serial.println(" PPM");

  Serial.print("CO: ");
  Serial.print(gasdataCO,1);
  Serial.println(" PPM");

  Serial.print("CH4: ");
  Serial.print(gasdataCH4,1);
  Serial.println(" PPM");

  Serial.print("C3H8: ");
  Serial.print(gasdataC3H8,1);
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
  float magnitude = sqrt(sq(xFiltered - 512) + sq(yFiltered - 512) + sq(zFiltered - 512));
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
  if (analogReading < 10)       // from 0 to 9
    Serial.println(" -> no pressure");
  else if (analogReading < 200) // from 10 to 199
    Serial.println(" -> light touch");
  else if (analogReading < 500) // from 200 to 499
    Serial.println(" -> light squeeze");
  else if (analogReading < 800) // from 500 to 799
    Serial.println(" -> medium squeeze");
  else // from 800 to 1023
    Serial.println(" -> big squeeze");
}


