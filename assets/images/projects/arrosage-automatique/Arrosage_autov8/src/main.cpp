//include lib
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <NTPClient.h>

//INTERNET
const char *ssid = "Freebox-A172AE";
const char *password = "ostendendo!-capenas?-saltastis-ariste#7";

//Function declarations
int relay(int,bool);// int (numero du relay 0-5) , bool (etat du relay true/false Alumer/Eteint)
int ModeManuTime(int,int);// int (numero du relay 0-5) , int (temps d'activation 1-60)
int ModeAuto(int,int);// int (numero du relay 0-5) , int (heure d'activation en minute)
int Stop(int,bool);
int RefreshTime();
int saveState();
int loadState();
int Pompe(bool);
int Stop_all();

int leds[2]={22,23};

int pin_relay[5] = {32,33,25,26,27};
bool state_relay[5] = {false,false,false,false,false};
bool stop_relay[5] = {false,false,false,false,false};

int pin_pompe = 14;
bool state_pompe = false;


AsyncWebServer server(80);

#define UTC_OFFSET_IN_SECONDS 3600*2   
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", UTC_OFFSET_IN_SECONDS);
int realHours = 0;
int realMinutes = 0;

int ManuTime = 1;
bool startmanuT[5] = {false,false,false,false,false};

int AutoStartTime[5] = {0,0,0,0,0};
int AutoTime[5] = {1,1,1,1,1};
bool startauto[5] = {false,false,false,false,false};


unsigned long rebootInterval = 6UL * 60UL * 60UL * 1000UL; // toutes les 6 heures
unsigned long lastReboot = millis();


void setup() {
  //----------------------------------------------------Serial
  Serial.begin(115200);
  Serial.println("\n");

  //----------------------------------------------------GPIO
  for (int i = 0; i < 5; i++) {
    pinMode(pin_relay[i], OUTPUT);
    digitalWrite(pin_relay[i], false);
  }

  pinMode(pin_pompe, OUTPUT);
  digitalWrite(pin_pompe, false);

  pinMode(leds[0], OUTPUT);
  pinMode(leds[1], OUTPUT);
  digitalWrite(leds[0], true);
  delay(1500);
  digitalWrite(leds[0], false);

  
  //----------------------------------------------------SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while (file) {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }

  loadState();

  //----------------------------------------------------WIFI
  WiFi.begin(ssid, password);
  Serial.print("Tentative de connexion...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  
  Serial.println("\n");
  Serial.println("Connexion établie!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());

  //----------------------------------------------------NTP
  timeClient.begin();
  timeClient.update();
  realHours = timeClient.getHours();
  realMinutes = timeClient.getMinutes();
  Serial.print(realHours);
  Serial.print(":");
  Serial.print(realMinutes);
  Serial.print(":");
  Serial.println(timeClient.getSeconds());

  //----------------------------------------------------SERVER
  
  //____________________________________________________Envoit la page web et ses composants
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/icone.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/icone.jpg", "image/png");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  //___________________________________________________Interation avec les Bouton de la page 

  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request) {
    String valve = request->getParam("valve")->value();
    String action = request->getParam("action")->value();
    int num = valve.toInt() - 1;
    if (num == 5){
      if (action == "on") {
        Pompe(true);
      } 
      else if (action == "off") {
        Stop_all();
      }
    }
    else{
      if (action == "on") {
      relay(num,true);
      Stop(num,false);
      Pompe(true);
      } 
      else if (action == "off") {
        relay(num,false);
        Stop(num,true);
        Pompe(false);
      }
    }

    digitalWrite(leds[1], true);
    delay(750);
    digitalWrite(leds[1], false);
    request->send(200, "text/plain", "OK");
  });

  server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request) {
    String valve = request->getParam("valve")->value();
    String time = request->getParam("time")->value();
    int num = valve.toInt() - 1;
    ManuTime = time.toInt();

    startmanuT[num] = true;
    bool list[5]={false,false,false,false,false};
    for (int i = 0; i < 5; i++) {
      state_relay[i]=list[i];
      digitalWrite(pin_relay[i], list[i]);
      Stop(i,true);
    }
    Stop(num,false);
    Serial.println("Start");
    Serial.print("Van numéro : ");
    Serial.print(num+1);

    request->send(200, "text/plain", "OK");
  });

  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    String valve = request->getParam("valve")->value();
    String start = request->getParam("start")->value();
    String duration = request->getParam("duration")->value();
    
    int num= valve.toInt() - 1;
    int startHour = start.substring(0, 2).toInt();
    int startMinute = start.substring(3, 5).toInt();
    AutoTime[num]= duration.toInt();

    
    if (startauto[num]) {
      startauto[num] = false;
      Serial.println("Mode Auto Désactivé");
    } else {
      startauto[num] = true;
      Serial.println("Mode Auto Activé");
      AutoStartTime[num] = startHour * 60 + startMinute;
      Serial.print("A :");
      Serial.print(startHour);
      Serial.print(":");
      Serial.print(startMinute);
      
      Serial.print(" Pour : ");
      Serial.print(AutoTime[num]);
      Serial.println(" minutes");

      Serial.print("Van numéro : ");
      Serial.println(num+1);
    }

    saveState();
    //loadState();


    
    digitalWrite(leds[1], true);
    delay(750);
    digitalWrite(leds[1], false);
    request->send(200, "text/plain", "OK");
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "";
    // Ajouter l'état des relais (électrovannes)
    for (int i = 0; i < 5; i++) {
        response += String(state_relay[i]) + ","; // 0 ou 1 pour état OFF/ON
    }
    // Ajouter l'état du mode automatique
    for (int i = 0; i < 5; i++) {
        response += String(startauto[i] ? 1 : 0) + ","; // 0 ou 1 pour mode Auto OFF/ON
    }
    response += String(state_pompe) + ",";

    request->send(200, "text/plain", response);
  });

  server.on("/InfoModeAuto", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "";
    for (int i = 0; i < 5; i++) {
        response += String(AutoStartTime[i]) + ",";
    }
    for (int i = 0; i < 5; i++) {
        response += String(AutoTime[i]) + ",";
    }
    request->send(200, "text/plain", response);

  });

  server.begin();
  Serial.println("Serveur actif!");
}

