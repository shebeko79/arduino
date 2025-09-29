#define CTL_OUTPUT 15

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CTL_OUTPUT, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(CTL_OUTPUT, LOW);
  delay(60000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(CTL_OUTPUT, HIGH);
  delay(10000);
}
