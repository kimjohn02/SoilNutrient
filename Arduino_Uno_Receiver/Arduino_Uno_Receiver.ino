#include <SPI.h>
#include <LoRa.h>

// ====================================================================================
// SMART AGRICULTURE LORA RECEIVER BASE STATION (GATEWAY)
// Authors: KJ M. Lupogan & Kurt Christian Ramos (IT10 - 3454)
// Hardware: Arduino Uno + Ai-Thinker Ra-02 (SX1278 433MHz)
// ====================================================================================
// WIRING CHECKLIST:
//   Ra-02 Pin      Arduino Uno Pin
//   ---------      ---------------
//   GND            GND
//   3.3V           3.3V  (CRITICAL: 3.3V ONLY! NEVER 5V!)
//   NSS / CS       Pin 10
//   RST            Pin 9
//   DIO0           Pin 2
//   SCK            Pin 13
//   MISO           Pin 12
//   MOSI           Pin 11
//   ANT            433MHz Antenna (Attach to IPEX jack)
// ====================================================================================

#define LORA_SS    10
#define LORA_RST    9
#define LORA_DIO0   2

#define LORA_BAND  433E6 // 433 MHz (Must match ESP32 transmitter)

// Onboard LED indicator
#define LED_PIN    13

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println();
  Serial.println(F("========================================================="));
  Serial.println(F(" SMART AGRICULTURE LORA RECEIVER BASE STATION (GATEWAY)"));
  Serial.println(F(" Authors : KJ M. Lupogan & Kurt Christian Ramos"));
  Serial.println(F(" Hardware: Arduino Uno + Ai-Thinker Ra-02 (SX1278 433MHz)"));
  Serial.println(F("========================================================="));

  // Configure SPI pins for Ra-02 on Arduino Uno
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  Serial.print(F("[INIT] Starting SX1278 radio transceiver at 433 MHz... "));

  if (!LoRa.begin(LORA_BAND)) {
    Serial.println(F("FAILED!"));
    Serial.println(F("[ERROR] Cannot detect SX1278 on SPI bus."));
    Serial.println(F("Please check:"));
    Serial.println(F(" 1. Red wire is on 3.3V (NOT 5V)."));
    Serial.println(F(" 2. Pins: NSS=10, RST=9, SCK=13, MISO=12, MOSI=11."));
    
    // Fast blink to indicate wiring fault
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }

  Serial.println(F("SUCCESS!"));
  Serial.println(F("[STATUS] Gateway is ONLINE and actively listening for packets..."));
  Serial.println(F("---------------------------------------------------------"));

  // Solid blink to indicate radio is ready
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Check if a radio packet has arrived over the air
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String incomingPayload = "";

    // Read packet payload from LoRa FIFO buffer
    while (LoRa.available()) {
      incomingPayload += (char)LoRa.read();
    }

    // Flash onboard LED briefly on packet reception
    digitalWrite(LED_PIN, HIGH);

    // Output the raw CSV string directly for dashboard.html
    // Format: ID:PROBE_01,AREA:Zone_A,M:xx,N:xx,P:xx,K:xx,ALERT:xx,PKT:xx
    Serial.println(incomingPayload);

    delay(30);
    digitalWrite(LED_PIN, LOW);
  }
}
