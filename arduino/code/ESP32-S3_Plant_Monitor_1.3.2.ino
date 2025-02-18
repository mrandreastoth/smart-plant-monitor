/*
  Hello from Ovidiu
  https://github.com/ovidiu4/smart-plant-monitor
*/

#include "images.h"
#include <GxEPD.h>
//#include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w
#include <GxGDEH0154Z90/GxGDEH0154Z90.h>  // 1.54" b/w/r 200x200 SSD1681
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <ESP32httpUpdate.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Time.h>
#include "SPIFFS.h"
#include "EEPROM.h"
#include <Adafruit_AHTX0.h>
#include "Adafruit_LTR329_LTR303.h"


// ---------- PINS ---------- //
#define wakeup_pin             GPIO_NUM_0
#define face_pin               1
#define back_pin               2

//#define GPIO3                3 //DO NOT CHANGE! DO NOT USE!

#define batteryPin             4
#define check_pin_out          5
#define check_pin_in           6

//#define GPIO7                7 // CAN BE USED. Pad connected directly to Pin. Keep in mind that a current limiting resistor may be needed

#define ThermistorPin          8
#define face_led_pin           9
#define back_led_pin           10
#define PCBThermistorPin       11
#define IRint                  12
#define usbPin                 13

#define MOSI                   14 //DO NOT CHANGE!
#define SCK                    15 //DO NOT CHANGE!

#define SS                     16 //DO NOT CHANGE!
#define CS_Pin                 16
#define DC_Pin                 17
#define RES_Pin                18

//#define D-                   19 //DO NOT CHANGE!
//#define D+                   20 //DO NOT CHANGE!

#define BUSY_Pin               21

#define MISO                   37 //DO NOT CHANGE!
#define button_pin             38
#define external_led_pin       40

#define SCL                    41 //DO NOT CHANGE!
#define SDA                    42 //DO NOT CHANGE!
// ---------- PINS ---------- //


// ---------- THERMISTOR ---------- //
float Vo;
float R1 = 10000; // value of R1 on board
float logR2, R2, T;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; 
// ---------- THERMISTOR ---------- //

// ---------- VERSION ---------- //
String own_version="1.3.2b";
String server_version="N/A";
int update_code = 0;
int firmware_code = 0;
bool perform_update = false;
const char* version_file = "https://raw.githubusercontent.com/ovidiu4/smart-plant-monitor/main/firmware/version.txt";
const char* firmware_file = "https://raw.githubusercontent.com/ovidiu4/smart-plant-monitor/main/firmware/firmware.bin";
const char* tree_file = "https://raw.githubusercontent.com/ovidiu4/smart-plant-monitor/main/firmware/data/tree.txt";
String github_data_path = "https://raw.githubusercontent.com/ovidiu4/smart-plant-monitor/main/firmware/data/";
// ---------- VERSION ---------- //

// ---------- WIFI ---------- //
char* ssid = (char*)malloc(64);
char* password = (char*)malloc(64);
bool wifi_status = false;
IPAddress local_ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);
String hostname = "SMARTPLANT" + String(random(99)); //Initial value. Should be updated later in the code
String wifi_networks = "<option value=\"SCANNING\">SCANNING</option>"; //Initial value. Should be updated later in the code
// ---------- WIFI ---------- //

// ---------- EEPROM ---------- //
#define EEPROM_SIZE            640 //SSID=64, PASS=64, APIKEY=256, URL=256
String eeprom_ssid = "";
String eeprom_password = "";
String eeprom_apiKey = ""; 
String eeprom_home_assistant_url = ""; 
// ---------- EEPROM ---------- //


// ---------- SENSORS ---------- //
float soil_humidity_max = 800; 
float soil_humidity_min = 0; 
float soil_face_value = 0;
float soil_back_value = 0;
float soil_measurement_average = 0;
float soil_humidity = 0;
float soil_humidity_percent = 0; 
float soil_temperature = 0;
float battery_voltage = 0;
int battery_percent = 0;
float bus_voltage = 0;
float board_temperature = 0;
float air_temperature = 0;
float air_humidity = 0;
uint16_t ambient_light = 0;
uint16_t ir_light = 0;
bool valid;
// ---------- SENSORS ---------- //

// ---------- TIMEOUTS ---------- //
float deepsleeptime = 1800e6; //Measured in uS
int wifi_timeout = 10000; //Measured in mS
int http_timeout = 10000; //Measured in mS
unsigned long loop_time = 0;
int boot_mode_delay = 3000;
int reset_delay = 10000;
// ---------- TIMEOUTS ---------- //


// ---------- TIME ---------- //
const long utcOffsetInSeconds = 3600;
unsigned long rtcOffset = 0;
unsigned long seconds = 0;
unsigned long minutes = 0;
unsigned long hours = 0;
unsigned long days = 0;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
// ---------- TIME ---------- //

