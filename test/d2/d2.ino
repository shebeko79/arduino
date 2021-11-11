void setup() 
{
  Serial.begin(115200);
  pinMode(D2, INPUT);
}

int c=0;
void loop() 
{
  int val=digitalRead(D2); 
  if(val ==1)
  {
    Serial.print("val=");
    Serial.print(val);
    Serial.print(" c=");
    Serial.println(c);
  }
  
  c++;
  if(c>=100)
    ESP.deepSleep(5*1000000, WAKE_RF_DEFAULT);
}
