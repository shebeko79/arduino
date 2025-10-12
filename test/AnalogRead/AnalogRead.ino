#define ADC_PIN 36

void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  
  //set the resolution to 12 bits (0-4096)
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
}

void readAdc(adc_attenuation_t attenuation, float a, float b)
{
  analogSetPinAttenuation(ADC_PIN, attenuation);
  
  int analogValue = analogRead(ADC_PIN);
  int analogVolts = analogReadMilliVolts(ADC_PIN);

  float vcc = analogValue*a+b;
  
  Serial.printf("ADC attenuation=%d value=%d  V=%dmV VCC=%.2f\n", (int)attenuation, analogValue, analogVolts, vcc);
}

float getVoltage()
{
  constexpr float x1=1347.0;
  constexpr float y1=25.0;
  constexpr float x2=747.0;
  constexpr float y2=15.0;

  constexpr float a=(y1-y2)/(x1-x2);
  constexpr float b=y1-a*x1;
  
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
  int v = analogRead(ADC_PIN);
  return v*a+b;
}

void loop()
{
  readAdc(ADC_11db,14.0/831.0,2.503);
  //readAdc(ADC_6db,14.0/1544.0,1.948);
  Serial.printf("vcc=%f\n\n",getVoltage());
  
  delay(1000);  // delay in between reads for clear read from serial
}