// ---------- HOMEASSISTANT ---------- //
const char* plant_monitor_air_temperature = "_air_temperature";
const char* plant_monitor_air_humidity = "_air_humidity";
const char* plant_monitor_battery_percent = "_battery_percent";
const char* plant_monitor_battery_voltage = "_battery_voltage";
const char* plant_monitor_board_temperature = "_board_temperature";
const char* plant_monitor_bus_voltage = "_bus_voltage";
const char* plant_monitor_soil_moisture_face = "_soil_moisture_face";
const char* plant_monitor_soil_moisture_back = "_soil_moisture_back";
const char* plant_monitor_soil_moisture_percent = "_soil_moisture_percent";
const char* plant_monitor_soil_temperature = "_monitor_soil_temperature";
const char* plant_monitor_ambient_light = "_ambient_light";
const char* plant_monitor_ir_light = "_ir_light";
const char* plant_monitor_version = "_firmware_version";
int test_connection_code ;
String apiKey = ""; 
String home_assistant_url = ""; 
// ---------- HOMEASSISTANT ---------- //

// ---------- OTHER ---------- //
bool restarting = false;
bool boot_into_config = false; 
bool reset_complete = false;
bool board_check = false;
bool home_assistant_connection = false;
bool wifi_saved_eeprom = false;
bool config_saved_eeprom = false;
// ---------- OTHER ---------- //

String restart_button_value = "RESTART";
String firmware_status = "COULD NOT CHECK SERVER";

ESP32Time rtc;
Adafruit_AHTX0 aht;
Adafruit_LTR303 ltr = Adafruit_LTR303();
AsyncWebServer server(80);

GxIO_Class io(SPI, /*CS*/ CS_Pin, /*DC*/ DC_Pin, /*RST*/ RES_Pin); 
GxEPD_Class display(io, /*RST*/ RES_Pin, /*BUSY*/ BUSY_Pin); 



