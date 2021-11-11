//Simple sketch to access the internal hall effect detector on the esp32.
//values can be quite low. 
//Brian Degger / @sctv  

int val = 0;

int peak_val = 0;
int peak_milis = 0;

int prev_peak_milis = 0;

int mil_delay= 0;

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  int val = hallRead();

/*
  if(val>35)
  {
    Serial.print("val=");
    Serial.println(val);
  }
*/
  if(val>47)
  {
    if(val>peak_val)
    {
      peak_val = val;
      peak_milis = millis();

//    Serial.print("peak_val=");
//    Serial.println(peak_val);
    }
  }
  else if(val<30 && peak_val > 0)
  {
    mil_delay = peak_milis - prev_peak_milis;
    prev_peak_milis = peak_milis;
    peak_val = 0;
    
    Serial.print("mil_delay=");
    Serial.println(mil_delay);
  }
}
