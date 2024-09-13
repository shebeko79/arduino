#include <SoftwareSerial.h>
#include <DFRobot_SIM7000.h>

#define Debug Serial
bool gpsInited = false;

SoftwareSerial Modem(8, 7);
DFRobot_SIM7000 sim7000(&Modem);

int sendPacketFailsCount = 0;

bool modemSendOkCommand(const __FlashStringHelper *flashStr, const unsigned long max_timeout = 1000);
bool modemSendOkCommand(const char* command, const unsigned long max_timeout = 1000);
void modemSendCommand(const char* command, bool show_debug=true);
void modemSendCommand(const __FlashStringHelper *flashStr, bool show_debug=true);
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

void modemSendCommand(const __FlashStringHelper *flashStr, bool show_debug)
{
  String str(flashStr);
  modemSendCommand(str.c_str(),show_debug);
}


void modemSendCommand(const char* command, bool show_debug)
{
  modemSkipGarbidge();

  if(show_debug)
    Debug.println(command);

  Modem.print(command);
  Modem.write(13);
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
    Debug.write(ch);
    if(ch != (int)pattern[i])
    {
      return false;
    }

    yield();
  }

  return true;
}

bool modemEnsureAnswer(const __FlashStringHelper *flashStr, const unsigned long max_timeout = 1000)
{
  String str(flashStr);
  return modemEnsureAnswer(str.c_str(), max_timeout);
}


bool modemSendOkCommand(const __FlashStringHelper *flashStr, const unsigned long max_timeout)
{
  String str(flashStr);
  return modemSendOkCommand(str.c_str(),max_timeout);
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
  
  if(!modemSendOkCommand(F("ATE0")))
    return false;

  delay(5000);

  modemSendCommand(F("AT+CPIN?"));

  if(!modemEnsureAnswer(F("\r\n+CPIN: READY\r\n")))
    return false;
  
  if(!modemEnsureAnswer("\r\nOK\r\n"))
    return false;

  modemSendCommand(F("AT+CNACT=0,0"));
  readAnswer(modem_answer,sizeof(modem_answer));

  modemSendCommand(F("AT+CGNAPN"));

  if(!modemEnsureAnswer(F("\r\n+CGNAPN: 1,\""),6000))
    return false;
    
  readAnswer(modem_answer,sizeof(modem_answer));
  char* end_qoute = strchr(modem_answer,'"');

  String cmd(F("AT+CNCFG=0,1,\""));
  if(end_qoute == nullptr)
  {
    cmd+="internet\"";
  }
  else
  {
    end_qoute[1]=0;
    cmd+=modem_answer;
  }

  if(!modemSendOkCommand(cmd.c_str()))
    return false;

  if(!modemSendOkCommand(F("AT+CNACT=0,2"),6000))
    return false;
  delay(5000);
  modemSkipGarbidge();

  return true;
}

bool isendPacket(const String& message, char* buf, unsigned long buf_len)
{
  if(!modemSendOkCommand(F("AT+SHCHEAD")))
    return false;

  if(!modemSendOkCommand(F("AT+SHAHEAD=\"Connection\",\"keep-alive\"")))
    return false;

  Modem.print("AT+SHREQ=\"/");
  Modem.print(message);
  Modem.print("\",1");
  Modem.write(13);

  if(!modemEnsureAnswer("\r\nOK\r\n",10000))
    return false;

  if(!modemEnsureAnswer(F("\r\n+SHREQ: \"GET\","),30000))
    return false;

  readAnswer(modem_answer,sizeof(modem_answer));
  Debug.print("shreq=");
  Debug.println(modem_answer);

  const char* comaPos = strchr(modem_answer,',');
  if(comaPos == nullptr)
    return false;

  int data_len = atoi(comaPos+1);
  Debug.print("data_len=");
  Debug.println(data_len);

  if(data_len>=buf_len)
    data_len=buf_len-1;

  String str(F("AT+SHREAD=0,"));
  str+=String(data_len);
  modemSendCommand(str.c_str());

  if(!modemEnsureAnswer("\r\nOK\r\n",2000))
    return false;

  String shread_answ(F("\r\n+SHREAD: "));
  shread_answ+=String(data_len);
  shread_answ+="\r\n";
  
  Debug.println("ens()1");

  if(!modemEnsureAnswer(shread_answ.c_str()))
    return false;

  Debug.println("ens()2");
  delay(1000);

  //readAnswer(buf,data_len,60000);
  //Debug.println("");
  //Debug.print("buf=");
  //Debug.println(buf);

  modemSkipGarbidge();

  Debug.println("ens()3");

  return true;
}

bool sendPacket(const String& message, char* buf, unsigned long buf_len)
{
  Debug.println(message);
  sendPacketFailsCount++;
  
  modemSendCommand(F("AT+SHSTATE?"));

  if(!modemEnsureAnswer("\r\n+SHSTATE: "))
    return false;

  readAnswer(modem_answer,modem_answer);
  Debug.println(modem_answer);
  //Establish HTTPS connection
  if(modem_answer[0] != '1')
  {
    modemSendCommand(F("AT+CNACT?"));
    readAnswer(modem_answer,sizeof(modem_answer));

    if(!modemSendOkCommand(F("AT+CSSLCFG=\"sslversion\",1,3")))
      return false;

    if(!modemSendOkCommand(F("AT+CSSLCFG=\"ignorertctime\",1,1")))
      return false;

    if(!modemSendOkCommand(F("AT+SHSSL=1,\"godaddy.crt\"")))
      return false;

    if(!modemSendOkCommand(F("AT+SHCONF=\"URL\",\"https://api.telegram.org\"")))
      return false;

    if(!modemSendOkCommand(F("AT+SHCONF=\"BODYLEN\",1024")))
      return false;

    if(!modemSendOkCommand(F("AT+SHCONF=\"HEADERLEN\",350")))
      return false;

    if(!modemSendOkCommand(F("AT+SHCONN"),120000))
    {
      modemSendCommand(F("AT+CNACT=0,0"));
      readAnswer(modem_answer,sizeof(modem_answer));
      return false;
    }
  }

  bool ret=isendPacket(message,buf,buf_len);
  
  if(!ret || *buf==0)
  {
    return false;
  }
  
  sendPacketFailsCount--;

  return true;
}

bool initGps()
{
  return modemSendOkCommand(F("AT+CGNSPWR=1"));
}

bool getGps(bool& fixed, double& lon, double& lat, double& alt, String& utc_time, double& speed, double& course)
{
  modemSendCommand(F("AT+CGNSINF"),false);
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
