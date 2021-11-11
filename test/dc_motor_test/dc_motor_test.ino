
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

class StepperMotor
{
public:
  int step_delay;

  StepperMotor(int in1,int in2,int in3,int in4):
    IN1(in1),IN2(in2),IN3(in3),IN4(in4) 
  { 
    step_delay=5; 
    currentState = 0;
  }

  void init()
  {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
  }

  void move( int val = 1)
  {
    if(val == 0)
      return;
      
    int dstVal = currentState + val;
    
    int dt = val>0? 1:-1;
    
//    Serial.print("currentState=");
//    Serial.println(currentState);
//    Serial.print("dstVal=");
//    Serial.println(dstVal);
//    Serial.print("dt=");
//    Serial.println(dt);

    do 
    {
      currentState+=dt;
      step();
    }
    while(currentState!=dstVal);
    
  }

private:
  const int IN1;
  const int IN2;
  const int IN3;
  const int IN4;

  int currentState;

  void step()
  {
//    Serial.print("currentState%4=");
//    Serial.println(currentState);
    switch(currentState%4)
    {
    case 1:
    case -3:
      digitalWrite( IN1, HIGH ); 
      digitalWrite( IN2, HIGH ); 
      digitalWrite( IN3, LOW ); 
      digitalWrite( IN4, LOW );
//    Serial.println("step1");
      break;
    case 2:
    case -2:
      digitalWrite( IN1, LOW ); 
      digitalWrite( IN2, HIGH ); 
      digitalWrite( IN3, HIGH ); 
      digitalWrite( IN4, LOW );
//    Serial.println("step2");
      break;
    case 3:
    case -1:
      digitalWrite( IN1, LOW ); 
      digitalWrite( IN2, LOW ); 
      digitalWrite( IN3, HIGH ); 
      digitalWrite( IN4, HIGH );
//    Serial.println("step3");
      break;
    case 0:
      digitalWrite( IN1, HIGH ); 
      digitalWrite( IN2, LOW ); 
      digitalWrite( IN3, LOW ); 
      digitalWrite( IN4, HIGH );
//    Serial.println("step4");
      break;
    }
    
    delay(step_delay);
  }
  
};


//DcMotor dc(D1,D2,D3);
StepperMotor st(D0,D1,D2,D3);

void setup() 
{
  Serial.begin(9600);
//  dc.init();
  st.init();
}

void loop() 
{
  if(!Serial.available())
    return;

  delay(100);
   
  int len = Serial.available();

  static uint8_t sbuf[256];
    
  if(len>sizeof(sbuf)-1)
    len = sizeof(sbuf)-1;
      
  Serial.readBytes(sbuf, len);
  sbuf[len] = 0;

  if(sbuf[0] == 'm')
  {
    int cycleCount=atol((const char*)sbuf +1);
    Serial.print("cycleCount=");
    Serial.println(cycleCount);

    st.move(cycleCount);
    

/*
    if(sbuf[0] == 'f')
    {
      Serial.print("forward=");
      Serial.println(pwm_val);
      dc.forward(pwm_val);
    }
    else
    {
      Serial.print("backward=");
      Serial.println(pwm_val);
      dc.backward(pwm_val);
    }
*/    
  }
  else if(sbuf[0]=='d')
  {
    st.step_delay=atol((const char*)sbuf +1);
    Serial.print("delay=");
    Serial.println(st.step_delay);
  }
  
}
