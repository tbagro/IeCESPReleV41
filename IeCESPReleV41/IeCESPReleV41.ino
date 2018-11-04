/*******************************************************************************
**  CANAL INTERNET E COISAS                                                   **
**  IeCESPRele V4.0 Library - Generic Functions                               **
**  10/2018 - Andre Michelon                                                  **
**  Adaptado por Tiago Batista                                                **
*******************************************************************************/

/*******************************************************************************
  AUXILIARY FILES
*******************************************************************************/
#include "IeCESPReleV4Lib.h"

/*******************************************************************************
  AUXILIARY FUNCTIONS
*******************************************************************************/
String softwareStr() {
  // Return source file name as String
  return String(__FILE__).substring(String(__FILE__).lastIndexOf("\\") + 1);
}

/*******************************************************************************
  WEB REQUESTS HANDLING
*******************************************************************************/
void handleHome() {
  // Home
  if (!pwdNeeded() || chkWebAuth()) {
    File file = SPIFFS.open(F("/Home.htm"), "r");
    if (file) {
      file.setTimeout(100);
      String s = file.readString();
      file.close();
      // Replace markers

      s.replace(F("#networkID#"), networkID());
      s.replace(F("#reference#"), reference());
      s.replace(F("#serial#")   , hexStr(ESP.getChipId()));
      s.replace(F("#software#") , softwareStr());
      s.replace(F("#swVersion#"), SW_VERSION);
      s.replace(F("#swBuild#")  , BUILD);
      s.replace(F("#fsVersion#"), fsVersion.substring(21));
      s.replace(F("#fsBuild#")  , fsVersion.substring(0, 20));
      s.replace(F("#sysIP#")    , ipStr(WiFi.localIP()));
      s.replace(F("#clientIP#") , ipStr(server.client().remoteIP()));

      s.replace(F("#bootDT#")   , dateTimeStr(bootDT));
      s.replace(F("#activeDT#") , activeTimeStr(now() - bootDT));
      s.replace(F("#userAgent#"), server.header(F("User-Agent")));
      // Send data
      server.send(200, F("text/html"), s);
      log(F("WebHome"), "Cliente: " + ipStr(server.client().remoteIP()) +
          (server.uri() != "/" ? " [" + server.uri() + "]" : ""));
    } else {
      server.send(500, F("text/plain"), F("Home - ERROR 500"));
      logFile(F("WebHome"), F("ERRO lendo arquivo"), true);
    }
  }
}

void handleRelay() {
  // Relay
  if (!pwdNeeded() || chkWebAuth()) {
    File file = SPIFFS.open(F("/Relay.htm"), "r");
    if (file) {
      file.setTimeout(100);
      String s = file.readString();
      file.close();
      // Replace markers
      s.replace("#reference#" , reference());
      // Send data
      server.send(200, F("text/html"), s);
      log(F("WebRelay"), "Cliente: " + ipStr(server.client().remoteIP()));
    } else {
      server.send(500, F("text/plain"), F("Relay - ERROR 500"));
      logFile(F("WebRelay"), F("ERRO lendo arquivo"), true);
    }
  }
}

void handleRelayStatus() {
  // Relay status
  if (!pwdNeeded() || chkWebAuth()) {
    String s = String(digitalRead(RELAY_PIN)) + "&" +
               dateTimeStr(now())            + "&" +
               lastEvent;
    server.send(200, F("text/plain"), s);
    log(F("WebRelayStatus"), "Cliente: " + ipStr(server.client().remoteIP()) +
        " [" + s + "]");
  }
}

void handleRelaySet() {
  // Set Relay status
  if (!pwdNeeded() || chkWebAuth()) {
    String s = server.arg("set");
    if (s == "1") {
      // Set relay on
      digitalWrite(RELAY_PIN, HIGH);
      lastEvent = "Ligado - " + dateTimeStr(now());
    } else if (s == "0") {
      // Set relay off
      digitalWrite(RELAY_PIN, LOW);
      lastEvent = "Desligado - " + dateTimeStr(now());
    }
    // Reset Schedule intervals
    scheduleChk("", 0);
    s = String(digitalRead(RELAY_PIN))  + "&" +
        dateTimeStr(now())              + "&" +
        lastEvent;
    server.send(200, F("text/plain"), s);
    log(F("WebRelaySet"), "Cliente: " + ipStr(server.client().remoteIP()) +
        " [" + s + "]");
  }
}

