const int pin_no = 18;

const int freq = 2;
const int ledChannel = 0;
const int resistor_pin = 34;

void setup()
{
//  Serial.begin(115200);
  
//  ledcSetup(ledChannel, freq, 10);
//  ledcAttachPin(pin_no, ledChannel);
//  ledcWrite(ledChannel, 8);
  pinMode(pin_no, OUTPUT);
  digitalWrite(pin_no, LOW);
  
}

void loop() 
{
  int ristorValue = analogRead(resistor_pin);

  int delay_value = map(ristorValue,0,4095,1,3000);
//  Serial.println(delay_value);

  delay(10000);
    
//  for(int i=0;i<3;i++)
//  {
    digitalWrite(pin_no, HIGH);
    delay(delay_value);
    digitalWrite(pin_no, LOW);
//    delay(1200);
//  }
}
