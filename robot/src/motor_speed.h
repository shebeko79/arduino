#pragma once
#include "robot_defs.h"
#include "motor_zsx11h.h"

class MotorSpeed
{
public:
  static constexpr unsigned PERIODS_COUNT = 50;
  static constexpr float KPER_SEC=1000.0/(PERIODS_COUNT*TIMER_MS);

  typedef MotorZsx11h Motor;

  enum BrakeState
  {
    bs_zero_speed,
    bs_fail_safe,
    bs_change_direction,
    bs_speed_compensation
  };
  
public:
  MotorSpeed(const Motor& motor) : 
    m_motor(motor)
  {
  }

  void init()
  {
    m_motor.init();
  }

  void set_speed(float speed)
  {
    m_dst_speed = constrain(speed, -MAX_SPEED, MAX_SPEED);
  }

  void speed_pin_isr();
  void timer_isr(unsigned timer_val);
  
  inline float get_speed_ticks()const{return m_ticks_count*KPER_SEC;}
  inline float get_speed_meters()const{return m_ticks_count*KPER_SEC/WHEEL_PULSES_PER_METER;}

  void apply();
  
  void fail_safe();
  void dump_state(const String& caption);

private:
  float calc_pwm(float cur_speed);
  void reset_pid();
  
private:
  volatile float m_dst_speed = 0.0;

  Motor m_motor;

  volatile unsigned m_timer_val=0;
  volatile unsigned m_periods[PERIODS_COUNT];
  volatile unsigned m_ticks_count = 0;

  bool m_pid_active = 0.0;
  float m_pid_integral = 0.0;
  float m_pid_prev_error = 0.0;
  float m_prev_speed = 0.0;
  unsigned int m_prev_time = 0;
  float m_expected_delay = EXPECTED_PWM_DELAY;
  
  BrakeState m_brake_state = bs_zero_speed;
};
