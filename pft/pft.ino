#define TOUCH_MODULES_CST_SELF        //Use CST816 by default

#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "TouchLib.h"
#include "Wire.h"
#include "pin_config.h"

TFT_eSPI tft = TFT_eSPI();
TouchLib touch(Wire, PIN_IIC_SDA, PIN_IIC_SCL, CTS820_SLAVE_ADDRESS, PIN_TOUCH_RES);

hw_timer_t *Timer0_Cfg = nullptr;

int32_t tick_count = 0;
unsigned long last_time = 0;

#define TACHO_PIN 1
int last_tacho_state = LOW;

#define TICK_MS 100
#define TEST_MS 10000
constexpr unsigned ticks_count = TEST_MS/TICK_MS;
int32_t ticks[ticks_count];
unsigned tick_pos=0;

int16_t scren_height = 2;
int16_t scren_width = 2;

void startTest();

void IRAM_ATTR Timer0_ISR()
{
  int tacho_state = digitalRead(TACHO_PIN);
  
  if(!last_tacho_state && tacho_state)
  {
    ++tick_count;
  }

  last_tacho_state = tacho_state;
}

void setup()
{
  Serial.begin(115200);
  
  pinMode(TACHO_PIN, INPUT);

  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  Serial.begin(115200);
  Serial.println("");

  tft.begin();

  tft.setRotation(3);
  tft.setSwapBytes(true);

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
  ledcSetup(0, 2000, 8);
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, 255);
#else
  ledcAttach(PIN_LCD_BL, 200, 8);
  ledcWrite(PIN_LCD_BL, 255);
#endif

  scren_height = tft.height();
  scren_width = tft.width();

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);

  Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);
  if (!touch.init())
  {
      Serial.println("Touch IC not found");
  }
  
  last_tacho_state = digitalRead(TACHO_PIN);

  tick_pos = ticks_count;

  Timer0_Cfg = timerBegin(0, ESP.getCpuFreqMHz(), true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 500, true);
  timerAlarmEnable(Timer0_Cfg);
}

void drawResults()
{
  tft.fillScreen(TFT_BLACK);

  for(unsigned i=0;i<tick_pos;i++)
  {
    int32_t h = ticks[i]*2;
    tft.drawLine(i,scren_height,i,scren_height - h,TFT_RED);
  }
}

void startTest()
{
    tick_pos=0;
    last_time = millis();
    tick_count = 0;
    tft.fillScreen(TFT_BLACK);
    tft.drawString("321...", 0, 0, 7);
}

void fillTicks()
{
  unsigned long ms = millis();
  unsigned long diff_ms = ms - last_time;

  if(diff_ms < TICK_MS)
    return;

  last_time = ms;

  int32_t tick = tick_count;
  tick_count = 0;

  Serial.print(" tick=");
  Serial.println(tick);

  unsigned affected_intervals = diff_ms/TICK_MS;

  int32_t last_tick = (tick_pos>0)? ticks[tick_pos-1] : 0;

  for(unsigned i=1;tick_pos<ticks_count && i<=affected_intervals ; ++tick_pos,++i)
  {
    int32_t v = (tick - last_tick)*i/affected_intervals + last_tick;
    ticks[tick_pos] = v;
  
    Serial.print("[");
    Serial.print(tick_pos);
    Serial.print("]=");
    Serial.println(v);
  }
}

void loop()
{
  if(tick_pos < ticks_count)
  {
    fillTicks();
    
    if(tick_pos >= ticks_count)
    {
      drawResults();
    }

    return;
  }
  
  if(touch.read())
    startTest();
}
