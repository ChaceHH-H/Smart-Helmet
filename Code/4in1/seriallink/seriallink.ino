void setup() {
  Serial.begin(115200);  // 初始化串口通信，波特率为9600
}

void loop() {
  if (Serial.available() >= sizeof(int)) {  // 检查是否有足够的数据可读
    int beatAvg;
    Serial.readBytes((char*)&beatAvg, sizeof(beatAvg));  // 读取传感器数据
    // 在此处处理数据，例如，打印到串口监视器
    Serial.print("Received sensor value: ");
    Serial.println(beatAvg);
  }
  //delay(5000);
}