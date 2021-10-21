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
const char test[]="1";
int flag =0;

enum State{ChuaCoMu, DaCaiMu}; 
State state ;

void setup()
{
  pinMode(Relay,OUTPUT);
  pinMode(Dong,INPUT);
  Serial.begin(9600);
  Serial.println("READY..........");
  radio.begin();
  //radio.openWritingPipe(addresses[0]); // 00001
 radio.openReadingPipe(0, addresses[1]); // 00002
 
  radio.setPALevel(RF24_PA_MIN);
  state = ChuaCoMu;
   Serial.println("READY..........");  
  radio.startListening();
    if(radio.available())
  {
    
    radio.read(&ans,1);
    //Serial.println(ans[0]);
  }
 switch(ans[0]){
  case '1':
  Serial.println("nhan 1");
  flag = 1;
  break;
 }
}

void loop(){
  radio.startListening();
  currentMillis = millis();
  if (currentMillis - prevMillis >= 500) 
  {
    flag = 0;
    Serial.println("Vao Xet");
      if(radio.available())
        {
        Serial.println("Vao Doc");
        radio.read(&ans,1);
        
        Serial.print("Ans la:");
        //Serial.println(ans[0]);
     switch(ans[0]){
        case '1':
        Serial.println("nhan 1");
        flag = 1;
        break; 
     }}
   prevMillis = millis();
  }
  else
  {
Serial.println(state);
  switch(state)
  {
    case ChuaCoMu:
    if(flag == 1)
      {
        Serial.println("DaCaiMu");
        state = DaCaiMu;
        digitalWrite(Relay,HIGH);
      }
     
    break;
    case DaCaiMu:
    if(flag != 1)
      {state = ChuaCoMu;
        
        digitalWrite(Relay,LOW);
      }
      else
    Serial.println("Dang CaiMu");
    break;
    
  }
  
  }
}
