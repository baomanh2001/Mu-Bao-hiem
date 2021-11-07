#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
#define Relay 8
#define Dong 4
#define interruptPin 2
unsigned long currentMillis;
unsigned long prevMillis = 0;
unsigned long ThoigianCoMu, ThoigianKhongMu;
unsigned long ThenCoMu = 0;
unsigned long ThenKhongMu = 0;
unsigned long NowKhongMu;
unsigned long NowCoMu; 
char ans[1];
const char test[] = "1";
int flag = 0;

enum State
{
  ChuaCoMu,
  DaCaiMu,
  Bypass
};
State state;

void NoRF()
{
  if (state != Bypass){
    state = Bypass;
    ThenKhongMu = millis();
  }
  else
  {
    digitalWrite(Relay, LOW);
    state = ChuaCoMu;
  }
}

void TinhThoiGianHoatDong()
{
  if ((NowKhongMu - ThenKhongMu) >= 1000 && NowKhongMu > ThenKhongMu)
  {
  
    ThoigianKhongMu += 1;
    ThenKhongMu = millis() + (NowKhongMu - ThenKhongMu - 1000);
    
  }
 
  if (  (NowCoMu - ThenCoMu) >= 1000  && NowCoMu > ThenCoMu)
  {
    ThoigianCoMu += 1;
    ThenCoMu = millis() + (NowCoMu - ThenCoMu - 1000);
   
  }
}

void setup()
{
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), NoRF, RISING);

  pinMode(Relay, OUTPUT);
  pinMode(Dong, INPUT);
  Serial.begin(9600);
  Serial.println("READY..........");
  radio.begin();

  radio.setChannel(1);
  //radio.openWritingPipe(addresses[0]); // 00001
  radio.openReadingPipe(1, addresses[1]); // 00002

  radio.setPALevel(1);
  state = ChuaCoMu;
  Serial.println("READY..........");
  radio.startListening();
  if (radio.available())
  {

    radio.read(&ans, 1);
    //Serial.println(ans[0]);
  }
  switch (ans[0])
  {
  case '1':
    Serial.println("nhan 1");
    flag = 1;
    break;
  }
}
void XetMu()
{
  Serial.println("Vao Doc");
  radio.read(&ans, 1);
  Serial.print(ans[0]);
  Serial.println();
  switch (ans[0])
  {
  case '1':
    Serial.println("nhan 1");
    flag = 1;
    break;
  }
}
void loop()
{
  radio.startListening();

  //Serial.println(state);
  switch (state)
  {
  case ChuaCoMu:
    if (digitalRead(Relay))
      digitalWrite(Relay, LOW);
    Serial.println("Vao Xet Khi Chua Co Mu");
    if (radio.available())
    {
      XetMu();
      prevMillis = millis();
    }
    if (flag == 1)
    {
      Serial.println("DaCaiMu");
      state = DaCaiMu;
      digitalWrite(Relay, HIGH);
      ThenCoMu = millis();
      NowCoMu = millis();
    }
    break;
  case DaCaiMu:
    NowCoMu = millis();
    Serial.println("Dang CaiMu");
    currentMillis = millis();
    if (currentMillis - prevMillis >= 500)
    {
      flag = 0;
      Serial.println("Vao Xet");
      if (radio.available())
      {
        XetMu();
        prevMillis = millis();
      }
    }
    if (flag != 1)
    {
      state = ChuaCoMu;
      digitalWrite(Relay, LOW);
      
    }
    break;
  case Bypass:
    NowKhongMu = millis();
    Serial.println("Dang Bypass");
    digitalWrite(Relay, HIGH);
    break;
  }
  TinhThoiGianHoatDong();
   Serial.print("Thoi gian \t");
   Serial.print(ThoigianCoMu);
   Serial.print("\t Thoi gian \t");
   Serial.println(ThoigianKhongMu);
}
