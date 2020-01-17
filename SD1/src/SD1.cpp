/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "application.h"
#line 1 "c:/Users/13479/Documents/SD1/src/SD1.ino"
#include <stdio.h>
#include <Wire.h>
#include "RTClibrary.h"
#include <SPI.h>
#include "Sdfat.h"
#include "HTU21D.h"
#include "HttpClient.h"
#include "Adafruit_Si7021.h"

//to be able to use without wifi basically the code will still run when wifi is down
void setup();
void loop();
#line 11 "c:/Users/13479/Documents/SD1/src/SD1.ino"
SYSTEM_THREAD(ENABLED);
//wifi information
char wifissid[] = "PotsdamSensors";//wifi ssid
char wifipassword[] = "potsdam_sensors";//wifi password
//Select sensor/device type
int sensortype = 1;
// char server[] = "18.220.67.192"; // Potsdam Sensor server
// char accountID[] = "POTSDAM01";
// char deviceID[] = "PT01"; 
// char sensorType[] = "POTS006";
// char ch1[20] = "1001";
// char ch2[20] = "1002";
char token[200] = "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ0aW1lc3RhbXAiOjE1MzI0NTY0ODcsImRldmljZV9pZCI6IkRFVklDRTAwMSIsImFjY291bnRfaWQiOiJQT1RTREFNMDEifQ.2-uok5PvRILXwSINDMdo0vT-KM8hHcVJr-BDuOEx-GQ";
//Http variables
HttpClient http;
http_header_t headers[] = {
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};
http_request_t request;
http_response_t response;
int portnum = 80;
char pathdata[1000];
char jsondata[1000];
//mysql information
char username[20] = "labusers"; //cuairlab
char password[20] = "labuserspass"; //cuairlab@camp292
char dbname[20] = "Particle";
char tablename[20] = "argon1";
//Shinyei variables
int Shinyei = D3;
unsigned long duration1;
unsigned long starttime;
unsigned long sampletime_ms = 5000;
unsigned long lowpulseoccupancy1 = 0;
float ratio1 = 0;
float concentration1 = 0;
int ratioint;
//SD card
const uint8_t chipSelect = A5;
const uint32_t SAMPLE_INTERVAL_MS = 1000;
int header_done = 0;
SdFat sd;
SdFile sdfile1; 
char sdfilename[20] = "ar18n.csv";//always use .csv
//RTC
DateTime now;
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//temp and humidity
Adafruit_Si7021 htu;
float humd;
float temp;
int templ, tempr;
int humdl, humdr;
//Struct to store plantower data
struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
struct pms5003data data;
//Plantower vairables
int shiftedtxdata;
int prevdata1, prevdata2, prevdata3;
int count = 1; 
//functions
void writeHeader1(SdFile file, char fileName[]);
void logData1(SdFile file, char fileName[]);
void handler(const char *eventName, const char *data);
void handler2(const char *eventName, const char *data);
void Postrequest();

