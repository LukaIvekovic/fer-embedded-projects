#include <Arduino.h>

int sensorVal;
const int ANALOG_READ_PIN = 36; // or A0
const int RESOLUTION = 12; // Could be 9-12
 
void setup()
{
  Serial.begin(115200);
  delay(10000);

  pinMode(ANALOG_READ_PIN, INPUT);
  Serial.println("Kaj se dogada");
}
 
void loop()
{\
  analogReadResolution(RESOLUTION);
  //Read and print the sensor pin value
  sensorVal = analogRead(ANALOG_READ_PIN);
  Serial.println(sensorVal);
  Serial.println("Kaj se dogada");
  //sleep for some time before next read
  delay(1000); 
}