#include "esp_camera.h"
#include <WiFi.h>
#include "stepper.h"

const char* ssid = "333";
const char* password = "12c357409ee5";

StepperMotor stHorizontal(12,13,15,14);

bool initCamera();
void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Starting...");

//  pinMode(LEDC_CHANNEL_0, OUTPUT);
//  digitalWrite(LEDC_CHANNEL_0, LOW);

  if(!initCamera())
    return;

  stHorizontal.init();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop()
{
  static int count =0;
  delay(1000);
  count++;
  
//  for(int i=0;i<=16;i++)
//  {
//    digitalWrite( i, (count%2)==1? HIGH:LOW); 
//  }

  Serial.print("Count=");
  Serial.println(count);
  
}
