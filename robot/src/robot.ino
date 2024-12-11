//Board ESP32-WROOM-DA Module
#include "motor_speed.h"
#include <BluetoothSerial.h>

static const int FAIL_SAFE_DELAY = 2000;
#define DEVICE_NAME "rccar"
static const int portNumber = 1500;

const char *bluetooth_pin = "1234";

MotorZsx11h leftWheel(12,13,0,27,true);
MotorZsx11h rightWheel(25,26,1,32,false);

MotorSpeed leftSpeed(leftWheel);
MotorSpeed rightSpeed(rightWheel);

int dst_left_engine = 0;
int cur_left_engine = 0;
int dst_right_engine = 0;
int cur_right_engine = 0;
unsigned long last_ms = 0;
uint8_t sbuf[256];
bool auto_cmd_blocked = false;

char incomingUdpPacket[256];
bool processCommand(const char* buf);

BluetoothSerial SerialBT;
#define SerialAuto Serial2

hw_timer_t *timer0 = nullptr;
unsigned timer0_count=0;


void IRAM_ATTR Timer0_ISR()
{
  ++timer0_count;
  leftSpeed.timer_isr(timer0_count);
  rightSpeed.timer_isr(timer0_count);
}

void IRAM_ATTR left_tick_isr()
{
  leftSpeed.speed_pin_isr();
}

void IRAM_ATTR right_tick_isr()
{
  rightSpeed.speed_pin_isr();
}

void setup() 
{
  Serial.begin(115200);
  SerialAuto.begin(115200);

  SerialBT.begin(DEVICE_NAME); //Bluetooth device name
  SerialBT.setPin(bluetooth_pin);

  leftWheel.init();
  rightWheel.init();

  attachInterrupt(digitalPinToInterrupt(14), left_tick_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(33), right_tick_isr, CHANGE);

  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &Timer0_ISR, true);
  timerAlarmWrite(timer0, TIMER_MS*1000, true);
  timerAlarmEnable(timer0);
}

template<typename T>
void processStream(T& stream, const char* caption,bool blocked)
{
  if (!stream.available())
    return;


  unsigned long timeout = millis() + 1000;

  for(int i = 0 ; timeout>millis();)
  {
    int len = stream.available();
    
    if(len == 0)
    {
      delay(100);
      continue;
    }
    
    if(i + len>sizeof(sbuf)-1)
      len = sizeof(sbuf)-1 - i;
    
    stream.readBytes(sbuf + i, len);
    
    sbuf[i+len] = 0;
    i+=len;

    if(strchr((const char*)sbuf,'\r') != nullptr)
      break;
  }

  if(!blocked)
  {
    Serial.print(caption);
    Serial.print(": ");
    Serial.println((const char*)sbuf);
  }

  char* cur = (char*)sbuf;
  for(char* next = strchr(cur, '\r'); next!=nullptr;)
  {
    *next = 0;

    if(blocked)
    {
      stream.print("reject:cmd:" DEVICE_NAME ":drive\r");
    }
    else if(processCommand(cur))
    {
      stream.print("accept:cmd:" DEVICE_NAME ":drive\r");
    }

    cur=next+1;
    next= strchr(cur, '\r');
  }
}

bool processDriveCommand(const char* buf)
{
  buf += sizeof("cmd:" DEVICE_NAME ":drive:")-1;
  
  int left_val=atol(buf);
  if(left_val<-255 || left_val>255)
    return false;

  const char* cdiv = strstr (buf,";");
  if(!cdiv)
    return false;
  
  int right_val=atol(cdiv + 1);
  if(right_val<-255 || right_val>255)
    return false;

  dst_left_engine = left_val;
  dst_right_engine = right_val;
  last_ms = millis();

  return true;
}

bool processBlockCommand(const char* buf)
{
  buf += sizeof("cmd:" DEVICE_NAME ":block:")-1;

  auto_cmd_blocked=atol(buf)!=0;

  return true;
}

bool processCommand(const char* buf)
{
  const char* pattern = strstr(buf, "cmd:" DEVICE_NAME ":drive:");
  if(pattern)
    return processDriveCommand(pattern);

  pattern = strstr(buf, "cmd:" DEVICE_NAME ":block:");
  if(pattern)
    return processBlockCommand(pattern);

  return false;
}

void failSafe()
{
  if(last_ms + FAIL_SAFE_DELAY < millis())
  {
    dst_left_engine = 0;
    dst_right_engine = 0;
  }
}

template<typename Motor>
void apply(Motor& dc, int dst,int& cur)
{
  if(cur == dst)
    return;
    
  if(dst == 0)
  {
//Serial.println("apply()1 stop");
    cur = 0;
    dc.soft_stop();
    return;
  }
    
  if(cur !=0 &&  (dst>0) != (cur>0) )
  {
//Serial.println("apply()2 stop for reverse");
    cur = 0;
    dc.soft_stop();
  }

  int step;
  if(dst>0)step=100;
  else step=-100;

  cur+=step;
  if(abs(cur) > abs(dst))
    cur = dst;


//Serial.print("apply()5 dst=");
//Serial.print(dst);
//Serial.print(" cur=");
//Serial.println(cur);

  if(cur>0)dc.forward(cur);
  else dc.backward(-cur);
}

void loop()
{
  unsigned long prev_last_ms = last_ms;
  int prev_dst_left_engine = dst_left_engine;
  int prev_dst_right_engine = dst_right_engine;

  processStream(SerialBT,"BT",false);
  
  if(!auto_cmd_blocked && last_ms != prev_last_ms)
  {
    if(abs(dst_left_engine)>10 || abs(dst_right_engine)>10)
      auto_cmd_blocked = true;
    else
    {
      dst_left_engine = prev_dst_left_engine;
      dst_right_engine = prev_dst_right_engine;
    }
  }
  
  processStream(SerialAuto,"S2",auto_cmd_blocked);
  failSafe();
  apply(leftWheel,dst_left_engine,cur_left_engine);
  apply(rightWheel,dst_right_engine,cur_right_engine);
  delay(50);
}
