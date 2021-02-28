#include <DFMiniMp3.h>
#include <FastLED.h>
#include <SoftwareSerial.h>
#include "TheLibrary.h"

TAnalogOutput LeftForward(9);
TAnalogOutput LeftBackward(3);
TAnalogOutput RightForward(10);
TAnalogOutput RightBackward(5);

TAnalogInput PIR(A7);

TAnalogInput Battery(A2, 0, 650, 85);

TAnalogInput LeftFoto(A1, 900, 10, 255);
TAnalogInput RightFoto(A0, 900, 10, 255);

TDigitalInput RF(6);
TDigitalInput RB(A4);
TDigitalInput RL(A5);
TDigitalInput RR(A6);

TDigitalOutput PowerOff(7);
TDigitalOutput Light(12);

#define DisapearSound 12
#define DeniedSound 13
#define MusicSound 14
#define SpaceEngineSound 15
#define BootSound 16
#define BulkSound 11
#define TryMoveSound 5
#define ImTiredSound 9
#define LowPowerSound 10
#define WakeUpSound 6
#define SleepSound 7
#define PowerOffSound 2
#define ScanSound 4
#define PoliceSound 8

#define Dark 50
#define LightDiff 20

#define MinSpeed 90
#define StepSpeed 5
#define SpeedDelay 5
#define Slow 85
#define Fast 150

#define LED_COUNT 16

TDigitalInput IRR(8);
TDigitalInput IRL(13);
TDigitalInput IRM(A3);

TTimer PulseTimer;
TTimer PoliceTimer;
TTimer LoadTimer;

unsigned long fc = 0;
unsigned long fv = 0;
int FotoMax = 1000;

int ForceLight()
{
  return 1.2 * (fv / fc);
};

int ls = 0;
bool ld = false;

int SoundID = 0;

CRGBArray<LED_COUNT> Circle;

class Mp3Notify
{
public:
  static void OnError(uint16_t errorCode){};
  static void OnPlayFinished(uint16_t track){};
  static void OnCardOnline(uint16_t code){};
  static void OnCardInserted(uint16_t code){};
  static void OnCardRemoved(uint16_t code){};
};

SoftwareSerial MP3Serial(2, 4);
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(MP3Serial);

int Mode = -1;
int Driving = 0;
bool PrevNoDrive = false;

void setup()
{
  //Serial.end();
  Serial.begin(9600);

  PowerOff.On();

  Serial.println(Battery.Get());

  mp3.begin();

  if (Battery.Get() > 55)
    mp3.setVolume(30);
  else
    mp3.setVolume(5);

  FotoMax = Foto();

  FastLED.addLeds<NEOPIXEL, 11>(Circle, LED_COUNT);
  Mode = -1;

  // Fast start
  //Mode = 2;
  //return;

  ls = 0;

  LoadTimer.Start(600, Load);

  Sound(BootSound, false);
};

void SetCircle(CRGB AColor)
{
  for (int i = 0; i < LED_COUNT; i++)
    Circle[i] = AColor;

  FastLED.show();
};

void CircleOff()
{
  SetCircle(CRGB::Black);
};

int Foto()
{
  return (LeftFoto.Get() + RightFoto.Get()) / 2;
};

void loop()
{
  //delay(10);
  //Serial.println(IRR.Get());
  //return;

  Remote();
  //CheckPeople();

  switch (Mode)
  {
  case 1: // WakeUp
    Serial.println("WakeUp");

    PowerOff.Off();
    Drive(0, 0);

    Mode = 2;

    Sound(WakeUpSound, true);
    delay(1500);

    //Dance();

    break;

  case 2:
    // if (Foto() < FotoMax * 0.8)
    // FindLight();

    CheckBattery();

    /* if (PIR.Get() < 500)
          {
           Mode = 100;

           return;
          };*/

    CheckIR();
    UpdateLight();
    //ReverseOnDark();

    Remote();

    FollowToLight(0, 30);

    break;
  };
};

unsigned long SoundTick = 0;

bool IsSilent()
{
  mp3.loop();
  return ((mp3.getStatus() != 513) /* || (mp3.getStatus() == 512)*/);
};

