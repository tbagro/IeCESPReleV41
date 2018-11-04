/*******************************************************************************
**  CANAL INTERNET E COISAS                                                   **
**  IeCESPRele V4.0 Library - Generic Functions                               **
**  10/2018 - Andre Michelon                                                  **
**  Adaptado por Tiago Batista                                                **
*******************************************************************************/


/*******************************************************************************
* LIBRARIES
*******************************************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

/*******************************************************************************
* CONSTANT DEFINITION
*******************************************************************************/
// Build Information
const char      BUILD[]                         = __DATE__ " " __TIME__;

// Web Interface Username
const char      WWW_USERNAME[]                  = "esprele";

// System Password
const char      PASSWORD[]                      = "Rele8266";

// WebServer Port
const byte      WEBSERVER_PORT                  = 80;

// DNS Server Port
const byte      DNSSERVER_PORT                  = 53;

// WebServer Headers
const char*     WEBSERVER_HEADER_KEYS[]         = {"User-Agent"};

// Directory Max Number of Files
const byte      DIRECTORY_MAX_FILES             = 16;

// NTP sync interval
const int       NTP_INT                         = 6 * 60 * 60; // 6h

// Max log entries
const byte      LOG_ENTRIES                     = 50;


// Config Parameters
const byte      CFG_REFERENCE                   =  0;
const byte      CFG_PWD_ALLWAYS_NEEDED          = 35 + CFG_REFERENCE;
const byte      CFG_SOFTAP_ALLWAYS_ON           =  1 + CFG_PWD_ALLWAYS_NEEDED;
const byte      CFG_WIFI_SSID                   =  1 + CFG_SOFTAP_ALLWAYS_ON;
const byte      CFG_WIFI_PWD                    = 35 + CFG_WIFI_SSID;
const byte      CFG_NETWORK_ID                  = 35 + CFG_WIFI_PWD;
const byte      CFG_TIME_ZONE                   = 35 + CFG_NETWORK_ID;
const byte      CFG_LOG_DAY                     =  1 + CFG_TIME_ZONE;
const byte      CFG_LAST_SAVED                  =  1 + CFG_LOG_DAY;
const byte      CFG_AG_ALLWAYS_ON               =  1 + CFG_LAST_SAVED;
const byte      CFG_TOTAL_LENGHT                =  4 + CFG_AG_ALLWAYS_ON;

// Pin Mapping
const byte      RELAY_PIN                       =  D1;

// Software Version
const char      SW_VERSION[]                    = "4.1";

/*******************************************************************************
* ENTITIES
*******************************************************************************/
// Web Server
ESP8266WebServer  server(WEBSERVER_PORT);

// DNS Server
DNSServer         dnsServer;

/*******************************************************************************
* VARIABLES
*******************************************************************************/
// WiFi Status
wl_status_t       wifiStatus    = WL_DISCONNECTED;

// WiFi SoftAP flag
boolean           softAPActive  = false;

boolean           AgActive      = false; // schedule

// File System Version
String            fsVersion     = "";

// Boot DateTime
time_t            bootDT        = 0;

// Last relay event
String            lastEvent     = "N/D";

// System Log
String            logStr[LOG_ENTRIES];
byte              logIndex      = 0;



// Generic String;
String s;

// Schedule entries;
String schedule;
