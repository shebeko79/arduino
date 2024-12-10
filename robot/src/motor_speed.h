#pragma once
#include "robot_defs.h"
#include "motor_zsx11h.h"

class MotorSpeed
{
  static constexpr unsigned PERIODS_COUNT = 50;
  static constexpr float KPER_SEC=1000.0/(PERIODS_COUNT*TIMER_MS);

  
public:
  MotorSpeed(const MotorZsx11h& motor) : 
    m_motor(motor)
  {
  }

  void init()
  {
    m_motor.init();
  }

  void set_speed(float speed)
  {
    m_dst_speed = max(-MAX_SPEED, min(MAX_SPEED, speed));
  }

  
  void apply()
  {

  }

  void speed_pin_isr()
  {
    unsigned t = m_timer_val;
    ++m_periods[t%PERIODS_COUNT];
    ++m_ticks_count;
  }

  void timer_isr(unsigned timer_val)
  {
    unsigned new_pi=timer_val%PERIODS_COUNT;
    unsigned old_pi=m_timer_val%PERIODS_COUNT;

    if(new_pi != old_pi)
    {
      m_ticks_count-=m_periods[new_pi];
      m_periods[new_pi] = 0;
    }

    m_timer_val = timer_val;
  }

  inline float get_speed_ticks()const{return m_ticks_count*KPER_SEC;}
  inline float get_speed_meters()const{return m_ticks_count*KPER_SEC/WHEEL_PULSES_PER_METER;}

private:
  float m_dst_speed = 0.0;

  MotorZsx11h m_motor;

  volatile unsigned m_timer_val=0;
  volatile unsigned m_periods[PERIODS_COUNT];
  volatile unsigned m_ticks_count = 0;
};
