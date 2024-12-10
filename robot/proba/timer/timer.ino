#include "Arduino.h"

constexpr unsigned timer_us=1000;
volatile int32_t timer_cycle=0;
hw_timer_t *Timer0_Cfg = nullptr;

void IRAM_ATTR Timer0_ISR()
{
  ++timer_cycle;
}

void setup()
{
  Serial.begin(115200);

  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, timer_us, true);
  timerAlarmEnable(Timer0_Cfg);
  //timerAlarmDisable(Timer0_Cfg);
}

void loop()
{
  delay(1000);
  Serial.print("timer_cycle=");
  Serial.print(timer_cycle);
  Serial.print(" cycles=");
  Serial.println(ESP.getCycleCount());
  
  
  timer_cycle=0;
}
