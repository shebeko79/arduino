//Board LOLIN D32
#include <BluetoothSerial.h>

const String device_name("rcontrol");
#define REMOTE_NAME "rccar"
const char *bluetooth_pin = "1234";

#define VRX_PIN  32
#define VRY_PIN  33
#define BUTTON_PIN  35
#define LED 22

int central_minx=0;
int central_maxx=0;
int central_miny=0;
int central_maxy=0;

int last_vx = 0;
int last_vy = 0;
unsigned long last_send_time=0;


bool processCommand(const char* buf);
void detectCentralPoint();


BluetoothSerial SerialBT;

void setup() 
{
  Serial.begin(115200);

  pinMode(BUTTON_PIN ,INPUT_PULLUP);

  analogSetAttenuation(ADC_11db);

  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);  
  delay(500);
  digitalWrite(LED,HIGH);  

  detectCentralPoint();

  SerialBT.begin(device_name,true); //Bluetooth device name
  SerialBT.setPin(bluetooth_pin);
  Serial.println("Connecting...");
}

void detectCentralPoint()
{
  central_minx = central_maxx = analogRead(VRX_PIN);
  central_miny = central_maxy = analogRead(VRY_PIN);
  
  for(int i=0;i<50;i++)
  {
    int x = analogRead(VRX_PIN);
    int y = analogRead(VRY_PIN);

    if(x<central_minx)
      central_minx = x;
    else if(x>central_maxx)
      central_maxx = x;

    if(y<central_miny)
      central_miny = y;
    else if(y>central_maxy)
      central_maxy = y;
    
    delay(50);
  }

  central_minx-=10;
  central_maxx+=10;
  central_miny-=10;
  central_maxy+=10;

  Serial.print("CenterX=[");
  Serial.print(central_minx);
  Serial.print(",");
  Serial.print(central_maxx);
  Serial.println("]");

  Serial.print("CenterY=[");
  Serial.print(central_miny);
  Serial.print(",");
  Serial.print(central_maxy);
  Serial.println("]");
}

void connectBT()
{
  if(SerialBT.connected())
  {
    return;
  }

  digitalWrite(LED,LOW);  
  delay(250);
  digitalWrite(LED,HIGH);  


  if(SerialBT.connect(REMOTE_NAME))
  {
    Serial.println("Connected");
    digitalWrite(LED,LOW);
    return;
  }
}

void readBT()
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

  //Serial.println((const char*)sbuf);
}

int convert(int val,int central_min,int central_max)
{
  if(val<central_min)
  {
    return map(val,0,central_min-1,-255,-1);
  }

  if(val>central_max)
  {
    return map(val,central_max+1,4095,1,255);
  }

  return 0;
}

void processJoystick()
{
  if(!SerialBT.connected())
  {
    return;
  }

  // read X and Y analog values
  int x = analogRead(VRX_PIN);
  int y = analogRead(VRY_PIN);

  int vx = convert(x,central_minx,central_maxx);
  int vy = convert(y,central_miny,central_maxy);

  unsigned long cur_time=millis();

  if(last_vx==vx && last_vy==vy && cur_time-last_send_time<1000)
    return;

  //Serial.print("vx=");
  //Serial.print(vx);
  //Serial.print(" vy=");
  //Serial.println(vy);

  last_vx=vx;
  last_vy=vy;
  last_send_time=cur_time;

  int l=vy;
  int r=vy;

  l+=vx;
  r-=vx;

  l=constrain(l,-255,255);
  r=constrain(r,-255,255);

  String str = "cmd:"REMOTE_NAME":drive:";
  str+=String(l);
  str+=";";
  str+=String(r);
  str+="\r";

  Serial.println(str);
  SerialBT.print(str);
}

void processButton()
{
  if(!SerialBT.connected())
  {
    return;
  }

  if(digitalRead(BUTTON_PIN) != LOW)
    return;

  // read X and Y analog values
  int x = analogRead(VRX_PIN);
  int y = analogRead(VRY_PIN);

  int vx = convert(x,central_minx,central_maxx);
  int vy = convert(y,central_miny,central_maxy);

  if(vx!=0 || vy!=0)
    return;

  //Serial.print("vx=");
  //Serial.print(vx);
  //Serial.print(" vy=");
  //Serial.println(vy);

  String str = "cmd:"REMOTE_NAME":block:0\r";

  Serial.println(str);
  SerialBT.print(str);
  delay(1000);
}

void loop()
{
  connectBT();
  readBT();
  processJoystick();
  processButton();
  delay(50);
}
