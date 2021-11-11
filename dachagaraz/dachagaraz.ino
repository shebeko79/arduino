#include <CmAlarm.h>
#include <ssid_brov.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* alarm_name = "dachagaraz";
DelayedDigitalInt<18> move_sensor(RISING);
DelayedDigitalSoft gas_sensor(17, FALLING);
static const int relePin = 13;
OneWire oneWire(18);
DallasTemperature tempSensor(&oneWire);

//int loops_count = 0;
time_t startTime = 0;
int updateFailsCount = 0;
int releSwitchCount = 0;

void setup() 
{
  Serial.begin(115200);

  digitalWrite(relePin, HIGH);
  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, HIGH);

  move_sensor.setup();
  gas_sensor.setup();
  
  syncTime();
  sendSensorNotification("power_on");
  commandUpdate();

  tempSensor.begin();

  startTime = time(nullptr);
}

void loop() 
{
//  checkDigitalSensor(move_sensor,"move");
  checkDigitalSensor(gas_sensor,"gas");

  updateFailedFlag = false;
  commandDelayedUpdate();

  if(updateFailedFlag)
  {
    updateFailsCount++;
    Serial.print("updateFailsCount=");
    Serial.println(updateFailsCount);
    wifiOff();
    delay(500);

    if(updateFailsCount>10)
    {
      updateFailsCount = 0;
      Serial.println("rele LOW");
      digitalWrite(relePin, LOW);
      delay(10000);
      Serial.println("rele HI");
      digitalWrite(relePin, HIGH);
   
      releSwitchCount++;    
    }
  }
}  

void processOtherCommand(const String& cmd)
{
  if(cmd == "state")
  {
    tempSensor.requestTemperatures();
    double temp = tempSensor.getTempCByIndex(0);

//ssss    sendSensorNotification("accept:"+cmd+": move_pin="+String(digitalRead(D2)) + " move_ticks="+String(move_sensor.ticks)+" gas_pin="+String(digitalRead(D1)) + " gas_ticks="+String(gas_sensor.ticks) + " rele_switch="+String(releSwitchCount) + " temp="+String(temp));
  }
}
