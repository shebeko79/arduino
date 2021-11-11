#include <SoftwareSerial.h>
SoftwareSerial modem(7, 8);
#define Debug Serial
//SoftwareSerial Debug(7, 8);
//HardwareSerial& modem=Serial;

void sendCommand(String cmd)
{
  modem.print(cmd);
  modem.write(13);
  modem.write(10);
  Debug.println(cmd);
}

void readAnswer()
{
    String response = "";    
    long int time = millis();   
    while( (time+1000) > millis())
    {
      while(modem.available())
      {       
        char c = modem.read(); 
        response+=c;
      }  
    }    

    Debug.println(response);
}

void setup()
{
  modem.begin(4800);
  Debug.begin(19200);

  sendCommand("ATE0");
  readAnswer();
  
  sendCommand("AT+CGPSPWR=1");
  readAnswer();
}

static int speed=0;

void loop()
{
  Debug.println("");

//  sendCommand("ATE0");
  sendCommand("AT+CGPSINF=0");
  readAnswer();
  
  delay(5000);
}
