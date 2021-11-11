#include "motor.h"
#include <ESP8266WiFiMulti.h>
#include <Servo.h>
#include <ESP8266NetBIOS.h>
#include <WiFiUdp.h>

static const int FAIL_SAFE_DELAY = 2000;
const String device_name("rccar");
static const int portNumber = 1500;

namespace SSBrov
{
#include <ssid_brov.h>
}

namespace SSBerezan
{
#include <ssid_berezan.h>
}

namespace SSTel
{
#include <ssid_tel.h>
}

DcMotor dcEngine(D5,D3,D6);
Servo servoWheel;

ESP8266WiFiMulti wifiMulti;
bool connectioWasAlive = true;

WiFiServer server(portNumber);
WiFiUDP    udpServer;
WiFiClient client;

int dst_engine = 0;
int cur_engine = 0;
int dst_wheel = 0;
int cur_wheel = 0;
unsigned long last_ms = 0;

char incomingUdpPacket[256];

void processCommand(const char* buf);


void setup() 
{
  Serial.begin(9600);

  wifiMulti.addAP(SSBrov::ssid, SSBrov::password);
  wifiMulti.addAP(SSBerezan::ssid, SSBerezan::password);
  wifiMulti.addAP(SSTel::ssid, SSTel::password);
    
  dcEngine.init();
  servoWheel.attach(D1);
  analogWriteFreq(200);
}

void monitorWiFi()
{
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
}

void connectClient()
{
  if(!server.hasClient())
    return;

  if(client.connected())
    client.stop();

  client = server.available();
}

void replyIp()
{
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
}

void processClient()
{
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
}

void processCommand(const char* buf)
{
  buf = strstr(buf, "cmd:rccar:drive:");
  if(!buf)
    return;

  buf += sizeof("cmd:rccar:drive:")-1;
  
  int engine_val=atol(buf);
  if(engine_val<-1023 || engine_val>1023)
    return;

  const char* cdiv = strstr (buf,";");
  if(!cdiv)
    return;
  
  int wheel_val=atol(cdiv + 1);
  if(wheel_val<-1023 || wheel_val>1023)
    return;

//  Serial.print("new: engine=");
//  Serial.print(engine_val);
//  Serial.print(" wheel=");
//  Serial.println(wheel_val);

  dst_engine = engine_val;
  dst_wheel = wheel_val;
  last_ms = millis();

  client.print("accept:cmd:rccar:drive\r");
}

void failSafe()
{
  if(last_ms + FAIL_SAFE_DELAY < millis())
  {
    dst_engine = 0;
    dst_wheel = 0;
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

void apply(Servo& dc, int dst,int& cur)
{
  if(cur == dst)
    return;

  cur = dst;

  dst = map(dst, -1023, 1023, 76, 104);
  Serial.print("apply_serov()2 dst=");
  Serial.println(dst);
  dc.write(dst);
}

void loop()
{
  monitorWiFi();
  replyIp();
  connectClient();
  processClient();
  failSafe();
  apply(dcEngine,dst_engine,cur_engine);
  apply(servoWheel,dst_wheel,cur_wheel);
  delay(50);
}