void setup(void)
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("*** WAKING UP ***");

  pinMode(IRint, INPUT);
  pinMode(button_pin, INPUT);
  pinMode(external_led_pin, OUTPUT);
  pinMode(face_led_pin, OUTPUT);
  pinMode(back_led_pin, OUTPUT);

  digitalWrite(external_led_pin, HIGH);
  delay(200);
  digitalWrite(external_led_pin, LOW);

  battery_voltage = get_battery_voltage();
  
  String mac = WiFi.macAddress();
  hostname = "SMARTPLANT" + mac.substring(mac.length()-5, mac.length()-3) + mac.substring(mac.length()-2, mac.length());
  Serial.print("Hostname: ");
  Serial.println(hostname);
  Serial.print("Version: ");
  Serial.println(own_version);
     
  Serial.print("LTR Begin: ");
  if(ltr.begin()){
    Serial.println("OK");
    ltr.setGain(LTR3XX_GAIN_1);
    ltr.setIntegrationTime(LTR3XX_INTEGTIME_50);
    ltr.setMeasurementRate(LTR3XX_MEASRATE_2000);
    ltr.enableInterrupt(false);
    ltr.setInterruptPolarity(false);
    ltr.setLowThreshold(2);
    ltr.setHighThreshold(100);
    ltr.setIntPersistance(5);

    Serial.print("  Gain : ");
    switch (ltr.getGain()) {
      case LTR3XX_GAIN_1: Serial.println(1); break;
      case LTR3XX_GAIN_2: Serial.println(2); break;
      case LTR3XX_GAIN_4: Serial.println(4); break;
      case LTR3XX_GAIN_8: Serial.println(8); break;
      case LTR3XX_GAIN_48: Serial.println(48); break;
      case LTR3XX_GAIN_96: Serial.println(96); break;
    }
    
    Serial.print("  Integration Time (ms): ");
    switch (ltr.getIntegrationTime()) {
      case LTR3XX_INTEGTIME_50: Serial.println(50); break;
      case LTR3XX_INTEGTIME_100: Serial.println(100); break;
      case LTR3XX_INTEGTIME_150: Serial.println(150); break;
      case LTR3XX_INTEGTIME_200: Serial.println(200); break;
      case LTR3XX_INTEGTIME_250: Serial.println(250); break;
      case LTR3XX_INTEGTIME_300: Serial.println(300); break;
      case LTR3XX_INTEGTIME_350: Serial.println(350); break;
      case LTR3XX_INTEGTIME_400: Serial.println(400); break;
    }
    
    Serial.print("  Measurement Rate (ms): ");
    switch (ltr.getMeasurementRate()) {
      case LTR3XX_MEASRATE_50: Serial.println(50); break;
      case LTR3XX_MEASRATE_100: Serial.println(100); break;
      case LTR3XX_MEASRATE_200: Serial.println(200); break;
      case LTR3XX_MEASRATE_500: Serial.println(500); break;
      case LTR3XX_MEASRATE_1000: Serial.println(1000); break;
      case LTR3XX_MEASRATE_2000: Serial.println(2000); break;
    }
    
    Serial.print("  Consecutive counts for IRQ: ");  Serial.println(ltr.getIntPersistance());
    Serial.print("  Thresholds: "); Serial.print(ltr.getLowThreshold()); Serial.print(" & "); Serial.println(ltr.getHighThreshold());
  }

  
  Serial.print("AHT Begin: ");
  if(aht.begin()){
    Serial.println("OK");
  }

  Serial.print("SPIFFS Begin: ");
  if(SPIFFS.begin(true)){
    Serial.println("OK");
  }

  Serial.print("EEPROM Begin: ");
  if(EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("OK");
  }

  loop_time = millis();
  
  while(digitalRead(button_pin) == 0 && reset_complete == false){
    digitalWrite(external_led_pin, HIGH);
    delay(500);
    digitalWrite(external_led_pin, LOW);
    delay(500);

    if(millis() > loop_time + boot_mode_delay && boot_into_config == false){
      digitalWrite(external_led_pin, HIGH);
      delay(100);
      digitalWrite(external_led_pin, LOW);
      delay(100);
      digitalWrite(external_led_pin, HIGH);
      delay(100);
      digitalWrite(external_led_pin, LOW);
      delay(100);
      boot_into_config = true;
    }
    if(millis() > loop_time + reset_delay){
      
      ssid = "";
      password = "";
      apiKey = ""; 
      home_assistant_url = ""; 

      int addr = 0;
      for (int i = 0; i < EEPROM_SIZE; i++) {
          EEPROM.write(addr, 0);
          addr += 1;
      }
      
      if(EEPROM.commit()){
        digitalWrite(external_led_pin, HIGH);
        delay(100);
        digitalWrite(external_led_pin, LOW);
        delay(100);
        digitalWrite(external_led_pin, HIGH);
        delay(100);
        digitalWrite(external_led_pin, LOW);
        delay(100);
        digitalWrite(external_led_pin, HIGH);
        delay(100);
        digitalWrite(external_led_pin, LOW);
        delay(100);
        digitalWrite(external_led_pin, HIGH);
        delay(100);
        digitalWrite(external_led_pin, LOW);
        delay(100);
        digitalWrite(external_led_pin, HIGH);
        delay(100);
        digitalWrite(external_led_pin, LOW);
        delay(100);
        reset_complete == true;
      }
    
      break;
    }
  }

  eeprom_ssid = "";
  eeprom_password = "";
  eeprom_apiKey = ""; 
  eeprom_home_assistant_url = ""; 

  Serial.println("*** READING EEPROM ***");
  
  eeprom_ssid = EEPROM.readString(0);
  Serial.print("  SSID: ");
  Serial.println(eeprom_ssid);
  eeprom_password = EEPROM.readString(64);
  Serial.print("  PASS: ");
  Serial.println(eeprom_password);
  eeprom_apiKey = EEPROM.readString(128);
  Serial.print("  API: ");
  Serial.println(eeprom_apiKey);
  eeprom_home_assistant_url = EEPROM.readString(384);
  Serial.print("  URL: ");
  Serial.println(eeprom_home_assistant_url);
  
  Serial.println("*** EEPROM READ COMPLETE***");

  
  eeprom_ssid.toCharArray(ssid, eeprom_ssid.length()+1);
  eeprom_password.toCharArray(password, eeprom_password.length()+1);
  
  apiKey = String(eeprom_apiKey);
  home_assistant_url = String(eeprom_home_assistant_url);
  
  Serial.print("  SSID: ");
  Serial.println(ssid);
  Serial.print("  PASS: ");
  Serial.println(password);
  Serial.print("  API: ");
  Serial.println(apiKey);
  Serial.print("  URL: ");
  Serial.println(home_assistant_url);

  String mdns_string = "smartplant" + String(mac.substring(mac.length()-5, mac.length()-3)) + String(mac.substring(mac.length()-2, mac.length()));
  char* mdns = (char*)malloc(mdns_string.length() + 1);
  mdns_string.toCharArray(mdns, mdns_string.length()+1);
  
  
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(mdns);
  WiFi.begin(ssid, password);
  Serial.println("*** Connecting WIFI ***");
  Serial.print("  ");
  
  loop_time = millis();
  while (WiFi.status() != WL_CONNECTED && millis() < loop_time + wifi_timeout) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("FAILED!");
    wifi_status = false;
    if(boot_into_config == true){
      Serial.println("*** Starting AP! ***");
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
      //WiFi.setHostname(mdns);
      WiFi.softAP(hostname,"SMARTPLANT");
      WiFi.softAPConfig(local_ip, gateway, subnet);
      IPAddress IP = WiFi.softAPIP();  /*IP address is initialized*/
      Serial.print("  AP IP address: ");
      Serial.println(IP);
    }
  }else{
    Serial.println("  SUCCESS!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());

    // ----- Get Time ----- //
    Serial.print("NTP: ");
    timeClient.begin();
    if (timeClient.update()) {
      rtc.setTime(timeClient.getEpochTime());
      rtcOffset = millis();
      Serial.println("OK");
    }else{
      Serial.println("Failed");
    }
    // ----- Get Time ----- //
    
    wifi_status = true;
  }

  
  Serial.print("  MDNS: ");
  Serial.println(mdns);
  MDNS.begin(mdns);
  MDNS.addService("http", "tcp", 80);

  if(boot_into_config == true){
    Serial.println("*** CONFIG MODE ***");
    server.begin();
    server.onNotFound(notFound);
  
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/style.css", "text/css");
    });
  
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/script.js", "text/javascript");
    });
  
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });
  
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
      restarting = true;
      request->send(SPIFFS, "/index.html", String(), false, processor);
      
    });
    
    server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifi.html", String(), false, processor);
    });
  
    server.on("/wifi", HTTP_POST, [] (AsyncWebServerRequest *request) {
      
      int params = request->params();
      
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST input1 value (direction)
          if (p->name() == "input_ssid") {
            eeprom_ssid = p->value().c_str();
          }
          // HTTP POST input2 value (steps)
          if (p->name() == "input_password") {
            eeprom_password = p->value().c_str();
          }
        }
      }
  
      if(eeprom_ssid == ""){
        eeprom_ssid = "00000";
      }
      if(eeprom_password == ""){
        eeprom_password = "00000";
      }
      Serial.print("Submitted SSID: ");
      Serial.println(eeprom_ssid);
      Serial.print("Submitted Password: ");
      Serial.println(eeprom_password);
      
      EEPROM.writeString(0, eeprom_ssid);
      EEPROM.writeString(64, eeprom_password);
      
      if(EEPROM.commit()){
        wifi_saved_eeprom = true;
      }else{
        wifi_saved_eeprom = false;
      }
      
  
      request->send(SPIFFS, "/wifi.html", String(), false, processor);
  
    });
    
  
  
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
      test_connection_code = test_home_assistant_connection();
      request->send(SPIFFS, "/config.html", String(), false, processor);
    });
  
    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request){
      
      int params = request->params();
          
          for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
              // HTTP POST input1 value (direction)
              if (p->name() == "url") {
                home_assistant_url = p->value().c_str();
              }
              // HTTP POST input2 value (steps)
              if (p->name() == "api") {
                apiKey = p->value().c_str();
              }
            }
          }
          
       test_connection_code = test_home_assistant_connection();
  
      if(apiKey == ""){
        apiKey = "00000";
      }
      if(home_assistant_url == ""){
        home_assistant_url = "00000";
      }
      Serial.print("Submitted API Key: ");
      Serial.println(apiKey);
      Serial.print("Submitted URL: ");
      Serial.println(home_assistant_url);
     
      EEPROM.writeString(128, apiKey);
      EEPROM.writeString(384, home_assistant_url);
  
      if(EEPROM.commit()){
        config_saved_eeprom = true;
      }else{
        config_saved_eeprom = false;
      }
       
         
      request->send(SPIFFS, "/config.html", String(), false, processor);
    });
  
    server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/sensors.html", String(), false, processor);
    });
  
    server.on("/sensors", HTTP_POST, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/sensors.html", String(), false, processor);
    });

    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/settings.html", String(), false, processor);
    });
  
    server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/settings.html", String(), false, processor);
    });
  
    server.on("/firmware", HTTP_GET, [](AsyncWebServerRequest *request){
      update_code = check_version();
      if(update_code==1){
        firmware_code = check_firmware();
        if(firmware_code==1){
          //We're just checking don't worry
        }
      }
      request->send(SPIFFS, "/firmware.html", String(), false, processor);
    });
  
    server.on("/firmware", HTTP_POST, [](AsyncWebServerRequest *request){
      update_code = check_version();
      if(update_code==1){
        firmware_code = check_firmware();
        if(firmware_code==1){
          perform_update = true;
        }
      }
      request->send(SPIFFS, "/firmware.html", String(), false, processor);  
      

      
    });
  }else if(boot_into_config == false){
    
    //display.init(115200); // enable diagnostic output on Serial
    display.init(); // disable diagnostic output on Serial
    
    display.setTextColor(GxEPD_BLACK);
    display.fillScreen(GxEPD_WHITE);
    display.setRotation(0);
    // draw background
    display.drawExampleBitmap(layout, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    display.setFont(&FreeMonoBold12pt7b);
    // partial update to full screen to preset for partial update of box window
    // (this avoids strange background effects)
    //display.drawExampleBitmap(layout, sizeof(layout), GxEPD::bm_default | GxEPD::bm_partial_update);
    display.setRotation(3);

    battery_voltage = get_battery_voltage();
    battery_percent = get_battery_percent();
    bus_voltage = get_bus_voltage();

    soil_face_value = touchRead(face_pin) / 1000;
    soil_back_value = touchRead(back_pin) / 1000;
    delay(100);
    soil_humidity = (soil_face_value + soil_back_value) / 2;
  
  }

 
  Serial.println("*** SETUP DONE ***");
  
}

