/*******************************************************************************
**  CANAL INTERNET E COISAS                                                   **
**  IeCESPRele V4.0 Library - Generic Functions                               **
**  10/2018 - Andre Michelon                                                  **
**  Adaptado por Tiago Batista                                                **
*******************************************************************************/

/*******************************************************************************
* LIBRARIES AND AUXILIARY FILES
*******************************************************************************/
#include <TimeLib.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#include "IeCESPReleV4Def.h"

/*******************************************************************************
* FUNCTIONS
*******************************************************************************/
void hold(const unsigned int &ms) {
  // Non blocking delay
  unsigned long m = millis();
  while (millis() - m < ms) {
    yield();
  }
}

String dateTimeStr(const time_t &t, const bool flBr = true) {
  // Return time_t as String "yyyy-mm-dd hh:mm:ss" or "dd/mm/yyyy hh:mm:ss"
  String sFn;
  if (t == 0) {
    return F("N/A");
  } else if (flBr) {
    // dd/mm/yyyy hh:mm:ss
    sFn = "";
    if (day(t) < 10) {
      sFn += '0';
    }
    sFn += String(day(t)) + '/';
    if (month(t) < 10) {
      sFn += '0';
    }
    sFn += String(month(t)) + '/' + String(year(t)) + ' ';
  } else {
    // yyyy-mm-dd hh:mm:ss
    sFn = String(year(t)) + '-';
    if (month(t) < 10) {
      sFn += '0';
    }
    sFn += String(month(t)) + '-';
    if (day(t) < 10) {
      sFn += '0';
    }
    sFn += String(day(t)) + ' ';
  }
  if (hour(t) < 10) {
    sFn += '0';
  }
  sFn += String(hour(t)) + ':';
  if (minute(t) < 10) {
    sFn += '0';
  }
  sFn += String(minute(t)) + ':';
  if (second(t) < 10) {
    sFn += '0';
  }
  sFn += String(second(t));
  return sFn;
}

void log(const String &type, const String &msg) {
  // Generate log in memory
  logStr[logIndex] = dateTimeStr(now()) + ";" + type + ";" + msg;
  Serial.println(logStr[logIndex]);
  // Increment log index;
  logIndex = (logIndex + 1) % LOG_ENTRIES;
}

String hexStr(const unsigned long &h, const byte &l = 8) {
  // Return value as Hexadecimal String
  String sFn;
  sFn= String(h, HEX);
  sFn.toUpperCase();
  sFn = ("00000000" + sFn).substring(sFn.length() + 8 - l);
  return sFn;
}

String deviceID() {
  // Return the Device ID
  return "ESPRele" + hexStr(ESP.getChipId());
}

String configLastSavedStr() {
  // Return last Config save date/time as string
  unsigned long lFn;
  lFn  = EEPROM.read(CFG_LAST_SAVED    );
  lFn  = lFn << 8;
  lFn |= EEPROM.read(CFG_LAST_SAVED + 1);
  lFn  = lFn << 8;
  lFn |= EEPROM.read(CFG_LAST_SAVED + 2);
  lFn  = lFn << 8;
  lFn |= EEPROM.read(CFG_LAST_SAVED + 3);
  return dateTimeStr(lFn);
}

String activeTimeStr(const time_t &t){
  // Return time_t as String "d:hh:mm:ss"
  String sFn = String(t / SECS_PER_DAY) + ':';
  if (hour(t) < 10) {
    sFn += '0';
  }
  sFn += String(hour(t)) + ':';
  if (minute(t) < 10) {
    sFn += '0';
  }
  sFn += String(minute(t)) + ':';
  if (second(t) < 10) {
    sFn += '0';
  }
  sFn += String(second(t));
  return sFn;
}

int timeZone() {
  // Return Time Zone config value
  return int8(EEPROM.read(CFG_TIME_ZONE));
}

boolean pwdNeeded() {
  // Return CFG_PWD_ALLWAYS_NEEDED config value
  return EEPROM.read(CFG_PWD_ALLWAYS_NEEDED);
}

