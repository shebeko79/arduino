#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ssid_brov.h>

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 1

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) {
    delay(500);
  }

  NBNS.begin("telnet");

  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
  delay(1500);

  digitalWrite(D1, HIGH);
  pinMode(D1, INPUT);
  delay(500);

  //start UART and the server
  Serial.begin(9600);
  server.begin();
  server.setNoDelay(true);
}

void loop() {
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) {
          serverClients[i].stop();
        }
        serverClients[i] = server.available();
        break;
      }
    }
    //no free/disconnected spot so reject
    if (i == MAX_SRV_CLIENTS) {
      WiFiClient serverClient = server.available();
      serverClient.stop();
    }
  }
  //check clients for data
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) {
        //get data from the telnet client and push it to the UART
        while (serverClients[i].available()) {
          int ch = serverClients[i].read();
//          serverClients[i].println(ch);
          Serial.write(ch);
        }
      }
    }
  }
  //check UART for data
  if (Serial.available()) {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
        serverClients[i].write(sbuf, len);
        delay(1);
      }
    }
  }
}