//----------------------------------------------------FONCTION
void printWiFiDebugInfo() {
  Serial.println("=== INFOS DEBUG WIFI ===");
  Serial.print("SSID : "); Serial.println(WiFi.SSID());
  Serial.print("IP locale : "); Serial.println(WiFi.localIP());
  Serial.print("Force du signal (RSSI) : "); Serial.println(WiFi.RSSI());
  Serial.print("Etat de la connexion : ");
  switch (WiFi.status()) {
    case WL_IDLE_STATUS: Serial.println("IDLE"); break;
    case WL_NO_SSID_AVAIL: Serial.println("SSID non disponible"); break;
    case WL_SCAN_COMPLETED: Serial.println("Scan terminé"); break;
    case WL_CONNECTED: Serial.println("Connecté"); break;
    case WL_CONNECT_FAILED: Serial.println("Échec de connexion"); break;
    case WL_CONNECTION_LOST: Serial.println("Connexion perdue"); break;
    case WL_DISCONNECTED: Serial.println("Déconnecté"); break;
    default: Serial.println("Inconnu");
  }
  Serial.println("========================");
}


void checkWiFiReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi déconnecté ! Tentative de reconnexion...");
    WiFi.disconnect();// Déconnexion au cas où l'état serait instable
    ESP.restart();  
    WiFi.begin(ssid, password);

    int essais = 0;
    while (WiFi.status() != WL_CONNECTED && essais < 20) {
      delay(500);
      Serial.print(".");
      essais++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconnexion réussie !");
      Serial.print("IP : ");
      Serial.println(WiFi.localIP());
      printWiFiDebugInfo();
    } else {
      Serial.println("\nÉchec de reconnexion WiFi.");
    }
  }
}

int relay(int num ,bool state ){
  bool list[5]={false,false,false,false,false};
  list[num]=state_relay[num];
  for (int i = 0; i < 5; i++) {
    state_relay[i]=list[i];
    digitalWrite(pin_relay[i], list[i]);
  }
  
  digitalWrite(pin_relay[num], state);
  state_relay[num]=state;
  Serial.print("Relay numéro : ");
  Serial.print(num);
  Serial.print(" --> ");
  Serial.println(state);
  return 0;
}

int Stop_all(){
  bool list[5]={false,false,false,false,false};
  for (int i = 0; i < 5; i++) {
    state_relay[i]=list[i];
    digitalWrite(pin_relay[i], list[i]);
    Stop(i,true);
  }
  Pompe(false);
  return 0;
}

int Stop(int num,bool state){
  stop_relay[num]=state;
  Serial.print("Relay numéro : ");
  Serial.print(num);
  Serial.print(" Stop = ");
  Serial.println(state);
  return 0;
}

