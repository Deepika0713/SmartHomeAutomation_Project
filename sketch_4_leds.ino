#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi credentials
const char* ssid = "PVP-LABS";        // Replace with your WiFi name
const char* password = "Pvpsit@2023"; // Replace with your WiFi password

// Create web server on port 80
ESP8266WebServer server(80);

// LED pin definitions optimized for NodeMCU ESP-12E
// Using safe GPIO pins that don't interfere with boot process
const int THUMB_LED = D1;   // GPIO5  - Thumb LED (Safe pin)
const int INDEX_LED = D2;   // GPIO4  - Index LED (Safe pin)
const int MIDDLE_LED = D5;  // GPIO14 - Middle LED (Safe pin, SPI CLK when needed)
const int RING_LED = D6;    // GPIO12 - Ring LED (Safe pin, SPI MISO when needed)

// Alternative safe pins for ESP-12E if needed:
// D7 = GPIO13 (SPI MOSI), D8 = GPIO15 (CS, has pull-down)

// Built-in LED for NodeMCU ESP-12E (GPIO2/D4)
const int STATUS_LED = 2; // GPIO2 - Blue LED on ESP-12E module

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
  
  // Initialize LED pins as outputs
  pinMode(THUMB_LED, OUTPUT);
  pinMode(INDEX_LED, OUTPUT);
  pinMode(MIDDLE_LED, OUTPUT);
  pinMode(RING_LED, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  
  // Turn off all LEDs initially
  digitalWrite(THUMB_LED, LOW);
  digitalWrite(INDEX_LED, LOW);
  digitalWrite(MIDDLE_LED, LOW);
  digitalWrite(RING_LED, LOW);
  digitalWrite(STATUS_LED, HIGH); // Built-in LED is inverted (HIGH = OFF)
  
  Serial.println();
  Serial.println("Starting NodeMCU ESP-12E Hand Gesture LED Controller...");
  
  // Set WiFi mode to station (ESP-12E specific optimization)
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  
  // ESP-12E specific: Reduce power consumption
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // Keep WiFi awake for web server
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  // Wait for connection with timeout
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    Serial.print(".");
    attempts++;
    
    // Blink status LED while connecting
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Use this IP in your Python code!");
    
    // Turn on status LED to indicate successful connection
    digitalWrite(STATUS_LED, LOW); // LOW = ON for built-in LED
  } else {
    Serial.println();
    Serial.println("WiFi connection failed!");
    Serial.println("Check your WiFi credentials and try again.");
    
    // Blink status LED to indicate connection failure
    for (int i = 0; i < 10; i++) {
      digitalWrite(STATUS_LED, LOW);
      delay(200);
      digitalWrite(STATUS_LED, HIGH);
      delay(200);
    }
    return; // Exit setup if WiFi failed
  }
  
  // Define web server routes
  server.on("/", handleGestureControl);
  server.on("/test", handleTest);
  server.on("/status", handleStatus);
  server.on("/reset", handleReset);  // ADDED: Reset endpoint
  server.onNotFound(handleNotFound);
  
  // Enable CORS for web requests - ADDED
  server.enableCORS(true);
  
  // Start the server
  server.begin();
  Serial.println("Web server started!");
  Serial.println("=================================");
  Serial.println("Ready to receive gesture commands!");
  Serial.println("Available endpoints:");
  Serial.println("  GET /?t=0&i=1&m=0&r=1 - Control LEDs");
  Serial.println("  GET /test - Test all LEDs");
  Serial.println("  GET /status - Get system status");
  Serial.println("  GET /reset - Reset all LEDs");
  Serial.println("=================================");
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
  
  // IMPROVED: WiFi connection monitoring with exponential backoff
  static unsigned long lastWiFiCheck = 0;
  static int reconnectDelay = 1000;
  
  if (millis() - lastWiFiCheck > 10000) { // Check every 10 seconds
    lastWiFiCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected! Attempting to reconnect...");
      digitalWrite(STATUS_LED, HIGH); // Turn off status LED
      
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(reconnectDelay);
        Serial.print(".");
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println(" Reconnected!");
        digitalWrite(STATUS_LED, LOW); // Turn on status LED
        reconnectDelay = 1000; // Reset delay
      } else {
        Serial.println(" Failed to reconnect!");
        reconnectDelay = min(reconnectDelay * 2, 30000); // Exponential backoff
      }
    }
  }
  
  // Small delay to prevent watchdog reset
  yield();
}