boolean softAPOn() {
  // Return CFG_SOFTAP_ALLWAYS_ON config value
  return EEPROM.read(CFG_SOFTAP_ALLWAYS_ON);
}

boolean AgOn() {
  // Return CFG_AG_ALLWAYS_ON schedule config value
  return EEPROM.read(CFG_AG_ALLWAYS_ON);
}

String networkID() {
  // Return Network ID config parameter
  String sFn = "";
  char c;
  for (byte bFn = CFG_NETWORK_ID; bFn < CFG_TIME_ZONE; bFn++) {
    if (EEPROM.read(bFn) == 0) {
      break;
    }
    c = char(EEPROM.read(bFn));
    // Check for valid characters
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || c == '_') {
      sFn += c;
    }
  }
  if (sFn == "") {
    sFn = deviceID();
  }
  return sFn;
}

String reference() {
  // Return Reference config parameter
  String sFn = "";
  for (byte bFn = CFG_REFERENCE; bFn < CFG_SOFTAP_ALLWAYS_ON; bFn++) {
    if (EEPROM.read(bFn) == 0) {
      break;
    }
    sFn += char(EEPROM.read(bFn));
  }
  return sFn;
}

String wifiSSID() {
  // Return WiFi SSID config parameter
  String sFn = "";
  for (byte bFn = CFG_WIFI_SSID; bFn < CFG_WIFI_PWD; bFn++) {
    if (EEPROM.read(bFn) == 0) {
      break;
    }
    sFn += char(EEPROM.read(bFn));
  }
  return sFn;
}

String wifiPwd() {
  // Return WiFi password config parameter
  String sFn = "";
  for (byte bFn = CFG_WIFI_PWD; bFn < CFG_TOTAL_LENGHT; bFn++) {
    if (EEPROM.read(bFn) == 0) {
      break;
    }
    sFn += char(EEPROM.read(bFn));
  }
  return sFn;
}

void getSPIFFSBuild() {
  // Inicialize SPIFFS build variable
  File file = SPIFFS.open(F("/SPIFFSBuild.txt"), "r");
  if (file) {
    file.setTimeout(100);
    fsVersion = file.readString();
    file.close();
  } else {
    fsVersion = F("N/A");
  }
}

String ipStr(const IPAddress &ip) {
  // Return IPAddress as String "n.n.n.n"
  String sFn = "";
  for (byte bFn = 0; bFn < 3; bFn++) {
    sFn += String((ip >> (8 * bFn)) & 0xFF) + ".";
  }
  sFn += String(((ip >> 8 * 3)) & 0xFF);
  return sFn;
}

String wifiStatusStr() {
  // Return WiFi Status as a readable String
  String s;
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
      s = F("Ocioso");
      break;
    case WL_NO_SSID_AVAIL:
      s = F("SSID indisponível");
      break;
    case WL_SCAN_COMPLETED:
      s = F("Scan concluído");
      break;
    case WL_CONNECTED:
      s = F("Conectado");
      break;
    case WL_CONNECT_FAILED:
      s = F("Falha de conexão");
      break;
    case WL_CONNECTION_LOST:
      s = F("Conexão perdida");
      break;
    case WL_DISCONNECTED:
      s = F("Desconectado");
      WiFi.reconnect();
      break;
    default:
      s = F("N/D");
      break;
  }
  return s;
}

boolean chkWebAuth() {
  // Check for Web Interface credencials
  if (server.authenticate(WWW_USERNAME, PASSWORD)) {
    // Authenticated
    return true;
  } else {
    // Need Username and Password
    log(F("WebAut"), F("ERRO falha na autenticacao"));
    server.sendHeader(F("WWW-Authenticate"), F("Basic realm=\"ESPRele Web Interface\""));
    server.send(401);
    return false;
  }
}