void loop()
{
  soil_face_value = touchRead(face_pin) / 1000;
  soil_back_value = touchRead(back_pin) / 1000;
  delay(100);
  soil_humidity = (soil_face_value + soil_back_value) / 2;
  soil_humidity_percent = map(soil_humidity,soil_humidity_min,soil_humidity_max,0,100);
  if(soil_humidity_percent < 0){
    soil_humidity_percent = 0;
  }

  if(restarting == true){
    ESP.restart();
  }
  
  
  sensors_event_t aht_humidity, aht_temperature;
  aht.getEvent(&aht_humidity, &aht_temperature);// populate temp and humidity objects with fresh data
  air_humidity = aht_humidity.relative_humidity;
  air_temperature = aht_temperature.temperature;
  
  //battery_percent = 0;
  //battery_voltage = get_battery_voltage();

  if (ltr.newDataAvailable()) {
    valid = ltr.readBothChannels(ambient_light, ir_light);
    if (valid) {

    }
  }

  battery_voltage = get_battery_voltage();
  battery_percent = get_battery_percent();
  board_temperature = get_pcb_thermistor();
  soil_temperature = get_thermistor();
  bus_voltage = get_bus_voltage();
  board_check = check_board();

  if(boot_into_config == false){

    Serial.println("*** UPDATING DISPLAY ***");
    display_update_temperature();
    display_update_battery();
    display_update_humidity();
    display_update_humidity_icon();
    display_update_capacitance();
    display_update_date_time();
    
    display.updateWindow(0, 0, 200, 200, true);

    Serial.println("*** UPDATING DISPLAY DONE***");
    
    if(wifi_status = true){
      send_data();
    }
    
    esp_sleep_enable_ext0_wakeup(wakeup_pin,0);
    esp_sleep_enable_timer_wakeup(deepsleeptime);
    Serial.println("*** SLEEPING ***");
    digitalWrite(external_led_pin, HIGH);
    delay(200);
    digitalWrite(external_led_pin, LOW);
    esp_deep_sleep_start();
  }
  if(firmware_code == 1 && perform_update == true && wifi_status == true){
    update_firmware();
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", String(), false, processor);
}


float get_battery_voltage(){
  float sensorValue = 0;
  for (int i = 1; i <= 10; i++) {
    sensorValue = sensorValue + analogReadMilliVolts(batteryPin);
    delay(5);
  }
  sensorValue = sensorValue/10;
  return sensorValue/480;
}

float get_bus_voltage(){
  float sensorValue = 0;
  for (int i = 1; i <= 10; i++) {
    sensorValue = sensorValue + analogReadMilliVolts(usbPin);
    delay(5);
  }
  sensorValue = sensorValue/10;
  return sensorValue/462;
}

float get_battery_percent(){
  int i = 0;
  float percentage = 0;
  float values[] = {2.55, 3.05, 3.19, 3.27, 3.31, 3.35, 3.36, 3.37, 3.39, 3.395, 3.4, 3.41, 3.42, 3.43, 3.44, 3.45, 3.46, 3.47, 3.475, 3.48, 3.485, 3.49, 3.5, 3.51, 3.513, 3.516, 3.52, 3.525, 3.53, 3.535, 3.54, 3.545, 3.55, 3.5525, 3.555, 3.5575, 3.56, 3.563, 3.566, 3.57, 3.575, 3.58, 3.585, 3.59, 3.5925, 3.595, 3.5975, 3.6, 3.605, 3.61, 3.62, 3.625, 3.63, 3.633, 3.636, 3.64, 3.65, 3.66, 3.67, 3.68, 3.685, 3.69, 3.7, 3.71, 3.72, 3.73, 3.74, 3.75, 3.76, 3.77, 3.79, 3.8, 3.81, 3.82, 3.83, 3.84, 3.85, 3.86, 3.87, 3.88, 3.885, 3.9, 3.91, 3.92, 3.93, 3.94, 3.96, 3.97, 3.98, 3.99, 4, 4.01, 4.02, 4.04, 4.05, 4.06, 4.08, 4.09, 4.11, 4.13, 4.25};
  for (i = 0; i <= sizeof(values)-1; i++) {
    if(battery_voltage >= values[i] && battery_voltage < values[i+1]){
      percentage = i + ((battery_voltage-values[i])/(values[i+1]-values[i]));
      if(percentage > 100){
        percentage = 100;
      }
      return percentage;
    }
  }
  return -1;
}

float get_thermistor(){

  float thermistorAverage = 0;
  
  for (int i=0; i<10; i++){
    Vo = analogRead(ThermistorPin);
    R2 = R1 * (4096 / Vo - 1); //calculate resistance on thermistor
    logR2 = log(R2);
    T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)) - 273.15;
    
    thermistorAverage = thermistorAverage + T; 
    delay(2);
  }
  thermistorAverage = thermistorAverage / 10;
  return thermistorAverage;

}

