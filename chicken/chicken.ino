#include <CmAlarm.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Stepper.h>

const char* alarm_name = "chicken";
static const int M1A=13,M1B=12,M2A=14,M2B=27;
static const int LIGHT_PIN = 32;
static const int DOOR2_A=25,DOOR2_B=33;
static const int ONE_WIRE_PIN = 18;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature tempSensor(&oneWire);

int loops_count = 0;
time_t startTime = 0;
int updateFailsCount = 0;

Stepper st(200,M1A,M1B,M2A,M2B);

bool doorIsOpened;
bool doorIsAuto;

bool door2IsOpened;
bool door2IsAuto;

static const int samplesCount = 120;
int samples[samplesCount];
int currentSample;
time_t lastLightTick = 0;

static const int closeLight=5;
static const int openLight=700;
static const int door2CloseTemp=5;

void openDoor(int steps = 4300);
void closeDoor(int steps = 4200);

void addWifiPoints()
{
  {
#include "ssid_brov.h"
    WiFiMulti.addAP(ssid, password);
  }
  {
#include "ssid_berezan.h"
    WiFiMulti.addAP(ssid, password);
  }
  {
#include "ssid_tel.h"
    WiFiMulti.addAP(ssid, password);
  }
}

void setup() 
{
  Serial.begin(115200);

  doorIsOpened = false;
  doorIsAuto = true;

  door2IsOpened = false;
  door2IsAuto = true;

  pinMode(LIGHT_PIN, INPUT); 
  pinMode(DOOR2_A, OUTPUT); 
  pinMode(DOOR2_B, OUTPUT); 

  st.setSpeed(40);
  analogSetAttenuation(ADC_0db);


  int curLight = analogRead(LIGHT_PIN);
  for(int i=0;i<samplesCount;i++)
    samples[i] = curLight;

  currentSample = 0;

  WiFi.mode(WIFI_STA);

  syncTime();
  sendSensorNotification("power_on");
  commandUpdate();


  tempSensor.begin();

  startTime = time(nullptr);
  lastLightTick = startTime;
}

void offEngine()
{
  digitalWrite(M1A, LOW);
  digitalWrite(M1B, LOW);
  digitalWrite(M2A, LOW);
  digitalWrite(M2B, LOW);
}


int calcAvgLight()
{
  double ret = (double)samples[0];
  for(int i =1; i<samplesCount;i++)
    ret+=(double)samples[i];

  ret/=samplesCount;

  return (int)ret;
}

void openDoor(int steps)
{
    st.step(steps);
    offEngine();
    doorIsOpened = true;
}

void closeDoor(int steps)
{
    st.step(-steps);
    offEngine();
    doorIsOpened = false;
}

void openDoor2()
{
  door2IsOpened = true;
  
  digitalWrite(DOOR2_A, HIGH);
  digitalWrite(DOOR2_B, LOW);

  delay(30000);

  digitalWrite(DOOR2_A, LOW);
  digitalWrite(DOOR2_B, LOW);
}

void closeDoor2()
{
  door2IsOpened = false;
  
  digitalWrite(DOOR2_A, LOW);
  digitalWrite(DOOR2_B, HIGH);

  delay(30000);

  digitalWrite(DOOR2_A, LOW);
  digitalWrite(DOOR2_B, LOW);
}

void checkDoorLoop()
{
  time_t t = time(nullptr);
  if(lastLightTick + 30 > t )
    return;
  lastLightTick = t;

  int sampleIndex = currentSample%samplesCount;
  samples[sampleIndex]=analogRead(LIGHT_PIN);
  
  Serial.print("sampleIndex=");
  Serial.print(sampleIndex);
  Serial.print(" light=");
  Serial.print(samples[sampleIndex]);
  
  currentSample++;

  int light = calcAvgLight();
  tempSensor.requestTemperatures();
  double temp = tempSensor.getTempCByIndex(0);

  Serial.print(" avg=");
  Serial.print(light);
  Serial.print(" temp=");
  Serial.println(temp);

  if(doorIsAuto)
  {
    if(doorIsOpened)
    {
      if(light<=closeLight)
        closeDoor();
    }
    else
    {
      if(light>=openLight)
        openDoor();
    }
  }

  if(door2IsAuto)
  {
    if(door2IsOpened)
    {
      if(light<=closeLight && temp < door2CloseTemp)
        closeDoor2();
    }
    else
    {
      if(light>=openLight)
        openDoor2();
    }
  }
}


void loop() 
{
  checkDoorLoop();
  
  updateFailedFlag = false;
  commandDelayedUpdate();

  if(updateFailedFlag)
  {
    updateFailsCount++;
    Serial.print("updateFailsCount=");
    Serial.println(updateFailsCount);
    wifiOff();
    delay(500);
  }
}  

void sendAccept(const String& cmd)
{
    tempSensor.requestTemperatures();
    double temp = tempSensor.getTempCByIndex(0);

    int light = calcAvgLight();

    sendSensorNotification("accept:"+cmd+":temp="+String(temp)+" light="+String(light)
           +" doorIsOpened="+doorIsOpened+" doorIsAuto=" +doorIsAuto
           +" door2IsOpened="+door2IsOpened+" door2IsAuto=" +door2IsAuto);
}

void processOtherCommand(const String& cmd)
{
  if(cmd == "state")
  {
    sendAccept(cmd);
  }
  else if(cmd == "dooropen")
  {
    doorIsAuto = false;
    openDoor();
    sendAccept(cmd);
  }
  else if(cmd == "doorclose")
  {
    doorIsAuto = false;
    closeDoor();
    sendAccept(cmd);
  }
  else if(cmd == "doorauto")
  {
    doorIsAuto = true;
    sendAccept(cmd);
  }
  else if(cmd == "door2open")
  {
    door2IsAuto = false;
    openDoor2();
    sendAccept(cmd);
  }
  else if(cmd == "door2close")
  {
    door2IsAuto = false;
    closeDoor2();
    sendAccept(cmd);
  }
  else if(cmd == "door2auto")
  {
    door2IsAuto = true;
    sendAccept(cmd);
  }
}
