// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>


OneWire oneWire(18);
DallasTemperature sensors(&oneWire);

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Dallas Temperature IC Control Library Demo");

  sensors.begin();
}

void loop(void)
{ 
  static double lastTemp= 0.0;
  
  sensors.requestTemperatures();
  double temp = sensors.getTempCByIndex(0);
  if(temp != lastTemp)
  {
    Serial.println(temp);
    lastTemp = temp;
  }
}
