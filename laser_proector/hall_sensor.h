
class Esp32HallSensor
{
public:

   Esp32HallSensor()
  {
    val = 0;
    peak_val = 0;
    peak_milis = 0;
    prev_peak_milis = 0;
    mil_delay= 0;
  }

  void do_cycle()
  {
    int val = hallRead();
    
    if(val>46 && val>peak_val)
    {
      peak_val = val;
      peak_milis = ESP.getCycleCount();
    }
    else if(val<30 && peak_val > 0)
    {
      mil_delay = peak_milis - prev_peak_milis;
      prev_peak_milis = peak_milis;
      peak_val = 0;
    
      //Serial.print("mil_delay=");
      //Serial.println(mil_delay);
    }
  }

    inline uint32_t get_delay() const{return mil_delay;}
    inline uint32_t get_peak_millis() const{return prev_peak_milis;}
private:
  uint32_t val;
  uint32_t peak_val;
  uint32_t peak_milis;
  uint32_t prev_peak_milis;

  uint32_t mil_delay;
};
