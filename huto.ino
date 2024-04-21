// Ez a 192.168.4.1 ip címet meghívva egy böngészővel bekéri a router SSID-t ill jelszót, melj felmegy rá és kijut az internetre
// KAp egy IP-t a routertől majd a böngészőből az IP/tempereture ra kiadja a hőmérsékletet.
// a mért értékek nem biztos hogy jók azt még meg kell vizsgálni
// Az A0-ás lábra van kötve az LM35 hőmérő
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

const char* ssid = "ESP8266_AP"; // Az ESP8266 hálózati neve
const char* password = "password"; // Az ESP8266 hálózati jelszava
IPAddress apIP(192, 168, 4, 1); // Az ESP8266 IP címe hálózati módú üzemben

ESP8266WebServer server(80);

const int LM35Pin = A0; // Az LM35 OUT lába az A0-as analóg bemenetre van csatlakoztatva

void setup() {
  Serial.begin(115200);1

  // ESP8266 hálózati módba állítása
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  SPIFFS.begin();

  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/index.html", "r");
    size_t sent = server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/connect", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    // Wi-Fi hálózatra csatlakozás
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // Sikeres csatlakozás esetén visszairányítás
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Connected to Wi-Fi!");
  });

  server.on("/temperature", HTTP_GET, []() {
    int rawValue = analogRead(LM35Pin);
    float voltage = (rawValue / 1023.0) * 5.0;
    float temperature = voltage * 100;
    server.send(200, "text/plain", String(temperature));
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
