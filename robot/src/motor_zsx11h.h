#pragma once
#include <Arduino.h>

class MotorZsx11h
{
public:
  static const int MAX_PWM=255;
  enum State{st_stop,st_forward,st_backward};
  
  MotorZsx11h(int pwm,int dir,int pwm_channel,bool dir_forward):
    PWM(pwm),DIR(dir),PWM_CHANNEL(pwm_channel),DIR_FORWARD(dir_forward), m_state(st_stop){}

  void init()
  {
    pinMode(PWM, OUTPUT);
    digitalWrite(PWM, LOW);

    pinMode(DIR, OUTPUT);
    digitalWrite(DIR, DIR_FORWARD);

    ledcSetup(PWM_CHANNEL, 2000, 8);
    soft_stop();
  }

  void soft_stop()
  {
    ledcDetachPin(PWM);
    digitalWrite(PWM, LOW);
    
    m_state = st_stop;
  }

  void forward(int val=MAX_PWM)
  {
    if(m_state != st_forward)
    {
      soft_stop();
      digitalWrite(DIR, DIR_FORWARD);
      ledcAttachPin(PWM, PWM_CHANNEL);
    }
    
    ledcWrite(PWM_CHANNEL, val);
    m_state = st_forward;
  }
  
  void backward(int val=MAX_PWM)
  {
    if(m_state != st_backward)
    {
      soft_stop();
      digitalWrite(DIR, !DIR_FORWARD);
      ledcAttachPin(PWM, PWM_CHANNEL);
    }
    
    ledcWrite(PWM_CHANNEL, val);
    m_state = st_backward;
  }

private:
  const int PWM;
  const int DIR;
  const int PWM_CHANNEL;
  const int DIR_FORWARD;
  State m_state;
};