void handleFileList() {
  // File list
  if (!pwdNeeded() || chkWebAuth()) {
    File file = SPIFFS.open(F("/FileList.htm"), "r");
    if (file) {
      file.setTimeout(100);
      String  s = file.readString(),
              sort = "",
              files[DIRECTORY_MAX_FILES];
      file.close();
      Dir dir = SPIFFS.openDir("/");
      byte b = 0;
      while (dir.next()) {
        files[b] = "<li>" + dir.fileName() + " - " + String(dir.fileSize() / 1024.0, 2) + F("kb</li>");
        b++;
      }
      // Sort entries
      sortArray(files, sort);
      // Replace markers
      s.replace(F("#files#")   , "<ul>" + sort + F("</ul>"));
      s.replace(F("#fsSpace#") , fsSpaceStr());
      // Send data
      server.send(200, F("text/html"), s);
      log(F("WebFileList"), "Cliente: " + ipStr(server.client().remoteIP()));
    } else {
      server.send(500, F("text/plain"), F("FileList - ERROR 500"));
      logFile(F("WebFileList"), F("ERRO lendo arquivo"), true);
    }
  }
}

void handleConfig() {
  // Config
  if (chkWebAuth()) {
    File file = SPIFFS.open(F("/Config.htm"), "r");
    if (file) {
      file.setTimeout(100);
      String s = file.readString();
      file.close();
      // Replace markers
      s.replace(F("#configDT#") , configLastSavedStr());
      s.replace(F("#networkID#"), networkID());
      s.replace(F("#reference#"), reference());
      s.replace(F("#timeZone#") , String(timeZone()));
      s.replace(F("#aut#")      , (pwdNeeded() ? " checked" : ""));
      s.replace(F("#ap#")       , (softAPOn() ? " checked" : ""));
      s.replace(F("#ag#")       , (AgOn() ? " checked" : "")); // schedule
      s.replace(F("#ssid#")     , wifiSSID()); 
      s.replace(F("#sched#")    , schedule);
      // Send data
      server.send(200, F("text/html"), s);
      log(F("WebConfig"), "Cliente: " + ipStr(server.client().remoteIP()));
    } else {
      server.send(500, F("text/plain"), F("Config - ERROR 500"));
      logFile(F("WebConfig"), F("ERRO lendo arquivo"), true);
    }
  }
}

void handleConfigSave() {
  // Config Save
  if (chkWebAuth()) {
    // Check for number os fields received
    if (server.args() >= 5 && server.args() <= 7) {
      // Save CFG_NETWORK_ID
      byte b = 0;
      String s = server.arg("id");
      s.trim();
      s.replace(" ", "_");
      s.toLowerCase();
      if (s == "") {
        s = deviceID();
      }
      for (int i = CFG_NETWORK_ID; i < CFG_TIME_ZONE; i++) {
        EEPROM.write(i, s[b++]);
      }
      // Save CFG_REFERENCE
      b = 0;
      s = server.arg(F("reference"));
      s.trim();
      if (s == "") {
        s = deviceID();
      }
      for (int i = CFG_REFERENCE; i < CFG_PWD_ALLWAYS_NEEDED; i++) {
        EEPROM.write(i, s[b++]);
      }
      // Save CFG_TIME_ZONE
      EEPROM.write(CFG_TIME_ZONE,   server.arg(F("timeZone")).toInt());
      // Save CFG_PWD_ALLWAYS_NEEDED
      EEPROM.write(CFG_PWD_ALLWAYS_NEEDED, server.arg(F("aut")).toInt());
      // Save CFG_SOFTAP_ALLWAYS_ON
      EEPROM.write(CFG_SOFTAP_ALLWAYS_ON, server.arg(F("ap")).toInt());
      // Save CFG_AG_ALLWAYS_ON schedule
      EEPROM.write(CFG_AG_ALLWAYS_ON, server.arg(F("ag")).toInt());
      // Save CFG_WIFI_SSID
      b = 0;
      s = server.arg(F("ssid"));
      s.trim();
      for (int i = CFG_WIFI_SSID; i < CFG_WIFI_PWD; i++) {
        EEPROM.write(i, s[b++]);
      }
      // Save CFG_WIFI_PWD
      s = server.arg(F("pwd"));
      s.trim();
      if (s != "") {
        b = 0;
        for (int i = CFG_WIFI_PWD; i < CFG_NETWORK_ID; i++) {
          EEPROM.write(i, s[b++]);
        }
      }
      // Last save date/time
      unsigned long l = now();
      EEPROM.write(CFG_LAST_SAVED + 3, byte(l));
      l = l >> 8;
      EEPROM.write(CFG_LAST_SAVED + 2, byte(l));
      l = l >> 8;
      EEPROM.write(CFG_LAST_SAVED + 1, byte(l));
      l = l >> 8;
      EEPROM.write(CFG_LAST_SAVED    , byte(l));
      // Save to flash
      EEPROM.commit();
      // Save Schedule entries
      schedule = server.arg(F("sched"));
      schedule.trim();
      schedule.toUpperCase();
      scheduleSet(schedule);
      // Done
      server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Configuração salva.');window.location = 'config';</script></html>"));
      log(F("WebConfigSave"), "Cliente: " + ipStr(server.client().remoteIP()) +
          " [" + configStr() + "]");
      logFile(F("WebConfigSave"), "Cliente: " + ipStr(server.client().remoteIP()) +
              " [" + configStr() + "]");
    } else {
      server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Erro de parâmetros.');history.back()</script></html>"));
    }
  }
}

