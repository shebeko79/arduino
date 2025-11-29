constexpr int VCC_CUT_PIN = 15;
constexpr int V5_CUT_PIN = 19;
constexpr int V12_CUT_PIN = 4;
constexpr int V12_CUT2_PIN = 5;

constexpr int ML_PWM_PIN = 26;
constexpr int ML_DIR_PIN = 27;
constexpr int ML_STOP_PIN = 25;
constexpr int ML_A = 34;
constexpr int ML_B = 39;
constexpr int ML_C = 18;

constexpr int MR_PWM_PIN = 12;
constexpr int MR_DIR_PIN = 13;
constexpr int MR_STOP_PIN = 14;
constexpr int MR_A = 33;
constexpr int MR_B = 32;
constexpr int MR_C = 35;

constexpr int VCC_ADC_PIN = 36;

void enablePowerModules(bool power_on)
{
  int val = power_on? HIGH: LOW;
  digitalWrite(VCC_CUT_PIN, val);
  digitalWrite(V5_CUT_PIN, val);
  digitalWrite(V12_CUT_PIN, val);
  digitalWrite(V12_CUT2_PIN, val);
}

void setupPowerPins()
{
  enablePowerModules(false);
  
  pinMode(VCC_CUT_PIN, OUTPUT);
  pinMode(V5_CUT_PIN, OUTPUT);
  pinMode(V12_CUT_PIN, OUTPUT);
  pinMode(V12_CUT2_PIN, OUTPUT);
}

void allOtherPinsToInputState()
{
  pinMode(ML_PWM_PIN, INPUT);
  pinMode(ML_DIR_PIN, INPUT);
  pinMode(ML_STOP_PIN, INPUT);
  pinMode(ML_A, INPUT);
  pinMode(ML_B, INPUT);
  pinMode(ML_C, INPUT);

  pinMode(MR_PWM_PIN, INPUT);
  pinMode(MR_DIR_PIN, INPUT);
  pinMode(MR_STOP_PIN, INPUT);
  pinMode(MR_A, INPUT);
  pinMode(MR_B, INPUT);
  pinMode(MR_C, INPUT);

  pinMode(VCC_ADC_PIN, INPUT);
}

void setup() {
  allOtherPinsToInputState();
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  setupPowerPins();
  
  //set the resolution to 12 bits (0-4096)
  analogReadResolution(12);
  analogSetPinAttenuation(VCC_ADC_PIN, ADC_11db);

  enablePowerModules(true);
}

void readAdc(adc_attenuation_t attenuation, float a, float b)
{
  //analogSetPinAttenuation(VCC_ADC_PIN, attenuation);
  
  int analogValue = analogRead(VCC_ADC_PIN);
  int analogVolts = analogReadMilliVolts(VCC_ADC_PIN);

  float vcc = analogValue*a+b;
  
  Serial.printf("ADC attenuation=%d value=%d  V=%dmV VCC=%.2f\n", (int)attenuation, analogValue, analogVolts, vcc);
}

float getVoltage()
{
  constexpr float x1=1333.0;
  constexpr float y1=25.0;
  constexpr float x2=739.0;
  constexpr float y2=15.0;

  constexpr float a=(y1-y2)/(x1-x2);
  constexpr float b=y1-a*x1;
  
  //analogSetPinAttenuation(VCC_ADC_PIN, ADC_11db);
  int v = analogRead(VCC_ADC_PIN);
  return v*a+b;
}

void loop()
{
  //readAdc(ADC_11db,14.0/831.0,2.503);
  Serial.printf("vcc=%f\n\n",getVoltage());
  
  delay(1000);  // delay in between reads for clear read from serial
}