void Sound(int AID, bool AWait)
{
  if ((SoundID != AID) || /*IsSilent()*/ (millis() - SoundTick > 3000))
  {
    SoundTick = millis();
    SoundID = AID;

    mp3.playMp3FolderTrack(AID);

    /*  while (AWait && !IsSilent())
        delay(100);*/
  };
};

void Drive(int ALeft, int ARight)
{
  return;
  if (((ALeft == 0) && (ARight == 0)) || !PowerEnough())
  {
    LeftForward.Off();
    RightForward.Off();
    RightBackward.Off();
    LeftBackward.Off();
  }
  else
  {
    int Left = MinSpeed;
    int Right = MinSpeed;

    while (1)
    {
      if (ALeft > 0)
      {
        LeftBackward.Off();
        LeftForward.Set(Left);
      }
      else
      {
        LeftForward.Off();
        LeftBackward.Set(Left);
      };

      if (ARight > 0)
      {
        RightBackward.Off();
        RightForward.Set(Right);
      }
      else
      {
        RightForward.Off();
        RightBackward.Set(Right);
      };

      if ((Left == abs(ALeft)) && (Right == abs(ARight)))
        break;

      Left = min(abs(ALeft), Left + StepSpeed);
      Right = min(abs(ARight), Right + StepSpeed);

      delay(SpeedDelay);
    };
  };
};

void Load()
{
  if (ls == LED_COUNT)
  {
    LoadTimer.Stop();
    CircleOff();

    Mode = 1;

    return;
  };

  fc++;
  fv += (LeftFoto.Get() + RightFoto.Get()) / 2;

  Circle[ls++] = CRGB(0, 10, 0);

  FastLED.show();
};

void Pulse()
{
  if (ld)
  {
    ls++;

    if (ls == 50)
      ld = false;
  }
  else
  {
    ls--;

    if (ls < 5)
      ld = true;
  };

  SetCircle(CRGB(0, 0, ls));
};

int lms = 0;
bool lm;
bool lf = true;

void Police()
{
  lf ^= 1;

  CircleOff();

  if (lf)
  {
    if (lm)
    {
      for (int i = (LED_COUNT / 2); i < LED_COUNT; i++)
        Circle[i] = CRGB::Red;
    }
    else
    {
      for (int i = 0; i < (LED_COUNT / 2); i++)
        Circle[i] = CRGB::Blue;
    };

    if (++lms >= 4)
    {
      lms = 0;
      lm ^= 1;
    };

    FastLED.show();
  };
};

void Dance()
{
  delay(1000);

  Drive(90, 250);

  delay(1000);

  Drive(150, -150);

  delay(1000);

  Drive(-90, -90);

  delay(1000);

  Drive(90, -90);

  delay(1000);

  Drive(-90, 90);

  delay(1000);

  Drive(-150, 150);

  delay(1000);

  Drive(0, 0);
};

void Stop()
{
  PoliceTimer.Stop();
  PulseTimer.Stop();
  LoadTimer.Stop();
  CircleOff();
  Drive(0, 0);
};

void BackWard(int ATime)
{
  Drive(-Slow, -Slow);

  delay(ATime);
};

void Forward()
{
  Drive(Slow, Slow);
};

void Left()
{
  Serial.println("Left");
  Drive(Slow, -Slow);
};

void Right()
{
  Serial.println("Right");
  Drive(-Slow, Slow);
};

unsigned long FindFinish;

bool Step(int A, int B)
{
  for (int i = 0; i < LED_COUNT; i += 2)
  {
    Circle[i] = CRGB(A / 10, A / 10, A / 10);
    Circle[i + 1] = CRGB(B / 10, B / 10, B / 10);
  };

  FastLED.show();
  Left();
  //  delay(90);
  //  Drive(0, 0);
  //  delay(10);
  return (millis() < FindFinish);
};

void ResetFind()
{
  FindFinish = millis() + 2000;
};

void FindLight()
{
  FotoMax = 0;

#define FindFotoCount 20

  bool Found = false;

  Sound(ScanSound, false);

  PoliceTimer.Stop();
  PulseTimer.Stop();

  while (!Found)
  {
    ResetFind();

    while (Step(0, FotoMax))
      FotoMax = max(FotoMax, Foto());

    Serial.println("found");

    FotoMax *= 0.9;

    ResetFind();

    while (Step(Foto(), FotoMax))
    {
      if (FotoMax <= Foto())
      {
        Found = true;

        break;
      };
    };
  };
};

