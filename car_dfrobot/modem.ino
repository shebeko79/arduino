#include <SoftwareSerial.h>
#include <DFRobot_SIM7000.h>


SoftwareSerial Modem(8, 7);
DFRobot_SIM7000 sim7000(&Modem);

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
//      Debug.write(*buf);
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
    {
      int ch = Modem.read();
//      Debug.write(ch);
    }
  }
}

void modemSendCommand(const char* command, bool show_debug)
{
  modemSkipGarbidge();

  if(show_debug)
    Debug.println(command);

  Modem.print(command);
  Modem.write(13);
//  Modem.write(10);
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
        Debug.println("timeout");
        return false;
      }
      delay(200);
    }

    int ch = Modem.read();
//    Debug.write(ch);
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

  sim7000.turnON();
}

bool modemInit()
{
  static int modem_on_count=0;

  modem_on_count++;
  if((modem_on_count%5)==0)
    modemPowerSwitch();
  
  if(!modemSendOkCommand("ATE0"))
    return false;

  delay(5000);

  modemSendCommand("AT+CPIN?");

  if(!modemEnsureAnswer("\r\n+CPIN: READY\r\n"))
    return false;
  
  if(!modemEnsureAnswer("\r\nOK\r\n"))
    return false;

  if(!modemSendOkCommand("AT+CGACT=1,1"))
    return false;

  modemSendCommand("AT+CSTT=\"internet\"");
  delay(1000);
  modemSkipGarbidge();

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

  Debug.println(buf);

  return true;
}

bool sendUdp(const String& message, char* buf, unsigned long buf_len)
{
  Debug.println(message);
  sendUdpFailsCount++;
  
//  if(modemEnsureAnswer("\r\n\r\n"))
//    return false;

  modemSendCommand("AT+CIPSHUT");

  if(!modemEnsureAnswer("\r\nSHUT OK\r\n",2000))
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
  return modemSendOkCommand("AT+CGNSPWR=1");
}

bool getGps(bool& fixed, double& lon, double& lat, double& alt, String& utc_time, double& speed, double& course)
{
  modemSendCommand("AT+CGNSINF",false);
  readAnswer(modem_answer,sizeof(modem_answer));

  fixed = strstr(modem_answer,"+CGNSINF: 1,1")!=nullptr;

  char* answp=modem_answer;
  
  answp = strstr(answp,"+CGNSINF: ");
  if(answp == nullptr)
    return false;

  answp+= sizeof("+CGNSINF: ") - 1;

  answp = strchr(answp,',');
  if(answp == nullptr)
    return false;
  ++answp;

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

  speed=atof(answp);

  answp = strchr(answp,',');
  if(answp == nullptr)
    return false;
  ++answp;

  course=atof(answp);

  return true;
}
