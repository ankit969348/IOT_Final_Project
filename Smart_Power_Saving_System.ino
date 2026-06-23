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
#define VPIN_OCCUPANCY  V0  
#define VPIN_LED_STATUS V1  
#define VPIN_OVERRIDE   V3  
#define VPIN_ENERGY     V4  

// Power Management State Variables
unsigned long lastMotionTime = 0;
const unsigned long powerSaveDelay = 6000; // 6 seconds timeout
bool roomOccupied = false;
bool manualOverrideActive = false;
double totalEnergySaved = 0.0;

// Countdown tracking variable to prevent flood-printing
int lastSecondsPrinted = -1; 

// Software Sensitivity Filter Configuration
unsigned long motionTriggerStart = 0;
const unsigned long continuousMotionRequired = 800; 

const int LED_FULL_BRIGHT = 255; 
const int LED_IDLE_GLOW   = 40;  

BlynkTimer timer;

BLYNK_WRITE(VPIN_OVERRIDE) {
  int switchState = param.asInt();
  if (switchState == 1) {
    manualOverrideActive = true;
    digitalWrite(LED_PIN, HIGH);
    Blynk.virtualWrite(VPIN_OCCUPANCY, "Manual Override: ON");
    Blynk.virtualWrite(VPIN_LED_STATUS, LED_FULL_BRIGHT); 
    Serial.println("Serial State -> OVERRIDE: Forced ON");
  } else {
    manualOverrideActive = false; 
    roomOccupied = false;
    digitalWrite(LED_PIN, LOW);
    Blynk.virtualWrite(VPIN_OCCUPANCY, "Automation Active");
    Blynk.virtualWrite(VPIN_LED_STATUS, LED_IDLE_GLOW); 
    Serial.println("Serial State -> OVERRIDE: Disabled (Automation Active)");
  }
}

void checkEnergySystem() {
  if (manualOverrideActive) return; 

  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH) {
    if (motionTriggerStart == 0) {
      motionTriggerStart = millis();
    }
    
    if ((millis() - motionTriggerStart) > continuousMotionRequired) {
      lastMotionTime = millis(); 
      lastSecondsPrinted = -1; // Reset countdown memory
      
      if (!roomOccupied) {
        roomOccupied = true;
        digitalWrite(LED_PIN, HIGH);  
        Blynk.virtualWrite(VPIN_OCCUPANCY, "Occupied");
        Blynk.virtualWrite(VPIN_LED_STATUS, LED_FULL_BRIGHT); 
        Serial.println("\nSerial State -> Motion Sensed: LED ON");
      }
    }
  } else {
    motionTriggerStart = 0; 
    
    if (roomOccupied) {
      // Calculate how many seconds have passed, and invert it for a remaining countdown
      unsigned long msPassed = millis() - lastMotionTime;
      int secondsRemaining = ((powerSaveDelay - msPassed) / 1000) + 1;
      
      if (secondsRemaining < 1) secondsRemaining = 1;

      // Only print if the second integer has actually decremented
      if (secondsRemaining != lastSecondsPrinted) {
        Serial.print("No Motion! Turning off in: ");
        Serial.print(secondsRemaining);
        Serial.println("s...");
        lastSecondsPrinted = secondsRemaining;
      }
    }

    if (roomOccupied && (millis() - lastMotionTime > powerSaveDelay)) {
      roomOccupied = false;
      digitalWrite(LED_PIN, LOW);   
      totalEnergySaved += 0.05; 
      Blynk.virtualWrite(VPIN_OCCUPANCY, "Unoccupied");
      Blynk.virtualWrite(VPIN_LED_STATUS, LED_IDLE_GLOW); 
      Blynk.virtualWrite(VPIN_ENERGY, totalEnergySaved); 
      Serial.println("Serial State -> Timeout Expired: LED OFF\n");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(200L, checkEnergySystem); 
}

void loop() {
  Blynk.run();
  timer.run();
}


