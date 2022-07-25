//SLAVE
#include <Wire.h> //barramento i2c
#include <SPI.h> //barramento serial
#include <LoRa.h>
#include "SSD1306.h" //display
#include <DHT.h>
#include "RTClib.h"
#define DHTPIN 13 //pino 13

// definição para os pinos LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 15 //tava 14
#define DI00 26
#define BAND 915E6 //set freq 915mhz

float humidity, temperature;
float humidity_new = -1, temperature_new = -1;
String packetLora;
String temp;

const String GETSENSOR = "getsensor";
const String SETSENSOR = "setsensor";

const int pinLed = 23;

char daysOfTheWeek[7][12] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};

DHT dht(DHTPIN, DHT11);
SSD1306 display(0x3c, 4 , 15); //construtor = 0x3c é o identificador, 4 e 15 o de comunicação
RTC_DS1307 rtc;

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
}
bool setLoRa(){
  SPI.begin(SCK,MISO,MOSI,SS); // set barramento SPI
  LoRa.setPins(SS,RST,DI00);  //set pinos chip LoRa
return LoRa.begin(BAND);  
}
void readDHTSensor(){
  delay(500);
  String input = "";
  int packetSize = LoRa.parsePacket();   
  humidity_new = dht.readHumidity();
  temperature_new = dht.readTemperature();
  if(packetSize == GETSENSOR.length()){
    while(LoRa.available()){
      input = input + (char)LoRa.read();
    }
  }
  if((isnan(humidity_new) || isnan(temperature_new || humidity_new == -1 || temperature_new == -1 ))) {
    Serial.println("Não foi possivel ler o sensor:");
    display.clear();
    display.flipScreenVertically();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(20, 20, "Falha ao ler o DHT");
    display.display();   
      if(input.equals(GETSENSOR)){
        digitalWrite(pinLed,HIGH);
        String sensor = "NULL.|NULL.";
        Serial.println("Criando pacote");  
        LoRa.beginPacket();
        LoRa.print(SETSENSOR + sensor);
        LoRa.endPacket();
        digitalWrite(pinLed,LOW);
        input = "";         
      }
      
  }else if(humidity_new != humidity || temperature_new != temperature){
    humidity = humidity_new;
    temperature = temperature_new;
    showDisplay();
    if(input.equals(GETSENSOR)){ 
      digitalWrite(pinLed,HIGH);   
      String sensor = String(humidity) + "|" + String(temperature);
      Serial.println("Criando pacote");  
      LoRa.beginPacket();
      LoRa.print(SETSENSOR + sensor);
      LoRa.endPacket();
      showDisplay();
      digitalWrite(pinLed,LOW);
      input = "";
    }       
   }
   else if(humidity_new == humidity || temperature_new == temperature){
    humidity = humidity_new;
    temperature = temperature_new;
    showDisplay();
    if(input.equals(GETSENSOR)){ 
      digitalWrite(pinLed,HIGH);   
      String sensor = String(humidity) + "|" + String(temperature);
      Serial.println("Criando pacote");  
      LoRa.beginPacket();
      LoRa.print(SETSENSOR + sensor);
      LoRa.endPacket();
      showDisplay();
      digitalWrite(pinLed,LOW);
      input = "";
    }         
  }
}
void showDisplay(){
  delay(100); 
  display.clear();
  display.flipScreenVertically(); 
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);  
  display.drawString(0, 0, "Slave");
  display.drawString(0, 16, "Hum:" + String(humidity)); 
  display.drawString(0, 32, "Tem:" + String(temperature));
  display.display();
  delay(100); 
}


void setup() {
  Serial.begin(9600); //57600???
  Wire.begin(21,22);
  pinMode(pinLed, OUTPUT);  
 
  if(!setDisplay()){
    Serial.println("Display failed!");
    while(1);
  }  
  configDisplay();
  
  if(!setLoRa()){
    Serial.println("Display failed!");
    while(1);
  }
  if (!rtc.begin()) {
    Serial.println("Não foi possível encontrar RTC");
    while (1);
  }  
  //rtc.adjust(DateTime(2021, 8, 27, 19, 55, 0));  
  dht.begin(); 
}


void loop(){
  display.clear();  
  DateTime now = rtc.now();
  Serial.println(now.second(), DEC);
  Serial.println(now.minute(), DEC);
  Serial.println(now.hour(), DEC); 
  readDHTSensor();
  temp = String(humidity) + "|" + String(temperature);
  Serial.println(String(temp));  
  display.clear();
}
