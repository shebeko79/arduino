#include "Arduino.h"
#include <esp_adc_cal.h>

int PWM=12;
int DIR=13;
int PWM_CHANNEL=0;
int TACHO_PIN=14;

int32_t ticks=0;
int32_t printed_ticks=0;


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

void print_ticks()
{
  if(printed_ticks == ticks)
    return;

  printed_ticks = ticks;

  Serial.print("tick=");
  Serial.println(printed_ticks);
}

void cycle()
{
  for(int i=0;i<255;i++)
  {
    ledcWrite(PWM_CHANNEL, i);
    delay(20);
  }

  delay(5000);

  for(int i=0;i<255;i++)
  {
    ledcWrite(PWM_CHANNEL, 255-i);
    delay(20);
  }
}

void loop()
{
  print_ticks();
  if(ticks>90*100)
    ledcWrite(PWM_CHANNEL, 0);
  
  
  digitalWrite(DIR, HIGH);
//  cycle();

  digitalWrite(DIR, LOW);
//  cycle();
}
