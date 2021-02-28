#include <TheLibrary.h>
#include <SoftwareSerial.h>

SoftwareSerial GSM(2, 3); // RX, TX

TAnalogInput Sensor_A(0);
TAnalogInput Sensor_B(1);
TAnalogInput Sensor_C(2);

TAnalogInput Sensor_PIR(2);

TDigitalOutput Led_A(12);
TDigitalOutput Led_B(11);
TDigitalOutput Led_C(10);

TDigitalOutput Led(13);

TDigitalOutput Siren(8);

String val = "";
bool Enabled = false;
bool SirenOn = false;
String Res;

void ExecGSM(String ACommand, char* ARes)
{
  Serial.println(ACommand);

  while (1)
  { 
    GSM.println(ACommand);

    Res = GSM.readString();
    Serial.println(Res);
    
    if (Res.indexOf(ARes) > -1)
      break;

    delay(100);
  };
};

const String MasterNum = "+79202744533";

void setup()
{
 /* Serial.begin(9600);
  Serial.println("Setup...");

  GSM.begin(9600);
 
  Serial.println("begin");

  //ExecGSM("ATE1", "OK");
  ExecGSM("AT+IPR=9600", "");

  Serial.println("1");
  
  ExecGSM("AT", "OK");

  Serial.println("2");  

  ExecGSM("AT+CLIP=1", "OK");         // AОН
  ExecGSM("AT+CMGF=1", "OK");         // Текстовый формат SMS
  ExecGSM("AT+CSCS=\"GSM\"", "OK");   // Кодкровка текста sms
  ExecGSM("AT+CMGD=1,4", "OK");       // Удаление всех СМС
  ExecGSM("AT+CNMI=2,2", "OK");       // Автоприем СМС
  
  Serial.println("Initialized");

  ExecGSM("AT+COPS?", "+COPS: 0");

  Serial.println("Connected");

  SMS("Started", MasterNum); */
}

void loop()
{
  delay(1000);
  
  CheckSMS();

//  if (Enabled)
  {
    CheckAlarm();
  
    On();
    delay(50);
    Off();
  };
};

void Flash()
{
    Led_A.Invert();
    Led_B.Invert();
    Led_C.Invert();
};

void On()
{
    Led_A.On();
    Led_B.On();
    Led_C.On();
};

void Off()
{
    Led_A.Off();
    Led_B.Off();
    Led_C.Off();
};

void CheckSMS()
{
  if (GSM.available())
  {
    val = GSM.readString();

    ExecGSM("ATH0", "");

    if ((val.indexOf(MasterNum) > -1))
    {
      if (val.indexOf("off") > -1)
      {
        Enabled = false;
        SMS("Off", MasterNum);
      }
      else if (val.indexOf("ons") > -1)
      {
        Enabled = true;
        SirenOn = false;
        SMS("OnS", MasterNum);
      }
      else if (val.indexOf("on") > -1)
      {
        Enabled = true;
        SirenOn = true;
        SMS("On", MasterNum);
      };

    Serial.println(val);  //печатаем в монитор порта пришедшую строку
    };
  };
}

void CheckAlarm()
{
  if (Sensor_A.Get() || Sensor_B.Get() || Sensor_C.Get())
  {    
/*    Led.On();
    delay(1000);
    Led.Off();*/

    for (int i = 550;i > 0;i-=100)
    {
      On();

      if (SirenOn)
        Siren.On();

      delay(i);
      Siren.Off();
      Off();
      delay(i);
    };

    if (SirenOn)
      Siren.On();

    On();
    SMS("Alarm", MasterNum);
    delay(2000);
    Siren.Off();
    delay(3000);
  };
}

void SMS(String text, String phone)
{
  GSM.println("AT+CMGS=\"" + phone + "\"");
  delay(500);
  GSM.print(text);
  delay(500);
  GSM.print((char)26);
  delay(500);
  GSM.flush();
}
