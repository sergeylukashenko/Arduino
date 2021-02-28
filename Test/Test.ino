#include <TheLibrary.h>
#include <Servo.h>

Servo Shlag;
TDigitalInput Sensor(7);
TDigitalOutput Red(9);
TDigitalOutput Green(10);

void setup()
{
  Shlag.attach(8);
}

void loop()
{
  if (!Sensor.Get())
  {
    Red.Off();
    Green.On();
    SetS(90);
    delay(5000);

    Red.On();
  };

  SetS(0);
  Red.Invert();
  Green.Off();

  delay(1000);
};

void SetS(int AValue)
{
  int i = Shlag.read();
  int Offset = -1;

  if (i < AValue)
    Offset = 1;

  for (; i != AValue; i += Offset)
  {
    delay(25);

    Shlag.write(i);
  }
}