void UpdateLight()
{
  if ((LeftFoto.Get() < Dark) && (RightFoto.Get() < Dark))
    Light.On();
  else
    Light.Off();
};

void FollowToLight(int AForce, int ADiff)
{
  Serial.println("Follow to light");

  if ((LeftFoto.Get() < AForce) && (RightFoto.Get() < AForce))
  {
    Serial.println("Stop");

    Drive(0, 0);

    PoliceTimer.Stop();
    PulseTimer.Start(18, Pulse);

    Sound(ScanSound, false);

    PrevNoDrive = true;
  }
  else
  {
    if (PrevNoDrive)
      Driving++;

    /*  if (Driving > 200)
      {
        Stop();
        SetCircle(CRGB(10, 10, 0));
        Sound(ImTiredSound, true);

        delay(10000);

        Stop();

        Driving = 0;
      };*/

    if (abs(LeftFoto.Get() - RightFoto.Get()) < ADiff)
    {
      Stop();
      PoliceTimer.Start(100, Police);

      Sound(PoliceSound, false);
      Drive(Slow, Slow);
    }
    else
    {
      Stop();
      Sound(TryMoveSound, false);

      SetCircle(CRGB(CRGB::DarkOrange) / 2);

      if (LeftFoto.Get() > RightFoto.Get())
        Left();
      else
        Right();
    };
  };
};

void Rotate()
{
  Serial.println("Rotate");
  Stop();

  SetCircle(CRGB(CRGB::DarkOliveGreen) / 2);

  BackWard(400);

  if (LeftFoto.Get() > RightFoto.Get())
    Left();
  else
    Right();

  delay(700);
  Forward();
  delay(400);
  Stop();
};

void ReverseOnDark()
{
  Serial.println("---");
  Serial.println(FotoMax);
  Serial.println(Foto());

  if (Foto() < FotoMax * 0.9)
  {
    Stop();

    Sound(BulkSound, false);

    for (int r = 2; r < 9; r++)
    {
      for (int i = 0; i < LED_COUNT; i += 2)
      {
        Circle[i] = CRGB(CRGB::Yellow) / r;
        Circle[i + 1] = CRGB(CRGB::Pink) / r;
      };

      FastLED.show();
      delay(30);
    };

    CircleOff();

    delay(1000);

    BackWard(300);

    Rotate();

    FotoMax = Foto();
  };

  FotoMax = (FotoMax + Foto()) / 2;
};

void ToSleep()
{
  Stop();
  Light.Off();

  Sound(PowerOffSound, true);

  delay(2000);

  mp3.stop();

  PowerOff.On();

  Mode = 0;
};

void CheckBattery()
{
  Serial.println(Battery.Get());

  if (!PowerEnough())
  {
    Stop();

    Sound(LowPowerSound, false);
    delay(3000);

    ToSleep();

    delay(10000);
  };
};

bool PowerEnough()
{
  return Battery.Get() > 65;
};

unsigned long PeopleTime = 0;

void CheckPeople()
{
  Serial.println("CheckPeople...");

  if ((Mode == 0) && (PIR.Get() > 500) && PowerEnough())
  {
    Mode = 1;

    Serial.println("People: Yes");
  };

  if (Mode > 0)
  {
    if ((millis() - PeopleTime) > (1000 * 15))
    {
      Stop();
      PulseTimer.Start(18, Pulse);
      delay(3000);

      PeopleTime = millis();

      if (PIR.Get() < 500)
      {
        Serial.println("People: No");
        Stop();

        Sound(SleepSound, true);

        delay(3000);

        ToSleep();
      };
    };
  };
};

void CheckIR()
{
  if (!IRM.Get() || (!IRL.Get() && !IRR.Get()))
  {
    Sound(BulkSound, false);
    Rotate();
  }
  else if (!IRL.Get())
  {
    while (!IRL.Get())
      Right();

    delay(100);
  }
  else if (!IRR.Get())
  {
    while (!IRR.Get())
      Left();

    delay(100);
  };
};

void Remote()
{
  if (RF.Get())
    Serial.println("F");

  if (RB.Get())
    Serial.println("B");

  if (RL.Get())
    Serial.println("L");

  if (RR.Get())
    Serial.println("R");
}