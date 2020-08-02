/*
  ESP-NOW based sensor using a BME280 temperature/pressure/humidity sensor

  Details:  https://arduinodiy.wordpress.com/2020/02/06/very-deep-sleep-and-energy-saving-on-esp8266-part-5-esp-now/#:~:text=Raspberry%20Pi%20stuff-,Very%20Deep%20Sleep%20and%20energy%20saving%20on%20ESP8266,5%3A%20ESP%2DNOW%20and%20WiFi&text=ESP%2DNOW%20is%20a%20kind,device%20to%20another%20without%20connection.

  Sends readings every 15 minutes to a server with a fixed mac address

  It takes about 215 milliseconds to wakeup, send a reading and go back to sleep,
  and it uses about 70 milliAmps while awake and about 25 microamps while sleeping,
  so it should last for a good year even AAA alkaline batteries.

  Anthony Elder
  License: Apache License v2
*/

#include <esp_now.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <sys/time.h>  // struct timeval --> Needed to sync time
#include <time.h>   // time() ctime() --> Needed to sync time
#include <BME280I2C.h>   //Use the Arduino Library Manager, get BME280 by Tyler Glenn
#include <Wire.h>    //Part of version 1.0.4 ESP32 Board Manager install  -----> Used for I2C protocol


//The gateway access point credentials
const char* APssid = "ESP32-Access-Point";
const char* APpassword = "123456789";

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

// this is the MAC Address of the remote ESP server which receives these sensor readings
uint8_t remoteMac[] = {0x24, 0x6F, 0x28, 0xA9, 0x7D, 0xD4};
//uint8_t remoteMac2[] = {0xEE, 0xFA, 0xBC, 0x9B, 0xF5, 0x6D};


//Wi-Fi channel (must match the gateway wi-fi channel as an access point)
#define CHAN_AP 9

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to minutes */
#define TIME_TO_SLEEP 15  // Out of 5 minutes
//#define SEND_TIMEOUT .2 // 245 millis seconds timeout

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

//MAC Address of the receiver
uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0xA9, 0x7D, 0xD4};   //------------Receiver MAC address?-----------------------------

BME280I2C bme;

float temp(NAN), temperature, hum(NAN), pres(NAN), currentPressure, pressure, millibars, fahrenheit, RHx, T, heat_index, dew, dew_point, atm;

unsigned long previousMillis = 0;   // Stores last time temperature was published
//const long interval = 10000;        // Interval at which to publish sensor readings
//const long interval = 4.5 * 60 * 1000;       // Out of 5 minute, interval at which to publish sensor readings

unsigned int readingId = 0;

unsigned long delayTime;

RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n\n", wakeup_reason); break;
  }

}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t\n");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}//volatile boolean callbackCalled;

WiFiClient client;

///Are we currently connected?
boolean connected = false;

///////////////////////////////////////////////
WiFiUDP udp;
// local port to listen for UDP packets
//Settings pertain to NTP
const int udpPort = 1337;
//NTP Time Servers
const char * udpAddress1 = "us.pool.ntp.org";
const char * udpAddress2 = "time.nist.gov";
char incomingPacket[255];
char replyPacket[] = "Hi there! Got the message :-)";

/*

  Found this reference on setting TZ: http://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html

  Here are some example TZ values, including the appropriate Daylight Saving Time and its dates of applicability. In North American Eastern Standard Time (EST) and Eastern Daylight Time (EDT), the normal offset from UTC is 5 hours; since this is west of the prime meridian, the sign is positive. Summer time begins on Marchâ€™s second Sunday at 2:00am, and ends on Novemberâ€™s first Sunday at 2:00am.
*/
#define TZ "EST+5EDT,M3.2.0/2,M11.1.0/2"

int DOW, MONTH, DATE, YEAR, HOUR, MINUTE, SECOND;

int lc = 0;
time_t tnow;

char strftime_buf[64];

String dtStamp(strftime_buf);

/////////////////////////////////////////////

// Replace with your network details
const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

//setting the addresses
IPAddress staticIP(10, 0, 0, 200);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);


