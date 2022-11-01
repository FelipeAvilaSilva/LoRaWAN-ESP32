/*
 * HelTec Automation(TM) LoRaWAN 1.0.2 OTAA example use OTAA, CLASS A
 *
 * Function summary:
 *
 * - use internal RTC(150KHz);
 *
 * - Include stop mode and deep sleep mode;
 *
 * - 15S data send cycle;
 *
 * - Informations output via serial(115200);
 *
 * - Only ESP32 + LoRa series boards can use this library, need a license
 *   to make the code run(check you license here: http://www.heltec.cn/search );
 *
 * You can change some definition in "Commissioning.h" and "LoRaMac-definitions.h"
 *
 * HelTec AutoMation, Chengdu, China.
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 * support@heltec.cn
 *
 *this project also release in GitHub:
 *https://github.com/HelTecAutomation/ESP32_LoRaWAN
*/

//INCLUDE
#include "Arduino.h"
#include "OTAA_OLED_KEYS.h"
#include <ESP32_LoRaWAN.h>
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Rtc_Pcf8563.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <LoraEncoder.h>
#include <OneWire.h>
#include <DallasTemperature.h>


//DEFINE
#define DHTPIN 21//13

#define SD_CS 23//23
#define SD_SCK 17 //17
#define SD_MOSI 12 //12
#define SD_MISO 13 //13
#define LOG_PATH "/hidrosens.txt"

#define vmax 20.1
#define vin_port 36
#define ONE_WIRE_BUS 2 //gpio 2

SPIClass sd_spi(HSPI);

//INIT
DHT dht(DHTPIN, DHT11);
Rtc_Pcf8563 rtc;
HardwareSerial ss(1);
TinyGPSPlus gps;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

//GLOBAL
float humidity, temperature;
float humidity_new = -1, temperature_new = -1;
float voltage = 0;
float temp_agua = 0;

//GPS
static const int RXPin = 3, TXPin = 1; //RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
double lat = 0;
double lng = 0;
byte buffer[8];
LoraEncoder encoder(buffer);
char location[30];


//FLAG
int d_dht = 0, d_rtc = 0, d_sd = 0, d_gps = 0;

//DHT11
void readDHT(){  
  DS18B20.requestTemperatures();
  delay(50);
  temp_agua = DS18B20.getTempCByIndex(0);
  humidity_new = dht.readHumidity();
  delay(50);
  temperature_new = dht.readTemperature();
  delay(50);
  if((isnan(humidity_new) || isnan(temperature_new) || humidity_new == -1 || temperature_new == -1 )){
    Serial.println("Não foi possivel ler o sensor:"); 
    d_dht = 0;     
  }else if(humidity_new != humidity || temperature_new != temperature){
    humidity = humidity_new;
    temperature = temperature_new;
    d_dht = 1;
  }
}

void statusGPS(){ 
  digitalWrite(LED_BUILTIN, HIGH); 
  unsigned long start = millis();
  while(millis() - start < 1000){
    if(ss.available() > 0){ 
      if(gps.encode(ss.read())){
          readGPS();
      }    
    }
  }
  digitalWrite(LED_BUILTIN, LOW); 
}

void readGPS(){
  if(gps.location.isValid()){
    lat = double (gps.location.lat());
    lng = double (gps.location.lng());     
    sprintf(location, "%.6f,%.6f", gps.location.lat(), gps.location.lng());
    d_gps = 1;   
  }else{
    d_gps = 0;
  }
  encoder.writeLatLng(lat,lng);
}

