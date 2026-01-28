#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// 1. ADD YOUR DATA HERE
const char* ssid = "";
const char* password = "";

// 2. PIN DEFINITIONS
#define SS_PIN    5
#define RST_PIN   22
#define SERVO_PIN 13
#define LED       12 

MFRC522 rfid(SS_PIN, RST_PIN);
Servo myServo;
WebServer server(80);

bool isOpen = false;
String lastAction = "WAITING FOR PLAYER 1...";

// 3. RETRO ARCADE INTERFACE
String getHTML() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{background:#000; color:#39FF14; font-family:'Courier New'; text-align:center; padding-top:50px;}";
  html += ".box{border: 4px solid #FF00FF; display:inline-block; padding:20px; box-shadow: 0 0 15px #FF00FF;}";
  html += "h1{color:#00FFFF; text-shadow: 2px 2px #FF00FF;} .status{font-size:24px; margin:20px;}</style></head><body>";
  html += "<div class='box'><h1>RETRO DOOR CONTROL</h1>";
  html += "<div class='status'>DOOR STATUS: " + String(isOpen ? "LEVEL CLEAR (OPEN)" : "INSERT COIN (CLOSED)") + "</div>";
  html += "<div style='color:#FFFF00'>LOG: " + lastAction + "</div>";
  html += "<br><a href='/toggle' style='color:#FF00FF; text-decoration:none; border:2px solid; padding:5px;'>[ REMOTE OVERRIDE ]</a>";
  html += "</div><script>setTimeout(function(){location.reload();}, 3000);</script></body></html>";
  return html;
}

void handleRoot() { server.send(200, "text/html", getHTML()); }

void handleToggle() {
  isOpen = !isOpen;
  myServo.write(isOpen ? 90 : 0);
  lastAction = "REMOTE OVERRIDE USED";
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(0);
  
  SPI.begin();
  rfid.PCD_Init();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.begin();
}

void loop() {
  server.handleClient();
  
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);

  isOpen = !isOpen;
  myServo.write(isOpen ? 90 : 0);
  lastAction = "BADGE SCANNED AT " + String(millis()/1000) + " SEC";
  
  delay(1000);
  rfid.PICC_HaltA();
}
