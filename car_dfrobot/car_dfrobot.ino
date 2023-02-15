#include <time.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

#define Debug Serial


#define DEG_TO_RAD 0.017453292519943295769236907684886

const char* alarm_name = "car";

enum ModemState
{
  ms_not_inited,
  ms_inited,
  ms_restart,
};

ModemState modemState = ms_not_inited;
bool gpsInited = false;

unsigned long last_get_gps = 0;

unsigned long trackUpdateDelay = 120;
double trackDistance = 100.0;
unsigned long trackLastUpdateTime = 0;
double lastTrackLat =0.0;
double lastTrackLon =0.0;

extern bool notificationIsOn;
extern int sendUdpFailsCount;

extern SoftwareSerial Modem;

#if 0
bool readEEPROMConfig();
void writeEEPROMConfig();
#endif

void sendPosition(const String& messageName);
void sendPosition(const String& messageName, bool fixed, const String& utc_time,double lat, double lon, double alt,double speed, double course,bool track);
void track();

void setup() 
{
//  readEEPROMConfig();

  //start UART and the server
  Modem.begin(19200);
  Debug.begin(19200);

  for(int i=0;i<4;i++)
  if(modemSendOkCommand("ATE0", 1000))
  {
    modemState = ms_restart;
    break;
  }
}

void loop()
{
  if(sendUdpFailsCount>10)
  {
    sendUdpFailsCount = 0;
    modemState = ms_restart;
    modemPowerSwitch();
  }

  if(modemState != ms_inited)
  {
    if(!modemInit())
      return;
      
    if(modemState == ms_not_inited)
      sendSensorNotification("power_on");
    else
      sendSensorNotification("restart");
    
    modemState = ms_inited;
  }

  if(!gpsInited)
  {
    if(initGps())
    {
      gpsInited = true;
    }
  }

  track();
}

void sendPosition(const String& messageName)
{
    bool fixed = false;
    double lon=0.0, lat=0.0, alt=0.0, speed=0.0, course=0.0;
    String utc_time;
    
    getGps(fixed,lon,lat,alt,utc_time,speed,course);
    
    sendPosition(messageName,fixed, utc_time,lat,lon,alt,speed,course, false);
}

void sendPosition(const String& messageName, bool fixed, const String& utc_time,double lat, double lon, double alt,double speed, double course,bool track)
{
    if(!track)
    {
      String str = ":"+utc_time + "," + String(fixed? 1:0) + "," + String(lat,6) + "," + String(lon,6) + "," + String(alt) + "," + String(speed) + "," + String(course);
      Debug.println(str);
      sendSensorNotification(messageName+str);
    }
    
    if(fixed)
      sendLocation(lat,lon,track);
}

double getDistance(double lat1, double lon1, double lat2, double lon2)
{
  const double R = 6371000.0;
  double dLat = (lat2-lat1)*DEG_TO_RAD;
  double dLon = (lon2-lon1)*DEG_TO_RAD; 
  
  double a = sin(dLat/2.0) * sin(dLat/2.0) +
          cos(lat1*DEG_TO_RAD) * cos(lat2*DEG_TO_RAD) * 
          sin(dLon/2) * sin(dLon/2);
          
  double c = 2.0 * atan2(sqrt(a), sqrt(1.0-a)); 
  double d = R * c;
  return d;
}

void track()
{
  if(!gpsInited)
    return;
 
  unsigned long ms = millis();
  if(last_get_gps + 2000 > ms)
    return;

  bool fixed = false;
  double lon=0.0, lat=0.0, alt=0.0, speed=0.0, course=0.0;
  String utc_time;
    
  if(!getGps(fixed,lon,lat,alt,utc_time,speed,course))
    return;

  if(!fixed)
    return;

  unsigned long t = millis();

  double distDiff = getDistance(lastTrackLat,lastTrackLon,lat,lon);

  bool trackByTime = trackUpdateDelay!=0 && (trackLastUpdateTime==0 || trackLastUpdateTime+trackUpdateDelay*1000<=t);
  bool trackByDistance = trackDistance!=0.0 && distDiff>=trackDistance;

  if(!trackByTime && !trackByDistance)
    return;

  sendPosition("track",fixed,utc_time,lat,lon,alt,speed,course,true);

  trackLastUpdateTime = t;
  lastTrackLat = lat;
  lastTrackLon = lon;
}