//SD
void writeSD(float temperature, float humidity){  
  File test = SD.open(LOG_PATH, FILE_APPEND);
  if(!test){
    Serial.println("SD Card: writing file failed.");
    d_sd = 0;
  }else{
    //digitalWrite(LED_BUILTIN, HIGH); 
    Serial.print(rtc.getDay());    
    Serial.print('/');
    Serial.print(rtc.getMonth());
    Serial.print('/');
    Serial.print(rtc.getYear());
    Serial.print('/');
    Serial.print(rtc.getHour());
    Serial.print(':');
    Serial.print(rtc.getMinute());
    Serial.print(':');
    Serial.print(rtc.getSecond());
    Serial.println();
      Serial.printf("SD Card: appending data to %s.\n", LOG_PATH);
      test.print(temperature);
      test.print("|");
      test.print(humidity);
      test.print(" ");                
      test.print(rtc.getHour());
      test.print(":");
      test.print(rtc.getMinute());
      test.print(":");
      test.print(rtc.getSecond());
      test.print(" ");
      test.print(rtc.getDay());
      test.print("/");
      test.print(rtc.getMonth());
      test.print("/");
      test.print(rtc.getYear());  
      test.print("||");
      test.print(location);   
      test.print("|||");   
      test.print(temp_agua);
      test.printf("\n");      
      test.close();
     // digitalWrite(LED_BUILTIN, LOW);
      d_sd = 1;  
  }  
}

//ENVIA OS DADOS
static void prepareTxFrame( uint8_t port ){
  voltage = (analogRead(vin_port)*vmax)/4095;

  //int8 -> int16
  uint16_t temp = (uint16_t) (temperature * 100);
  uint16_t hum = (uint16_t) (humidity * 100);
  uint16_t volt = (uint16_t) (voltage * 100);
  uint16_t temp_a = (uint16_t) (temp_agua * 100);
  
  //envio
    appDataSize = 25;                 //AppDataSize max value is 64
    appData[0] = temp >> 8;
    appData[1] = temp & 0xFF;
    appData[2] = hum >> 8;
    appData[3] = hum & 0xFF;
    appData[4] = volt >> 8;
    appData[5] = volt & 0xFF;
    appData[6] = d_dht;
    appData[7] = d_sd;
    appData[8] = buffer[0];
    appData[9] = buffer[1];
    appData[10] = buffer[2];
    appData[11] = buffer[3];
    appData[12] = buffer[4];
    appData[13] = buffer[5];
    appData[14] = buffer[6];
    appData[15] = buffer[7]; 
    appData[16] = buffer[8]; 
    appData[17] = d_gps;  
    appData[18] = temp_a >> 8;
    appData[19] = temp_a & 0xFF;    
}


void setup(){  
  Serial.begin(115200);
  Wire.begin(4, 15);
  sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  pinMode(LED_BUILTIN, OUTPUT);
  
  /*rtc.initClock(); //clear out the registers  
  rtc.setDate(9, 1, 8, 0, 22); //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setTime(1, 33, 0); //hr, min, sec*/   

  // TEMP INIT
  DS18B20.begin(); 

  //GPS INIT
  ss.begin(GPSBaud, SERIAL_8N1, RX, TX);

  //SD INIT    
  if(!SD.begin(SD_CS, sd_spi)){
    Serial.println("Couldn't find SD Card");
    d_sd = 0;
  }else{
    d_sd = 1;
  }
  //DHT INIT
  dht.begin();
  //DISPLAY INIT
  if(mcuStarted == 0){  
    LoRaWAN.displayMcuInit();
  }  
  //LORA INIT
  SPI.begin(SCK,MISO,MOSI,SS);
  Mcu.init(SS,RST_LoRa,DIO0,DIO1,license);
  deviceState = DEVICE_STATE_INIT;  
}

// The loop function is called in an endless loop
void loop(){
  switch( deviceState )
  {
    case DEVICE_STATE_INIT:
    {
#if(LORAWAN_DEVEUI_AUTO)
			LoRaWAN.generateDeveuiByChipID();
#endif
      LoRaWAN.init(loraWanClass,loraWanRegion);
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRaWAN.displayJoining();
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND:
    {
      readDHT();
      statusGPS();
      writeSD(temperature, humidity); // GRAVA NO SD, MESMO SE O GATEWAY NAO RECEBER O LORA
      LoRaWAN.displaySending();
      prepareTxFrame( appPort );      
      LoRaWAN.send(loraWanClass);      
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle; //+ randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      LoRaWAN.displayAck();
      LoRaWAN.sleep(loraWanClass,debugLevel);
      break;
    }
    default:
    {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}