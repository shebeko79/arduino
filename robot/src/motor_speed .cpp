#include "motor_speed.h"

void MotorSpeed::speed_pin_isr()
{
  unsigned t = m_timer_val;
  ++m_periods[t%PERIODS_COUNT];
  ++m_ticks_count;
}

void MotorSpeed::timer_isr(unsigned timer_val)
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

void MotorSpeed::apply()
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

  if(m_dst_speed==0.0)
  {
    m_motor.brake();
    m_brake_state = bs_zero_speed;
    reset_pid();
    return;
  }

  float ctl_pwm=calc_pwm(cur_speed);
  if(ctl_pwm<-0.5)
  {
    m_motor.brake();
    m_brake_state = bs_speed_compensation;
    return;
  }

  ctl_pwm = constrain(ctl_pwm, 0.0, 1.0);

  int pwm = ctl_pwm*Motor::MAX_PWM;
  pwm = constrain(pwm, 0, Motor::MAX_PWM);

  if(m_dst_speed<0)
    m_motor.backward(pwm);
  else
    m_motor.forward(pwm);
}

float MotorSpeed::calc_pwm(float cur_speed)
{
  unsigned long cur_time = millis();

  float cur_delay;
  if(!m_pid_active)
    cur_delay = m_expected_delay;
  else if(cur_time == m_prev_time)
    cur_delay=1;
  else
    cur_delay = (cur_time - m_prev_time)/1000.0;
    

  float expected_speed = abs(m_dst_speed);
  float cur_acceleration = (cur_speed-m_prev_speed)/cur_delay;
  if(abs(cur_acceleration) > MAX_ACCELERATION)
  {
    expected_speed = cur_speed + (cur_acceleration>0? 1:-1)*MAX_ACCELERATION*cur_delay;
  }
  

  float constant = expected_speed/MAX_SPEED;
  float error = expected_speed - cur_speed;
  float res;

  if(!m_pid_active)
  {
    res = constant;
  }
  else
  {
    float proportional = PID_kp * error;

    m_pid_integral += error * cur_delay;
    float integral = PID_ki * m_pid_integral;
    
    float derivative = PID_kd * (error - m_pid_prev_error) / cur_delay;
    res = constant + proportional + integral + derivative;

    if(abs(res) > 1.0)
    {
      res = res>0.0? 1.0 : -1.0;
      integral=res - proportional - derivative - constant;
      m_pid_integral=integral/PID_ki;
    }

    // Serial.print(" dst_sp=");
    // Serial.print(m_dst_speed);
    // Serial.print(" acc=");
    // Serial.print(cur_acceleration);
    // Serial.print(" exp_sp=");
    // Serial.print(expected_speed);
    // Serial.print(" cur_sp=");
    // Serial.print(cur_speed);
    // Serial.print(" err=");
    // Serial.print(error);
    // Serial.print(" res=");
    // Serial.print(res);
    // Serial.print(" PIDi=");
    // Serial.print(m_pid_integral);
    // Serial.print(" delay=");
    // Serial.print(cur_delay);
    // Serial.print(" p=");
    // Serial.print(proportional);
    // Serial.print(" i=");
    // Serial.print(integral);
    // Serial.print(" d=");
    // Serial.print(derivative);
    // Serial.println("");
  }

  m_pid_active = true;
  m_pid_prev_error = error;
  m_prev_speed = cur_speed;
  m_prev_time = cur_time;
  m_expected_delay = cur_delay;
  
  return  res;
}

void MotorSpeed::reset_pid()
{
  m_pid_active = false;
  m_pid_integral = 0.0;
  m_pid_prev_error = 0.0;
  m_prev_speed = 0.0;
}

void MotorSpeed::fail_safe()
{
  m_dst_speed = 0.0;
  m_motor.brake();
  m_brake_state = bs_fail_safe;
  reset_pid();
}

void MotorSpeed::dump_state(const String& caption)
{
  float cur_speed = get_speed_meters();
  Motor::State st = m_motor.get_state();

  Serial.print(caption);
  Serial.print(":");
  Serial.print(" dst_speed=");
  Serial.print(m_dst_speed);
  Serial.print(" cur_speed=");
  Serial.print(cur_speed);
  Serial.print(" st=");
  Serial.print(st);
  Serial.print(" brake_st=");
  Serial.print(m_brake_state);
  Serial.print(" PIDi=");
  Serial.print(m_pid_integral);
  Serial.print(" PIDe=");
  Serial.print(m_pid_prev_error);
  Serial.println("");
}
