#include <TheLibrary.h>

TDigitalInput Light(5);
TDigitalOutput LED(6);
TDigitalOutput Speaker(4);

#define CheckPeriod 100
#define WaterDensity 70
#define VegsCount 2
#define PumpDuration 1000

void Sound(int ANum)
{
  for (int i = 0; i < ANum; i++)
  {
    Speaker.On();
    delay(20);
    Speaker.Off();
  };
};

class TVeg
{
private:
  TAnalogInput *FWaterCheck;
  TDigitalOutput *FPump;

  unsigned long FCheckTime;
  int FID;

public:
  TVeg(int AID)
  {
    FID = AID;
    FWaterCheck = new TAnalogInput(A0 + FID, 815, 475, 100);
    FPump = new TDigitalOutput(2 + FID);
    FCheckTime = 0;

    Loop();
  };

  void Loop()
  {
    //Serial.print("Check: ");
    //Serial.println(FID);
    //Serial.println(FCheckTime);

    if (++FCheckTime > CheckPeriod)
    {
      //Serial.print("Water: ");
      //Serial.println(FID);

      if (FWaterCheck->Get() < WaterDensity)
      {
        //Serial.print("pump: ");
        //Serial.println(FID);

        FCheckTime = 0;

        Sound(1);

        FPump->On();
        delay(PumpDuration);
        FPump->Off();

        delay(1000);
      };
    };
  };
};

TVeg *Vegs[VegsCount];

void setup()
{
  //Serial.begin(9600);

  for (int i = 0; i < VegsCount; i++)
    Vegs[i] = new TVeg(i);

  //Serial.println("Start");
};

void loop()
{
  if (!Light.Get())
  {
    LED.On();
    delay(200);
    LED.Off();

    for (int i = 0; i < VegsCount; i++)
      Vegs[i]->Loop();
  };

  Sleep();
};