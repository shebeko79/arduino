#pragma once
#include <Arduino.h>

class MotorZsx11h
{
public:
  static const int MAX_PWM=255;
  enum State{st_off,st_forward,st_backward,st_brake};
  
  MotorZsx11h(int pwm,int dir,int pwm_channel, int stop,bool dir_forward):
    PWM(pwm),DIR(dir),PWM_CHANNEL(pwm_channel),STOP(stop),
    DIR_FORWARD(dir_forward), m_state(st_off)
    {}

  void init()
  {
    pinMode(STOP, OUTPUT);
    digitalWrite(STOP, HIGH);

    ledcSetup(PWM_CHANNEL, 20000, 8);
    ledcDetachPin(PWM);
    digitalWrite(PWM, LOW);
    pinMode(PWM, OUTPUT);

    pinMode(DIR, OUTPUT);
    digitalWrite(DIR, DIR_FORWARD);

    soft_stop();
  }

  void soft_stop()
  {
    ledcDetachPin(PWM);
    digitalWrite(PWM, LOW);
    digitalWrite(STOP, LOW);
    
    m_state = st_off;
  }

  void brake()
  {
    ledcDetachPin(PWM);
    digitalWrite(PWM, LOW);
    digitalWrite(STOP, HIGH);
    
    m_state = st_brake;
  }

  void forward(int val=MAX_PWM)
  {
    if(m_state != st_forward)
    {
      soft_stop();
      digitalWrite(STOP, LOW);
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
      digitalWrite(STOP, LOW);
      digitalWrite(DIR, !DIR_FORWARD);
      ledcAttachPin(PWM, PWM_CHANNEL);
    }
    
    ledcWrite(PWM_CHANNEL, val);
    m_state = st_backward;
  }

  inline State get_state() const{return m_state;}
  
private:
  const int PWM;
  const int DIR;
  const int PWM_CHANNEL;
  const int STOP;

  const bool DIR_FORWARD;
  State m_state;
};
