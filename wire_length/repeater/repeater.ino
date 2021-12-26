
const int sigOutPin = 14;
const int sigInPin = 12;

const uint32_t sigOutPinMask = ((uint32_t)1)<<sigOutPin;
const uint32_t sigInMask = ((uint32_t)1)<<sigInPin;

void setup()
{
  pinMode(sigInPin, INPUT_PULLDOWN);
  pinMode(sigOutPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  noInterrupts();
  digitalWrite(LED_BUILTIN, HIGH);
  
  int val;
  while(true)
  {
//    val=digitalRead(sigInPin); 
//    digitalWrite(sigOutPin, val);
    if(GPIO.in & sigInMask)
    {
      GPIO.out_w1ts = sigOutPinMask;
    }
    else
    {
      GPIO.out_w1tc = sigOutPinMask;
    }
  }
}
