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
    bs_change_direction,
    bs_speed_compensation,
    bs_fail_safe
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

  void apply()
  {
    float cur_speed = get_speed_meters();
    Motor::State st = m_motor.get_state();

    if(st == Motor::st_brake && m_brake_state == bs_change_direction)
    {
      if(cur_speed>OFF_SPEED)
        return;
      
      m_motor.soft_stop();
      cur_speed = 0.0;
      // Serial.println("after brake off");
    }

    if(st==Motor::st_forward && m_dst_speed<-OFF_SPEED ||
       st==Motor::st_backward && m_dst_speed>OFF_SPEED)
    {
      // Serial.println("brake");
      m_motor.brake();
      m_brake_state = bs_change_direction;
      reset_pid();
      return;
    }

    float ctl_speed=calc_pid(cur_speed);
    if(ctl_speed<-MAX_SPEED/2)
    {
      m_motor.brake();
      m_brake_state = bs_change_direction;
      reset_pid();
      return;
    }

    ctl_speed = constrain(ctl_speed, 0.0, MAX_SPEED);

    int pwm = ctl_speed*SPEED_TO_PWM;
    pwm = constrain(pwm, 0, Motor::MAX_PWM);

  
    // Serial.print("dst_speed=");
    // Serial.print(m_dst_speed);
    // Serial.print(" cur_speed=");
    // Serial.print(cur_speed);
    // Serial.print(" st=");
    // Serial.print(st);
    // Serial.print(" ctl_speed=");
    // Serial.print(ctl_speed);
    // Serial.print(" pwm=");
    // Serial.print(pwm);
    // Serial.println("");


    if(m_dst_speed<0)
      m_motor.backward(pwm);
    else
      m_motor.forward(pwm);
  }

  float calc_pid(float cur_speed)
  {
    float error = abs(m_dst_speed) - cur_speed;
    
    float proportional = PID_kp * error;

    m_pid_integral += error;
    float integral = PID_ki * m_pid_integral;
    if(integral>MAX_SPEED)
    {
      integral=MAX_SPEED;
      m_pid_integral=integral/PID_ki;
    }
    else if(integral<-MAX_SPEED)
    {
      integral=-MAX_SPEED; 
      m_pid_integral=integral/PID_ki;
    }
    
    float derivative = PID_kd * (error - m_pid_prev_error);
    float res = proportional + integral + derivative;

    // Serial.print("cur_speed=");
    // Serial.print(cur_speed);
    // Serial.print(" dst_speed=");
    // Serial.print(abs(m_dst_speed));
    // Serial.print(" res=");
    // Serial.print(res);
    // Serial.print(" err=");
    // Serial.print(error);
    // Serial.print(" prev_err=");
    // Serial.print(m_pid_prev_error);
    // Serial.print(" pid_integral=");
    // Serial.print(m_pid_integral);
    // Serial.print(" i=");
    // Serial.print(integral);
    // Serial.print(" p=");
    // Serial.print(proportional);
    // Serial.print(" d=");
    // Serial.print(derivative);
    // Serial.println("");

    m_pid_prev_error = error;
    
    return  res;
  }

  void reset_pid()
  {
    m_pid_integral = 0.0;
    m_pid_prev_error = 0.0;
  }

  void fail_safe()
  {
    m_dst_speed = 0.0;
    m_motor.brake();
    m_brake_state = bs_fail_safe;
    reset_pid();
  }

private:
  float m_dst_speed = 0.0;

  Motor m_motor;

  volatile unsigned m_timer_val=0;
  volatile unsigned m_periods[PERIODS_COUNT];
  volatile unsigned m_ticks_count = 0;

  float m_pid_integral = 0.0;
  float m_pid_prev_error = 0.0;
  BrakeState m_brake_state = bs_speed_compensation;
};