float get_pcb_thermistor(){
 
  float thermistorAverage = 0;
  
  for (int i=0; i<10; i++){
    Vo = analogRead(PCBThermistorPin);
    R2 = R1 * (4096 / Vo - 1); //calculate resistance on thermistor
    logR2 = log(R2);
    T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)) - 273.15;
    
    thermistorAverage = thermistorAverage + T; 
    delay(2);
  }
  thermistorAverage = thermistorAverage / 10;
  return thermistorAverage;
  
}


void display_update_temperature(){
  uint16_t box_x = 0;
  uint16_t box_y = 0;
  uint16_t box_w = 58;
  uint16_t box_h = 45;
  uint16_t cursor_y = box_y + 16;
  //display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  display.setCursor(box_x, cursor_y);
  display.println("TEMP");
  display.print(air_temperature,1);
  //display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void display_update_battery(){
  
  uint16_t box_x = 72;
  uint16_t box_y = 0;
  uint16_t box_w = 66;
  uint16_t box_h = 45;
  uint16_t cursor_y = box_y + 16;
  //display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  display.setCursor(box_x, cursor_y);
  display.println("BATT");
  if(battery_voltage == 100){
    display.setCursor(box_x, cursor_y+24);
  }else{
    display.setCursor(box_x-9, cursor_y+24);
  }
  if(battery_voltage > 4.3){
    display.print("CHRG");
  }else{
    display.print(battery_voltage,2);
    display.print("V");
  }
  //display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void display_update_humidity(){
  uint16_t box_x = 144;
  uint16_t box_y = 0;
  uint16_t box_w = 56;
  uint16_t box_h = 45;
  uint16_t cursor_y = box_y + 16;
  //display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  display.setCursor(box_x, cursor_y);
  display.println("HUMI");
  display.setCursor(box_x, cursor_y+24);
  display.print(air_humidity,1);
  //display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void display_update_humidity_icon(){
  uint16_t box_x = 60;
  uint16_t box_y = 50;
  uint16_t box_w = 80;
  uint16_t box_h = 80;
  uint16_t cursor_y = box_y + 16;
  
  //display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);

  if(check_board()==false){
    display.setRotation(3);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(88, 90);
    display.print("NO");
    display.setCursor(68, 110);
    display.print("BOARD");
    display.setTextColor(GxEPD_BLACK);
  }else{
    display.setRotation(0);
    if(soil_humidity_percent <= 20){
      display.drawExampleBitmap(humidity_0, 54, 60, 80, 80, GxEPD_BLACK); //68, 60, 80, 80
    }else if(soil_humidity_percent > 20 && soil_humidity_percent <= 40){
      display.drawExampleBitmap(humidity_1, 54, 60, 80, 80, GxEPD_BLACK);    
    }else if(soil_humidity_percent > 40 && soil_humidity_percent <= 60){
      display.drawExampleBitmap(humidity_2, 54, 60, 80, 80, GxEPD_BLACK);    
    }else if(soil_humidity_percent > 60 && soil_humidity_percent <= 80){
      display.drawExampleBitmap(humidity_3, 54, 60, 80, 80, GxEPD_BLACK);    
    }else if(soil_humidity_percent > 80 && soil_humidity_percent <= 100){
      display.drawExampleBitmap(humidity_4, 54, 60, 80, 80, GxEPD_BLACK);    
    }else if(soil_humidity_percent > 100){
      display.drawExampleBitmap(humidity_5, 54, 60, 80, 80, GxEPD_BLACK);    
    }
  }
  //display.updateWindow(box_x, box_y, box_w, box_h, true);
  display.setRotation(3);
  
  display.setCursor(78, 150);
  if(soil_humidity_percent < 0 | check_board()==false){
    display.setTextColor(GxEPD_BLACK);
    display.print("ERR!");
    display.setTextColor(GxEPD_BLACK);
  }else{
    display.print(soil_humidity_percent,0);
    display.print("%");
  }
}


