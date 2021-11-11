
class DelayedDigitalSensor
{
protected:
  int pin;
  int mode;
  int repeat_delay;
  
public:
  time_t processedTime;
  time_t lastTickTime;
  int ticks;

  DelayedDigitalSensor(int _pin,int _mode,int _repeat_delay=300) : 
    pin(_pin),mode(_mode),repeat_delay(_repeat_delay)
  {
    processedTime = 0;
    lastTickTime = 0;
    ticks = 0;
  }
  
	void readRTC(uint32_t& offset)
	{
/*		
		ESP.rtcUserMemoryRead(offset, (uint32_t*)&processedTime,sizeof(processedTime));
		offset += sizeof(processedTime);

		ESP.rtcUserMemoryRead(offset, (uint32_t*)&lastTickTime,sizeof(lastTickTime));
		offset += sizeof(lastTickTime);

		ESP.rtcUserMemoryRead(offset, (uint32_t*)&ticks,sizeof(ticks));
		offset += sizeof(ticks);
*/		
	}

	void writeRTC(uint32_t& offset)
	{
/*		
		ESP.rtcUserMemoryWrite(offset, (uint32_t*)&processedTime,sizeof(processedTime));
		offset += sizeof(processedTime);

		ESP.rtcUserMemoryWrite(offset, (uint32_t*)&lastTickTime,sizeof(lastTickTime));
		offset += sizeof(lastTickTime);

		ESP.rtcUserMemoryWrite(offset, (uint32_t*)&ticks,sizeof(ticks));
		offset += sizeof(ticks);
*/		
	}

protected:
  
  bool onOnLoopAllert()
  {
      lastTickTime=time(nullptr);
      ticks++;
      
      if(lastTickTime>processedTime + repeat_delay)
      {
        processedTime=lastTickTime;
        return true;
      }
	  return false;
  }
  
  void onOffLoopAlert()
  {
	  time_t t =time(nullptr);
	  if(ticks!=0 && t>lastTickTime+repeat_delay)
	  {
		ticks=0;
	  }
  }
};

template<int PIN>
class DelayedDigitalInt : public DelayedDigitalSensor
{
	static bool flag;

	static void Interrupt()
	{
	  flag = true;
	}

public:
  DelayedDigitalInt(int _mode,int _repeat_delay=300) : 
	DelayedDigitalSensor(PIN,_mode,_repeat_delay)
  {
  }

  void setup()
  {
    pinMode(pin, INPUT);
	enableInterrupt();
  }

  void enableInterrupt()
  {
	  attachInterrupt(digitalPinToInterrupt(pin), Interrupt, mode);
  }
  
  void disableInterrupt()
  {
	  detachInterrupt(digitalPinToInterrupt(pin));
  }

  bool isLoopAllert()
  {
    if(flag)
    {
      flag = false;
	  return onOnLoopAllert();
    }
	else
	{
		onOffLoopAlert();
	}

    return false;
  }
};

template<int PIN>
bool DelayedDigitalInt<PIN>::flag = false;





class DelayedDigitalSoft : public DelayedDigitalSensor
{
public:
	int lastVal;

	DelayedDigitalSoft(int _pin, int _mode,int _repeat_delay=300) : 
	DelayedDigitalSensor(_pin,_mode,_repeat_delay)
	{
	  lastVal = 0;
	}

	void setup()
	{
		pinMode(pin, INPUT);
		lastVal = digitalRead(pin);
	}

	bool isLoopAllert()
	{
		int val = digitalRead(pin);
		if(lastVal != val)
		{
			lastVal = val;

			if((val==HIGH) == (mode == RISING))
			{
			  return onOnLoopAllert();
			}
		}

		onOffLoopAlert();
		return false;
	}

	void readRTC(uint32_t& offset)
	{
		DelayedDigitalSensor::readRTC(offset);
/*
		ESP.rtcUserMemoryRead(offset, (uint32_t*)&lastVal,sizeof(lastVal));
		offset += sizeof(lastVal);
*/		
	}

	void writeRTC(uint32_t& offset)
	{
		DelayedDigitalSensor::writeRTC(offset);
/*
		ESP.rtcUserMemoryWrite(offset, (uint32_t*)&lastVal,sizeof(lastVal));
		offset += sizeof(lastVal);
*/		
	}
};
