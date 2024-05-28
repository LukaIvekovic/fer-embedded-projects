#define RED_LED_PIN 14

void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  Serial.begin(115200);

  while (!Serial) {
    ;
  }

}

void loop() {
  digitalWrite(RED_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(RED_LED_PIN, LOW);
  delay(2000);
}
