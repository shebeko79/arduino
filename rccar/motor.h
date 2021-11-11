
class DcMotor
{
public:
  static const int MAX_PWM=1023;
  
  DcMotor(int in1,int in2,int pwm):
    IN1(in1),IN2(in2),PWM(pwm){}

  void init()
  {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(PWM, OUTPUT);
    soft_stop();
  }

  void soft_stop()
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(PWM, LOW);
  }

  void forward(int val=MAX_PWM)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(PWM, val);
  }
  
  void backward(int val=MAX_PWM)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(PWM, val);
  }
    
private:
  const int IN1;
  const int IN2;
  const int PWM;
};
