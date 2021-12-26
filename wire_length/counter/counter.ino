
const int sigOutPin = 12;
const int sigInPin = 14;

const uint32_t sigOutPinMask = ((uint32_t)1)<<sigOutPin;
const uint32_t sigInMask = ((uint32_t)1)<<sigInPin;

void setup()
{
  Serial.begin(115200);

  pinMode(sigInPin, INPUT_PULLDOWN);
  pinMode(sigOutPin, OUTPUT);
}

void loop()
{
  static int cycles = 0;

//  digitalWrite(sigOutPin, HIGH);
  GPIO.out_w1ts = sigOutPinMask;
  uint32_t cc = ESP.getCycleCount();

//  while(!digitalRead(sigInPin))
  while(!(GPIO.in & sigInMask))
  {
  }

  cc = ESP.getCycleCount() - cc;

  Serial.print(cycles);
  Serial.print(": cc=");
  Serial.println(cc);
//  Serial.print(" i=");
//  Serial.println(i);

  digitalWrite(sigOutPin, LOW);
  cycles++;

  delay(5000);
}