void display_update_capacitance()
{
  uint16_t box_x = 55;
  uint16_t box_y = 0;
  uint16_t box_w = 90;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + 16;
  //display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  display.setRotation(2);
  display.setCursor(box_x, cursor_y);
  display.print("F:"); 
  display.print(soil_face_value,0); 
  
  display.setCursor(box_x, 199);
  display.print("B:"); 
  display.print(soil_back_value,0); 
  //display.updateWindow(box_x, box_y, box_w, box_h, true);
  display.setRotation(3);
}

void display_update_date_time(){
  uint16_t box_x = 0;
  uint16_t box_y = 170;
  uint16_t box_w = 200;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + 16;
  //display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
  display.setCursor(box_x, cursor_y);
  display.print(String(rtc.getTime("%d/%m/%y %H:%M")));
  //display.updateWindow(box_x, box_y, box_w, box_h, true);
}




bool check_board(){
  pinMode(check_pin_out, OUTPUT);
  pinMode(check_pin_in, INPUT);
  digitalWrite(check_pin_out, HIGH);
  delay(10);
  
  int pin_value = analogRead(check_pin_in);

  digitalWrite(check_pin_out, LOW);
  
  if(pin_value > 2048){
    return true;
  }else{
    return false;
  }
}

int check_version(){
  Serial.println("Checking for updates");
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(http_timeout);
  http.begin(client, version_file);
 
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0 && httpResponseCode<300) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    server_version = http.getString();
    server_version.trim();
    Serial.print("  Server version: ");
    Serial.print(server_version);
    Serial.print(" (Own version: ");
    Serial.print(own_version);
    Serial.println(")");
    if(own_version != server_version){
      Serial.println("  Update required");
      firmware_status = "UPDATE REQUIRED";
      http.end();
      return 1;
    }
    else{
      Serial.println("  No update required");
      firmware_status = "NO UPDATE REQUIRED";
      http.end();
      return 0;
    }
  }
  else {
    Serial.print("  Error code: ");
    Serial.println(httpResponseCode);
    firmware_status = "VERSION NOT FOUND ("+String(httpResponseCode)+")";
    http.end();
    return -1;
  }
  // Free resources
  
  http.end();
  return 0;
}


