/* Park Easy ESP32 - Firebase RTDB diagnostic + gate controller
   ESP32 Arduino core 3.3.x, ArduinoJson 7, MFRC522, ESP32Servo.
   Prototype TLS only: setInsecure() skips certificate verification.
   Keep credentials private; do not upload this sketch with secrets to public GitHub.
   IMPORTANT: 5V ultrasonic ECHO -> level shifter/divider to 3.3V.
   Servos require separate regulated power and common ground.
*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
//#include <Wire.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

const char* WIFI_SSID = "Oppo A9 2020";
const char* WIFI_PASSWORD = "chricel21";
const char* FIREBASE_API_KEY = "AIzaSyD4KDS_I3LwaFb8J-lJRTv_pmcV3uT8yt8";
const char* DEVICE_EMAIL = "vjlontoc035@gmail.com";
const char* DEVICE_PASSWORD = "VJLontoc09";
const char* DB_URL = "https://park-easy-system-default-rtdb.asia-southeast1.firebasedatabase.app";

#define EXIT_RFID_RST 4
#define ENTRY_RFID_SS 5
#define ENTRY_TRIG 13
#define EXIT_TRIG 14
#define ENTRY_RFID_RST 16
#define EXIT_RFID_SS 17
#define RFID_SCK 18
#define RFID_MISO 19
#define I2C_SDA 21
#define I2C_SCL 22
#define RFID_MOSI 23
#define ENTRY_SERVO_PIN 25
#define EXIT_SERVO_PIN 26
#define IR_SENSOR_PIN 27
#define ENTRY_ECHO 34
#define EXIT_ECHO 35

MFRC522 entryRFID(ENTRY_RFID_SS, ENTRY_RFID_RST);
MFRC522 exitRFID(EXIT_RFID_SS, EXIT_RFID_RST);
Servo entryServo, exitServo;
String idToken;
uint32_t tokenAt = 0, tokenExpiresMs = 0;
uint32_t lastPoll = 0, lastHeartbeat = 0, lastAuthTry = 0;
const int CLOSED_ANGLE = 0, OPEN_ANGLE = 90;
const int DETECT_CM = 100;
const uint32_t OPEN_MS = 5000;
struct Gate { String mode = "auto"; String requested = "Closed"; bool open = false; uint32_t openedAt = 0; };
Gate entryGate, exitGate;

bool authenticate() {
  if (WiFi.status() != WL_CONNECTED) return false;
  lastAuthTry = millis();
  WiFiClientSecure client;
  client.setInsecure(); // diagnostic prototype only
  client.setTimeout(12000);
  HTTPClient http;
  String url = String("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=") + FIREBASE_API_KEY;
  Serial.println("Connecting to Firebase Authentication...");
  if (!http.begin(client, url)) {
    Serial.println("Auth HTTP begin() failed");
    return false;
  }
  http.setConnectTimeout(12000);
  http.setTimeout(12000);
  http.addHeader("Content-Type", "application/json");
  JsonDocument request;
  request["email"] = DEVICE_EMAIL;
  request["password"] = DEVICE_PASSWORD;
  request["returnSecureToken"] = true;
  String payload;
  serializeJson(request, payload);
  int status = http.POST(payload);
  String response = status > 0 ? http.getString() : "";
  if (status <= 0) {
    Serial.printf("Firebase Auth transport error %d: %s\n", status, http.errorToString(status).c_str());
    Serial.println("Check DNS, HTTPS reachability, WiFi internet and ESP32 TLS handshake.");
  } else if (status != 200) {
    // Only print Firebase's error message; do not print tokens or credentials.
    JsonDocument err;
    if (!deserializeJson(err, response)) {
      Serial.printf("Firebase Auth HTTP %d: %s\n", status, err["error"]["message"] | "unknown error");
    } else {
      Serial.printf("Firebase Auth HTTP %d (unparseable error body)\n", status);
    }
  }
  http.end();
  if (status != 200) return false;
  JsonDocument doc;
  if (deserializeJson(doc, response)) {
    Serial.println("Firebase Auth JSON parsing failed");
    return false;
  }
  idToken = doc["idToken"].as<String>();
  uint32_t expires = doc["expiresIn"].as<String>().toInt();
  if (expires == 0) expires = 3600;
  tokenAt = millis();
  tokenExpiresMs = (expires > 120 ? expires - 60 : 60) * 1000UL;
  if (idToken.isEmpty()) return false;
  Serial.println("Firebase authenticated successfully!");
  return true;
}

bool ensureAuth() {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (!idToken.isEmpty() && (uint32_t)(millis() - tokenAt) < tokenExpiresMs) return true;
  if ((uint32_t)(millis() - lastAuthTry) < 10000 && lastAuthTry != 0) return false;
  return authenticate();
}

bool dbRequest(const String& path, const String& method, const String& body, String& result) {
  if (!ensureAuth()) return false;
  WiFiClientSecure client;
  client.setInsecure(); // diagnostic prototype only
  client.setTimeout(12000);
  HTTPClient http;
  String url = String(DB_URL) + "/" + path + ".json?auth=" + idToken;
  if (!http.begin(client, url)) {
    Serial.println("RTDB HTTP begin() failed");
    return false;
  }
  http.setConnectTimeout(12000);
  http.setTimeout(12000);
  if (method != "GET") http.addHeader("Content-Type", "application/json");
  int status = method == "GET" ? http.GET() : http.sendRequest(method.c_str(), body);
  result = status > 0 ? http.getString() : "";
  if (status < 200 || status >= 300) {
    Serial.printf("RTDB %s %s HTTP %d: %s\n", method.c_str(), path.c_str(), status,
                  status < 0 ? http.errorToString(status).c_str() : result.c_str());
  }
  http.end();
  return status >= 200 && status < 300;
}

void writeGate(const char* name, const char* state) {
  String ignored;
  String json = String("{\"state\":\"") + state + "\"}";
  dbRequest(String("gates/") + name, "PATCH", json, ignored);
}

void setGate(Gate& gate, Servo& servo, const char* name, bool open) {
  if (gate.open == open) return;
  servo.write(open ? OPEN_ANGLE : CLOSED_ANGLE);
  gate.open = open;
  if (open) gate.openedAt = millis();
  Serial.printf("%s gate: %s\n", name, open ? "Open" : "Closed");
  writeGate(name, open ? "Open" : "Closed");
}

int distanceCm(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10); digitalWrite(trig, LOW);
  unsigned long us = pulseIn(echo, HIGH, 25000);
  if (us == 0) return -1;
  return (int)(us * 0.0343f / 2.0f);
}

void pollCommands() {
  String response;
  if (!dbRequest("gates", "GET", "", response)) return;
  JsonDocument doc;
  if (deserializeJson(doc, response)) return;
  entryGate.mode = doc["entry"]["mode"] | "auto";
  exitGate.mode = doc["exit"]["mode"] | "auto";
  entryGate.requested = doc["entry"]["state"] | "Closed";
  exitGate.requested = doc["exit"]["state"] | "Closed";
  if (entryGate.mode == "manual") setGate(entryGate, entryServo, "entry", entryGate.requested == "Open");
  if (exitGate.mode == "manual") setGate(exitGate, exitServo, "exit", exitGate.requested == "Open");
}

String readCard(MFRC522& reader) {
  if (!reader.PICC_IsNewCardPresent() || !reader.PICC_ReadCardSerial()) return "";
  String uid;
  for (byte i = 0; i < reader.uid.size; i++) {
    if (reader.uid.uidByte[i] < 16) uid += "0";
    uid += String(reader.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  reader.PICC_HaltA(); reader.PCD_StopCrypto1();
  return uid;
}

bool isAuthorized(const String& uid) {
  String result;
  if (!dbRequest("authorizedCards/" + uid, "GET", "", result)) return false;
  result.trim();
  return result == "true";
}

void checkRFID(MFRC522& reader, Gate& gate, Servo& servo, const char* name, int distance) {
  if (gate.mode != "auto") return;
  String uid = readCard(reader);
  if (uid.isEmpty()) return;
  Serial.printf("%s RFID UID: %s\n", name, uid.c_str());
  if (distance < 0 || distance > DETECT_CM) {
    Serial.println("RFID denied: no vehicle within 1m");
    return;
  }
  if (!isAuthorized(uid)) {
    Serial.println("RFID denied: UID not authorized");
    return;
  }
  setGate(gate, servo, name, true);
}

void heartbeat() {
  String ignored;
  if (dbRequest("devices/ESP32-MAIN", "PATCH",
    "{\"id\":\"ESP32-MAIN\",\"name\":\"Main ESP32 Controller\",\"type\":\"Sensor\",\"layer\":\"Gate Controller\",\"status\":\"Online\",\"health\":100,\"lastSeenMs\":{\".sv\":\"timestamp\"}}", ignored)) {
    Serial.println("Firebase heartbeat OK (ESP32-MAIN)");
  }


  /*
  bool isI2COnline(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

bool isRFIDOnline(MFRC522& reader) {
  byte v = reader.PCD_ReadRegister(reader.VersionReg);
  return (v == 0x91 || v == 0x92);
}

void heartbeat() {
  // 1. Check live hardware states
  bool lcdOk   = isI2COnline(0x27);
  bool mcpOk   = isI2COnline(0x20);
  bool rfid1Ok = isRFIDOnline(entryRFID);
  bool rfid2Ok = isRFIDOnline(exitRFID);
  bool us1Ok   = (distanceCm(ENTRY_TRIG, ENTRY_ECHO) >= 0);
  bool us2Ok   = (distanceCm(EXIT_TRIG, EXIT_ECHO) >= 0);

  // 2. Build a single JSON object for /devices
  JsonDocument doc;

  auto addNode = [&](const char* id, const char* name, const char* type, const char* layer, bool online) {
    JsonObject obj = doc[id].to<JsonObject>();
    obj["id"] = id;
    obj["name"] = name;
    obj["type"] = type;
    obj["layer"] = layer;
    obj["status"] = online ? "Online" : "Offline";
    obj["health"] = online ? 100 : 0;
    if (online) {
      obj["lastSeenMs"][".sv"] = "timestamp";
    } else {
      obj["lastSeenMs"] = 0;
    }
  };

  addNode("ESP32-MAIN", "Main Controller", "Controller", "Core Layer", true);
  addNode("BR-01", "Entrance Gate", "Barrier", "Access Layer", true);
  addNode("BR-02", "Exit Gate", "Barrier", "Access Layer", true);
  addNode("RFID-01", "Entry RFID Scanner", "RFID Reader", "Access Layer", rfid1Ok);
  addNode("RFID-02", "Exit RFID Scanner", "RFID Reader", "Access Layer", rfid2Ok);
  addNode("LCD-01", "Entry I2C Status Display", "Display", "Display Layer", lcdOk);
  addNode("LCD-02", "Exit I2C Status Display", "Display", "Display Layer", lcdOk);
  addNode("LED-01", "Parking Slot 1 Indicator", "LED Indicator", "Display Layer", mcpOk);
  addNode("LED-02", "Parking Slot 2 Indicator", "LED Indicator", "Display Layer", mcpOk);
  addNode("LED-03", "Parking Slot 3 Indicator", "LED Indicator", "Display Layer", mcpOk);
  addNode("LED-04", "Parking Slot 4 Indicator", "LED Indicator", "Display Layer", mcpOk);
  addNode("SNS-01", "Parking Slot 1 IR Sensor", "Sensor", "Sensing Layer", true);
  addNode("SNS-02", "Parking Slot 2 IR Sensor", "Sensor", "Sensing Layer", true);
  addNode("SNS-03", "Parking Slot 3 IR Sensor", "Sensor", "Sensing Layer", true);
  addNode("SNS-04", "Parking Slot 4 IR Sensor", "Sensor", "Sensing Layer", true);
  addNode("US-01", "Entrance Ultrasonic Sensor", "Sensor", "Sensing Layer", us1Ok);
  addNode("US-02", "Exit Ultrasonic Sensor", "Sensor", "Sensing Layer", us2Ok);

  String payload, ignored;
  serializeJson(doc, payload);

  // 3. Send all 17 devices in one single HTTPS PATCH request
  if (dbRequest("devices", "PATCH", payload, ignored)) {
    Serial.println("Firebase heartbeat OK (All Devices Synced)");
  }
}

  */


}

