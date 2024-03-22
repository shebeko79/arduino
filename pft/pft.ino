//Pulmonary function tests
#define TOUCH_MODULES_CST_SELF        //Use CST816 by default

#include "Arduino.h"
#include "TFT_eSPI.h"
#include "TouchLib.h"
#include "Wire.h"
#include "pin_config.h"
#include "NotoSansBold15.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

TouchLib touch(Wire, PIN_IIC_SDA, PIN_IIC_SCL, CTS820_SLAVE_ADDRESS, PIN_TOUCH_RES);

hw_timer_t *Timer0_Cfg = nullptr;

int32_t tick_count = 0;
unsigned long last_time = 0;

#define TACHO_PIN 1
int last_tacho_state = LOW;

#define TICK_MS 100
constexpr unsigned test_seconds=8;
constexpr unsigned ticks_count = test_seconds*1000/TICK_MS;
int32_t ticks[ticks_count];
unsigned tick_pos=0;

int16_t screen_height = 2;
int16_t screen_width = 2;

unsigned long last_testing_ms=0;

void startTest();
void drawReady();
void drawTesting();

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

  tft.init();
  tft.invertDisplay(1);
  tft.setRotation(1);

  ledcSetup(0, 2000, 8);
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, 255);

  screen_height = tft.height();
  screen_width = tft.width();

  sprite.createSprite(screen_width, screen_height);
  sprite.setSwapBytes(true);

  drawReady();

  Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);
  if (!touch.init())
  {
      Serial.println("Touch IC not found");
  }
  
  last_tacho_state = digitalRead(TACHO_PIN);

  tick_pos = ticks_count;

  Timer0_Cfg = timerBegin(0, ESP.getCpuFreqMHz(), true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 200, true);
  timerAlarmEnable(Timer0_Cfg);
}

void drawResults()
{
  sprite.fillSprite(TFT_BLACK);

  int32_t x_pad = screen_width / (test_seconds + 2);
  int32_t x_multiply = x_pad*test_seconds;

  constexpr int32_t y_pad = 16;
  constexpr int32_t y_grid_count = 4;
  constexpr int32_t y_grid_size = 32;

  for(int32_t i = 0; i<=test_seconds;i++)
  {
    int32_t x = (i+1)*x_pad;
    int32_t y = y_grid_count*y_grid_size+y_pad;
    sprite.drawLine(x,screen_height - y_pad,x,screen_height - y,TFT_BLUE);
  }

  for(int32_t i = 0; i<=y_grid_count;i++)
  {
    int32_t y = i*y_grid_size + y_pad;
    sprite.drawLine(x_pad,screen_height - y,screen_width-x_pad,screen_height - y,TFT_BLUE);
  }

  for(unsigned i=1;i<ticks_count;i++)
  {
    int x1 = x_multiply*(i-1)/ticks_count + x_pad;
    int x2 = x_multiply*i/ticks_count + x_pad;
    
    int32_t y1 = ticks[i-1]*2 + y_pad;
    int32_t y2 = ticks[i]*2 + y_pad;

    sprite.drawLine(x1,screen_height - y1,x2,screen_height - y2,TFT_RED);
  }

  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.loadFont(NotoSansBold15);
  sprite.setTextDatum(TR_DATUM);
  sprite.drawString("Results", screen_width, 0);

  sprite.pushSprite(0, 0);
}

void drawReady()
{
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_GREEN, TFT_BLACK);
  sprite.loadFont(NotoSansBold15);
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString("PRESS", screen_width/2, screen_height/2);
  sprite.pushSprite(0, 0);
}

void drawTesting()
{
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_RED, TFT_BLACK);
  sprite.loadFont(NotoSansBold15);
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString("Testing...", screen_width/2, screen_height/2);
  sprite.pushSprite(0, 0);
}

void startTest()
{
    tick_pos=0;
    last_time = millis();
    tick_count = 0;

    drawTesting();
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

//  Serial.print(" tick=");
//  Serial.println(tick);

  unsigned affected_intervals = diff_ms/TICK_MS;

  int32_t last_tick = (tick_pos>0)? ticks[tick_pos-1] : 0;

  for(unsigned i=1;tick_pos<ticks_count && i<=affected_intervals ; ++tick_pos,++i)
  {
    int32_t v = (tick - last_tick)*i/affected_intervals + last_tick;
    ticks[tick_pos] = v;
  
//    Serial.print("[");
//    Serial.print(tick_pos);
//    Serial.print("]=");
//    Serial.println(v);
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
      last_testing_ms = millis();
    }

    return;
  }
  
  if(touch.read())
  {
    bool inside_start_button = false;
    int mi = touch.getPointNum();
    for(int i=0;i<mi;i++)
    {
      TP_Point t = touch.getPoint(i);
      inside_start_button = inside_start_button || (
        //Screen is rotated
        t.y>screen_width/4 && t.y<screen_width/4*3 &&
        t.x>screen_height/4 && t.x<screen_height/4*3);
    }

    if(inside_start_button && millis()-last_testing_ms>3000)
    {
      startTest();
    }
  }
}