// Main gesture control handler - IMPROVED with better error handling
void handleGestureControl() {
  // Add CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  
  // Handle OPTIONS request for CORS preflight
  if (server.method() == HTTP_OPTIONS) {
    server.send(200, "text/plain", "");
    return;
  }
  
  // Get parameters from URL with default values
  String thumbState = server.hasArg("t") ? server.arg("t") : "0";
  String indexState = server.hasArg("i") ? server.arg("i") : "0";
  String middleState = server.hasArg("m") ? server.arg("m") : "0";
  String ringState = server.hasArg("r") ? server.arg("r") : "0";
  
  // Validate input parameters (0 or 1 only)
  if (!isValidBinaryString(thumbState) || !isValidBinaryString(indexState) ||
      !isValidBinaryString(middleState) || !isValidBinaryString(ringState)) {
    server.send(400, "text/plain", "Error: Invalid parameters. Use 0 or 1 only.");
    return;
  }
  
  // Debug output
  Serial.println("Received gesture command:");
  Serial.println("Thumb: " + thumbState + ", Index: " + indexState + 
                ", Middle: " + middleState + ", Ring: " + ringState);
  
  // Control LEDs based on parameters
  digitalWrite(THUMB_LED, thumbState.toInt() ? HIGH : LOW);
  digitalWrite(INDEX_LED, indexState.toInt() ? HIGH : LOW);
  digitalWrite(MIDDLE_LED, middleState.toInt() ? HIGH : LOW);
  digitalWrite(RING_LED, ringState.toInt() ? HIGH : LOW);
  
  // Send JSON response for better parsing
  String response = "{";
  response += "\"status\":\"OK\",";
  response += "\"thumb\":" + thumbState + ",";
  response += "\"index\":" + indexState + ",";
  response += "\"middle\":" + middleState + ",";
  response += "\"ring\":" + ringState + ",";
  response += "\"timestamp\":" + String(millis());
  response += "}";
  
  server.send(200, "application/json", response);
}

// ADDED: Input validation helper function
bool isValidBinaryString(String input) {
  input.trim();
  return (input == "0" || input == "1");
}

// Test handler - IMPROVED with better feedback
void handleTest() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  Serial.println("Running LED test sequence...");
  
  String testResult = "LED Test Sequence:\n";
  
  // Turn on all LEDs
  testResult += "1. All LEDs ON\n";
  digitalWrite(THUMB_LED, HIGH);
  digitalWrite(INDEX_LED, HIGH);
  digitalWrite(MIDDLE_LED, HIGH);
  digitalWrite(RING_LED, HIGH);
  delay(1000);
  
  // Turn off all LEDs
  testResult += "2. All LEDs OFF\n";
  digitalWrite(THUMB_LED, LOW);
  digitalWrite(INDEX_LED, LOW);
  digitalWrite(MIDDLE_LED, LOW);
  digitalWrite(RING_LED, LOW);
  delay(500);
  
  // Sequential test
  testResult += "3. Sequential test:\n";
  
  testResult += "   - Thumb LED\n";
  digitalWrite(THUMB_LED, HIGH);
  delay(300);
  digitalWrite(THUMB_LED, LOW);
  
  testResult += "   - Index LED\n";
  digitalWrite(INDEX_LED, HIGH);
  delay(300);
  digitalWrite(INDEX_LED, LOW);
  
  testResult += "   - Middle LED\n";
  digitalWrite(MIDDLE_LED, HIGH);
  delay(300);
  digitalWrite(MIDDLE_LED, LOW);
  
  testResult += "   - Ring LED\n";
  digitalWrite(RING_LED, HIGH);
  delay(300);
  digitalWrite(RING_LED, LOW);
  
  testResult += "4. Test completed successfully!\n";
  
  server.send(200, "text/plain", testResult);
}