void sortArray(String a[], String &s) {
  // Sort a String Array
  boolean flFn = true;
  String sFn;
  while (flFn) {
    flFn = false;
    byte bFn = 0;
    while(a[bFn + 1] != "") {
      if(a[bFn] > a[bFn + 1]) {
        sFn = a[bFn + 1];
        a[bFn + 1] = a[bFn];
        a[bFn] = sFn;
        flFn = true;
      }
      bFn++;
    }
    bFn = 0;
    s = "";
    while (a[bFn] != "") {
      s += a[bFn];
      bFn++;
    }
  }
}

String fsSpaceStr() {
  // Return information about FileSystem space
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  return "Total: " +
          String((fs_info.totalBytes) / 1024.0, 1) + F("kb<br>") +
          "Usado: " +
          String(fs_info.usedBytes / 1024.0, 1) + F("kb<br>") +
          "Disponível: " +
          String((fs_info.totalBytes - fs_info.usedBytes) / 1024.0, 1) + F("kb<br>");
}

time_t timeNTP() {
  // Return time_t from NTP Server
  if (wifiStatus != WL_CONNECTED) {
    // No WiFi connection
    return 0;
  }

  // NTP Server
  const char  NTP_SERVER[]    = "pool.ntp.br";
  // NTP Packet Size
  const byte  NTP_PACKET_SIZE = 48;
  // UDP Port
  const int   UDP_LOCALPORT   = 2390;
  // NTP Packet
  byte        ntp[NTP_PACKET_SIZE];

  memset(ntp, 0, NTP_PACKET_SIZE);
  ntp[ 0] = 0b11100011; // LI, Version, Mode
  ntp[ 1] = 0;          // Stratum, or type of clock
  ntp[ 2] = 6;          // Polling Interval
  ntp[ 3] = 0xEC;       // Peer Clock Precision
  ntp[12] = 49;
  ntp[13] = 0x4E;
  ntp[14] = 49;
  ntp[15] = 52;
  // Get time from server
  WiFiUDP udp;
  udp.begin(UDP_LOCALPORT);
  udp.beginPacket(NTP_SERVER, 123);
  udp.write(ntp, NTP_PACKET_SIZE);
  udp.endPacket();
  hold(1000);
  unsigned long l;
  if (udp.parsePacket()) {
    // Success
    udp.read(ntp, NTP_PACKET_SIZE);
    l = word(ntp[40], ntp[41]) << 16 | word(ntp[42], ntp[43]);
    l -= 2208988800UL;      // Calculate from 1900 to 1970
    l += timeZone() * 3600; // Adjust time zone (+- timeZone * 60m * 60s)
    logStr[logIndex] = dateTimeStr(l) + F(";NTP;Ok");
  } else {
    //Error
    logStr[logIndex] = dateTimeStr(1) + F(";NTP;ERRO");
    l = 0;
  }
  // Simulate Log
  Serial.println(logStr[logIndex]);
  logIndex = (logIndex + 1) % LOG_ENTRIES;
  // Return DateTime
  return l;
}

String dateTimeISO8601(const time_t &t) {
  // Return time_t as ISO8601 Date/Time String
  String sFn = dateTimeStr(t, false);
  sFn.replace(" ", "T");
  sFn = sFn                            +
        ((timeZone() < 0) ? "-" : "+") +
        ((timeZone() < 10) ? "0" : "") +
        String(abs(timeZone()))        +
        ":00";
  return sFn;
}  

void logDelete() {
  // Delete log in memory
  for (byte bFn = 0; bFn < LOG_ENTRIES; bFn++) {
    logStr[bFn] = "";
  }
  logIndex = 0;
}

void logFileDelete() {
  // Delete log in files
  for (byte bFn = 1; bFn <= 7; bFn++) {
    SPIFFS.remove("/Log/Dia" + String(bFn) + F(".csv"));
  }
}

byte logDay() {
  // Return last log file day
  byte b = EEPROM.read(CFG_LOG_DAY);
  return b < 1 || b > 7 ? 1 : b;
}

