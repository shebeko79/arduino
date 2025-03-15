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

  bool is_brake = false;
  float ctl_pwm=calc_pwm(cur_speed,is_brake);
  if(is_brake)
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

float MotorSpeed::calc_pwm(float cur_speed, bool &is_brake)
{
  float dst_speed = abs(m_dst_speed);
  unsigned long cur_time = millis();
  float cur_acc = 0.0;

  float res;
  float kick = 0.0;

  if(m_prev_steps < 2)
  {
    res = m_speed2pwm * dst_speed;
    ++m_prev_steps;
  }
  else
  {
    float cur_delay;
    if(cur_time == m_prev_time)
      cur_delay=EXPECTED_PWM_DELAY;
    else
      cur_delay = (cur_time - m_prev_time)/1000.0;
    
    cur_acc = (cur_speed-m_prev_speed)/cur_delay;
    const float dx = (cur_speed<=dst_speed)? 1.0:-1.0;
    const float speed_correction = dx>0? UP_SPEED_CORRECTION:DOWN_SPEED_CORRECTION;

    res = m_prev_pwm;

    if(cur_acc*dx<0.0 || cur_acc*dx<CLOSE_TO_ZERO_ACCELERATION)
    {
      res = m_prev_pwm+(dst_speed-cur_speed)*speed_correction;
    }

    if(dx>0)
    {
      if(m_prev_acc == 0.0 && cur_acc==0.0 && cur_speed == 0.0 && dst_speed>CLOSE_TO_ZERO_SPEED_DIFF)
         kick = KICK_PWM_CORRECTION;
    }
    else
    {
      if(cur_acc>CLOSE_TO_ZERO_ACCELERATION && (cur_speed-dst_speed)>BREAK_MAX_SPEED_DIFF_WITH_ACCELERATION || (cur_speed-dst_speed)>BREAK_MAX_SPEED_DIFF)
      {
        is_brake=true;
        res -= BREAK_PWM_CORRECTION;
      }
    }

    res = constrain(res, 0.0, 1.0);
    

    // Serial.print(" dst_sp=");
    // Serial.print(m_dst_speed,4);
    // Serial.print(" cur_sp=");
    // Serial.print(cur_speed,4);
    // Serial.print(" acc=");
    // Serial.print(cur_acc,4);
    // Serial.print(" prev_acc=");
    // Serial.print(m_prev_acc,4);
    // Serial.print(" new_pwm=");
    // Serial.print(res,4);
    // Serial.print(" prev_pwm=");
    // Serial.print(m_prev_pwm,4);
    // Serial.print(" speed2pwm=");
    // Serial.print(m_speed2pwm,4);
    // Serial.println("");
  }

  m_prev_speed=cur_speed;
  m_prev_acc = cur_acc;
  m_prev_time=cur_time;
  m_prev_pwm = res;

  res += kick;
  res = constrain(res, 0.0, 1.0);
  
  return  res;
}

void MotorSpeed::reset_pid()
{
  m_prev_steps = 0;
  m_prev_speed = 0.0;
  m_prev_time = 0;
  m_prev_acc = 0.0;
  m_prev_pwm = 0.0;
}

void MotorSpeed::fail_safe()
{
  m_dst_speed = 0.0;
  m_motor.brake();
  m_brake_state = bs_fail_safe;
  reset_pid();
}

void MotorSpeed::dump_state(const String& caption, Stream& stream)
{
  float cur_speed = get_speed_meters();
  Motor::State st = m_motor.get_state();

  if(st == Motor::st_brake && cur_speed == 0.0)
  {
    return;
  }

  stream.print(caption);
  stream.print(":");
  stream.print(" time=");
  stream.print(millis());
  stream.print(" dst_speed=");
  stream.print(abs(m_dst_speed),4);
  stream.print(" cur_speed=");
  stream.print(cur_speed,4);
  stream.print(" PWM=");
  stream.print(m_prev_pwm,4);
  stream.print(" speed2pwm=");
  stream.print(m_speed2pwm,4);
  stream.println("");
}
