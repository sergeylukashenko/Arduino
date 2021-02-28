#include <SoftwareSerial.h>
#include <TheLibrary.cpp>

SoftwareSerial GSM(3, 2); // RX, TX

TDigitalOutput LED(13);

String val = "";

void ExecGSM(String ACommand, char* ARes)
{
  while (1)
  {
    GSM.println(ACommand);

    if (GSM.readString().indexOf(ARes) > -1)
      break;
  };
};

const String MasterNum = "9202744533";

void setup()
{
  Serial.begin(9600);
  Serial.println("Setup...");

  LED.On();

  GSM.begin(9600);

  //ExecGSM("AT+IPR=9600", "");
  ExecGSM("AT", "OK");
  ExecGSM("AT+CLIP=1", "OK");         // AОН
  ExecGSM("AT+CMGF=1", "OK");         // Текстовый формат SMS
  ExecGSM("AT+CSCS=\"GSM\"", "OK");   // Кодкровка текста sms
  ExecGSM("AT+CMGD=1,4", "OK");       // Удаление всех СМС
  ExecGSM("AT+CNMI=3,2,2,0,1", "");   // Автоприем СМС

  Serial.println("Initialized");

  ExecGSM("AT+COPS?", "+COPS: 0");

  Serial.println("Connected");

  LED.Off();
};

void loop()
{
  if (GSM.available())
  {
    val = GSM.readString();

    ExecGSM("ATH0", "OK");

    /*if (val.indexOf("RING") > -1) {  //если звонок обнаружен, то проверяем номер
      if (val.indexOf(MasterNum) > -1) {  //если номер звонящего наш. Укажите свой номер без "+"
        Serial.println("--- MASTER RING DETECTED ---");
        GSM.println("ATH0");  //разрываем связь

        digitalWrite(led, HIGH);  //включаем светодиод на 3 сек
        delay(3000);
        digitalWrite(led, LOW);  //выключаем реле
      }
      } else*/
    Serial.println(val);  //печатаем в монитор порта пришедшую строку
  };
};

void SMS(String text, String phone)
{
  GSM.println("AT+CMGS=\"+7" + phone + "\"");
  GSM.print(text);
  GSM.print((char)26);
}
