#include <time.h>
#include <ESP8266WiFi.h>

void set_time()
{
  time_t t = time(nullptr);
  Serial.print("time=");
  Serial.println(t);

  timeval tv;
  tv.tv_sec = t -3*3600;
  timezone tz;
  tz.tz_minuteswest = 3*60;
  settimeofday(&tv,&tz);
  
  t = time(nullptr);
  Serial.print("out_time=");
  Serial.println(t);
}

void setup() 
{
  Serial.begin(115200);

  for(int i=0; i<3; i++)
    set_time();
}

void loop() {
  // put your main code here, to run repeatedly:

}
