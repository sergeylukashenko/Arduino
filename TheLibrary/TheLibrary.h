#ifndef TheLibrary
#define TheLibrary

#include "Arduino.h"
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

extern unsigned long time_units;
extern void (*func)();
extern volatile unsigned long count;
extern volatile char overflowing;
extern volatile unsigned int tcnt2;

unsigned long time_units;
volatile unsigned long count;
volatile char overflowing;
volatile unsigned int tcnt2;

/**
@param resolution
0.001 implies a 1 ms (1/1000s = 0.001s = 1ms) resolution. Therefore,
0.0005 implies a 0.5 ms (1/2000s) resolution. And so on.
*/
void set(unsigned long units, double resolution) {
	float prescaler = 0.0;

	if (units == 0)
	time_units = 1;
	else
	time_units = units;

	TIMSK2 &= ~(1 << TOIE2);
	TCCR2A &= ~((1 << WGM21) | (1 << WGM20));
	TCCR2B &= ~(1 << WGM22);
	ASSR &= ~(1 << AS2);
	TIMSK2 &= ~(1 << OCIE2A);

	if ((F_CPU >= 1000000UL) && (F_CPU <= 16000000UL))
	{  // prescaler set to 64
		TCCR2B |= (1 << CS22);
		TCCR2B &= ~((1 << CS21) | (1 << CS20));
		prescaler = 64.0;
		} else if (F_CPU < 1000000UL) { // prescaler set to 8
		TCCR2B |= (1 << CS21);
		TCCR2B &= ~((1 << CS22) | (1 << CS20));
		prescaler = 8.0;
		} else { // F_CPU > 16Mhz, prescaler set to 128
		TCCR2B |= ((1 << CS22) | (1 << CS20));
		TCCR2B &= ~(1 << CS21);
		prescaler = 128.0;
	}

	tcnt2 = 256 - (int)((float)F_CPU * resolution / prescaler);
}

void start()
{
	count = 0;
	overflowing = 0;
	TCNT2 = tcnt2;
	TIMSK2 |= (1<<TOIE2);
}

void stop()
{
	TIMSK2 &= ~(1<<TOIE2);
}

const int  MaxTimers = 10;

class THandler
{
	public:
	virtual void OnEvent() = 0;
	
	void Init();
};

THandler *Timers[MaxTimers];

void THandler::Init()
{
	if (!Timers[0])
	{
		set(1, 0.001);
		start();
	};
	
	for (int i = 0; i < MaxTimers; i++)
	{
		if (!Timers[i])
		{
			Timers[i] = this;
			
			break;
		};
	};
};

void CheckTimers()
{
	for (int i = 0; i < MaxTimers; i++)
	if (Timers[i])
	Timers[i]->OnEvent();
};

ISR(TIMER2_OVF_vect)
{
	TCNT2 = tcnt2;

	CheckTimers();
}

class TTimer : public THandler
{
	private:
	unsigned long FPrevTime;
	int FPeriod;
	int FCount;
	void (*FRun)();
	void (*FFinish)();
	
	protected:
	void OnEvent()
	{
		if (FPeriod <= 0)
		return;
		
		unsigned long Time = millis();

		if (Time - FPrevTime > FPeriod)
		{
			FPrevTime = Time;

			if (FCount < 0)
			FRun();
			else
			{
				if (FCount-- > 0)
				{
					FRun();
					
					if (FCount == 0)
					{
						FPeriod = 0;
						
						FFinish();
					};
				};
			};
		};
	};
	
	public :
	void Start(int APeriod, void (*ARun)())
	{
		Start(APeriod, ARun, -1, 0);
	};
	
	void Start(int APeriod, void (*ARun)(), int ACount, void (*AFinish)())
	{
		FPrevTime = millis();
		FRun = ARun;
		FCount = ACount;
		FFinish = AFinish;
		FPeriod = APeriod;
	};
	
	
	void Stop()
	{
		FPeriod = 0;
	};
	
	TTimer()
	{
		Stop();
		Init();
	};
	
	TTimer(int APeriod, void (*ARun)())
	{
		TTimer();
		Start(APeriod, ARun);
	};
};

class TPin
{
	protected :
	int FID;
	int FValue;

	public :
	TPin(int AID)
	{
		FID = AID;
	};
};

class TInput : public TPin
{
	protected :
	virtual int Read() = 0;

	public :
	TInput(int AID) : TPin(AID)
	{
		pinMode(AID, INPUT);
	};

	bool IsChanged()
	{
		int Value = Read();

		bool Res = (FValue != Value);

		FValue = Value;

		return Res;
	};

	int Get()
	{
		FValue = Read();

		return FValue;
	};

	int Value()
	{
		return FValue;
	};
};

class TDigitalInput : public TInput
{
	public :
	TDigitalInput(int AID) : TInput(AID)
	{

	};

	protected :
	int Read()
	{
		return digitalRead(FID);
	};
};

class TAnalogInput : public TInput
{
	private:
	int FInputMin;
	int FInputMax;
	double FRange;
	
	public:
	TAnalogInput(int AID) : TAnalogInput(AID, 0, 0, 0)
	{

	};
	
	TAnalogInput(int AID, int AInputMin, int AInputMax, int ARange) : TInput(AID)
	{
		FInputMin = AInputMin;
		FInputMax = AInputMax;
		FRange = ARange;
	};

	protected:
	int Read()
	{
		if (FRange == 0)
	       return analogRead(FID);
  		else
		   return min(FRange, max(0, FRange * double(analogRead(FID) - FInputMin) / double(FInputMax - FInputMin)));
	};
};

class TOutput : public TPin
{
	public :
	TOutput (int AID) : TPin(AID)
	{
		pinMode(AID, OUTPUT);
	};

	void On()
	{
		Set(HIGH);
	};

	void Off()
	{
		Set(LOW);
	};

	void Invert()
	{
		Set(HIGH - Get());
	};

	int Get()
	{
		return FValue;
	};

	virtual void Set(int AValue) = 0;
};

class TDigitalOutput : public TOutput
{
	public :
	TDigitalOutput(int AID) : TOutput(AID)
	{

	};

	void Set(int AValue)
	{
		FValue = AValue;

		digitalWrite(FID, AValue);
	};
};

class TAnalogOutput : public TOutput
{
	public:
	TAnalogOutput(int AID) : TOutput(AID)
	{

	};

	void Set(int AValue)
	{
		FValue = AValue;

		analogWrite(FID, AValue);
		
		//noTone(FID);
	};
	
	void Tone(int AFreq)
	{
		tone(FID, AFreq);
	};
};

void Sleep()
{
	wdt_enable(WDTO_8S); //Задаем интервал сторожевого таймера
	WDTCSR |= (1 << WDIE); //Устанавливаем бит WDIE регистра WDTCSR для разрешения прерываний от сторожевого таймера
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Устанавливаем интересующий нас режим
	sleep_mode(); // Переводим МК в спящий режим
};

ISR (WDT_vect) {
	wdt_disable();
};

#endif