//Board ESP32-WROOM-DA Module
#include "motor_speed.h"
#include <BluetoothSerial.h>

#define DEVICE_NAME "rccar"
static const int portNumber = 1500;

const char *bluetooth_pin = "1234";

MotorSpeed leftWheel(MotorZsx11h(12,13,0,27,true));
MotorSpeed rightWheel(MotorZsx11h(25,26,1,32,false));

unsigned long last_ms = 0;
uint8_t sbuf[256];
bool auto_cmd_blocked = false;

char incomingUdpPacket[256];
bool processCommand(const char* buf);

BluetoothSerial SerialBT;
#define SerialAuto Serial2

hw_timer_t *timer0 = nullptr;
unsigned timer0_count=0;

float relative_max_speed = RELATIVE_MAX_SPEED;

struct DriveRequest
{
  bool active = false;
  float speed_ms = 0.0;
  float rel_rotation = 0.0;
};

DriveRequest drive_request;


void IRAM_ATTR Timer0_ISR()
{
  ++timer0_count;
  leftWheel.timer_isr(timer0_count);
  rightWheel.timer_isr(timer0_count);
}

void IRAM_ATTR left_tick_isr()
{
  leftWheel.speed_pin_isr();
}

void IRAM_ATTR right_tick_isr()
{
  rightWheel.speed_pin_isr();
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
  
  float rel_speed=atof(buf);
  rel_speed=constrain(rel_speed, -1.0, 1.0);

  const char* cdiv = strstr (buf,";");
  if(!cdiv)
    return false;
  
  int rel_rotation=atof(cdiv + 1);
  rel_rotation=constrain(rel_rotation, -1.0, 1.0);

  drive_request.active=true;
  drive_request.speed_ms = rel_speed * relative_max_speed;
  drive_request.rel_rotation = rel_rotation;

  last_ms = millis();

  return true;
}

bool processBlockCommand(const char* buf)
{
  buf += sizeof("cmd:" DEVICE_NAME ":block:")-1;

  auto_cmd_blocked=atol(buf)!=0;

  return true;
}

bool processRelMaxSpeedCommand(const char* buf)
{
  buf += sizeof("cmd:" DEVICE_NAME ":rel_max_speed:")-1;

  float speed = atof(buf);
  if(speed<=0.0 || speed>MAX_SPEED)
    return false;
  
  relative_max_speed = speed;

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

  pattern = strstr(buf, "cmd:" DEVICE_NAME ":rel_max_speed:");
  if(pattern)
    return processRelMaxSpeedCommand(pattern);

  return false;
}

void failSafe()
{
  if(last_ms + FAIL_SAFE_DELAY < millis())
  {
    leftWheel.fail_safe();
    rightWheel.fail_safe();
  }
}

void applyDriveRequest(const DriveRequest& dr)
{
  if(!dr.active)
    return;
  
  float lspeed = dr.speed_ms;
  float rspeed = dr.speed_ms;

  float diff = constrain(dr.rel_rotation,-1.0,1.0)*RELATIVE_MAX_ROT/2.0;
  lspeed += diff;
  rspeed -= diff;

  lspeed = constrain(lspeed, -MAX_SPEED, MAX_SPEED);
  rspeed = constrain(rspeed, -MAX_SPEED, MAX_SPEED);

  leftWheel.set_speed(lspeed);
  rightWheel.set_speed(rspeed);
}

void loop()
{
  drive_request = DriveRequest();
  processStream(SerialBT,"BT",false);
  
  if(!auto_cmd_blocked && drive_request.active)
  {
    if(abs(drive_request.speed_ms)>OFF_SPEED || abs(drive_request.rel_rotation)>0.0)
    {
      auto_cmd_blocked = true;
    }
    else
    {
      drive_request = DriveRequest();
    }
  }
  
  processStream(SerialAuto,"S2",auto_cmd_blocked);
  applyDriveRequest(drive_request);
  failSafe();
  leftWheel.apply();
  rightWheel.apply();
  leftWheel.dump_state("L");
  rightWheel.dump_state("R");
  delay(50);
}