int ModeManuTime(int num,int ManuTime) {
  if (startmanuT[num]) {
    int GoTime = realHours * 60 + realMinutes;
    Serial.print("Start manu time : ");
    Serial.println(ManuTime);
    while (ManuTime + GoTime >= realHours * 60 + realMinutes) {
      RefreshTime();
      
      if (stop_relay[num]==true) {
        break;
      }
      relay(num,true);
      Pompe(true);
    }
    Pompe(false);
    relay(num,false);
    
    startmanuT[num] = false;
  }
  return 0;
}

int ModeAuto(int num, int AutoStartTime) {
  bool EndModeAuto = true;
  if (startauto[num]) {
    while ((realHours * 60 + realMinutes > AutoStartTime) && (realHours * 60 + realMinutes < AutoTime[num] + AutoStartTime)) {
      RefreshTime();
      EndModeAuto = false;
      if (stop_relay[num]==true) {
        startauto[num] = false;
        break;
      }
      relay(num,true);
      Pompe(true);
    }
    if (EndModeAuto==false){
      relay(num,false);
      Pompe(false);
      EndModeAuto=true;
    }

  }
  return 0;
}


int RefreshTime(){
  realHours = timeClient.getHours();
  realMinutes = timeClient.getMinutes();
  delay(50);
  return 0;
}

int saveState() {
  File file = SPIFFS.open("/InfoModeAuto.txt", FILE_WRITE);
  if (!file) {
    Serial.println("Erreur d'ouverture du fichier pour la sauvegarde de l'état");
    return -1;  // Retourne une erreur si le fichier ne peut pas être ouvert
  }

  // Sauvegarder AutoStartTime
  for (int i = 0; i < 5; i++) {
    file.print(AutoStartTime[i]);
    if (i < 5) {
      file.print(",");  // Ajouter une virgule après chaque valeur sauf la dernière
    }
    Serial.print("---AutoStartTime[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(AutoStartTime[i]);
  }
  
  file.print(",");  // Séparer les sections avec une virgule

  // Sauvegarder AutoTime
  for (int i = 0; i < 5; i++) {
    file.print(AutoTime[i]);
    if (i < 5) {
      file.print(",");
    }
    Serial.print("---AutoTime[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(AutoTime[i]);
  }

  file.print(",");  // Séparer les sections avec une virgule

  // Sauvegarder startauto
  for (int i = 0; i < 5; i++) {
    file.print(startauto[i] ? 1 : 0);  // Convertir le booléen en 1 ou 0
    if (i < 5) {
      file.print(",");
    }
    Serial.print("---startauto[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(startauto[i] ? 1 : 0);
  }

  file.close();
  Serial.println("État sauvegardé");
  return 0;
}



int loadState() {
  File file = SPIFFS.open("/InfoModeAuto.txt", FILE_READ);
  if (!file) {
    Serial.println("Erreur d'ouverture du fichier pour la lecture de l'état");
    return -1;  // Retourne une erreur si le fichier n'est pas trouvé
  }

  String content = file.readString();
  file.close();
  
  Serial.println("Contenu du fichier : " + content);

  int startIndex = 0;
  int commaIndex = 0;
  
  // Lire AutoStartTime
  for (int i = 0; i < 5; i++) {
    commaIndex = content.indexOf(',', startIndex);
    AutoStartTime[i] = content.substring(startIndex, commaIndex).toInt();
    startIndex = commaIndex + 1;
    Serial.print("AutoStartTime[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(AutoStartTime[i]);
  }

  // Lire AutoTime
  for (int i = 0; i < 5; i++) {
    commaIndex = content.indexOf(',', startIndex);
    AutoTime[i] = content.substring(startIndex, commaIndex).toInt();
    startIndex = commaIndex + 1;
    Serial.print("AutoTime[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(AutoTime[i]);
  }

  // Lire startauto
  for (int i = 0; i < 5; i++) {
    commaIndex = content.indexOf(',', startIndex);
    startauto[i] = content.substring(startIndex, commaIndex).toInt() == 1;
    startIndex = commaIndex + 1;
    Serial.print("startauto[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(startauto[i]);
  }

  Serial.println("État chargé");
  return 0;
}

int Pompe(bool state){
  state_pompe=state;
  //delay(50);
  digitalWrite(pin_pompe,state);
  return 0;
}

void loop() {
  
  for (int i = 0; i<5; i++){
  ModeManuTime(i,ManuTime);
  }
  
  /*
  for (int i = 0; i<5; i++){
  ModeAuto(i,AutoStartTime[i]);
  }*/
  RefreshTime();
  checkWiFiReconnect();


  if (millis() - lastReboot > rebootInterval) {
    Serial.println("Redémarrage périodique...");
    ESP.restart();
  }

}  
