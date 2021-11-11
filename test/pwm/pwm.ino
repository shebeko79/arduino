void setup() {
  analogWriteRange(1023);
  analogWriteFreq(1000);
  digitalWrite(D3, LOW);
}

void loop() {
  analogWrite(D3, 512);
}
