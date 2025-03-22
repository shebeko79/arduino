//Board ESP32-WROOM-DA Module
#include "motor_speed.h"
#include <BluetoothSerial.h>

#define DEVICE_NAME "rccar"
static const int portNumber = 1500;

const char *bluetooth_pin = "1234";

MotorSpeed leftWheel(MotorZsx11h(12,13,0,27,true));
MotorSpeed rightWheel(MotorZsx11h(25,26,1,32,false));

volatile unsigned long last_ms = 0;
bool auto_cmd_blocked = false;
static volatile TaskHandle_t motor_task_handle = NULL;
uint8_t sbuf[256];
char incomingUdpPacket[256];

template<typename T>
void processCommand(T& stream, const char* buf,bool blocked);

static void motors_task(void *);

BluetoothSerial SerialBT;
#define SerialAuto Serial2

hw_timer_t *speed_timer = nullptr;
unsigned speed_timer_count=0;

float relative_max_speed = RELATIVE_MAX_SPEED;

struct DriveRequest
{
  bool active = false;
  float speed_ms = 0.0;
  float rel_rotation = 0.0;
};

DriveRequest drive_request;


void IRAM_ATTR SpeedTimer_ISR()
{
  ++speed_timer_count;
  leftWheel.timer_isr(speed_timer_count);
  rightWheel.timer_isr(speed_timer_count);
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

  speed_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(speed_timer, &SpeedTimer_ISR, true);
  timerAlarmWrite(speed_timer, TIMER_MS*1000, true);
  timerAlarmEnable(speed_timer);

  xTaskCreateUniversal(motors_task, "motors", 4096, NULL, 1, (TaskHandle_t*)&motor_task_handle, CONFIG_ARDUINO_UDP_RUNNING_CORE);
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

  // if(!blocked)
  // {
  //   Serial.print(caption);
  //   Serial.print(": ");
  //   Serial.println((const char*)sbuf);
  // }

  char* cur = (char*)sbuf;
  for(char* next = strchr(cur, '\r'); next!=nullptr;)
  {
    *next = 0;
    processCommand(stream, cur, blocked);

    cur=next+1;
    next= strchr(cur, '\r');
  }
}

bool processDriveCommand(const char* buf)
{
  float rel_speed=atof(buf);
  rel_speed=constrain(rel_speed, -1.0, 1.0);

  const char* cdiv = strstr (buf,";");
  if(!cdiv)
    return false;
  
  float rel_rotation=atof(cdiv + 1);
  rel_rotation=constrain(rel_rotation, -1.0, 1.0);

  drive_request.active=true;
  drive_request.speed_ms = rel_speed * relative_max_speed;
  drive_request.rel_rotation = rel_rotation;

  last_ms = millis();

  return true;
}

bool processBlockCommand(const char* buf)
{
  auto_cmd_blocked=atol(buf)!=0;

  return true;
}

bool processRelMaxSpeedCommand(const char* buf)
{
  float speed = atof(buf);
  if(speed<=0.0 || speed>MAX_SPEED)
    return false;
  
  relative_max_speed = speed;

  return true;
}

template<typename T>
void answerCommand(T& stream, const String& cmd, const String& answer)
{
    String str = answer+":cmd:" DEVICE_NAME ":" +cmd +"\r";
    //Serial.println(str);
    stream.print(str);
}

template<typename T>
void processCommand(T& stream, const char* buf,bool blocked)
{
  if(strncmp(buf,"cmd:" DEVICE_NAME ":",sizeof("cmd:" DEVICE_NAME ":")-1) !=0)
    return;
  
  buf+=sizeof("cmd:" DEVICE_NAME ":")-1;

  char* semi=strchr(buf, ':');
  if(semi ==0)
    return;

  *semi = 0;
  
  String cmd(buf);
  buf=semi+1;

  if(blocked)
  {
    answerCommand(stream,cmd,"reject");
    return;
  }

  bool res = false;

  if(cmd == "drive")
  {
    res = processDriveCommand(buf);
  }
  else if(cmd == "block")
  {
    res = processBlockCommand(buf);
  }
  else if(cmd == "rel_max_speed")
  {
    res = processRelMaxSpeedCommand(buf);
  }
  else
  {
    answerCommand(stream,cmd,"unknown");
    return;
  }

  if(res)
  {
    answerCommand(stream,cmd,"accept");
  }
  else
  {
    answerCommand(stream,cmd,"invalid");
  }
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

static void motors_task(void *)
{
    while(true)
    {
      failSafe();
      leftWheel.apply();
      rightWheel.apply();
      //leftWheel.dump_state("L", SerialBT);
      //rightWheel.dump_state("R", SerialBT);
      delay(MAIN_CYCLE_DELAY);
    }

    motor_task_handle = NULL;
    vTaskDelete(NULL);
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
}
