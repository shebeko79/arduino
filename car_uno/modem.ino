#include <SoftwareSerial.h>

SoftwareSerial Modem(7, 8);

int sendUdpFailsCount = 0;

bool modemSendOkCommand(const char* command, const unsigned long max_timeout = 1000);
void modemSendCommand(const char* command, bool show_debug=true);
void readAnswer(char* buf, unsigned long buf_len, const unsigned long max_timeout = 1000);

static char modem_answer[400];

void readAnswer(char* in_buf, unsigned long buf_len, const unsigned long max_timeout)
{
  unsigned long start_time = millis();

  char* buf = in_buf;

  while(buf_len>1 && start_time+max_timeout>millis())
  {
    while(Modem.available() && buf_len>1)
    {
      *buf=Modem.read();
      ++buf;
      --buf_len;
    }
  }

  *buf=0;
}


void modemSkipGarbidge()
{
  unsigned long start_time = millis();
  while(start_time+100>millis())
  {
    while(Modem.available())
      Modem.read();
  }
}

void modemSendCommand(const char* command, bool show_debug)
{
  modemSkipGarbidge();

//  if(show_debug)
//    Debug.println(command);

  Modem.print(command);
  Modem.write(13);
  Modem.write(10);
}

bool modemEnsureAnswer(const char* pattern, const unsigned long max_timeout = 1000)
{
  unsigned long start_time = millis();

  unsigned mi = strlen(pattern);
  for(unsigned i = 0; i<mi; i++)
  {
    while(!Modem.available())
    {
      if(start_time+max_timeout<millis())
      {
        return false;
      }
      delay(200);
    }

    int ch = Modem.read();
    if(ch != (int)pattern[i])
    {
      return false;
    }

    yield();
  }

  return true;
}

bool modemSendOkCommand(const char* command, const unsigned long max_timeout)
{
  modemSendCommand(command);
  return modemEnsureAnswer("\r\nOK\r\n", max_timeout);
}

void modemPowerSwitch()
{
  gpsInited = false;

  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);
  delay(1500);

  digitalWrite(A0, HIGH);
  pinMode(A0, INPUT);
  delay(500);

}

bool modemInit()
{
  static int modem_on_count=0;

  modem_on_count++;
  if((modem_on_count%5)==0)
    modemPowerSwitch();
  
  if(!modemSendOkCommand("ATE0"))
    return false;

  delay(10000);

  modemSendCommand("AT+CPIN?");

  if(!modemEnsureAnswer("\r\n+CPIN: READY\r\n"))
    return false;
  
  if(!modemEnsureAnswer("\r\nOK\r\n"))
    return false;

  return true;
}

bool isendUdp(const String& message, char* buf, unsigned long buf_len)
{
  if(!modemSendOkCommand("AT+CIPSTART=\"UDP\",\"31.214.157.79\",\"3203\""))
    return false;

  if(!modemEnsureAnswer("\r\nCONNECT OK\r\n"))
    return false;

  Modem.print("AT+CIPSEND=");
  Modem.print(message.length());
  Modem.write(13);
  Modem.write(10);

  if(!modemEnsureAnswer("\r\n> "))
    return false;

  Modem.print(message);

  if(!modemEnsureAnswer("\r\nSEND OK\r\n"))
    return false;

  readAnswer(buf,buf_len,10000);

  return true;
}

bool sendUdp(const String& message, char* buf, unsigned long buf_len)
{
//  Debug.println(message);
  sendUdpFailsCount++;
  
  if(modemEnsureAnswer("\r\n\r\n"))
    return false;

  if(!modemSendOkCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""))
    return false;

  for(int i =0;i<3;i++)
  {
    if(!modemSendOkCommand("AT+CIPSTATUS"))
      return false;
      
    readAnswer(modem_answer,sizeof(modem_answer));
    
    if(strstr(modem_answer,"STATE: IP STATUS") == nullptr ||
       strstr(modem_answer,"STATE: UDP CLOSED") == nullptr)
       break;

    delay(1000);
  }

  bool ret=isendUdp(message,buf,buf_len);
  
  modemSendCommand("AT+CIPCLOSE");
  
  bool retCloseOk = modemEnsureAnswer("\r\nCLOSE OK\r\n");

  if(!ret || !retCloseOk || *buf==0)
  {
    return false;
  }
  
  sendUdpFailsCount--;

  return true;
}

bool initGps()
{
  return modemSendOkCommand("AT+CGPSPWR=1");
}

bool getGps(bool& fixed, double& lon, double& lat, double& alt, String& utc_time, double& speed, double& course)
{
  modemSendCommand("AT+CGPSSTATUS?",false);
  
  readAnswer(modem_answer,sizeof(modem_answer));
  fixed = strstr(modem_answer,"Location 2D Fix") != nullptr || strstr(modem_answer,"Location 3D Fix") != nullptr;

  modemSendCommand("AT+CGPSINF=0",false);
  readAnswer(modem_answer,sizeof(modem_answer));

  char* answp=modem_answer;
  
  answp = strstr(answp,"+CGPSINF: ");
  if(answp == nullptr)
    return false;

  answp+= sizeof("+CGPSINF: ") - 1;

  answp = strchr(answp,',');
  if(answp == nullptr)
    return false;
  ++answp;

  lat=atof(answp);
             
  answp = strchr(answp,',');
  if(answp == nullptr)
    return false;
  ++answp;

  lon=atof(answp);

  answp = strchr(answp,',');
  if(answp == nullptr)
    return false;
  ++answp;

  alt=atof(answp);

  answp = strchr(answp,',');
  if(answp == nullptr)
    return false;
  ++answp;

  char* answn = strchr(answp,'.');
  if(answn == nullptr)
    return false;
  *answn=0;

  utc_time = answp;

  answp =answn;
  ++answp;

  answp = strchr(answp,',');
  if(answp == nullptr)
    return false;
  ++answp;

  speed=atof(answp);

  answp = strchr(answp,',');
  if(answp == nullptr)
    return false;
  ++answp;

  course=atof(answp);

  lon = floor(lon/100.0) + (lon-floor(lon/100.0)*100.0)/60.0;
  lat = floor(lat/100.0) + (lat-floor(lat/100.0)*100.0)/60.0;

  return true;
}