void setup(){
  Serial.begin(115200);
  while(!Serial);
  delay(4000); 
  Serial.println("hello");
    
  Serial.println("hello222");
  //setting wifi
  WiFi.setCredentials(wifissid, wifipassword, WPA2);//options are WPA2, WEP, WPA
  
  //Serial for plantower
  Serial1.begin(9600, SERIAL_8N1);
  
  //Serial for debugging

  
  //Setup for Shinyei
  
  pinMode(Shinyei,INPUT);
  
  //Setup for humidity and temperature sensor
  int count = 0;
  while(!htu.begin() && count != 3){
	    delay(1000); 
      count++;
      Serial.println(htu.begin());
	}

  sd.exists(sdfilename); // creates file
  //writeHeader1(sdfile1, sdfilename);
  //Setup for RTC
  rtc.begin();
  if (rtc.lostPower()) {// Note: comment this line (and end bracket) and change rtc.adjust below to change time manually
   //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //
     rtc.adjust(DateTime(2020, 1, 16, 16, 31, 0)); //manually change time here
  } // this end bracket
}
void loop(){
  
  //check for wifi connection
  if(!WiFi.ready()){
    WiFi.connect();
    Serial.println("Connecting...");
  }
  WiFiSignal sig = WiFi.RSSI();
  // Serial.println(sig.getStrength());
  //Plantower data collection and processing
  int txdata = Serial1.read();
  if(txdata != -260){
    if(count % 2 == 1){
      shiftedtxdata = txdata << 8;
      if(count == 1){
        if(txdata != 66){
          count = 0;
        }
      }
      count++;
    }
    else{
      int dataval = shiftedtxdata + txdata;
      switch(count){
        case 2 :
        case 4 :
          data.framelen = dataval;
        break;
        case 6 :
          data.pm10_standard = dataval;
        break;
        case 8 :
          data.pm25_standard = dataval;
        break;
        case 10 :
          data.pm100_standard = dataval;
        break;
        case 12 :
          data.pm10_env = dataval;
        break;
        case 14 :
          data.pm25_env = dataval;
        break;
        case 16 :
          data.pm100_env = dataval;
        break;
        case 18 :
          data.particles_03um = dataval;
        break;
        case 20 :
          data.particles_05um = dataval;
        break;
        case 22 :
          data.particles_10um = dataval;
        break;
        case 24 :
          data.particles_25um = dataval;
        break;
        case 26 :
          data.particles_50um = dataval;
        break;
        case 28 :
          data.particles_100um = dataval;
        break;
        case 32 :
          data.checksum = dataval;
          //RTC data collection
          now = rtc.now();
          //Temp and humidity code
          humd = htu.readHumidity();
          temp = htu.readTemperature();
          humd = round(100*humd)/100;
          temp = round(100*temp)/100;
          //Print header
          if(header_done>0){
          }
          else{
            writeHeader1(sdfile1, sdfilename);
            header_done=1;  
          }
          //Shinyei Code
          duration1 = pulseIn(Shinyei, LOW);
          lowpulseoccupancy1 = lowpulseoccupancy1+duration1;
          if ((millis()-starttime) > sampletime_ms){
            ratio1 = (lowpulseoccupancy1/((millis()-starttime)*10.0));
            lowpulseoccupancy1 = 0;
            snprintf(pathdata, sizeof(pathdata), "/indata01.php?username=%s&password=%s&dbname=%s&tablename=%s&pm10_std=%d&pm25_std=%d&pm100_std=%d&pm10_env=%d&pm25_env=%d&pm100_env=%d&p3=%d&p5=%d&p10=%d&p25=%d&p50=%d&p100=%d&checksum=%d&Temperature=%f&Humidity=%f", username, password, dbname, tablename, data.pm10_standard, data.pm25_standard, data.pm100_standard, data.pm10_env, data.pm25_env, data.pm100_env, data.particles_03um, data.particles_05um, data.particles_10um, data.particles_25um, data.particles_50um, data.particles_100um, data.checksum, temp, humd);
            Serial.println(pathdata);
            snprintf(jsondata, sizeof(jsondata), "{\"time\": \"%ld\", \"pm10\":\"%d\", \"pm25\":\"%d\", \"pm100\":\"%d\", \"pn03\":\"%d\", \"pn05\":\"%d\", \"pn10\":\"%d\", \"pn25\":\"%d\", \"pn50\":\"%d\", \"pn100\":\"%d\", \"Shinyei\":\"%f\", \"temp\":\"%f\", \"humidity\":\"%f\"}", now.unixtime(), data.pm10_standard, data.pm25_standard, data.pm100_standard, data.particles_03um, data.particles_05um, data.particles_10um, data.particles_25um, data.particles_50um, data.particles_100um, ratio1, temp, humd);
            logData1(sdfile1, sdfilename); 

            request.ip = IPAddress(128, 153, 15, 208);
            request.port = portnum;
            request.path = pathdata;
            http.get(request, response, headers);
            Serial.println("Sending Data");
            
            starttime = millis();
          }
          Serial1.end();
          Serial1.begin(9600, SERIAL_8N1);
        break;
        default :
        break;
      }
      count++;
    }
    if(count == 33){
      count = 1;
    }
  }
}
void writeHeader1(SdFile file, char fileName[]) {
  sd.begin(chipSelect, SD_SCK_MHZ(50));
  if(!sd.exists(fileName)){
    file.open(fileName, O_WRONLY | O_CREAT);
    file.sync();
    file.print("Time");
    file.print(',');
    file.print(",pm10_standard");
    file.print(",pm25_standard");
    file.print(",pm100_standard");
    file.print(",pm10_env");
    file.print(",pm25_env");
    file.print(",pm100_env");
    file.print(",P3um");
    file.print(",P5um");
    file.print(",P10um");
    file.print(",P25um");
    file.print(",P50um");
    file.print(",P100um");
    file.print(",checksum");
    file.print(",Shinyei");
    file.print(",Temperature");
    file.print(",Humidity");
    file.println();
    file.close();
  }
}
void logData1(SdFile file, char fileName[]) {
  //logging RTC data
  sd.begin(chipSelect, SD_SCK_MHZ(50));
  if(!sd.exists(fileName)){
    file.open(fileName, O_WRONLY | O_CREAT);
  }
  else{
    file.open(fileName, O_WRONLY | O_APPEND);
  }
  file.sync();
  file.print(now.year(), DEC);
  file.print('-');
  file.print(now.month(), DEC);
  file.print('-');
  file.print(now.day(), DEC);
  file.print(" ");
  file.print(now.hour(), DEC);
  file.print(':');
  file.print(now.minute(), DEC);
  file.print(':');
  file.print(now.second(), DEC);
  //logging tempa and humidity data
  templ = (int)temp;
  tempr = (temp-templ)*100;//rounded to 2 places
  humdl = (int)humd;
  humdr = (humd-humdl)*100;//rounded to 2 places
  file.print(',');
  file.print(',');
  file.print(data.pm10_standard);
  file.print(',');
  file.print(data.pm25_standard);
  file.print(',');
  file.print(data.pm100_standard);
  file.print(',');
  file.print(data.pm10_env);
  file.print(',');
  file.print(data.pm25_env);
  file.print(',');
  file.print(data.pm100_env);
  file.print(',');
  file.print(data.particles_03um);
  file.print(',');
  file.print(data.particles_05um);
  file.print(',');
  file.print(data.particles_10um);
  file.print(',');
  file.print(data.particles_25um);
  file.print(',');
  file.print(data.particles_50um);
  file.print(',');
  file.print(data.particles_100um);
  file.print(',');
  file.print(data.checksum);
  file.print(',');
  file.print(ratioint);
  file.print(',');
  file.print(templ);
  file.print('.');
  file.print(tempr);
  file.print(',');
  file.print(humdl);
  file.print('.');
  file.print(humdr);
  file.println();
  file.close();
}