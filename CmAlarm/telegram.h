#include "urlencode.h"
#include "telegram_bot.h"

const char* telegramHost = "api.telegram.org";
const int httpsPort = 443;

int telegramUpdateId = 0;

bool updateFailedFlag = false;

WiFiClientSecure client;

const char* rootCACertificate = 

"-----BEGIN CERTIFICATE-----\n"
"MIIE0DCCA7igAwIBAgIBBzANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\n"
"EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\n"
"EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\n"
"ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTExMDUwMzA3MDAwMFoXDTMxMDUwMzA3\n"
"MDAwMFowgbQxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\n"
"EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjEtMCsGA1UE\n"
"CxMkaHR0cDovL2NlcnRzLmdvZGFkZHkuY29tL3JlcG9zaXRvcnkvMTMwMQYDVQQD\n"
"EypHbyBEYWRkeSBTZWN1cmUgQ2VydGlmaWNhdGUgQXV0aG9yaXR5IC0gRzIwggEi\n"
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC54MsQ1K92vdSTYuswZLiBCGzD\n"
"BNliF44v/z5lz4/OYuY8UhzaFkVLVat4a2ODYpDOD2lsmcgaFItMzEUz6ojcnqOv\n"
"K/6AYZ15V8TPLvQ/MDxdR/yaFrzDN5ZBUY4RS1T4KL7QjL7wMDge87Am+GZHY23e\n"
"cSZHjzhHU9FGHbTj3ADqRay9vHHZqm8A29vNMDp5T19MR/gd71vCxJ1gO7GyQ5HY\n"
"pDNO6rPWJ0+tJYqlxvTV0KaudAVkV4i1RFXULSo6Pvi4vekyCgKUZMQWOlDxSq7n\n"
"eTOvDCAHf+jfBDnCaQJsY1L6d8EbyHSHyLmTGFBUNUtpTrw700kuH9zB0lL7AgMB\n"
"AAGjggEaMIIBFjAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNV\n"
"HQ4EFgQUQMK9J47MNIMwojPX+2yz8LQsgM4wHwYDVR0jBBgwFoAUOpqFBxBnKLbv\n"
"9r0FQW4gwZTaD94wNAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8v\n"
"b2NzcC5nb2RhZGR5LmNvbS8wNQYDVR0fBC4wLDAqoCigJoYkaHR0cDovL2NybC5n\n"
"b2RhZGR5LmNvbS9nZHJvb3QtZzIuY3JsMEYGA1UdIAQ/MD0wOwYEVR0gADAzMDEG\n"
"CCsGAQUFBwIBFiVodHRwczovL2NlcnRzLmdvZGFkZHkuY29tL3JlcG9zaXRvcnkv\n"
"MA0GCSqGSIb3DQEBCwUAA4IBAQAIfmyTEMg4uJapkEv/oV9PBO9sPpyIBslQj6Zz\n"
"91cxG7685C/b+LrTW+C05+Z5Yg4MotdqY3MxtfWoSKQ7CC2iXZDXtHwlTxFWMMS2\n"
"RJ17LJ3lXubvDGGqv+QqG+6EnriDfcFDzkSnE3ANkR/0yBOtg2DZ2HKocyQetawi\n"
"DsoXiWJYRBuriSUBAA/NxBti21G00w9RKpv0vHP8ds42pM3Z2Czqrpv1KrKQ0U11\n"
"GIo/ikGQI31bS/6kA1ibRrLDYGCD+H1QQc7CoZDDu+8CL9IVVO5EFdkKrqeKM+2x\n"
"LXY2JtwE65/3YR8V3Idv7kaWKK2hJn0KCacuBKONvPi8BDAB\n"
"-----END CERTIFICATE-----\n";

String receiveTelegramAnswer()
{
  long contentLength = 0;
  
  while (client.connected())
  {
    String line = client.readStringUntil('\n');

    if (line == "\r")
      break;

    int pos = line.indexOf("Content-Length: ");
    if(pos == 0)
    {
      String ss = line.substring(16);
      contentLength = ss.toInt();
    }
  }

  char tmp[257];
  const size_t tmp_len=sizeof(tmp)/sizeof(char) -1;
  
  String ret;

  long i=0;
  while(i< contentLength)
  {
    size_t toRead = contentLength-i;
    if(toRead>tmp_len)
      toRead = tmp_len;
      
    size_t sz = client.readBytes(tmp,toRead);
    if(sz == 0)
      break;
  
    tmp[sz]=0;
	ret+=tmp;
    
    i+=sz;
  }
  
  return ret;
}

void sendTelegramMessage(String message,bool silent = false)
{
  if(!client.connected())
	client.setCACert(rootCACertificate);
  
  if (!client.connect(telegramHost, httpsPort))
  {
    Serial.println("connection failed");
    return;
  }

  message = urlencode(message);
  
  String clStr = "GET /" TELEGRAM_BOT "/sendMessage?chat_id=-1001249602233&text=";
  clStr += message;
  
  if(silent)
	  clStr +=String("&disable_notification=true");
  
  clStr += String(" HTTP/1.1\r\n")+
           "Host: " + telegramHost + "\r\n"
           "Connection: close\r\n\r\n";
		   
  client.print(clStr);
}


String telegramUpdate()
{
  if(!client.connected())
	client.setCACert(rootCACertificate);

  if (!client.connect(telegramHost, httpsPort))
  {
    Serial.println("connection failed");
	updateFailedFlag = true;
    return String();
  }

  client.print(String("GET /" TELEGRAM_BOT "/getUpdates?allowed_updates=message&limit=10&offset=") + String(telegramUpdateId+1) +" HTTP/1.1\r\n" +
               "Host: " + telegramHost + "\r\n" +
               "Connection: close\r\n\r\n");

  String answer = receiveTelegramAnswer();
  Serial.println(answer);
  
  updateFailedFlag = answer.length() == 0;
  
  static const char update_idStr[] = "\"update_id\":";
  int pos = answer.lastIndexOf(update_idStr);
  if(pos != -1)
  {
	  telegramUpdateId = answer.substring(pos + sizeof(update_idStr) - 1).toInt();
	  Serial.print("telegramUpdateId=");
	  Serial.println(telegramUpdateId);
  }
  
  return answer;
}