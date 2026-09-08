#include <SoftwareSerial.h>
#include <DFRobot_SIM7000.h>

#define Debug Serial

SoftwareSerial Modem(8, 7);
DFRobot_SIM7000 sim7000(&Modem);


void setup()
{
  Modem.begin(19200);
  Debug.begin(19200);
  sim7000.turnON();
}

void loop()
{
  char c;
  if(Modem.available())
  {
    c=Modem.read();
    Debug.write(c);
  }

  if(Debug.available())
  {
    c=Debug.read();
    Modem.write(c);
  }
}
