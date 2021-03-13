#include <Arduino.h>
#include <TheLibrary.h>
#include <Servo.h>

Servo Shlag;

TDigitalInput ButtonRed(4);
TDigitalInput ButtonWhite(5);
TDigitalInput ButtonBlue(6);
TDigitalInput ButtonBlack(7);

TDigitalOutput Red(8);
TDigitalOutput Green(9);
TDigitalOutput Led(13);

int State;

void setup()
{
    Led.Off();
    Shlag.attach(3);
    Close();
}

void Close()
{
    Red.On();
    Rotate(95);
    State = 0;    
}

void Open()
{
    Green.On();
    Rotate(200);
}

bool Change(int from, int to)
{
    bool res = State == from;

    if (res)
    {
        Led.On();
        delay(100);
        Led.Off();

        State = to;
    }

    return res;
}

void loop()
{
    delay(300);

    if (State == 4)
    {
        Red.Off();
        Green.Invert();
    }
    else
    {
        Green.Off();
        Red.Invert();
    }

    if (!ButtonRed.Get())
    {
        Change(0, 1);
     
        if (Change(4, 0))
            Close();
    }

    if (!ButtonBlue.Get())
        Change(1, 2);

    if (!ButtonWhite.Get())
        Change(2, 3);

    if (!ButtonBlack.Get())
        if (Change(3, 4))           
            Open();
};

void Rotate(int AValue)
{
    int i = Shlag.read();
    int Offset = -1;

    if (i < AValue)
        Offset = 1;

    for (; i != AValue; i += Offset)
    {
        delay(50);

        Shlag.write(i);
    }
}