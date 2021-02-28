/*   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

class TKey
{
  private :
    MFRC522 FRFID;
    unsigned long FID;

  public :
    TKey()
    {
      SPI.begin();         // Init SPI bus
      FRFID = MFRC522(10, 9);
      FRFID.PCD_Init();   // Init MFRC522

      Serial.println("Key created");
    };

    unsigned long ID()
    {
      return FID;
    };

    bool IsKeyDetected()
    {
      if (!FRFID.PICC_IsNewCardPresent())
      {
        FID = 0;

        return false;
      };

      unsigned long CurID = 0;

      if (FRFID.PICC_ReadCardSerial())
      {
        for (byte i = 0; i < FRFID.uid.size; i++)
          CurID = CurID * 256 + FRFID.uid.uidByte[i];

        FRFID.PICC_HaltA();
      }
      else
        return false;

      bool Res = ((ID() != CurID));
      FID = CurID;

      return Res;
    };
};

#define CheckKeys 0
#define Program 1

TKey* Key;
int Mode = CheckKeys;

void setup()
{
  Serial.begin(9600);    // Initialize serial communications with the PC
  Key = new TKey();
};

void loop()
{
  if (Key->IsKeyDetected())
  {
    switch (Mode)
    {
      case CheckKeys:
        {
          int i = IndexOf(Key->ID());

          if (i == 0)
            Mode = Program;
          else if (i < 0)
          {
            Serial.println("Denied");
          }
          else
          {
            Serial.println("Granted");
          };

          break;
        };

      case Program :
        {
          Serial.println("Programm");
          int idx = IndexOf(Key->ID());

          if (idx == 0)
            Mode = CheckKeys;
          else if (idx < 0)
          {
            AddID(Key->ID());
            Serial.println("Added");
          }
          else
          {
            RemoveID(Key->ID());
            Serial.println("Removed");
          };

          break;
        };
    };
  };
};

unsigned long Address(int AIndex)
{
  return  2 + AIndex * 4;
};

unsigned long Read(int AIndex)
{
  unsigned long V = 0;

  for (int i = 0; i < 4; i++)
    V = V * 256 + EEPROM.read(Address(AIndex) + i);

  return V;
};

void Write(int AIndex, unsigned long AID)
{
  for (int i = 3; i >= 0; i--)
  {
    EEPROM.write(Address(AIndex) + i, AID);
    AID /= 256;
  };
};

#define KeysCount 50

int IndexOf(unsigned long AID)
{
  for (int i = 0; i < KeysCount; i++)
    if (Read(i) == AID)
      return i;

  return -1;
};

void RemoveID(unsigned long AID)
{
  int i = IndexOf(AID);

  if (i >= 0)
    Write(i, 0);
};

void AddID(unsigned long AID)
{
  for (int i = 0; i < KeysCount; i++)
    if (Read(i) == 0)
    {
      Write(i, AID);
      break;
    };
};

void Init()
{
  for (int i = 1; i < KeysCount; i++)
    Write(i, 0);
};