void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(ENTRY_TRIG, OUTPUT); pinMode(EXIT_TRIG, OUTPUT);
  pinMode(ENTRY_ECHO, INPUT); pinMode(EXIT_ECHO, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  // Wire.begin(I2C_SDA, I2C_SCL);
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI);
  entryRFID.PCD_Init(); exitRFID.PCD_Init();
  entryServo.setPeriodHertz(50); exitServo.setPeriodHertz(50);
  entryServo.attach(ENTRY_SERVO_PIN, 500, 2400);
  exitServo.attach(EXIT_SERVO_PIN, 500, 2400);
  entryServo.write(CLOSED_ANGLE); exitServo.write(CLOSED_ANGLE);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(400); Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nWiFi connected. IP: "); Serial.println(WiFi.localIP());
    IPAddress ip;
    if (WiFi.hostByName("identitytoolkit.googleapis.com", ip)) {
      Serial.print("Firebase Auth DNS OK: "); Serial.println(ip);
    } else {
      Serial.println("Firebase Auth DNS FAILED");
    }
    authenticate();
    pollCommands(); heartbeat();
  } else {
    Serial.println("\nWiFi unavailable");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    static uint32_t lastRetry = 0;
    if (millis() - lastRetry > 10000) {
      lastRetry = millis(); WiFi.reconnect();
    }
    // Avoid cloud writes while disconnected; close gates locally in auto mode.
    if (entryGate.mode == "auto" && entryGate.open) {
      entryServo.write(CLOSED_ANGLE); entryGate.open = false;
    }
    if (exitGate.mode == "auto" && exitGate.open) {
      exitServo.write(CLOSED_ANGLE); exitGate.open = false;
    }
    delay(100); return;
  }
  if (millis() - lastPoll >= 1500) { lastPoll = millis(); pollCommands(); }
  if (millis() - lastHeartbeat >= 10000) { lastHeartbeat = millis(); heartbeat(); }
  int entryDist = distanceCm(ENTRY_TRIG, ENTRY_ECHO);
  int exitDist = distanceCm(EXIT_TRIG, EXIT_ECHO);
  checkRFID(entryRFID, entryGate, entryServo, "entry", entryDist);
  checkRFID(exitRFID, exitGate, exitServo, "exit", exitDist);
  if (entryGate.mode == "auto" && entryGate.open && millis() - entryGate.openedAt > OPEN_MS &&
      (entryDist < 0 || entryDist > DETECT_CM)) setGate(entryGate, entryServo, "entry", false);
  if (exitGate.mode == "auto" && exitGate.open && millis() - exitGate.openedAt > OPEN_MS &&
      (exitDist < 0 || exitDist > DETECT_CM)) setGate(exitGate, exitServo, "exit", false);
  delay(80);
}
