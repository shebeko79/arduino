#include "urlencode.h"
#include "telegram_bot.h"

int telegramUpdateId = 0;
int locationMessageId = 0;
String telegram_request;

void sendTelegramMessage(String message)
{
  telegram_request=botStr;
  telegram_request+= "sendMessage?chat_id=-1001249602233&text=";
  message=urlencode(message);
  telegram_request+= message;

  sendUdp(telegram_request, modem_answer,sizeof(modem_answer));
}

void sendLocation(double lat, double lon, bool do_update)
{
  static const char idStr[] = "\"message_id\":";

  const char* pStr;

  if(do_update && locationMessageId!= 0)
  {
    telegram_request=botStr;
    telegram_request+="editMessageLiveLocation?chat_id=-1001249602233&latitude=";
    telegram_request+= String(lat,6);
    telegram_request+= "&longitude=";
    telegram_request+= String(lon,6);
    telegram_request+= "&message_id=";
    telegram_request+= String(locationMessageId);

    sendUdp(telegram_request, modem_answer,sizeof(modem_answer));
    
    pStr = strstr(modem_answer, "\"ok\":false");

    if(pStr == nullptr)
      return;
  }

  telegram_request=botStr;
  telegram_request+="sendLocation?chat_id=-1001249602233&latitude=";
  telegram_request+= String(lat,6);
  telegram_request+= "&longitude=";
  telegram_request+= String(lon,6);
  telegram_request+= "&live_period=1800";
  sendUdp(telegram_request, modem_answer,sizeof(modem_answer));

  pStr = strstr(modem_answer, idStr);
  if(pStr != nullptr)
  {
    pStr +=sizeof(idStr) - 1;
    
    while(true)
    {
      const char* pNext = strstr(pStr,idStr);
      if(!pNext)
        break;

      pStr=pNext+sizeof(idStr) - 1;
    }

    locationMessageId = atoi(pStr);
  }
}
