const int xPin = A1;
const int yPin = A2;
const int zPin = A3;

int prevX = 0, prevY = 0, prevZ = 0;
int threshold = 200;  // 根据数据分析，设置阈值

void setup() {
  Serial.begin(9600);
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(zPin, INPUT);

  // 初始化先前的加速度值
  prevX = analogRead(xPin);
  prevY = analogRead(yPin);
  prevZ = analogRead(zPin);
}

void loop() {
  int x = analogRead(xPin);
  int y = analogRead(yPin);
  int z = analogRead(zPin);

  // 计算当前加速度值与先前值的差值
  int deltaX = abs(x - prevX);
  int deltaY = abs(y - prevY);
  int deltaZ = abs(z - prevZ);

  // 更新先前的加速度值
  prevX = x;
  prevY = y;
  prevZ = z;

  // 打印加速度传感器的变化量
  Serial.print("Delta X: "); Serial.print(deltaX);
  Serial.print(" Delta Y: "); Serial.print(deltaY);
  Serial.print(" Delta Z: "); Serial.println(deltaZ);
  
  // 检测加速度变化量是否超过阈值
  if (deltaX > threshold || deltaY > threshold || deltaZ > threshold) {
    Serial.println("撞击检测！");
  }
  
  delay(10);  // 调整读取频率
}

