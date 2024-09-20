## Car tracking using telegram bot
This is 4th iteration of my car tracking device.
This device sends its GPS position in telegram LiveLocation message.

Previous versions uses UDP and requires intermediate server. This version sends HTTPS requests directly to telegram server.

## Hardware
1. [DFRduino Uno v3 - compatible with Arduino](https://botland.store/arduino-compatible-boards-dfrobot/2683-dfrduino-uno-v3-compatible-with-arduino-6959420900015.html)
2. [DFRobot Shield NB-IoT/LTE/GPRS/GPS SIM7000E v2.0 - Shield for Arduino](https://botland.store/arduino-shield-communication/12362-dfrobot-shield-nb-iotltegprsgps-sim7000e-v20-shield-for-arduino-6959420913602.html)
   Shield doesn't really much its description. Now it uses SIM7070G modem. And you need exactly SIM7070/SIM7080/SIM7090 family to make HTTPS request, because original SIM7000 can't work with HTTPS.
   And nowadays mostly all websites(including telegram) requires HTTPS.

## How to setup telegram environment
### Create telegram bot
1. Follow [this](https://core.telegram.org/bots/features#creating-a-new-bot) instructions to obtain your bot token.
2. Create new Group in telegram and add @YourBotName to this group
3. Obtain group's chat_id. Send some message in your chat. Execute right in the browser https://api.telegram.org/botYOUR_BOT_TOKEN/getUpdates. Search for "chat":{"id": parameter in response.
4. Add telegram_bot.h file in your local directory with next content:
   const char* botStr = "botYOUR_BOT_TOKEN/";
   #define CHAT_ID "YOUR_CHAT_ID"

## How to write SSL certificate into SIM7070
  1. Open save_crt.ino
  2. Uncomment commented code in setup() fucntion.
  3. Compile and upload firmware
  4. Open arduino COM terminal
  5. Check all AT commands executed without errors
  6. Wait for AT+CFSTERM
  7. Execute AT+CSSLCFG="convert",2,"godaddy.crt"

  Firmware should automatically save GoDaddy intermediate certificate for telegram. It should be enough. Certificate valid till 2031.

## How to use another SSL certificate
  1. Press "View site information" in Google Chrome, left to URL string
  2. Click "Connection is secure"->"Certificate is valid"
  3. In "Certificate" dialog click "Details"
  4. Choose some certificate from "Certificate Hierarchy".
     Any certificate will work. The only diference leaf certificate has shortest expiration time and root certificate has longest expiration time.
     But connection with leaf certification faster.
  6. Press "Export..." button. Save as "Base64-encoded ASCII, single certificate"
  7. Click on your saved .crt file, check expiration time. If it works for you go to next step.
  8. Open .crt file in text editor and copy-paste content into save_crt.ino instead of old certificate
  9. Wrap each line by "XXX\n" as it was with old certificate.
  10. Repeat uploading firmware steps

## Upload tracking firmware
  1. Open car_dfrobot.ino
  2. Change alarm_name in car_dfrobot.ino on what you like
  3. Make sure car_dfrobot.ino includes sim7070.h, not sim7070.h
  4. Make sure you have telegram_bot.h in same directory with car_dfrobot.ino. And telegram_bot.h contains proper botStr and CHAT_ID
  5. Compile and upload firmware

## How to use several devices
   I use same bot for all devices, but simply different chat's. I find it more convinient.
   But it also possible to create several bots which sends their position into one chat.
   

## Implementation details
  1. Firmware automatically detect APN with AT+CGNAPN. An set it with AT+CNCFG=0,1,"ApnName"
     If automatic detection fails, just comment detection section and simeply use AT+CNCFG=0,1,"YourApnName"

  2. SIM7070 can't work simultaneously with GPS and GSM networks, because it simply use the same hardware for both. Because of that we have to establish new HTTPS connection for each request.
     Yes, this is ridiculous. You can read more about here: https://stackoverflow.com/a/61884727/3115099
     

