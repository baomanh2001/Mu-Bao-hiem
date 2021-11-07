#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
#define Relay 8
#define Dong 4

unsigned long currentMillis;
unsigned long prevMillis = 0;
unsigned long txIntervalMillis = 300;
char ans[1];
const char test[] = "1";
int flag = 0;

enum State
{
  ChuaCoMu,
  DaCaiMu
};
State state;

void setup()
{
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
  {
    Serial.println(state);
    switch (state)
    {
    case ChuaCoMu:
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
      }
      break;
    case DaCaiMu:

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
    }
  }
}
