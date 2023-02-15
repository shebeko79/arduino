#include <time.h>

extern const char* alarm_name;

bool notificationIsOn = true;

void sendSensorNotification(const String& message)
{
  String s = String(alarm_name)+":"+message;
  sendTelegramMessage(s);
}

void sendSensorNotificationIfEnabled(const String& message)
{
  if(notificationIsOn)
    sendSensorNotification(message);
}
