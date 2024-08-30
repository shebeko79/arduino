#include "motor.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <BluetoothSerial.h>

static const int FAIL_SAFE_DELAY = 30000;
const String device_name("rccar");
static const int portNumber = 1500;

const char *bluetooth_pin = "1234";

DcMotor leftWheel(27,14,0);
DcMotor rightWheel(12,13,1);

int dst_left_engine = 0;
int cur_left_engine = 0;
int dst_right_engine = 0;
int cur_right_engine = 0;
unsigned long last_ms = 0;

char incomingUdpPacket[256];

bool processCommand(const char* buf);

BluetoothSerial SerialBT;

void setup() 
{
  Serial.begin(115200);

  SerialBT.begin(device_name); //Bluetooth device name
  SerialBT.setPin(bluetooth_pin);

  leftWheel.init();
  rightWheel.init();
}

void monitorWiFi()
{
/*  
  if (wifiMulti.run() != WL_CONNECTED)
  {
    if (connectioWasAlive == true)
    {
      connectioWasAlive = false;
      Serial.print("Looking for WiFi ");
    }
    delay(500);
  }
  else if (connectioWasAlive == false)
  {
    connectioWasAlive = true;
    Serial.printf(" connected to %s\n", WiFi.SSID().c_str());
    
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
    
    NBNS.begin(device_name.c_str());

    udpServer.begin(portNumber);

    server.begin();
    server.setNoDelay(true);
      
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
  }
*/  
}

void connectClient()
{
/*  
  if(!server.hasClient())
    return;

  if(client.connected())
    client.stop();

  client = server.available();
*/  
}

void replyIp()
{
/*  
  int packetSize = udpServer.parsePacket();
  if(packetSize <= 0)
    return;
    
  Serial.print("replyIp(): ip=");
  Serial.print(udpServer.remoteIP());
  Serial.print(" port=");
  Serial.println(udpServer.remotePort());
  
  int len = udpServer.read(incomingUdpPacket, sizeof(incomingUdpPacket)-1);
  if (len <= 0)
    return;
    
  incomingUdpPacket[len] = 0;
  Serial.printf("UDP packet contents: %s\n", incomingUdpPacket);

  if(device_name != incomingUdpPacket)
    return;

  //send back a reply, to the IP address and port we got the packet from
  udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
  udpServer.print(WiFi.localIP());
  udpServer.endPacket();  

  Serial.println("replyIp()2");
*/  
}

void processClient()
{
/*  
  if(!client.connected())
    return;

  if(!client.available())
    return;

  static uint8_t sbuf[256];

  unsigned long timeout = millis() + 1000;

  for(int i = 0 ; timeout>millis();)
  {
    int len = client.available();
    
    if(len == 0)
    {
      delay(100);
      continue;
    }
    
    if(i + len>sizeof(sbuf)-1)
      len = sizeof(sbuf)-1 - i;
    
    client.readBytes(sbuf + i, len);
    
    sbuf[i+len] = 0;
    i+=len;

    if(strchr((const char*)sbuf,'\r') != nullptr)
      break;
  }

//  Serial.println((const char*)sbuf);

  char* cur = (char*)sbuf;
  for(char* next = strchr(cur, '\r'); next!=nullptr;)
  {
    *next = 0;
    processCommand(cur);

    cur=next+1;
    next= strchr(cur, '\r');
  }
*/  
}

void processBT()
{
  if (!SerialBT.available())
    return;

  static uint8_t sbuf[256];

  unsigned long timeout = millis() + 1000;

  for(int i = 0 ; timeout>millis();)
  {
    int len = SerialBT.available();
    
    if(len == 0)
    {
      delay(100);
      continue;
    }
    
    if(i + len>sizeof(sbuf)-1)
      len = sizeof(sbuf)-1 - i;
    
    SerialBT.readBytes(sbuf + i, len);
    
    sbuf[i+len] = 0;
    i+=len;

    if(strchr((const char*)sbuf,'\r') != nullptr)
      break;
  }

  Serial.println((const char*)sbuf);

  char* cur = (char*)sbuf;
  for(char* next = strchr(cur, '\r'); next!=nullptr;)
  {
    *next = 0;
    if(processCommand(cur))
    {
      SerialBT.print("accept:cmd:rccar:drive\r");
    }

    cur=next+1;
    next= strchr(cur, '\r');
  }
}

bool processCommand(const char* buf)
{
  buf = strstr(buf, "cmd:rccar:drive:");
  if(!buf)
    return false;

  buf += sizeof("cmd:rccar:drive:")-1;
  
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

void failSafe()
{
  if(last_ms + FAIL_SAFE_DELAY < millis())
  {
    dst_left_engine = 0;
    dst_right_engine = 0;
  }
}

void apply(DcMotor& dc, int dst,int& cur)
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
  monitorWiFi();
  replyIp();
  connectClient();
  processClient();
  processBT();
  failSafe();
  apply(leftWheel,dst_left_engine,cur_left_engine);
  apply(rightWheel,dst_right_engine,cur_right_engine);
  delay(50);
}
