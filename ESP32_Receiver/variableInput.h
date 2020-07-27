//
//   "Rain Gaugeino" and  
//   variableInput.h place in sketch folder
//   William M. Lucid   07/27/2020 07:29 EDT    
// 

// Replace with your network details  
//const char * host  = "esp32"; 

// Replace with your network credentials (STATION)
const char* ssidStation = "ssid";
const char* passwordStation = "password";

Replace with your network credentials (Access Point)
const char* ssidAP = "ESP32-Access-Point";
const char* passwordAP = "123456789";  //Must be 8 characters min.
//Settings pertain to NTP
const int udpPort = 123;
//NTP Time Servers
const char * udpAddress1 = "198.50.238.156";
const char * udpAddress2 = "132.163.96.3";

//publicIP accessiable over Internet with Port Forwarding; know the risks!!!
//WAN IP Address.  Or use LAN IP Address --same as server ip; no Internet access. 
#define publicIP  "yourpublicIP"  //Part of href link for "GET" requests
String LISTEN_PORT = "Port used as String"; //Part of href link for "GET" requests

//"xx.xx.xx.xx" = publicIP  "yyyy" = PORT used in URL
String linkAddress = "xx.xx.xx.xx:yyyy";  

//Host ip address  --prevents logging host compute
String ip1String = "yourComputerIP";   

int PORT = yyyy;  //Web Server port

//Graphing requires "FREE" "ThingSpeak.com" account..  
//Enter "ThingSpeak.com" data here....
//Example data; enter yout account data..
unsigned long int myChannelNumber = 123456; 
const char * myWriteAPIKey = "E12345";

//Server settings  --ip address of web server  --AsynWebServer
#define ip {10,0,0,200}
#define subnet {255,255,255,0}
#define gateway {10,0,0,1}
#define dns {10,0,0,1}

//FTP Credentials
const char * ftpUser = "admin";
const char * ftpPassword = "admin";
 
//Restricted Access
const char* Restricted = "/ACCESS.TXT";  //Can be any filename.  
//Will be used for "GET" request path to pull up client ip list.

///////////////////////////////////////////////////////////
//   "pulicIP/LISTEN_PORT/reset" wiill restart the server
///////////////////////////////////////////////////////////

///////////////// OTA Support //////////////////////////

const char* http_username = "admin";
const char* http_password = "admin";

// xx.xx.xx.xx:yyyy/login will prompt for log in credentials; this will allow updating firmware using:
// xx.xx.xx.xx:yyyy/update
//
// xx.xx.xx.xx being publicIP and yyyy being PORT.
//
///////////////////////////////////////////////////////
