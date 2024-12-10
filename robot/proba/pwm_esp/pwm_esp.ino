#include "Arduino.h"
#include <esp_adc_cal.h>

int PWM=12;
int DIR=13;
int PWM_CHANNEL=0;
int TACHO_PIN=14;

int32_t ticks=0;

constexpr float ticks2speed=6.5*0.0254*M_PI/90.0;

void IRAM_ATTR pin_isr()
{
  ++ticks;
}

void setup()
{
  Serial.begin(115200);
  
  pinMode(PWM, OUTPUT);
  pinMode(DIR, OUTPUT);
  
  digitalWrite(PWM, LOW);
  
  ledcSetup(PWM_CHANNEL, 2000, 8);
  ledcAttachPin(PWM, PWM_CHANNEL);
  
  attachInterrupt(digitalPinToInterrupt(TACHO_PIN), pin_isr, CHANGE);
  
  ledcWrite(PWM_CHANNEL, 0);
  delay(10000);
  ticks=0;
  ledcWrite(PWM_CHANNEL, 20);
}

void print_ticks(int pwm)
{
  ticks=0;
  delay(1000);
  int tk=ticks;

  Serial.print("pwm=");
  Serial.print(pwm);
  Serial.print(" tick=");
  Serial.println(tk);
}

void cycle()
{
  for(int i=0;i<255;i+=5)
  {
    ledcWrite(PWM_CHANNEL, i);
    print_ticks(i);
  }

  delay(5000);

  for(int i=0;i<255;i+=5)
  {
    ledcWrite(PWM_CHANNEL, 255-i);
    print_ticks(255-i);
  }
}

void loop()
{
  Serial.println("dir=high");
  digitalWrite(DIR, HIGH);
  cycle();

  Serial.println("dir=low");
  digitalWrite(DIR, LOW);
  cycle();
}