void handleReboot() {
  // Reboot
  if (chkWebAuth()) {
    File file = SPIFFS.open(F("/Reboot.htm"), "r");
    if (file) {
      server.streamFile(file, F("text/html"));
      file.close();
      logFile(F("WebReboot"), "Cliente: " + ipStr(server.client().remoteIP()), true);
    } else {
      server.send(500, F("text/plain"), F("Reboot - ERROR 500"));
      logFile(F("WebReboot"), F("ERRO lendo arquivo"), true);
    }
  }
}

void handleLog() {
  // Log
  String files[DIRECTORY_MAX_FILES];
  String f;
  if (chkWebAuth()) {
    File file = SPIFFS.open(F("/Log.htm"), "r");
    if (file) {
      file.setTimeout(100);
      String s = file.readString();
      file.close();
      Dir dir = SPIFFS.openDir(F("/Log/"));
      byte b = 0;
      while (dir.next()) {
        f = dir.fileName().substring(5);
        files[b] = "<li><a href='logFileGet?l=" + f.substring(3, 4) + "'>" + f + F("</a> - ") +
                   String(dir.fileSize() / 1024.0, 2) + F("kb ") +
                   (logDay() == f.substring(3, 4).toInt() ? "(A)" : "") + F("</li>");
        b++;
      }
      String sort;
      if (files[0] == "") {
        // No entries
        sort = F("<li><i>Nenhum arquivo</i></li>");
      } else {
        // Sort entries
        sortArray(files, sort);
      }
      // Replace markers
      s.replace(F("#logFiles#"), "<ul>" + sort + F("</ul>"));
      s.replace(F("#fsSpace#") , fsSpaceStr());
      // Send data
      server.send(200, F("text/html"), s);
      log(F("WebLog"), "Client: " + ipStr(server.client().remoteIP()));
    } else {
      server.send(500, F("text/plain"), F("LogList - ERROR 500"));
      logFile(F("WebLogList"), F("ERRO lendo arquivo"), true);
    }
  }
}

void handleLogGet() {
  // Memory Log download
  if (chkWebAuth()) {
    byte bFn;
    String s = deviceID() +
               F(" - Log em Memoria\r\nData/Hora;Tipo;Mensagem\r\n");
    for (bFn = logIndex; bFn < LOG_ENTRIES; bFn++) {
      if (logStr[bFn] != "") {
        s += logStr[bFn] + F("\r\n");
      }
    }
    for (bFn = 0; bFn < logIndex; bFn++) {
      if (logStr[bFn] != "") {
        s += logStr[bFn] + F("\r\n");
      }
    }
    server.sendHeader(F("Content-Disposition"), "filename=\"" +
                      deviceID() + F("LogMemoria.csv\""));
    server.send(200, F("text/csv"), s);
    log(F("WebLogGet"), "Client: " + ipStr(server.client().remoteIP()));
  }
}

void handleLogFileGet() {
  // File Log download
  if (chkWebAuth()) {
    String s = server.arg("l");
    if (s != "") {
      File file = SPIFFS.open("/Log/Dia" + s + F(".csv"), "r");
      if (file) {
        server.sendHeader(F("Content-Disposition"), "filename=\"" +
                          deviceID() + F("LogDia") + s + F(".csv\""));
        server.streamFile(file, "text/csv");
        file.close();
        log(F("WebLogFileGet"), "Client: " + ipStr(server.client().remoteIP()));
      } else {
        server.send(500, F("text/plain"), F("LogFileGet - ERROR 500"));
        log(F("WebLogFileGet"), F("ERRO lendo arquivo"));
      }
    } else {
      server.send(500, F("text/plain"), F("LogFileGet - ERROR Bad parameter 500"));
      log(F("WebLogFileGet"), F("ERRO parametro incorreto"));
    }
  }
}