// ADDED: Reset handler to turn off all LEDs
void handleReset() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  digitalWrite(THUMB_LED, LOW);
  digitalWrite(INDEX_LED, LOW);
  digitalWrite(MIDDLE_LED, LOW);
  digitalWrite(RING_LED, LOW);
  
  Serial.println("All LEDs reset to OFF");
  
  String response = "{";
  response += "\"status\":\"OK\",";
  response += "\"message\":\"All LEDs reset to OFF\",";
  response += "\"timestamp\":" + String(millis());
  response += "}";
  
  server.send(200, "application/json", response);
}

// Status handler - IMPROVED with more details
void handleStatus() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  // Read current LED states
  bool thumbOn = digitalRead(THUMB_LED);
  bool indexOn = digitalRead(INDEX_LED);
  bool middleOn = digitalRead(MIDDLE_LED);
  bool ringOn = digitalRead(RING_LED);
  
  String status = "NodeMCU ESP-12E Hand Gesture Controller Status\n";
  status += "=============================================\n";
  status += "Board: NodeMCU ESP-12E (ESP8266)\n";
  status += "WiFi Status: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected") + "\n";
  status += "SSID: " + String(ssid) + "\n";
  status += "IP Address: " + WiFi.localIP().toString() + "\n";
  status += "MAC Address: " + WiFi.macAddress() + "\n";
  status += "Signal Strength: " + String(WiFi.RSSI()) + " dBm\n";
  status += "Chip ID: " + String(ESP.getChipId(), HEX) + "\n";
  status += "Flash Size: " + String(ESP.getFlashChipSize()) + " bytes\n";
  status += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
  status += "Core Version: " + String(ESP.getCoreVersion()) + "\n";
  status += "Uptime: " + String(millis() / 1000) + " seconds\n";
  status += "=============================================\n";
  status += "GPIO Pin Mapping (ESP-12E specific):\n";
  status += "  D1 (GPIO5)  -> Thumb LED:  " + String(thumbOn ? "ON" : "OFF") + "\n";
  status += "  D2 (GPIO4)  -> Index LED:  " + String(indexOn ? "ON" : "OFF") + "\n";
  status += "  D5 (GPIO14) -> Middle LED: " + String(middleOn ? "ON" : "OFF") + "\n";
  status += "  D6 (GPIO12) -> Ring LED:   " + String(ringOn ? "ON" : "OFF") + "\n";
  status += "  GPIO2       -> Status LED: " + String(!digitalRead(STATUS_LED) ? "ON" : "OFF") + "\n";
  status += "=============================================\n";
  
  server.send(200, "text/plain", status);
}

// 404 handler - IMPROVED
void handleNotFound() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  String message = "Error 404: Endpoint Not Found\n\n";
  message += "Available endpoints:\n";
  message += "  GET /?t=0&i=1&m=0&r=1 - Control LEDs (t=thumb, i=index, m=middle, r=ring)\n";
  message += "  GET /test - Test all LEDs\n";
  message += "  GET /status - Get system status\n";
  message += "  GET /reset - Reset all LEDs to OFF\n\n";
  message += "Request Details:\n";
  message += "URI: " + server.uri() + "\n";
  message += "Method: " + String(server.method() == HTTP_GET ? "GET" : "POST") + "\n";  // ✅ FIXED
  message += "Arguments: " + String(server.args()) + "\n";  // ✅ FIXED
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
}
