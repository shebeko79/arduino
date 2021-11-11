#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const char* ssid = "Keenetic-2618";
const char* password = "DvH92bCz";

ESP8266WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int led = 13;
int tick_count=0;
char buf[64];



void handleRoot() 
{
  digitalWrite(led, 1);
  sprintf(buf,"Tick count: %d\n",tick_count);
  server.send(200, "text/plain", buf);
  tick_count++;
  digitalWrite(led, 0);
}

void handleNotFound()
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i=0; i<server.args(); i++)
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}



void setup(void)
{
  Wire.begin(D2, D1); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
  lcd.begin();
  lcd.home();
  lcd.print("Start");

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  lcd.clear();
  lcd.print("Connecting WiFi...");

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());

  if (MDNS.begin("esp8266"))
    Serial.println("MDNS responder started");

  server.on("/", handleRoot);
  server.on("/inline", []()
  {
    server.send(200, "text/plain", "Seems work");
  });

  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started");

}



void loop(void){
  server.handleClient();
}