void handleLogReset() {
  // Memory Log reset
  if (chkWebAuth()) {
    // Delete log
    logDelete();
    // Send data
    server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Log em Memória excluído.');window.location = 'log';</script></html>"));
    log(F("WebLogReset"), "Cliente: " + ipStr(server.client().remoteIP()));
    logFile(F("WebLogReset"), "Cliente: " + ipStr(server.client().remoteIP()));
  }
}

void handleLogFileReset() {
  // File Log reset
  if (chkWebAuth()) {
    // Delete log files
    logFileDelete();
    // Send data
    server.send(200, F("text/html"), F("<html><meta charset='UTF-8'><script>alert('Log em Arquivo excluído.');window.location = 'log';</script></html>"));
    log(F("WebLogFileReset"), "Cliente: " + ipStr(server.client().remoteIP()));
    logFile(F("WebLogFileReset"), "Cliente: " + ipStr(server.client().remoteIP()));
  }
}

void handleStatus() {
  // Status
  if (!pwdNeeded() || chkWebAuth()) {
    String s = "{\"dtStr\":\""      + dateTimeStr(now())              + F("\","   // Current datetime as readable string
               "\"dt\":\"")        + dateTimeISO8601(now())          + F("\","   // Current datetime as ISO8601 string
                   "\"relay\":")       + String(digitalRead(RELAY_PIN))  + F(","     // Relay status
                       "\"lastEvent\":\"") + lastEvent                       + F("\"}"); // Last event
    server.send(200, F("application/json"), s);
    log(F("WebStatus"), "Cliente: " + ipStr(server.client().remoteIP()) +
        " [" + s + "]");
  }
}



