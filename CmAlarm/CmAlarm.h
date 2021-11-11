#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#include <time.h>
#include <EEPROM.h>
#include "telegram.h"
//#include "DelayedDigitalSensor.h"

extern const char* alarm_name;

int commandUpdateDelay = 300;
time_t commandLastUpdateTime = 0;
bool notificationIsOn = true;

const int syncTimeUpdateDelay = 3600;
time_t syncTimeLastUpdateTime = 0;


static unsigned deepSleepSeconds = 5;
static const int timeZone = 2;

WiFiMulti WiFiMulti;

bool readEEPROMConfig();
void writeEEPROMConfig();

void addWifiPoints();

bool wifiOn()
{
  if(WiFi.getMode() == WIFI_STA && WiFi.isConnected())
	  return true;
  
  WiFi.mode(WIFI_STA);
  
  addWifiPoints();

  for(int i = 0; i<50 && WiFiMulti.run() != WL_CONNECTED ; i++)
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");

  if(WiFi.status() != WL_CONNECTED)
  {
	  Serial.print("WiFi connection failed. status=");
	  Serial.println(WiFi.status());
	  return false;
  }
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  return true;
}

void wifiOff()
{
  if(WiFi.getMode() == WIFI_OFF)
	  return;
  
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  for(int i = 0; i<50 && WiFi.status() != WL_DISCONNECTED ; i++)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi disconnected");
}

void syncTime()
{
  time_t now = time(nullptr);
  Serial.print("Current time: ");
  Serial.println(now);

  if(now>=100000 && syncTimeLastUpdateTime + syncTimeUpdateDelay>now)
	  return;

  if(!wifiOn())
	  return;
  
  Serial.print("Setting time using SNTP");
  configTime(timeZone*3600,0, "pool.ntp.org", "time.nist.gov");
  for(int i = 0; i<50 && now < 100000; i++)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  
  syncTimeLastUpdateTime = time(nullptr);
}

String time2string(time_t t)
{
    tm tt;
    gmtime_r(&t, &tt);

    char tmp[24];

    sprintf(tmp,"%04d.%02d.%02d %02d:%02d:%02d",
    tt.tm_year+1900,tt.tm_mon+1,tt.tm_mday,
    tt.tm_hour,tt.tm_min,tt.tm_sec);
    return String(tmp);
}


void sendSensorNotification(const String& message,time_t t = 0)
{
	if(!wifiOn())
	  return;
	
	if(t==0)
		t = time(nullptr);
	
	String s = String(alarm_name)+":"+time2string(t)+":"+message;
	Serial.println(s);
	sendTelegramMessage(s);
	
	String answer = receiveTelegramAnswer();
	Serial.println(answer);
}

void sendSensorNotificationIfEnabled(const String& message,time_t t = 0)
{
	if(notificationIsOn)
		sendSensorNotification(message,t);
}


void sendCommandAccept(const String& cmd)
{
	sendSensorNotification("accept:"+cmd);
}

void processOtherCommand(const String& cmd);

void processCommand(const String& cmd)
{
	Serial.print("processCommand: ");
	Serial.println(cmd);
	
	if(cmd == "on")
	{
		notificationIsOn = true;
		writeEEPROMConfig();
		sendCommandAccept(cmd);
	}
	else if(cmd == "off")
	{
		notificationIsOn = false;
		writeEEPROMConfig();
		sendCommandAccept(cmd);
	}
	else 
		processOtherCommand(cmd);
}


void commandUpdate()
{
	if(!wifiOn())
	{
	  updateFailedFlag = true;
	  return;
	}
  
    commandLastUpdateTime = time(nullptr);
	while(true)
	{
		String answer = telegramUpdate();

		String pattern = String("\"text\":\"cmd:")+alarm_name+":";
		Serial.print("Pattern: ");
		Serial.println(pattern);

		int pos = answer.indexOf(pattern);
		if(pos == -1)
			break;
		
		for(; pos!=-1 ; pos = answer.indexOf(pattern,pos+1))
		{
			pos += pattern.length();
			int pos2 = answer.indexOf("\"",pos+1);
			
			if(pos2!=-1)
			{
				String cmd=answer.substring(pos,pos2);
				processCommand(cmd);
			}
			
			pos=pos2;
		}
	}
}

void commandDelayedUpdate()
{
  time_t t = time(nullptr);
  if(t<commandLastUpdateTime+commandUpdateDelay)
	  return;
  
  commandUpdate();
}

bool readEEPROMConfig()
{
	EEPROM.begin(2);
	uint8_t b = EEPROM.read(0);
	if(!b)
		return false;
	
	b = EEPROM.read(1);
	notificationIsOn = b!=0;
	
	return true;
}

void writeEEPROMConfig()
{
	EEPROM.begin(2);
	EEPROM.write(0,1);
	EEPROM.write(1,notificationIsOn? 1:0);
	EEPROM.commit();
}


template<typename Sensor>
void checkDigitalSensor(Sensor& sensor, const char* sensor_name)
{
  if(sensor.isLoopAllert())
  {
    String s= String(sensor_name)+":ticks=" + String(sensor.ticks);
    Serial.println(s);
    sendSensorNotificationIfEnabled(s,sensor.lastTickTime);
    String answer = receiveTelegramAnswer();

    sensor.ticks = 0;
  }

}

void commonReadRTC(uint32_t& offset)
{
/*	
	timeval tv;
	tv.tv_usec = 0;
	timezone tz;
	tz.tz_minuteswest = timeZone*60;
	ESP.rtcUserMemoryRead(offset, (uint32_t*)&tv.tv_sec,sizeof(tv.tv_sec));
    tv.tv_sec -=timeZone*3600;
	tv.tv_sec +=deepSleepSeconds;
	settimeofday(&tv,&tz);
	offset += sizeof(tv.tv_sec);

	ESP.rtcUserMemoryRead(offset, (uint32_t*)&commandLastUpdateTime,sizeof(commandLastUpdateTime));
	offset += sizeof(commandLastUpdateTime);
	
	ESP.rtcUserMemoryRead(offset, (uint32_t*)&telegramUpdateId,sizeof(telegramUpdateId));
	offset += sizeof(telegramUpdateId);
	
	ESP.rtcUserMemoryRead(offset, (uint32_t*)&syncTimeLastUpdateTime,sizeof(syncTimeLastUpdateTime));
	offset += sizeof(syncTimeLastUpdateTime);
*/	
}

void commonWriteRTC(uint32_t& offset)
{
/*	
	time_t seconds = time(nullptr);
	ESP.rtcUserMemoryWrite(offset, (uint32_t*)&seconds,sizeof(seconds));
	offset += sizeof(seconds);

	ESP.rtcUserMemoryWrite(offset, (uint32_t*)&commandLastUpdateTime,sizeof(commandLastUpdateTime));
	offset += sizeof(commandLastUpdateTime);
	
	ESP.rtcUserMemoryWrite(offset, (uint32_t*)&telegramUpdateId,sizeof(telegramUpdateId));
	offset += sizeof(telegramUpdateId);
	
	ESP.rtcUserMemoryWrite(offset, (uint32_t*)&syncTimeLastUpdateTime,sizeof(syncTimeLastUpdateTime));
	offset += sizeof(syncTimeLastUpdateTime);
*/	
}

