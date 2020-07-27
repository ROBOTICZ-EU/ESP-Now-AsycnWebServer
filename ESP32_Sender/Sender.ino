/*
  Rui Santos   Sender(Sensor)     
  Complete project details at https://RandomNerdTutorials.com/esp32-esp-now-wi-fi-web-server/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  tech500 modified code for ESP-Now-AsyncWebServer project.
  
*/

#include <esp_now.h>
#include <WiFi.h>
#include <BME280I2C.h>   //Use the Arduino Library Manager, get BME280 by Tyler Glenn
#include <Wire.h>    //Part of ESP32 Board Manager install  -----> Used for I2C protocol


//The gateway access point credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1   

// Digital pin connected to the DHT sensor
//---#define DHTPIN 4  

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define BMETYPE    BME280    // DHT 22 (AM2302) //--------Is this okay?-----------------------------BME280-----------------
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

BME280I2C bme; //----------------BME280I2C BMME280--------------------------

float temp(NAN), temperature, hum(NAN), pres(NAN), currentPressure, pressure, millibars, fahrenheit, RHx, T, heat_index, dew, dew_point, atm;

//MAC Address of the receiver 
uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0xA9, 0x7D, 0xD4};   //------------Receiver MAC address?-----------------------------

//Wi-Fi channel (must match the gateway wi-fi channel as an access point)
#define CHAN_AP 9  //-------------------ISP Router is on Channel 1----------Was channel 2--------

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int id;
    float temp;
    float hum;
    float press;     //BME280 pressure reading
    int readingId;
} struct_message;

//Create a struct_message called myData
struct_message myData;

unsigned long previousMillis = 0;   // Stores last time temperature was published
//const long interval = 10000;        // Interval at which to publish sensor readings
const long interval = 60 * 1000;       //  One minute, interval Interval at which to publish sensor readings

unsigned int readingId = 0;

unsigned long delayTime;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  //Init Serial Monitor
  Serial.begin(115200);

  Wire.begin(21, 22);  //Change this for your I2C connection
 
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Access Point...");
  }
  
  while(!bme.begin()) {
          Serial.println("Could not find BME280 sensor!");
          delayTime = 1000;
     }

     // bme.chipID(); // Deprecated. See chipModel().
     switch(bme.chipModel())  {
     case BME280::ChipModel_BME280:
          Serial.println("Found BME280 sensor! Success.");
          break;
     case BME280::ChipModel_BMP280:
          Serial.println("Found BMP280 sensor! No Humidity available.");
          break;
     default:
          Serial.println("Found UNKNOWN sensor! Error!");
     }

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  //Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = CHAN_AP;  
  peerInfo.encrypt = false;
  
  //Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    
    getWeatherData();
    
    //Set values to send
    myData.id = BOARD_ID;
    myData.temp = temperature;     //getWeather() needed variables-----------------
    myData.hum = hum;
    myData.press = currentPressure;
    myData.readingId = readingId++;
     
    //Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }
}

///////////////////////
void getWeatherData()
//////////////////////
{

     BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
     BME280::PresUnit presUnit(BME280::PresUnit_hPa);

     bme.read(pres, temp, hum, tempUnit, presUnit);

     delay(250);  //getting data

     pressure = pres + 30.602;  //Correction for relative pressure in milbars ////////////////  Relative pressure in millibars = Absoute pressure + (elevation(in meters)/ 8.3)
     currentPressure = pressure * 0.02953;
     //Absolute pressure is the reading from the Bosch BME280 in millibars.
     //Note: Elevation is in meters and Absolute pressure is in milbars; result is correction factor in millibars.  Elevation in code measure using Garmin Nuvi.

     temperature = (temp * 1.8) + 32;  //Convert to fahrenheight

     
}