/*******************************************************************************
  SETUP
*******************************************************************************/
void setup() {
  // Serial
  Serial.begin(115200);

  // Slow down
  hold(1000);
  Serial.println();

  // Relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // File System begin
  if (!SPIFFS.begin()) {
    reboot(F("ERRO Inicializando SPIFFS"));
  }

  // Read config
  EEPROM.begin(CFG_TOTAL_LENGHT);

  // FileSystem Version
  getSPIFFSBuild();

  // Starting
  logFile(F("Boot"), F("Iniciando..."));
  logFile(F("Boot"), "Dispositivo: " + deviceID());
  logFile(F("Boot"), "ID de Rede: " + networkID());
  logFile(F("Boot"), "Referencia: " + reference());
  logFile(F("Boot"), "Serial: " + hexStr(ESP.getChipId()));
  logFile(F("Boot"), "Software: " + softwareStr());
  logFile(F("Boot"), "Versao Software: " + String(BUILD) + " " + String(SW_VERSION));
  logFile(F("Boot"), "Versao Arquivos: " + fsVersion);
  logFile(F("Boot"), "Configuracao: " + configStr());
  logFile(F("Boot"), "Causa de Boot: " + ESP.getResetReason());

  // WiFi - Access Point & Station
  logFile(F("Boot"), F("Iniciando WiFi"));
  WiFi.hostname(networkID().c_str());
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);
  WiFi.mode(softAPOn() ? WIFI_AP_STA : WIFI_STA);
  WiFi.begin(wifiSSID().c_str(), wifiPwd().c_str());

  logFile(F("Boot"), "Conectando WiFi " + wifiSSID());
  byte b = 0;
  while (wifiStatus == WL_DISCONNECTED && b < 60) {
    b++;
    Serial.print(".");
    hold(500);
    wifiStatus = WiFi.status();
  }
  Serial.println();

  if (wifiStatus == WL_CONNECTED) {
    // WiFi connected
    logFile(F("Boot"), "WiFi Ok Modo Station RSSI " + String(WiFi.RSSI()) + F(" IP ") + ipStr(WiFi.localIP()));
  }

  // Check for SoftAP mode
  if (softAPOn() || wifiStatus != WL_CONNECTED) {
    if (wifiStatus != WL_CONNECTED) {
      // No Wifi connection, enter SoftAP
      WiFi.mode(WIFI_AP);
    }
    WiFi.softAP(networkID().c_str(), PASSWORD);

    // Set flag
    softAPActive = true;

    // Enable DNS routing
    dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dnsServer.start(DNSSERVER_PORT, "*", WiFi.softAPIP());

    logFile(F("Boot"), F("WiFi Ok Modo SoftAP"));
  }
  logFile(F("Boot"), "MAC: " + WiFi.macAddress());

  // Date/Time
  if (wifiStatus == WL_CONNECTED) {
    // Station mode, get date/time
    logFile(F("Boot"), F("Definindo Data/Hora"));

    setSyncProvider(timeNTP);
    setSyncInterval(NTP_INT);

    if (timeStatus() != timeSet) {
      logFile(F("Boot"), F("Data/Hora ERRO"));
    }
  } else {
    // Soft AP mode, ignore date/time
    logFile(F("Boot"), F("Data/Hora nao definida"));
  }
  // Date/Time set
  logFile(F("Boot"), F("Data/Hora Ok"));

  // Register Boot
  bootDT = now();


  // mDNS
  if (!MDNS.begin(networkID().c_str())) {
    logFile(F("Boot"), F("mDNS ERRO"), true);
  }

  // WebServer
  server.on(F("/relay")       , handleRelay);
  server.on(F("/relayStatus") , handleRelayStatus);
  server.on(F("/relaySet")    , handleRelaySet);
  server.on(F("/fileList")    , handleFileList);
  server.on(F("/config")      , handleConfig);
  server.on(F("/configSave")  , handleConfigSave);
  server.on(F("/reboot")      , handleReboot);
  server.on(F("/log")         , handleLog);
  server.on(F("/logReset")    , handleLogReset);
  server.on(F("/logFileReset"), handleLogFileReset);
  server.on(F("/logGet")      , handleLogGet);
  server.on(F("/logFileGet")  , handleLogFileGet);
  server.on(F("/status")      , handleStatus);

  server.onNotFound(handleHome);
  server.collectHeaders(WEBSERVER_HEADER_KEYS, 1);
  server.begin();
  logFile(F("Boot"), F("Servidor Web Ok"));

  // Schedule entries
  schedule = scheduleGet();
  //  // SET SCHEDULE ENTRIES - DEBUG ONLY
  time_t t = now() + 61;
  //  schedule = "SH"   + dateTimeStr( t      , false).substring(0, 16) +
  //             "\nSL" + dateTimeStr( t + 60 , false).substring(0, 16) +
  //             "\nMH" + dateTimeStr( t + 120, false).substring(8, 16) +
  //             "\nML" + dateTimeStr( t + 180, false).substring(8, 16) +
  //             "\nWH" +     weekday( t + 240) + " " + dateTimeStr( t + 240, false).substring(11, 16) +
  //             "\nWL" +     weekday( t + 300) + " " + dateTimeStr( t + 300, false).substring(11, 16) +
  //             "\nDH" + dateTimeStr( t + 360, false).substring(11, 16) +
  //             "\nDL" + dateTimeStr( t + 420, false).substring(11, 16) +
  //             "\nIH00:01\nIL00:01";

  logFile(F("Boot"), F("Agendamento Ok"));

  // Ready
  logFile(F("Boot"), F("ESPRele pronto"));
}

/*******************************************************************************
  LOOP
*******************************************************************************/
void loop() {
  // Take care of WatchDog ---------------------------------------------------
  yield();

  // DNS Requests ------------------------------------------------------------
  if (softAPActive) {
    dnsServer.processNextRequest();
  }

  // Web Requests ------------------------------------------------------------
  server.handleClient();

  // WiFi Status -----------------------------------------------------------
  if (WiFi.status() != wifiStatus) {
    wifiStatus = WiFi.status();
    log(F("WiFi"), wifiStatusStr() + F(" RSSI ") + String(WiFi.RSSI()) + F(" IP ") + ipStr(WiFi.localIP()));
    if (wifiStatus == WL_CONNECTED) {
      // WiFi reconnected, check No-IP
    }
  }


  //falta uma função para ativar ou não o agendamento - colocar aqui
  if (CFG_AG_ALLWAYS_ON == true) {
    // Schedule Check
    s = scheduleChk(schedule, RELAY_PIN);

    if (s != "") {
      // Event detected
      lastEvent = (digitalRead(RELAY_PIN) ? "Ligado " : "Desligado ") +
                  s + " - " + dateTimeStr(now());
      log(F("Agendamento"), lastEvent);
    }
  }
}
