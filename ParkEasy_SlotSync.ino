/* Park Easy ESP32 - Firebase RTDB diagnostic + gate controller
   ESP32 Arduino core 3.3.x, ArduinoJson 7, MFRC522, ESP32Servo.
   Prototype TLS only: setInsecure() skips certificate verification.
   Keep credentials private; do not upload this sketch with secrets to public GitHub.
   IMPORTANT: 5V ultrasonic ECHO -> level shifter/divider to 3.3V.
   Servos require separate regulated power and common ground.
*/
#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MCP23X17.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

const char* WIFI_SSID = "vj";
const char* WIFI_PASSWORD = "lontoc08";
const char* FIREBASE_API_KEY = "AIzaSyD4KDS_I3LwaFb8J-lJRTv_pmcV3uT8yt8";
const char* DEVICE_EMAIL = "vjlontoc035@gmail.com";
const char* DEVICE_PASSWORD = "VJLontoc09";
const char* DB_URL = "https://park-easy-system-default-rtdb.asia-southeast1.firebasedatabase.app";


#define EXIT_RFID_RST 16
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
const int irPins[] = {27, 32, 33, 36};
const char* slotIds[] = {"1A-01", "1A-02", "1A-03", "1A-04"};
const bool USE_IR_SENSORS = false;
LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2C exitLCD(0x26, 16, 2);
bool exitLCDOK = false;
int freeSlots = -1;
uint32_t entryMessageAt = 0, exitMessageAt = 0;
String lastLcdLines[2][2] = {{"", ""}, {"", ""}};

void lcdLine(LiquidCrystal_I2C& display, int displayIdx, int row, String msg) {
  while (msg.length() < 16) msg += " ";
  String trimmed = msg.substring(0, 16);
  if (lastLcdLines[displayIdx][row] == trimmed) return;
  lastLcdLines[displayIdx][row] = trimmed;
  display.setCursor(0, row);
  display.print(trimmed);
}

Adafruit_MCP23X17 mcp;
bool mcpOK = false;
bool lcdOK = false;
int stableIR[4] = {-1, -1, -1, -1}, rawIR[4] = {-1, -1, -1, -1};
uint32_t changedAt[4] = {};
uint32_t lastSlots = 0;
#define ENTRY_ECHO 34
#define EXIT_ECHO 35

MFRC522 entryRFID(ENTRY_RFID_SS, MFRC522::UNUSED_PIN);
MFRC522 exitRFID(EXIT_RFID_SS, MFRC522::UNUSED_PIN);
Servo entryServo, exitServo;
String idToken;
uint32_t tokenAt = 0, tokenExpiresMs = 0;
uint32_t lastPoll = 0, lastHeartbeat = 0, lastAuthTry = 0;
const int CLOSED_ANGLE = 0, OPEN_ANGLE = 90;
const int DETECT_CM = 100;
const uint32_t OPEN_MS = 5000;

struct Gate {
  String mode = "auto";
  String requested = "Closed";
  bool open = false;
  uint32_t openedAt = 0;
};

void setGate(Gate& gate, Servo& servo, const char* name, bool open);
void checkRFID(MFRC522& reader, Gate& gate, Servo& servo, const char* name, int distance);
Gate entryGate, exitGate;