int check_firmware(){
  Serial.print("Checking firmware file: ");
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(http_timeout);
  http.begin(client, firmware_file);
 
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0 && httpResponseCode<300) {
    firmware_status = "UPDATE AVAILABLE";
    Serial.println("OK");
    return 1;
  }
  else {
    Serial.print("  Error code: ");
    Serial.println(httpResponseCode);
    http.end();
    firmware_status = "FIRMWARE NOT FOUND ("+String(httpResponseCode)+")";
    return -1;
  }
  // Free resources
  
  http.end();
  return 0;
}


void update_firmware(){
  
  perform_update = false;
  
  for (int i = 1; i <= 10; i++) {
    digitalWrite(external_led_pin, LOW);
    delay(50);
    digitalWrite(external_led_pin, HIGH);
    delay(50);
  }
  
  Serial.println("*** UPDATING FIRMWARE ***");
    
  Serial.print("Checking tree file: ");
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(http_timeout);
  http.begin(client, tree_file);
 
  String payload = ""; 
  String lines[20];
  String newline[] = { "\r\n", "\r", "\n" };
  int StringCount = 0;
  int index;
  
  int httpResponseCode = http.GET();
  if (httpResponseCode>0 && httpResponseCode<300) {
    
    Serial.println("OK");

    /*
    Serial.print("Formatting storage: ");
    bool formatted = SPIFFS.format();
    if(formatted){
      Serial.println("OK");
    }else{
      Serial.println("Failed");
    }
    */
    
    payload = http.getString();
    
    //Serial.println(payload.length());
    //Serial.println(payload);
    
    while (payload.length() > 0)
    {
      index = payload.indexOf("\r\n");
      if (index == -1) // No space found
      {
        lines[StringCount] = payload;
        lines[StringCount].trim();
        StringCount++;
        break;
      }
      else
      {
        lines[StringCount] = (payload.substring(0, index));
        lines[StringCount].trim();
        //Serial.println(lines[StringCount]);
        StringCount++;
        payload = payload.substring(index+1);
      }
    }
    
    Serial.println("Downloading Github data files: ");
    
    // Show the resulting substrings
    for (int i = 0; i < StringCount; i++)
    {
      digitalWrite(external_led_pin, HIGH);
      
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;
      http.setTimeout(http_timeout);
      http.begin(client, github_data_path + String(lines[i]));
      int httpResponseCode = http.GET();

      Serial.print("  ");
      Serial.print(lines[i]);
      Serial.print(": ");
      Serial.println(httpResponseCode);

      digitalWrite(external_led_pin, LOW);
      File file = SPIFFS.open("/" + lines[i], FILE_WRITE);
     
      if (!file) {
        Serial.println("  There was an error opening the file for writing");
        return;
      }
      if (file.print(http.getString())) {
        Serial.println("    File was written");
      } else {
        Serial.println("    File write failed");
      }
     
      file.close();
  
    }
  
  }

  digitalWrite(external_led_pin, HIGH);
  Serial.print("Downloading firmware!");
  ESPhttpUpdate.update(firmware_file);

  //These will never print!
  Serial.println("OK");
  Serial.println("*** UPDATE COMPLETE ***");

}

