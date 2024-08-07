String receivedData = "";
bool readingAvg = false;

void setup() {
  Serial1.begin(9600); // 初始化Serial1串行通信，波特率为9600
  Serial.begin(9600);  // 初始化主串行通信，用于调试输出
}

void loop() {
  while (Serial1.available() > 0) {
    char incomingByte = Serial1.read();
    receivedData += incomingByte;

    if (receivedData.endsWith(" AVG_END")) {
      int avgStartIndex = receivedData.indexOf("AVG_START ");
      if (avgStartIndex != -1) {
        String avgValueStr = receivedData.substring(avgStartIndex + 10, receivedData.length() - 8);
        int avgValue = avgValueStr.toInt();
        Serial.print("Received Avg BPM: ");
        Serial.println(avgValue);
      }
      receivedData = ""; // 重置接收缓冲区
    }
  }
}
