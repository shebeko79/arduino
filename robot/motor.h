
class DcMotor
{
public:
  static const int MAX_PWM=255;
  enum State{st_stop,st_forward,st_backward};
  
  DcMotor(int in1,int in2,int pwm_channel):
    IN1(in1),IN2(in2),PWM_CHANNEL(pwm_channel),m_state(st_stop){}

  void init()
  {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    ledcSetup(PWM_CHANNEL, 2000, 8);
    soft_stop();
  }

  void soft_stop()
  {
    ledcDetachPin(IN1);
    ledcDetachPin(IN2);
 
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    
    m_state = st_stop;
  }

  void forward(int val=MAX_PWM)
  {
    if(m_state != st_forward)
    {
      soft_stop();
      ledcAttachPin(IN2, PWM_CHANNEL);
    }
    
    ledcWrite(PWM_CHANNEL, val);
    m_state = st_forward;
  }
  
  void backward(int val=MAX_PWM)
  {
    if(m_state != st_backward)
    {
      soft_stop();
      ledcAttachPin(IN1, PWM_CHANNEL);
    }
    
    ledcWrite(PWM_CHANNEL, val);
    m_state = st_backward;
  }
    
private:
  const int IN1;
  const int IN2;
  const int PWM_CHANNEL;
  State m_state;
};
