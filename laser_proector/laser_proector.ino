#include "hall_sensor.h"

const int pin_no = 18;

Esp32HallSensor vertical_hall;

const double max_pix_y=108;


void setup()
{
  Serial.begin(115200);
  
  pinMode(pin_no, OUTPUT);
  digitalWrite(pin_no, LOW);
  
}

bool getY(uint32_t m, double& y)
{
  int peak_millis = vertical_hall.get_peak_millis();

  if(peak_millis == 0)
    return false;
  
  int cycle_delay = vertical_hall.get_delay();
  if(cycle_delay == 0)
    return false;

  y = (m-peak_millis)*1.0/cycle_delay;
/*
  if(y>1.5)
  {
    Serial.print("m=");
    Serial.println(m);

    Serial.print("peak_millis=");
    Serial.println(peak_millis);

    Serial.print("cur_delay=");
    Serial.println(m-peak_millis);

    Serial.print(" cycle_delay=");
    Serial.println(cycle_delay);

    Serial.print(" y=");
    Serial.println(y);
  }
*/  
  return true;
}

double mapYToPixel(double y)
{
  const double min_y=0.125;
  const double max_y=0.375;
  
  return (y-min_y)/(max_y-min_y)*max_pix_y;
}

void loop() 
{
  vertical_hall.do_cycle();

  uint32_t m = ESP.getCycleCount();
  double y =0.0;
  
  if(!getY(m,y))
    return;

   
  double ypix=mapYToPixel(y);
  if(ypix<0.0 && ypix>max_pix_y)
    return;

  const int pix_to_show = ((int)max_pix_y)/2;

  noInterrupts();

  int last_ipix= (int)ypix;

  while(ypix<max_pix_y)
  {
    int yipix = (int)ypix;

    if(yipix-last_ipix>1)
    {
      Serial.print("last_ipix=");
      Serial.print(last_ipix);

      Serial.print(" yipix=");
      Serial.println(yipix);
    }

    last_ipix = yipix;
    
    if(yipix == pix_to_show)
    {
      digitalWrite(pin_no, HIGH);
    }
    else
      digitalWrite(pin_no, LOW);

    m = ESP.getCycleCount();
    if(!getY(m,y))
      break;
      
    ypix=mapYToPixel(y);
  }

  interrupts();
}
