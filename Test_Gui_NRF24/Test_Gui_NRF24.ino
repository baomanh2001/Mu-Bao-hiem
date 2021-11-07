#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h> // khai báo thư viện VirtualWire
#include <avr/sleep.h>

float rollLimitH = 1;
float rollLimitL = 0.09;
float pitchLimitH = -2;
float pitchLimitL = -4.60;
unsigned long DaGui = 0;
//Cài Mũ D8
//Đội Mũ D7 (Tam Thoi)
//CE CS gắn 9 10
RF24 radio(9, 10);

#define interruptPin 2
#define CaiMu 8
#define DoiMu 6
#define DenXanh 3
#define DenVang 4
#define DenDo 5
#define Addr 0x1C
const byte addresses[][6] = {"00001", "00002"}; // 2 Địa chỉ gửi nhận

enum State
{
  ChuaCaiMu,
  ChuaDoiLen,
  CaiMuDoiLen
};
State state = ChuaCaiMu;

enum Sleep
{
  Start,
  Stop
};
Sleep SleepStatus = Stop;

unsigned long NowSleep = 0;

unsigned long StartSleep = 0;

void BatDauDocGiaToc()
{
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select control register
  Wire.write(0x2A);
  // Active mode
  Wire.write(0x01);
  // Stop I2C Transmission
  Wire.endTransmission();
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select control register
  Wire.write(0x0E);
  // Set range to +/- 2g
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();
}
void DungDocGiaToc()
{
  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select control register
  Wire.write(0x2A);
  // StandBy mode
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();
}
bool DoiMuTrenDau()
{
  BatDauDocGiaToc();
  unsigned int data[7];

  // Request 7 bytes of data
  Wire.requestFrom(Addr, 7);

  // Read 7 bytes of data
  // staus, xAccl lsb, xAccl msb, yAccl lsb, yAccl msb, zAccl lsb, zAccl msb
  if (Wire.available() == 7)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
    data[6] = Wire.read();
  }

  //Convert the data to 12 - bits
  int xAccl = ((data[1] * 256) + data[2]) / 16;
  int yAccl = ((data[3] * 256) + data[4]) / 16;
  int zAccl = ((data[5] * 256) + data[6]) / 16;

  float roll, pitch, rollF, pitchF = 0;
  float gX, gY, gZ;
  gX = xAccl * 0.977 / 1000;
  gY = yAccl * 0.977 / 1000;
  gZ = zAccl * 0.977 / 1000;
  // Calculate Roll and Pitch (rotation around X-axis, rotation around Y-axis)
  roll = atan(gY / sqrt(pow(gX, 2) + pow(gZ, 2))) * 180 / PI;
  pitch = atan(-1 * gX / sqrt(pow(gY, 2) + pow(gZ, 2))) * 180 / PI;
  // Low-pass filter
  rollF = 0.94 * rollF + 0.06 * roll;
  pitchF = 0.94 * pitchF + 0.06 * pitch;

  // MMA8452Q I2C address is 0x1C(28)
  Serial.print("Roll: ");
  Serial.println(rollF);
  Serial.print("PitchF");
  Serial.println(pitchF);
  unsigned long time_now = 0;

  time_now = millis();
  while (millis() < time_now + 200)
  {
  }
  if (rollF <= rollLimitH && rollF >= rollLimitL || pitchF <= pitchLimitH && pitchF >= pitchLimitL)
  {
    Serial.println("Trang Thai 1");
    return 1;
  }
  Serial.print("Trang Thai 0");
  return 0;
}

bool CamBienAnhSang()
{
  if (digitalRead(DoiMu) == 0)
    return 1;
  return 0;
}

void Going_To_Sleep()
{
  digitalWrite(DenDo, LOW);
      digitalWrite(DenVang, LOW);
      digitalWrite(DenXanh, LOW);
  sleep_enable(); //Enabling sleep mode
 attachInterrupt(digitalPinToInterrupt(interruptPin), Wake_Up, RISING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Setting the sleep mode, in our case full sleep

  sleep_cpu(); //activating sleep mode
}
void Wake_Up()
{
  Serial.println("thuc day");
sleep_disable();
detachInterrupt(0);
      SleepStatus = Stop;
      state = ChuaCaiMu;
}
void setup()
{
  Wire.begin();
  radio.begin();
  Serial.begin(9600); // giao tiếp Serial với baudrate 9600
  Serial.println("Ready.........");
pinMode(2, INPUT);
 
  //radio.openReadingPipe(1, addresses[0]); // 00001
  radio.openWritingPipe(addresses[1]); // 00002
  radio.setPALevel(RF24_PA_MIN);

  pinMode(CaiMu, INPUT); //Cai Mu Hall SenSor
  pinMode(DoiMu, INPUT); // DoiMu
  pinMode(DenXanh, OUTPUT);
  pinMode(DenVang, OUTPUT);
  pinMode(DenDo, OUTPUT);
  BatDauDocGiaToc();
}

void loop()
{
  Serial.println("vl lun dau cat moi");
  Serial.println(digitalRead(2));
  if (digitalRead(CaiMu) != 0)
  {
    digitalWrite(DenDo, HIGH);
    digitalWrite(DenVang, LOW);
    digitalWrite(DenXanh, LOW);
    if (SleepStatus != Start)
    {
      SleepStatus = Start;
      StartSleep = millis();
      NowSleep = millis();
    }
    else

      NowSleep = millis();
    state = ChuaCaiMu;
  }
  switch (state)
  {
  case ChuaCaiMu:
    if (digitalRead(CaiMu) != 1)
    {
      Serial.println("Da Cai Mu");
      state = ChuaDoiLen;
      digitalWrite(DenDo, LOW);
      digitalWrite(DenVang, HIGH);
      digitalWrite(DenXanh, LOW);
      if (SleepStatus != Start)
      {
        SleepStatus = Start;
        StartSleep = millis();
        NowSleep = millis();
      }
      else
        NowSleep = millis();
    }
    break;

  case ChuaDoiLen:
    if (DoiMuTrenDau() == 1 && CamBienAnhSang() == 1)
    {
      Serial.println("Da Doi Len");
      digitalWrite(DenDo, LOW);
      digitalWrite(DenVang, LOW);
      digitalWrite(DenXanh, HIGH);
      state = CaiMuDoiLen;
    }
    if (SleepStatus != Start)
    {
      SleepStatus = Start;
      StartSleep = millis();
      NowSleep = 0;
    }
    else
      NowSleep = millis();
    break;

  case CaiMuDoiLen:

    if (DoiMuTrenDau() == 0)
    {
      state = ChuaDoiLen;
      digitalWrite(DenDo, LOW);
      digitalWrite(DenVang, HIGH);
      digitalWrite(DenXanh, LOW);
      if (SleepStatus != Start)
      {
        SleepStatus = Start;
        StartSleep = millis();
        NowSleep = millis();
      }
      else
        NowSleep = millis();
    }
    else
    {
      if (SleepStatus != Stop)
        SleepStatus = Stop;
      DungDocGiaToc();
      radio.stopListening();
        const char text[] = "1";
        radio.write(&text, 1); //Gửi dữ liệu có trong mảng text

        Serial.println("Gui Xong");
      unsigned long CBGui = millis();
      if (CBGui - DaGui >= 500)
      {
        radio.stopListening();
        const char text[] = "1";
        radio.write(&text, 1); //Gửi dữ liệu có trong mảng text

        Serial.println("Gui Xong");
        DaGui = millis();
      }
    }
    break;
  }
  
    //if(SleepStatus == Start &&  NowSleep - StartSleep >  5000)
     //     Going_To_Sleep();
}