bool authenticate() {
  if (WiFi.status() != WL_CONNECTED) return false;
  lastAuthTry = millis();

  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("WiFi RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  IPAddress ip;
  if (WiFi.hostByName("identitytoolkit.googleapis.com", ip)) {
    Serial.print("Firebase DNS OK: ");
    Serial.println(ip);
  } else {
    Serial.println("Firebase DNS FAILED");
    return false;
  }

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
  if (status == 401) {
    idToken = ""; // Force re-authentication on next request
  }
  if (status < 200 || status >= 300) {
    Serial.printf("RTDB %s %s HTTP %d: %s\n", method.c_str(), path.c_str(), status,
                  status < 0 ? http.errorToString(status).c_str() : result.c_str());
  }
  http.end();
  return status >= 200 && status < 300;
}

// Conditional root write: rejects concurrent database changes (HTTP 412).
bool paymentRoot(const String& method, String& body, String& etag) {
  if (!ensureAuth()) return false;
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  if (!http.begin(client, String(DB_URL) + "/.json?auth=" + idToken)) return false;
  http.setTimeout(12000);
  const char* headers[] = {"ETag"};
  http.collectHeaders(headers, 1);
  int status;
  if (method == "GET") {
    http.addHeader("X-Firebase-ETag", "true");
    status = http.GET();
    if (status == 200) { body = http.getString(); etag = http.header("ETag"); }
  } else {
    http.addHeader("Content-Type", "application/json");
    http.addHeader("if-match", etag);
    status = http.PUT(body);
  }
  http.end();
  Serial.printf("Payment %s HTTP %d\n", method.c_str(), status);
  return status == 200 && !etag.isEmpty();
}
String parkingError;

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

// Record RFID gate events using the existing website transaction schema.
bool recordParkingTap(const String& uid, bool entry) {
  time_t now = time(nullptr);
  if (now < 1700000000) { Serial.println("Parking: waiting for NTP clock"); parkingError = "Time sync error"; return false; }
  struct tm local;
  localtime_r(&now, &local);
  char clockText[6]; strftime(clockText, sizeof(clockText), "%H:%M", &local);
  String response;
  JsonDocument driver, slots, txs, rates, patch, root;
  String etag;
  parkingError = "Record failed";
  if (!entry) {
    if (!paymentRoot("GET", response, etag) || deserializeJson(root, response)) return false;
    driver.set(root["driverInfo"][uid]); slots.set(root["slots"]);
    txs.set(root["transactions"]); rates.set(root["rates"]);
    if (driver.isNull()) return false;
  } else {
    if (!dbRequest("driverInfo/" + uid, "GET", "", response) || deserializeJson(driver, response) || driver.isNull()) return false;
    if (!dbRequest("slots", "GET", "", response) || deserializeJson(slots, response)) return false;
    if (!dbRequest("transactions", "GET", "", response) || deserializeJson(txs, response)) return false;
  }
  String ticket, slotId;
  int matches = 0;
  for (JsonPair kv : txs.as<JsonObject>()) {
    JsonVariant t = kv.value();
    if (String(t["rfidUid"] | "") != uid || String(t["timeOut"] | "--:--") != "--:--") continue;
    ticket = kv.key().c_str(); matches++;
  }
  if (matches > 1) { Serial.println("Parking: multiple active tickets; resolve in website"); return false; }
  if (!ticket.isEmpty()) slotId = txs[ticket]["slotId"] | "";
  bool reserved = !slotId.isEmpty() && String(slots[slotId]["status"] | "") == "reserved" && String(slots[slotId]["rfidUid"] | "") == uid;
  if (entry && !ticket.isEmpty() && (!reserved || (txs[ticket]["gateEntryAtMs"] | 0LL) > 0)) { Serial.println("Parking: already checked in"); parkingError = "Already inside"; return false; }
  if (!entry && (ticket.isEmpty() || (reserved && (txs[ticket]["gateEntryAtMs"] | 0LL) == 0))) { Serial.println("Parking: no checked-in ticket"); return false; }
  String plate = reserved ? String(slots[slotId]["plate"] | "") : String(driver["plate"] | "");
  String vehicle = reserved ? String(slots[slotId]["vehicleType"] | "4w") : String(driver["vehicleType"] | "4W");
  if (!entry) { plate = txs[ticket]["plate"] | ""; vehicle = txs[ticket]["vehicle"] | "4W"; }
  if (plate.isEmpty()) { Serial.println("Parking: visitor needs reservation with plate"); parkingError = "No plate number"; return false; }
  bool two = vehicle.indexOf('2') >= 0;
  String rateKey = two ? "2w" : "4w";
  if (entry && (!dbRequest("rates", "GET", "", response) || deserializeJson(rates, response))) return false;
  double first = rates[rateKey]["firstHour"] | (two ? 20.0 : 40.0);
  double extra = rates[rateKey]["extraHour"] | (two ? 10.0 : 20.0);
  double cap = rates[rateKey]["dailyMax"] | (two ? 100.0 : 200.0);
  if (entry) {
    if (!reserved) {
      for (int i = 0; i < 4; i++) {
        JsonVariant sl = slots[slotIds[i]];
        if ((!USE_IR_SENSORS || stableIR[i] == 0) && !sl.isNull() && String(sl["status"] | "available") == "available" && !bool(sl["forced"] | false) && String(sl["mode"] | "auto") == "auto" && String(sl["ticketId"] | "").isEmpty()) { slotId = slotIds[i]; break; }
      }
      if (slotId.isEmpty()) { Serial.println("Parking: no available slot"); parkingError = "No vacant slot"; return false; }
      ticket = String("TKT-") + uid + "-" + String((unsigned long)now);
    }
    String base = "transactions/" + ticket + "/";
    patch[base+"id"] = ticket; patch[base+"slotId"] = slotId;
    patch[base+"plate"] = plate; patch[base+"rfidUid"] = uid;
    patch[base+"driverName"] = driver["name"] | "";
    patch[base+"passType"] = driver["passType"] | "Member";
    patch[base+"vehicle"] = two ? "2-Wheeler" : "4-Wheelers+";
    patch[base+"timeIn"] = clockText; patch[base+"timeOut"] = "--:--";
    patch[base+"amount"] = String(first, 2); patch[base+"status"] = "Unpaid";
    patch[base+"type"] = reserved ? "Reservation" : "Walk-In";
    patch[base+"timestamp"][".sv"] = "timestamp";
    patch[base+"gateEntryAtMs"][".sv"] = "timestamp";
    String sb = "slots/" + slotId + "/";
    patch[sb+"status"] = USE_IR_SENSORS ? "reserved" : "occupied"; patch[sb+"mode"] = "auto"; patch[sb+"forced"] = false;
    patch[sb+"light"] = "red"; patch[sb+"ticketId"] = ticket; patch[sb+"plate"] = plate;
    patch[sb+"rfidUid"] = uid; patch[sb+"vehicleType"] = rateKey;
    patch[sb+"passType"] = driver["passType"] | "Member";
    patch[sb+"note"] = driver["name"] | ""; patch[sb+"entry"] = clockText;
    patch[sb+"entryTimeMs"][".sv"] = "timestamp";
  } else {
    String base = "transactions/" + ticket + "/";
    int64_t started = txs[ticket]["timestamp"] | 0LL;
    if (started <= 0) return false;
    double hours = ceil(max(0.0, (double(now)*1000.0-started)/3600000.0));
    double fee = first + max(0.0, hours-1)*extra;
    if (cap > 0 && fee > cap) fee = cap;
    if (!isfinite(fee) || fee < 0) return false;
    JsonObject t = root["transactions"][ticket].as<JsonObject>();
    JsonObject d = root["driverInfo"][uid].as<JsonObject>();
    if (String(t["status"] | "Unpaid") != "Paid") {
      if (!d["balance"].is<double>()) { parkingError = "Invalid balance"; return false; }
      double balance = d["balance"].as<double>();
      double remaining = round((balance - fee)*100.0)/100.0;
      if (!isfinite(remaining) || remaining < -150.0) { parkingError = "Debt limit"; return false; }
      String historyKey = "DEDUCT-" + ticket;
      if (!root["topupHistory"][historyKey].isNull()) return false;
      d["balance"] = remaining; d["lastChargeAt"][".sv"] = "timestamp";
      JsonObject h = root["topupHistory"][historyKey].to<JsonObject>();
      h["uid"] = uid; h["driverName"] = d["name"] | ""; h["plate"] = plate;
      h["amount"] = -fee; h["previousBalance"] = balance; h["newBalance"] = remaining;
      h["method"] = "RFID Parking Fee"; h["ticketId"] = ticket; h["timestamp"][".sv"] = "timestamp";
      t["amount"] = String(fee, 2); t["status"] = "Paid"; t["paymentMethod"] = "RFID";
    }
    t["timeOut"] = clockText; t["gateExitAtMs"][".sv"] = "timestamp";
    JsonObject sl = root["slots"][slotId].as<JsonObject>();
    if (!sl.isNull() && String(sl["ticketId"] | "") == ticket && String(sl["mode"] | "auto") == "auto" && !bool(sl["forced"] | false)) {
      bool occupied = USE_IR_SENSORS;
      if (USE_IR_SENSORS) for (int i=0;i<4;i++) if (slotId == slotIds[i]) occupied = stableIR[i] != 0;
      sl["status"] = occupied ? "occupied" : "available"; sl["light"] = occupied ? "red" : "green";
      sl["ticketId"] = ""; sl["plate"] = ""; sl["rfidUid"] = ""; sl["passType"] = "";
      sl["note"] = ""; sl["entry"] = "--:--"; sl["entryTimeMs"] = 0;
    }
    String body; serializeJson(root, body);
    if (!paymentRoot("PUT", body, etag)) { parkingError = "Retry payment"; return false; }
  }
  if (entry) {
    String body, ignored; serializeJson(patch, body);
    if (!dbRequest("", "PATCH", body, ignored)) return false;
  }
  Serial.printf("Parking %s recorded: %s / %s\n", entry ? "TIME IN" : "TIME OUT", uid.c_str(), ticket.c_str());
  return true;
}

void checkRFID(MFRC522& reader, Gate& gate, Servo& servo, const char* name, int distance) {
  if (gate.mode != "auto" || gate.open) return;
  String uid = readCard(reader);
  if (uid.isEmpty()) return;
  Serial.printf("%s RFID UID: %s\n", name, uid.c_str());
  bool entry = String(name) == "entry";
  LiquidCrystal_I2C& display = entry ? lcd : exitLCD;
  int displayIdx = entry ? 0 : 1;
  bool displayOK = entry ? lcdOK : exitLCDOK;
  uint32_t& messageAt = entry ? entryMessageAt : exitMessageAt;
  if (distance < 0 || distance > DETECT_CM) {
    if (displayOK) { lcdLine(display, displayIdx, 1, "No vehicle"); messageAt = millis(); }
    Serial.println("RFID denied: no vehicle within 1m");
    return;
  }
  if (!isAuthorized(uid)) {
    if (displayOK) { lcdLine(display, displayIdx, 1, "Access denied"); messageAt = millis(); }
    Serial.println("RFID denied: UID not authorized");
    return;
  }
  parkingError = "Record failed";
  if (!recordParkingTap(uid, entry)) {
    if (displayOK) { lcdLine(display, displayIdx, 1, parkingError); messageAt = millis(); }
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
}

void sampleSlots() {
  if (!USE_IR_SENSORS) return;
  for (int i = 0; i < 4; i++) {
    int v = digitalRead(irPins[i]) == LOW;
    if (v != rawIR[i]) { rawIR[i] = v; changedAt[i] = millis(); }
    if (millis() - changedAt[i] >= 300) stableIR[i] = v;
  }
}

void syncSlots() {
  String response;
  if (!dbRequest("slots", "GET", "", response)) { freeSlots = -1; Serial.println("Slot sync: GET failed"); return; }
  JsonDocument doc;
  if (deserializeJson(doc, response)) { freeSlots = -1; Serial.println("Slot sync: invalid JSON"); return; }
  int available = 0;
  if (!USE_IR_SENSORS) {
    for (int i = 0; i < 4; i++) {
      JsonVariant slot = doc[slotIds[i]];
      String status = slot["status"] | "available";
      status.trim();
      status.toLowerCase();
      // Display the same status-based count as the web dashboard.
      bool free = !slot.isNull() && status == "available";
      // Keep the stricter entrance allocation checks unchanged.
      bool assignable = free && String(slot["ticketId"] | "").isEmpty() &&
                        String(slot["mode"] | "auto") == "auto" && !bool(slot["forced"] | false);
      if (free) available++;
      Serial.printf("Slot %s: status=%s, ticket=%s, mode=%s, forced=%d, assignable=%d\n",
                    slotIds[i], status.c_str(), slot["ticketId"] | "",
                    slot["mode"] | "auto", bool(slot["forced"] | false), assignable);
      if (mcpOK) { mcp.digitalWrite(i * 2, !free); mcp.digitalWrite(i * 2 + 1, free); }
    }
    freeSlots = available;
    Serial.printf("Available slots (dashboard): %d/4\n", freeSlots);
    return;
  }
  for (int i = 0; i < 4; i++) {
    if (stableIR[i] < 0) continue;
    JsonVariant slot = doc[slotIds[i]];
    String mode = slot["mode"] | "auto";
    String status = slot["status"] | "available";
    bool forced = slot["forced"] | false;
    bool occupied = stableIR[i] == 1;
    if (status == "reserved" && mode == "auto" && !forced && occupied && !String(slot["ticketId"] | "").isEmpty()) {
      String ignored;
      if (dbRequest(String("slots/") + slotIds[i], "PATCH", "{\"status\":\"occupied\",\"light\":\"red\"}", ignored)) status = "occupied";
    }
    bool managed = mode != "auto" || forced || status == "reserved" || status == "maintenance";
    String ticket = slot["ticketId"] | "";
    String plate = slot["plate"] | "";
    if (!managed && (occupied || (ticket.isEmpty() && plate.isEmpty()))) {
      String desired = occupied ? "occupied" : "available";
      if (status != desired || slot.isNull()) {
        JsonDocument patch;
        patch["status"] = desired;
        patch["light"] = occupied ? "red" : "green";
        patch["slotNum"] = i + 1;
        patch["mode"] = "auto";
        if (occupied) patch["entryTimeMs"][".sv"] = "timestamp";
        else { patch["entryTimeMs"] = 0; patch["entry"] = "--:--"; }
        String body, ignored;
        serializeJson(patch, body);
        if (dbRequest(String("slots/") + slotIds[i], "PATCH", body, ignored)) status = desired;
      }
    }
    bool free = status == "available" && !occupied;
    if (free) available++;
    if (mcpOK) { mcp.digitalWrite(i * 2, !free); mcp.digitalWrite(i * 2 + 1, free); }
  }
  freeSlots = available;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(ENTRY_TRIG, OUTPUT); pinMode(EXIT_TRIG, OUTPUT);
  pinMode(ENTRY_ECHO, INPUT); pinMode(EXIT_ECHO, INPUT);
  for (int i = 0; i < 4; i++) pinMode(irPins[i], INPUT);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.beginTransmission(0x27); lcdOK = Wire.endTransmission() == 0;
  if (lcdOK) { lcd.init(); lcd.backlight(); lcd.print("Connecting..."); }
  Wire.beginTransmission(0x26); exitLCDOK = Wire.endTransmission() == 0;
  if (exitLCDOK) { exitLCD.init(); exitLCD.backlight(); exitLCD.print("Connecting..."); }
  else Serial.println("Exit LCD not found at 0x26");
  mcpOK = mcp.begin_I2C(0x20);
  if (mcpOK) for (int i = 0; i < 8; i++) { mcp.pinMode(i, OUTPUT); mcp.digitalWrite(i, LOW); }
  else Serial.println("MCP23017 not found");
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI);
  pinMode(ENTRY_RFID_SS, OUTPUT);
  digitalWrite(ENTRY_RFID_SS, HIGH);
  pinMode(EXIT_RFID_SS, OUTPUT);
  digitalWrite(EXIT_RFID_SS, HIGH);
  pinMode(ENTRY_RFID_RST, OUTPUT);
  digitalWrite(ENTRY_RFID_RST, LOW);
  delay(50);
  digitalWrite(ENTRY_RFID_RST, HIGH);
  delay(50);
  entryRFID.PCD_Init(); exitRFID.PCD_Init();
  entryServo.setPeriodHertz(50); exitServo.setPeriodHertz(50);
  entryServo.attach(ENTRY_SERVO_PIN, 500, 2400);
  exitServo.attach(EXIT_SERVO_PIN, 500, 2400);
  entryServo.write(CLOSED_ANGLE); exitServo.write(CLOSED_ANGLE);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  configTime(8 * 3600, 0, "pool.ntp.org", "time.google.com");
  Serial.print("Connecting WiFi");
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(400); Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nWiFi connected. IP: "); Serial.println(WiFi.localIP());
    authenticate();
    pollCommands(); heartbeat();
  } else {
    Serial.println("\nWiFi unavailable");
  }
}