int test_home_assistant_connection(){
  WiFiClientSecure client;
  client.setInsecure();
  Serial.println("Testing HomeAssistant connection: ");
  
  String final_apiKey = "Bearer " + apiKey;
  String final_url = home_assistant_url+"/api/states/sensor." + hostname + "_test_connection";
  String final_data = "{\"state\": \""+String(random(1000))+"\", \"attributes\": {\"friendly_name\": \""+hostname+"_TEST\",\"icon\": \"mdi:flower\"}}";

  Serial.print("  API: ");
  Serial.println(final_apiKey);
  Serial.print("  URL: ");
  Serial.println(final_url);
  Serial.print("  DATA: ");
  Serial.println(final_data);
  
  HTTPClient http;
  http.setTimeout(http_timeout);
  http.begin(client, final_url);
  http.addHeader("Authorization", final_apiKey);
  http.addHeader("Content-Type", "application/json");
  String httpRequestData = final_data;           
  int httpResponseCode = http.POST(httpRequestData);
  Serial.print("  HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();

  if(httpResponseCode > 199 && httpResponseCode < 300){
    home_assistant_connection = true;
  }else{
    home_assistant_connection = false;
  }
  
  return httpResponseCode;
}

void send_data(){

  Serial.println("*** SENDING DATA ***");
  WiFiClientSecure client;
  client.setInsecure();
  String httpRequestData = "";
  int httpResponseCode = 0;
  HTTPClient http;

  apiKey = "Bearer " + apiKey;
  Serial.print("  URL: ");
  Serial.println(home_assistant_url);
  Serial.print("  API: ");
  Serial.println(apiKey);
  
  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_air_temperature);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(air_temperature)+"\", \"attributes\": {\"unit_of_measurement\": \"°C\",\"friendly_name\": \""+hostname+" Air Temperature\",\"icon\": \"mdi:thermometer\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_air_humidity);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(air_humidity)+"\", \"attributes\": {\"unit_of_measurement\": \"%\",\"friendly_name\": \""+hostname+" Air Humidity\",\"icon\": \"mdi:water-percent\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);
  
  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_battery_percent);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(battery_percent)+"\", \"attributes\": {\"unit_of_measurement\": \"%\",\"friendly_name\": \""+hostname+" Battery Percent\",\"icon\": \"mdi:current-dc\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_battery_voltage);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(battery_voltage)+"\", \"attributes\": {\"unit_of_measurement\": \"V\",\"friendly_name\": \""+hostname+" Battery Voltage\",\"icon\": \"mdi:current-dc\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_board_temperature);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(board_temperature)+"\", \"attributes\": {\"unit_of_measurement\": \"°C\",\"friendly_name\": \""+hostname+" Board Temperature\",\"icon\": \"mdi:thermometer\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_bus_voltage);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(bus_voltage)+"\", \"attributes\": {\"unit_of_measurement\": \"V\",\"friendly_name\": \""+hostname+" Bus Voltage\",\"icon\": \"mdi:current-dc\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_soil_moisture_face);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(soil_face_value)+"\", \"attributes\": {\"unit_of_measurement\": \"pF\",\"friendly_name\": \""+hostname+" Face Capacitance\",\"icon\": \"mdi:current-dc\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_soil_moisture_back);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(soil_back_value)+"\", \"attributes\": {\"unit_of_measurement\": \"pF\",\"friendly_name\": \""+hostname+" Back Capacitance\",\"icon\": \"mdi:current-dc\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_soil_moisture_percent);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(soil_humidity_percent)+"\", \"attributes\": {\"unit_of_measurement\": \"%\",\"friendly_name\": \""+hostname+" Soil Moisture Percent\",\"icon\": \"mdi:water-percent\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_soil_temperature);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(soil_temperature)+"\", \"attributes\": {\"unit_of_measurement\": \"°C\",\"friendly_name\": \""+hostname+" Soil Temperature\",\"icon\": \"mdi:thermometer\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_ambient_light);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(ambient_light)+"\", \"attributes\": {\"unit_of_measurement\": \"Lux\",\"friendly_name\": \""+hostname+" Ambient Light\",\"icon\": \"mdi:weather-sunny\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_ir_light);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(ir_light)+"\", \"attributes\": {\"unit_of_measurement\": \"Lux\",\"friendly_name\": \""+hostname+" IR Light\",\"icon\": \"mdi:weather-sunny\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);

  http.begin(client, home_assistant_url+"/api/states/sensor." + hostname + plant_monitor_version);
  http.addHeader("Authorization", apiKey);
  http.addHeader("Content-Type", "application/json");
  httpRequestData = "{\"state\": \""+String(own_version)+"\", \"attributes\": {\"friendly_name\": \""+hostname+" Firmware Version\",\"icon\": \"mdi:counter\"}}";           
  httpResponseCode = http.POST(httpRequestData);
  Serial.println(httpResponseCode);
  http.end();
  delay(10);
  
  Serial.println("*** SEND DATA COMPLETE ***");
}
