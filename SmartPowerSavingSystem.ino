#define BLYNK_TEMPLATE_ID "TMPL3mXxqRdBJ"
#define BLYNK_TEMPLATE_NAME "SmartPowerSavingSystem"
#define BLYNK_AUTH_TOKEN "dlL8ZQEAo3KJraGxIgzIMYzXt085xDq_"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Network Credentials
char ssid[] = "Nothing Phone (2a)";
char pass[] = "7759089589";

// Hardware Pins
#define PIR_PIN 13
#define LED_PIN 2

// Blynk Datastream Mapping
#define VPIN_OCCUPANCY  V0  // String Label Widget
#define VPIN_LED_STATUS V1  // LED Widget (0 to 255)
#define VPIN_OVERRIDE   V3  // Switch Widget (0 or 1)
#define VPIN_ENERGY     V4  // Chart Data (Double)

// Power Management State Variables
unsigned long lastMotionTime = 0;
const unsigned long powerSaveDelay = 6000; // 6 seconds timeout
bool roomOccupied = false;
bool manualOverrideActive = false;
double totalEnergySaved = 0.0;

// LED Brightness States for Dashboard Tuning
const int LED_FULL_BRIGHT = 255; // Solid bright color when ON
const int LED_IDLE_GLOW   = 40;  // Soft dimmed color when OFF (Prevents hollow look!)

BlynkTimer timer;

// Two-Way Remote Manual Control Override Switch
BLYNK_WRITE(VPIN_OVERRIDE) {
  int switchState = param.asInt();
  
  if (switchState == 1) {
    manualOverrideActive = true;
    digitalWrite(LED_PIN, HIGH);
    Blynk.virtualWrite(VPIN_OCCUPANCY, "Manual Override: ON");
    Blynk.virtualWrite(VPIN_LED_STATUS, LED_FULL_BRIGHT); 
    Serial.println("System Status: Override Enabled -> Light Forced ON");
  } else {
    manualOverrideActive = false; 
    roomOccupied = false;
    digitalWrite(LED_PIN, LOW);
    Blynk.virtualWrite(VPIN_OCCUPANCY, "Automation Active");
    Blynk.virtualWrite(VPIN_LED_STATUS, LED_IDLE_GLOW); // Dimmed look instead of 0
    Serial.println("System Status: Resetting back to Automatic PIR Mode");
  }
}

// Automated sensor loop
void checkEnergySystem() {
  if (manualOverrideActive) return; 

  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH) {
    lastMotionTime = millis(); 
    
    if (!roomOccupied) {
      roomOccupied = true;
      digitalWrite(LED_PIN, HIGH);  
      
      Blynk.virtualWrite(VPIN_OCCUPANCY, "Occupied");
      Blynk.virtualWrite(VPIN_LED_STATUS, LED_FULL_BRIGHT); 
      Serial.println("System Status: Motion Detected -> Light ON");
    }
  } else {
    if (roomOccupied && (millis() - lastMotionTime > powerSaveDelay)) {
      roomOccupied = false;
      digitalWrite(LED_PIN, LOW);   
      
      totalEnergySaved += 0.05; 
      
      Blynk.virtualWrite(VPIN_OCCUPANCY, "Unoccupied");
      Blynk.virtualWrite(VPIN_LED_STATUS, LED_IDLE_GLOW); // Keeps a nice background glow on dashboard
      Blynk.virtualWrite(VPIN_ENERGY, totalEnergySaved); 
      Serial.println("System Status: 6s Timeout Expired -> Light OFF to Save Energy");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Poll sensor logic safely every 500ms
  timer.setInterval(500L, checkEnergySystem);
}

void loop() {
  Blynk.run();
  timer.run();
}





