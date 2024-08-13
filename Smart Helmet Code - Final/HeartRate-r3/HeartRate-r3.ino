#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
// Create a MAX30105 sensor object to measure heart rate
MAX30105 particleSensor;

const byte RATE_SIZE = 8; 
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0; 

float beatsPerMinute;
int beatAvg;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) 
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0); 
}

void loop()
{
  // Read the value of the infrared sensor
  long irValue = particleSensor.getIR();
  // Check if a new heartbeat is detected
  // Calculate the time difference between the current heartbeat and the previous heartbeat
  if (checkForBeat(irValue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    // Calculate the number of heartbeats per minute
    beatsPerMinute = 60 / (delta / 1000.0);
    // Calculate average heart rate
    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; 
      rateSpot %= RATE_SIZE; 
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
  Serial.print(", AVG_START "); 
  Serial.print(beatAvg); 
  Serial.print(" AVG_END"); 

  if (irValue < 50000)
    Serial.print(" No finger?");
    
  Serial.println();
}