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

//DEFINE
#define DHTPIN 21//13

#define SD_CS 23//23
#define SD_SCK 17 //17
#define SD_MOSI 12 //12
#define SD_MISO 13 //13
#define LOG_PATH "/hidrosens.txt"

#define vmax 17.4
#define vin_port 36

SPIClass sd_spi(HSPI);

//INIT
DHT dht(DHTPIN, DHT11);
Rtc_Pcf8563 rtc;

//GLOBAL
float humidity, temperature;
float humidity_new = -1, temperature_new = -1;
float voltage = 0;

//FLAG
int d_dht = 0, d_rtc = 0, d_sd = 0;

//DHT11
void readDHTSensor(){  
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

//SD
void writeSD(float temperature, float humidity){  
  File test = SD.open(LOG_PATH, FILE_APPEND);
  if(!test){
    Serial.println("SD Card: writing file failed.");
    d_sd = 0;
  }else{
    digitalWrite(LED_BUILTIN, HIGH); 
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
      test.printf("\n");
      test.close();
      digitalWrite(LED_BUILTIN, LOW);
      d_sd = 1;  
  }  
}

//ENVIA OS DADOS
static void prepareTxFrame( uint8_t port ){
  //leituras
  readDHTSensor();
  voltage = (analogRead(vin_port)*vmax)/4095;


  //int8 -> int16
  uint16_t temp = (uint16_t) (temperature * 100);
  uint16_t hum = (uint16_t) (humidity * 100);
  uint16_t volt = (uint16_t) (voltage * 100);


  //debug  
  /*Serial.println("TempFI:"+ String(temp) + "HumFI:" + String(hum)); 
  Serial.print("Volts:" + String(voltage) + "\n");
  Serial.print("VoltsFI:" + String(volt) + "\n");*/
  
  //envio
    appDataSize = 8;                 //AppDataSize max value is 64
    appData[0] = temp >> 8;
    appData[1] = temp & 0xFF;
    appData[2] = hum >> 8;
    appData[3] = hum & 0xFF;
    appData[4] = volt >> 8;
    appData[5] = volt & 0xFF;
    appData[6] = d_dht;
    appData[7] = d_sd;        
}


void setup(){  
  Serial.begin(115200);
  Wire.begin(4, 15);
  sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);  
  pinMode(LED_BUILTIN, OUTPUT);
  
  /*rtc.initClock(); //clear out the registers  
  rtc.setDate(17, 1, 6, 0, 22); //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setTime(0, 49, 0); //hr, min, sec*/    

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
      LoRaWAN.displaySending();
      prepareTxFrame( appPort );
      LoRaWAN.send(loraWanClass);
      writeSD(temperature, humidity); // GRAVA NO SD, MESMO SE O GATEWAY NAO RECEBER O LORA
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle; + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
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