void logDay(const byte &d) {
  // Set the current log file day
  EEPROM.write(CFG_LOG_DAY, d);
  EEPROM.commit();
}

void reboot(const String msg = "Reiniciando...") {
  // Reboot device
  Serial.println(msg);
  hold(2000);
  ESP.restart();
}

void logFile(const String &type, const String &msg, const boolean flReboot = false) {
  // Generate log in file and display error code if any
  File fileFn;
  String sFn = dateTimeStr(now()) + ";" + type + ";" + msg;
  // Write to log file
  if (logDay() != weekday() && timeStatus() != timeNotSet) {
    // New day, new log file
    logDay(weekday());
    fileFn = SPIFFS.open("/Log/Dia" + String(logDay()) + F(".csv"), "w+");
    if (!fileFn) {
      // Could not create file, delete files e reboot
      logFileDelete();
      reboot();
    }
    // Write header
    fileFn.println(deviceID() + F(" - Arquivo de Log Dia ") + String(logDay()));
    fileFn.println(F("Data/Hora;Tipo;Mensagem"));
  } else {
    // Open current log file
    fileFn = SPIFFS.open("/Log/Dia" + String(logDay()) + F(".csv"), "a");
    if (!fileFn) {
      // Could not open file, delete files e reboot
      logFileDelete();
      reboot();
    }
  }
  fileFn.println(sFn);
  fileFn.close();
  // Write to serial
  Serial.println(sFn);
  // Check for reboot
  if (flReboot) {
    reboot();
  }
}

String configStr() {
  // Return Config parameters
  return "ID="      + networkID()               +
          " Ref="   + reference()               +
          " Fuso="  + String(timeZone())        +
          " Aut="   + (pwdNeeded() ? "S" : "N") +
          " AP="    + (softAPOn()  ? "S" : "N") +
          " SSID="  + wifiSSID()                +
          " Ag="    + (AgOn()      ? "S" : "N") +
          " Log="   + String(logDay())          +
          " "       + configLastSavedStr();
}


/*******************************************************************************
* SCHEDULE SYSTEM FUNCTIONS
* 10/2018 - Andre Michelon
* Options:
*   - Scheduled at a specific Date/Time
*     On (High): SHyyyy-mm-dd hh:mm
*     Off (Low): SLyyyy-mm-dd hh:mm
*   - Monthly
*     On (High): MHdd hh:mm
*     Off (Low): MLdd hh:mm
*   - Weekly
*     On (High): WHd hh:mm 1-7
*     Off (Low): WLd hh:mm
*   - Daily
*     On (High): DHhh:mm
*     Off (Low): DLhh:mm
*   - Intervaled
*     On (High): IHhh:mm
*     Off (Low): ILhh:mm
* Example Strings:
*   SH2018-10-12 16:30  - set On on Oct 12 2018 16:30
*   MH12 16:30          - set On monthly on day 12 16:30
*   WL6 16:30           - set Off weekly on Fridays 16:30 
*   DH16:30             - set On daily at 16:30
*   IH00:30             - set Off after being On for 30 minutes
*   IL00:10             - set On after being Off for 10 minutes
*******************************************************************************/
String hhmmStr(const time_t &t) {
  // Return time_t as String "hh:mm"
  String s = "";
  if (hour(t) < 10) {
    s += '0';
  }
  s += String(hour(t)) + ':';
  if (minute(t) < 10) {
    s += '0';
  }
  s += String(minute(t));
  return s;
}