void setup() {

  Serial.begin(115200); Serial.println();

  if (bootCount == 0) {
    WiFi.persistent( false ); // for time saving

    // Connecting to local WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS);
    WiFi.begin(ssid, password);
    delay(10);
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(100);
    }

    Serial.println("\n WiFi connected");
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Server IP:  ");
    Serial.println(WiFi.localIP());
    Serial.println("\n");

    configTime(0, 0, udpAddress1, udpAddress2);
    setenv("TZ", "EST+5EDT,M3.2.0/2,M11.1.0/2", 3);   // this sets TZ to Indianapolis, Indiana
    tzset();

    //udp only send data when connected
    if (connected)
    {
      //Send a packet
      udp.beginPacket(udpAddress1, udpPort);
      udp.printf("Seconds since boot: %u", millis() / 1000);
      udp.endPacket();
    }

    Serial.print("wait for first valid timestamp ");

    while (time(nullptr) < 100000ul)
    {
      Serial.print(".");
      delay(1000);
    }

    Serial.println(" time synced");



    WiFi.disconnect(true);

    WiFi.mode(WIFI_OFF);

  }



  Serial.println("WiFi Disconnected");

  WiFi.persistent( false ); // for time saving



  // read sensor first before awake generates heat
  getWeatherData();

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.begin(APssid, APpassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Access Point...");
  }
  WiFi.disconnect();

  Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
  Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", remoteMac[0], remoteMac[1], remoteMac[2], remoteMac[3], remoteMac[4], remoteMac[5]);
  Serial.printf(", channel: %i\n", CHAN_AP);

  Wire.begin(21, 22);

  while (!bme.begin()) {
    Serial.println("Could not find BME280 sensor!");
    delayTime = 1000;
  }

  // bme.chipID(); // Deprecated. See chipModel().
  switch (bme.chipModel())  {
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
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  getDateTime();

  Serial.println(dtStamp);


  /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("");
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Minutes");

  /*
    Next we decide what all peripherals to shut down/keep on
    By default, ESP32 will automatically power down the peripherals
    not needed by the wakeup source, but if you want to be a poweruser
    this is for you. Read in detail at the API docs
    http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
    Left the line commented as an example of how to configure peripherals.
    The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
    Now that we have setup a wake cause and if needed setup the
    peripherals state in deep sleep, we can now start going to
    deep sleep.
    In the case that no wake up sources were provided but deep
    sleep was started, it will sleep forever unless hardware
    reset occurs.
  */

}

void loop() {

  //udp only send data when connected
  if (connected)
  {
    //Send a packet
    udp.beginPacket(udpAddress1, udpPort);
    udp.printf("Seconds since boot: %u", millis() / 1000);
    udp.endPacket();
  }


  getDateTime();

  //unsigned long currentMillis = millis();
  //if (currentMillis - previousMillis >= interval) {
  if ((MINUTE % 15 == 0) && (SECOND == 0)) {
    // Save the last time a new reading was published
    //previousMillis = currentMillis;

    getWeatherData();

    //Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }

    delayTime = 2000;

    if (result == ESP_OK) {

      getDateTime();

      Serial.println(dtStamp);

      Serial.println("Going to sleep now");
      Serial.flush();
      esp_deep_sleep_start();
      Serial.println("This will never be printed");
    }
  }


}

String getDateTime()
{
  struct tm *ti;

  tnow = time(nullptr) + 1;
  //strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime(&tnow));
  ti = localtime(&tnow);
  DOW = ti->tm_wday;
  YEAR = ti->tm_year + 1900;
  MONTH = ti->tm_mon + 1;
  DATE = ti->tm_mday;
  HOUR  = ti->tm_hour;
  MINUTE  = ti->tm_min;
  SECOND = ti->tm_sec;

  strftime(strftime_buf, sizeof(strftime_buf), "%a , %m/%d/%Y , %H:%M:%S %Z", localtime(&tnow));
  dtStamp = strftime_buf;
  return (dtStamp);

}

void getWeatherData()
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

  //Set values to send
  myData.id = BOARD_ID;
  myData.temp = temperature;     //getWeather() needed variables-----------------
  myData.hum = hum;
  myData.press = currentPressure;
  myData.readingId = readingId++;


}
