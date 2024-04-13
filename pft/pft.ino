//Pulmonary function tests
#define TOUCH_MODULES_CST_SELF        //Use CST816 by default

#include "Arduino.h"
#include "TFT_eSPI.h"
#include "TouchLib.h"
#include "Wire.h"
#include "pin_config.h"
#include "NotoSansBold15.h"
#include <esp_adc_cal.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

TouchLib touch(Wire, PIN_IIC_SDA, PIN_IIC_SCL, CTS820_SLAVE_ADDRESS, PIN_TOUCH_RES);

hw_timer_t *Timer0_Cfg = nullptr;
bool timerEnabled = false;

#define TACHO_PIN 1

constexpr unsigned timer_us=200;
constexpr unsigned timers_per_interval=200;
constexpr unsigned test_seconds=8;

constexpr unsigned ticks_count = test_seconds*1000000/timer_us/ timers_per_interval;
volatile int32_t ticks[ticks_count];
volatile int32_t timer_cycle=0;

int16_t screen_height = 2;
int16_t screen_width = 2;

unsigned results_start_pos = 0;
unsigned results_end_pos = ticks_count;

unsigned show_result_cycle = 0;
unsigned long last_result_ms=0;

void startTest();
void drawReady();
void drawTesting();

unsigned cycle2interval(int32_t c)
{
  return c / timers_per_interval;
}

void IRAM_ATTR Timer0_ISR()
{
  ++timer_cycle;
}

void IRAM_ATTR pin_isr()
{
  unsigned pos = cycle2interval(timer_cycle);
  if(pos<ticks_count)
    ++ticks[pos];
}

void setup()
{
  gpio_hold_dis((gpio_num_t)PIN_TOUCH_RES);
  
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  
  pinMode(PIN_TOUCH_RES, OUTPUT);
  digitalWrite(PIN_TOUCH_RES, LOW);
  delay(500);
  digitalWrite(PIN_TOUCH_RES, HIGH);

  Serial.begin(115200);
  
  pinMode(TACHO_PIN, INPUT);

  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  Serial.begin(115200);

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
  
  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, timer_us, true);
}

void drawPower()
{
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    uint32_t raw = analogRead(PIN_BAT_VOLT);
    uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;
    
    String str;

    if(v1>4300)
      str="USB";
    else
    {
      long p=map(v1, 2700, 4200, 0, 100);
      str=String(p);
      str+="%";
    }
    
    sprite.setTextDatum(TR_DATUM);
    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    sprite.drawString(str, screen_width, 0);
    Serial.println(v1);
}

void calculateEffectiveRange()
{
  results_start_pos = 0;
  results_end_pos = ticks_count;
  
  for(unsigned i=0;i<ticks_count-4;i++)
  {
    if(ticks[i]!=0 && ticks[i+1]!=0 && ticks[i+2]!=0 && ticks[i+3]!=0)
    {
      results_start_pos = i;
      break;
    }
  }

  if(results_start_pos>0)
  for(unsigned i=results_start_pos;i<ticks_count-4;i++)
  {
    if(ticks[i]==0 && ticks[i+1]==0 && ticks[i+2]==0 && ticks[i+3]==0)
    {
      results_end_pos = i;
      break;
    }
  }
}

void drawResults()
{
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.loadFont(NotoSansBold15);

  int32_t x_pad = screen_width / (test_seconds + 2);

  constexpr int32_t y_pad = 16;
  constexpr int32_t y_grid_count = 4;
  constexpr int32_t y_grid_size = 32;


  int32_t x_multiply = x_pad*test_seconds;
  int32_t scale = int(x_multiply/(results_end_pos-results_start_pos));

  if(scale>8)
    scale = 8;

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

  sprite.setTextDatum(TC_DATUM);
  for(int32_t i = 0; i<=test_seconds;i++)
  {
    int32_t x = (i*scale+1)*x_pad;
    if(i*scale>test_seconds)
      break;

    String str(i);
    sprite.drawString(str, x, screen_height - (y_pad+1));
  }

  for(unsigned i=results_start_pos+1;i<results_end_pos;i++)
  {
    int x1 = x_multiply*(i-1-results_start_pos)/ticks_count*scale + x_pad;
    int x2 = x_multiply*(i-results_start_pos)/ticks_count*scale + x_pad;
    
    int32_t y1 = ticks[i-1] + y_pad;
    int32_t y2 = ticks[i] + y_pad;

    sprite.drawLine(x1,screen_height - y1,x2,screen_height - y2,TFT_RED);
  }

  sprite.setTextDatum(TC_DATUM);
  sprite.drawString("Results", screen_width/2, 0);

  drawPower();

  sprite.pushSprite(0, 0);
}

void drawResultsNumber()
{
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.loadFont(NotoSansBold15);

  int32_t tickSum = 0;

  for(unsigned i=results_start_pos;i<results_end_pos;i++)
  {
    tickSum += ticks[i];
  }

  String str(tickSum);

  sprite.setTextDatum(CC_DATUM);
  sprite.drawString(str, screen_width/2, screen_height/2);

  drawPower();

  sprite.pushSprite(0, 0);
}

void drawReady()
{
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_GREEN, TFT_BLACK);
  sprite.loadFont(NotoSansBold15);
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString("PRESS", screen_width/2, screen_height/2);
  drawPower();
  sprite.pushSprite(0, 0);
}

void drawTesting()
{
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_RED, TFT_BLACK);
  sprite.loadFont(NotoSansBold15);
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString("Testing...", screen_width/2, screen_height/2);
  drawPower();
  sprite.pushSprite(0, 0);
}

void startTest()
{
  drawTesting();

  for(unsigned i=0 ; i<ticks_count; i++)
    ticks[i] = 0;

  timerEnabled = true;
  timer_cycle = 0;
  timerAlarmEnable(Timer0_Cfg);
  show_result_cycle = 0;

  attachInterrupt(digitalPinToInterrupt(TACHO_PIN), pin_isr, RISING);
}

void loop()
{
  if(cycle2interval(timer_cycle) >= ticks_count && timerEnabled)
  {
    detachInterrupt(digitalPinToInterrupt(TACHO_PIN));
    timerAlarmDisable(Timer0_Cfg);

    calculateEffectiveRange();    
    drawResults();
    last_result_ms = millis();
    show_result_cycle = 1;
    timerEnabled = false;
  }

  if(show_result_cycle>0 && (millis()-last_result_ms)>10000)
  {
    if((show_result_cycle % 2) == 0)
      drawResults();
    else
      drawResultsNumber();
    
    show_result_cycle++;
    last_result_ms = millis();

    if(show_result_cycle>8)
    {
      touch.enableSleep();
      delay(2000);
      digitalWrite(PIN_POWER_ON, LOW);
      gpio_hold_en((gpio_num_t)PIN_TOUCH_RES);
      esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_TOUCH_INT, 0);
      esp_deep_sleep_start();
    }
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

    if(inside_start_button && (last_result_ms==0 || show_result_cycle>1))
    {
      startTest();
    }
  }
}
