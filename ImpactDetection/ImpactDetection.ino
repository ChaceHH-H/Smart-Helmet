const int xPin = A0;
const int yPin = A1;
const int zPin = A2;

const int sampleSize = 10; // 每次取样的数量
const float alpha = 0.5;   // 低通滤波的权重

float xFiltered = 0;
float yFiltered = 0;
float zFiltered = 0;

void setup() {
  Serial.begin(9600);
  xFiltered = analogRead(xPin);
  yFiltered = analogRead(yPin);
  zFiltered = analogRead(zPin);
}

void loop() {
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

  delay(100); // 每100ms检测一次
}