void loop() {
  sampleSlots();
  if (WiFi.status() != WL_CONNECTED) {
    static uint32_t lastRetry = 0;
    if (millis() - lastRetry > 10000) {
      lastRetry = millis(); WiFi.reconnect();
    }
    if (entryGate.mode == "auto" && entryGate.open) {
      entryServo.write(CLOSED_ANGLE); entryGate.open = false;
    }
    if (exitGate.mode == "auto" && exitGate.open) {
      exitServo.write(CLOSED_ANGLE); exitGate.open = false;
    }
    if (lcdOK) lcdLine(lcd, 0, 1, "WiFi offline");
    if (exitLCDOK) lcdLine(exitLCD, 1, 1, "WiFi offline");
    delay(100); return;
  }
  if (millis() - lastPoll >= 2000) { lastPoll = millis(); pollCommands(); }
  if (millis() - lastHeartbeat >= 13000) { lastHeartbeat = millis(); heartbeat(); } //mastrip ko nakikita ko last time elapsed sa website
  if (millis() - lastSlots >= 3000) { lastSlots = millis(); syncSlots(); }
  int entryDist = distanceCm(ENTRY_TRIG, ENTRY_ECHO);
  int exitDist = distanceCm(EXIT_TRIG, EXIT_ECHO);
  checkRFID(entryRFID, entryGate, entryServo, "entry", entryDist);
  checkRFID(exitRFID, exitGate, exitServo, "exit", exitDist);
  if (entryGate.mode == "auto" && entryGate.open && millis() - entryGate.openedAt > OPEN_MS &&
      (entryDist < 0 || entryDist > DETECT_CM)) setGate(entryGate, entryServo, "entry", false);
  if (exitGate.mode == "auto" && exitGate.open && millis() - exitGate.openedAt > OPEN_MS &&
      (exitDist < 0 || exitDist > DETECT_CM)) setGate(exitGate, exitServo, "exit", false);
  if (lcdOK) {
    lcdLine(lcd, 0, 0, freeSlots < 0 ? String("SLOTS: SYNC ERR") : String("FREE SLOTS: ") + freeSlots + "/4");
    if (entryGate.open) lcdLine(lcd, 0, 1, "Gate open");
    else if (!entryMessageAt || millis() - entryMessageAt >= 3000) lcdLine(lcd, 0, 1, "Tap RFID card");
  }
  if (exitLCDOK) {
    lcdLine(exitLCD, 1, 0, "EXIT GATE");
    if (exitGate.open) lcdLine(exitLCD, 1, 1, "Gate open");
    else if (!exitMessageAt || millis() - exitMessageAt >= 3000) lcdLine(exitLCD, 1, 1, "Tap RFID card");
  }
  delay(80);
}