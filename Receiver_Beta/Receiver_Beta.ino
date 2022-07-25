// MASTER
#include <SPI.h>  //BARRAMENTO SERIAL
#include <LoRa.h>
#include <Wire.h> //BARRAMENTO I2C
#include "SSD1306.h"

// definição para os pinos LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 15 //tava 14
#define DI00 26
#define BAND 915E6 //set freq 915mhz

#define INTERVAL 1000
long lastSendTime = 0;
int i=0;
bool AntenaLivre = false 

const String GETSENSOR = "getsensor";
const String SETSENSOR = "setsensor";
static uint8_t coreZero = 0;
static uint8_t coreOne = 1;

const int pinLed = 23; // pino para ligar led quando recebe dado

SSD1306 display(0x3c, 4 , 15); //construtor = 0x3c é o identificador, 4 e 15 o de comunicação


bool setDisplay(){
  pinMode(16, OUTPUT); //pino display
  digitalWrite(16, LOW); // desligado
  delay(50);
  digitalWrite(16, HIGH); // ligado
  delay(50);
 return display.init();
}
void configDisplay(){  
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  delay(50);
  display.drawString(65,20, "Hidrosens");
  display.display();
  delay(2000);
  display.clear();
  display.display();
}
bool setLoRa(){
  SPI.begin(SCK,MISO,MOSI,SS); // set barramento SPI
  LoRa.setPins(SS,RST,DI00);  //set pinos chip LoRa
return LoRa.begin(BAND);  
}
void showDisplay(String sensor, String waiting){
  display.clear();
  display.flipScreenVertically(); 
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);  
  display.drawString(0, 0, "Master");
  display.drawString(0, 16, "Hum:" + String(sensor.substring(0,5)));
  display.drawString(0, 32, "Tem:" + String(sensor.substring(6,11)));
  display.drawString(0, 48, String(waiting) + " ms.");
  display.display();
}
void master(){
  String input = "";
  int packetSize = LoRa.parsePacket();
  if(packetSize > SETSENSOR.length()){
    while(LoRa.available()){
      input = input + (char)LoRa.read();  
    }
  }
  int index = input.indexOf(SETSENSOR);  
  if(index >= 0){
    digitalWrite(pinLed,HIGH);
    String sensor = input.substring(SETSENSOR.length());
    String waiting = String(millis() - lastSendTime);
    showDisplay(sensor, waiting);
    digitalWrite(pinLed,LOW);
    i = i + 1;
    Serial.println("nº pacotes recebidos:" + String(i));
    Serial.println("temp:" + String(sensor.substring(6,11)));
    Serial.println("Hum:" + String(sensor.substring(0,5)));
    Serial.println("Ms:" + String(waiting));      
  }
}
void slave(){
  LoRa.beginPacket();
  LoRa.print(GETSENSOR);
  LoRa.endPacket();  
}
void taskZero(void *pvParameters){
  while(1){
    if (millis() - lastSendTime > INTERVAL){
      AntenaLivre = false;
      lastSendTime = millis();
      slave();    //slave manda os dados
    }else{
      AntenaLivre = true;
      master();    // master solicita os dados  
    }     
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(pinLed, OUTPUT);
  
  if(!setDisplay()){
    Serial.println("Display failed!");
    while(1);
  }
  configDisplay();
  if(!setLoRa()){
    Serial.println("LoRa failed!");
    while(1);
  }else{
    LoRa.enableCrc(); //Cyclic Redundancy Check) é um método de detecção de erros
    LoRa.receive();
  }
  xTaskCreatePinnedToCore(taskZero, "taskZero", 10000, NULL, 1, NULL, coreOne); //slave e master
  delay(500);
  //xTaskCreatePinnedToCore(taskOne, "taskOne", 10000, NULL, 1, NULL, coreOne); // the things network
  

  
}


void loop(){
/*  if (millis() - lastSendTime > INTERVAL){
    lastSendTime = millis();
    slave();    //slave manda os dados
    Serial.println("slave");
  }else{
    master();    // master solicita os dados
    Serial.println("master");
  }
  */
}