String scheduleChk(const String &schedule, const byte &pin) {
  // Schedule System Main Function

  // Local variables
  String        event     = "";
  byte          relay;
  static time_t lastCheck = 0,
                highDT    = now(),
                lowDT     = highDT;

  // Check for Interval reset - limpa as regras se a caixa estiver vazia
 
  if (schedule == "") {
    highDT = now(),
    lowDT = highDT;
    return "";
  }

  // Check for minimum interval (10s)
  if (now() - lastCheck < 10) {
    return "";
  }

  // Get DateTime
  lastCheck = now();

  // Get DateTime as "yyyy-mm-dd hh:mm" String
  String dt = String(year(lastCheck)) + '-';
  if (month(lastCheck) < 10) {
    dt += '0';
  }
  dt += String(month(lastCheck)) + '-';
  if (day(lastCheck) < 10) {
    dt += '0';
  }
  dt += String(day(lastCheck)) + ' ';
  if (hour(lastCheck) < 10) {
    dt += '0';
  }
  dt += String(hour(lastCheck)) + ':';
  if (minute(lastCheck) < 10) {
    dt += '0';
  }
  dt += String(minute(lastCheck));

  // Check Scheduled High - SHyyyy-mm-dd hh:mm
  String s = "SH" + dt;
  if (schedule.indexOf(s) != -1) {
    event = s;
    relay = HIGH;
    goto process;
  }

  // Check Scheduled Low - SLyyyy-mm-dd hh:mm
  s = "SL" + dt;
  if (schedule.indexOf(s) != -1) {
    event = s;
    relay = LOW;
    goto process;
  }

  // Get DateTime as "dd hh:mm" String
  dt = dt.substring(8);

  // Check Monthly High - MHdd hh:mm
  s = "MH" + dt;
  if (schedule.indexOf(s) != -1) {
    event = s;
    relay = HIGH;
    goto process;
  }

  // Check Monthly Low - MLdd hh:mm
  s = "ML" + dt;
  if (schedule.indexOf(s) != -1) {
    event = s;
    relay = LOW;
    goto process;
  }

  // Get DateTime as "d hh:mm" String
  dt = String(weekday(lastCheck)) + dt.substring(2);
  
  // Check Weekly High - WHd hh:mm
  s = "WH" + dt;
  if (schedule.indexOf(s) != -1) {
    event = s;
    relay = HIGH;
    goto process;
  }

  // Check Weekly Low - WLd hh:mm
  s = "WL" + dt;
  if (schedule.indexOf(s) != -1) {
    event = s;
    relay = LOW;
    goto process;
  }

  // Get DateTime as "hh:mm" String
  dt = dt.substring(2);

  // Check Daily High - DHhh:mm
  s = "DH" + dt;
  if (schedule.indexOf(s) != -1) {
    event = s;
    relay = HIGH;
    goto process;
  }

  // Check Daily Low - DLhh:mm
  s = "DL" + dt;
  if (schedule.indexOf(s) != -1) {
    event = s;
    relay = LOW;
    goto process;
  }

  // Check Intervaled High - IHhh:mm
  s = "IH" + hhmmStr(lastCheck - highDT);

  if (schedule.indexOf(s) != -1 && digitalRead(pin)) {
    event = s;
    relay = LOW;
    goto process;
  }

  // Check Intervaled Low - IDhh:mm
  s = "IL" + hhmmStr(lastCheck - lowDT);

  if (schedule.indexOf(s) != -1 && !digitalRead(pin)) {
    event = s;
    relay = HIGH;
  }

  process:  // Process event
  if (event != "" && relay != digitalRead(pin)) {
    digitalWrite(pin, relay);
    if (relay) {
      // Store HIGH DateTime
      highDT = lastCheck;
    } else {
      // Store LOW DateTime
      lowDT = lastCheck;
    }
    return event;
  }
  return "";
}

boolean scheduleSet(const String &schedule) {
  // Save Schedule entries
  File file = SPIFFS.open(F("/Schedule.txt"), "w+");
  if (file) {
    file.print(schedule);
    file.close();
    scheduleChk("", 0);
    return true;
  }
  return false;
}

String scheduleGet() {
  // Get Schedule entries
  String s = "";
  File file = SPIFFS.open(F("/Schedule.txt"), "r");
  if (file) {
    file.setTimeout(100);
    s = file.readString();
    file.close();
  }
  return s;
}

