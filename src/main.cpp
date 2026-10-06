  #include <Arduino.h>
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #include <SPI.h>
  #include <LoRa.h>

  // ==========================================
  // 1. PROBE IDENTIFICATION & CONFIGURATION
  // ==========================================
  const String PROBE_ID   = "PROBE_01"; // Unique Probe Identifier
  const String AREA_LABEL = "Zone_A";   // Monitoring Location / Zone

  // Agronomic Decision Thresholds
  const int MOISTURE_MIN_THRESHOLD   = 20;  // Min Soil Moisture % (below = DRY)
  const int MOISTURE_MAX_THRESHOLD   = 80;  // Max Soil Moisture % (above = WET)
  const int NITROGEN_MIN_THRESHOLD   = 20;  // Min Nitrogen (mg/kg)
  const int PHOSPHORUS_MIN_THRESHOLD = 15;  // Min Phosphorus (mg/kg)
  const int POTASSIUM_MIN_THRESHOLD  = 30;  // Min Potassium (mg/kg)

  // Capacitive Soil Moisture Sensor (HW-390) Real Soil Calibration
  // Tuned strictly for SOIL: In open air (ADC >= 800) = strictly 0%.
  // Saturated muddy soil (ADC <= 450) = 100%.
  // Capacitive Soil Moisture Sensor (HW-390) Calibration on GPIO 35
  // Open air baseline: ADC >= 1950 (strictly 0%). Saturated wet soil: ADC <= 700 (100%)
  const int MOISTURE_DRY_ADC = 1950;  // Open air baseline (0%)
  const int MOISTURE_WET_ADC = 700;   // Saturated wet soil baseline (100%)

  // ==========================================
  // 2. HARDWARE PIN DEFINITIONS
  // ==========================================
  // Capacitive Soil Moisture Sensor Analog Pin
  #define MOISTURE_PIN  34   // ADC1_CH6 (Input Only, safe for ADC)

  // 16x2 I2C LCD Pins
  #define I2C_SDA       21   // Hardware I2C SDA
  #define I2C_SCL       22   // Hardware I2C SCL

  // MAX485 RS-485 Transceiver Direction & Serial Pins
  #define RS485_RX_PIN  16   // RO -> GPIO 16 (Hardware Serial2 RX)
  #define RS485_TX_PIN  17   // DI -> GPIO 17 (Hardware Serial2 TX)
  #define RS485_DE_PIN   4   // DE -> GPIO 4  (Driver Enable:  HIGH to Transmit)
  #define RS485_RE_PIN  15   // RE -> GPIO 15 (Receiver Enable: LOW to Receive)

  // Ai-Thinker Ra-02 LoRa SX1278 SPI Pins
  #define LORA_SCK      18   // SPI Clock -> GPIO 18
  #define LORA_MISO     19   // SPI MISO  -> GPIO 19
  #define LORA_MOSI     23   // SPI MOSI  -> GPIO 23
  #define LORA_SS        5   // SPI Chip Select (NSS) -> GPIO 5
  #define LORA_RST      14   // Hardware Reset -> GPIO 14
  #define LORA_DIO0     -1   // Unused for TX-only probe (Leaves strapping pin D2 completely free!)

  // LoRa Operating Frequency (433MHz for Ai-Thinker Ra-02)
  #define LORA_BAND     433E6

  // ==========================================
  // 3. MODBUS RTU QUERY FRAMES FOR NPK SENSOR
  // ==========================================
  // Frame 1: Register 0x0000 (Standard for 3-prong agricultural NPK sensors)
  // Inquiry: 01 03 00 00 00 03 05 CB -> Reads 3 registers: N, P, K
  static const uint8_t NPK_QUERY_00[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xCB };

  // Frame 2: Register 0x001E (Alternate standard used by some manufacturers)
  // Inquiry: 01 03 00 1E 00 03 E5 C2 -> Reads 3 registers: N, P, K
  static const uint8_t NPK_QUERY_1E[] = { 0x01, 0x03, 0x00, 0x1E, 0x00, 0x03, 0xE5, 0xC2 };

  #define NPK_QUERY_LEN  8
  #define NPK_REPLY_LEN 11   // 1(Addr) + 1(FC) + 1(ByteCount) + 6(Data: 2N+2P+2K) + 2(CRC)

  // Active query pointer (defaults to 0x0000 verified working with this sensor)
  const uint8_t *activeNPKQuery = NPK_QUERY_00;
  uint32_t npkBaudRate          = 4800; // Verified working at 4800 baud

  // ==========================================
  // 4. OBJECTS & GLOBAL STATE VARIABLES
  // ==========================================
  LiquidCrystal_I2C *lcd    = nullptr;
  uint8_t lcdAddress        = 0;
  unsigned long cycleCount  = 0;

  uint16_t nitrogenVal   = 0;   // mg/kg
  uint16_t phosphorusVal = 0;   // mg/kg
  uint16_t potassiumVal  = 0;   // mg/kg
  bool npkValid          = false; // true when a valid Modbus response is parsed
  int npkFailCount       = 0;   // counter for auto-reconnect fallback

  bool loraOnline        = false; // true when LoRa SX1278 responds on SPI bus

  // Auto-detected active moisture pin (defaults to 34)
  uint8_t activeMoisturePin = MOISTURE_PIN;

  // ==========================================
  // 5. HELPER FUNCTIONS
  // ==========================================

  // Scan I2C bus to locate LCD address (typically 0x27 or 0x3F)
  uint8_t findI2CAddress() {
    for (uint8_t addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        if (addr == 0x27 || addr == 0x3F) return addr;
      }
    }
    return 0;
  }

  // High-precision ADC reading strictly locked to GPIO 35 (Active sensor pin)
  int readSoilMoistureADC() {
    activeMoisturePin = 35;

    // Collect 24 samples from GPIO 35 with settle delay
    int samples[24];
    for (int i = 0; i < 24; i++) {
      samples[i] = analogRead(35);
      delay(3);
    }

    // Bubble sort to find median and eliminate noise spikes
    for (int i = 0; i < 23; i++) {
      for (int j = i + 1; j < 24; j++) {
        if (samples[i] > samples[j]) {
          int temp = samples[i];
          samples[i] = samples[j];
          samples[j] = temp;
        }
      }
    }

    // Discard 4 lowest and 4 highest outliers (LM2596 buck ripple rejection)
    long trimmedSum = 0;
    for (int i = 4; i < 20; i++) {
      trimmedSum += samples[i];
    }
    return (int)(trimmedSum / 16);
  }

  // Convert raw ADC reading to calibrated soil moisture percentage (0 - 100%)
  // In open air (ADC >= 1950) or unplugged (< 400): strictly 0%
  int calculateMoisturePercent(int rawADC) {
    if (rawADC >= MOISTURE_DRY_ADC || rawADC < 400) {
      return 0;
    }
    if (rawADC <= MOISTURE_WET_ADC) {
      return 100;
    }
    // High-precision linear interpolation for actual soil
    float percent = ((float)(MOISTURE_DRY_ADC - rawADC) / (float)(MOISTURE_DRY_ADC - MOISTURE_WET_ADC)) * 100.0f;
    return constrain((int)round(percent), 0, 100);
  }

  // Standard Modbus RTU CRC-16 calculation
  uint16_t crc16Modbus(const uint8_t *buffer, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
      crc ^= buffer[i];
      for (uint8_t b = 0; b < 8; b++) {
        if (crc & 0x0001) {
          crc = (crc >> 1) ^ 0xA001;
        } else {
          crc >>= 1;
        }
      }
    }
    return crc;
  }

  // MAX485 Transmit Mode (Drive DE and RE HIGH)
  void rs485TxMode() {
    digitalWrite(RS485_DE_PIN, HIGH);
    digitalWrite(RS485_RE_PIN, HIGH);
  }

  // MAX485 Receive Mode (Drive DE and RE LOW)
  void rs485RxMode() {
    digitalWrite(RS485_DE_PIN, LOW);
    digitalWrite(RS485_RE_PIN, LOW);
  }

  // Query Soil NPK Sensor over RS-485 Modbus RTU
  bool readNPKSensor() {
    // Clear any residual bytes in UART buffer
    while (Serial2.available()) {
      Serial2.read();
    }

    // Switch to transmit mode and send Modbus query frame
    rs485TxMode();
    delay(2);
    Serial2.write(activeNPKQuery, NPK_QUERY_LEN);
    Serial2.flush(); // Wait until all bytes leave the hardware UART
    delayMicroseconds(60); // Ensure last stop bit has cleared line
    rs485RxMode(); // Return to receive mode to listen for response

    // Wait up to 500ms for 11-byte response
    unsigned long startTime = millis();
    while (Serial2.available() < NPK_REPLY_LEN) {
      if (millis() - startTime > 500) {
        return false; // Timeout
      }
    }

    // Read response frame
    uint8_t response[NPK_REPLY_LEN];
    for (int i = 0; i < NPK_REPLY_LEN; i++) {
      response[i] = Serial2.read();
    }

    // Validate Modbus header: Slave ID (0x01), Function Code (0x03), Byte Count (0x06)
    if (response[0] != 0x01 || response[1] != 0x03 || response[2] != 0x06) {
      return false;
    }

    // Validate CRC-16
    uint16_t receivedCRC   = (uint16_t)response[NPK_REPLY_LEN - 1] << 8 | response[NPK_REPLY_LEN - 2];
    uint16_t calculatedCRC = crc16Modbus(response, NPK_REPLY_LEN - 2);
    if (receivedCRC != calculatedCRC) {
      return false;
    }

    // Parse 16-bit big-endian nutrient values (in mg/kg)
    nitrogenVal   = (uint16_t)(response[3] << 8) | response[4];
    phosphorusVal = (uint16_t)(response[5] << 8) | response[6];
    potassiumVal  = (uint16_t)(response[7] << 8) | response[8];

    return true;
  }

  // Auto-scan routine to establish connection with NPK sensor
  bool autoConnectNPK() {
    uint32_t testBauds[]        = { 4800, 9600 };
    const uint8_t *testFrames[] = { NPK_QUERY_00, NPK_QUERY_1E };

    for (int b = 0; b < 2; b++) {
      Serial2.end();
      delay(40);
      Serial2.begin(testBauds[b], SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

      for (int f = 0; f < 2; f++) {
        activeNPKQuery = testFrames[f];
        delay(150);
        if (readNPKSensor()) {
          npkBaudRate = testBauds[b];
          npkValid    = true;
          npkFailCount = 0;
          return true;
        }
      }
    }
    // Default to 4800 / 0x0000 if not yet powered
    activeNPKQuery = NPK_QUERY_00;
    Serial2.end();
    Serial2.begin(4800, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    return false;
  }

  // ==========================================
  // 6. SYSTEM INITIALIZATION (SETUP)
  // ==========================================
  void setup() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    Serial.begin(115200);
    delay(1200);

    Serial.println("\n========================================================");
    Serial.println(" LOCATION-ENABLED SOIL NUTRIENT & MOISTURE PROBE");
    Serial.println(" Authors: KJ M. Lupogan, Kurt Christian Ramos (IT10-3454)");
    Serial.println("========================================================");
    Serial.printf(" Probe ID   : %s\n", PROBE_ID.c_str());
    Serial.printf(" Area Label : %s\n", AREA_LABEL.c_str());
    Serial.println("========================================================\n");

    // Step 1: Initialize 16x2 I2C LCD Display
    Wire.begin(I2C_SDA, I2C_SCL);
    lcdAddress = findI2CAddress();
    if (lcdAddress != 0) {
      lcd = new LiquidCrystal_I2C(lcdAddress, 16, 2);
      lcd->init();
      lcd->backlight();
      lcd->clear();
      lcd->setCursor(0, 0);
      lcd->print("SOIL PROBE INIT");
      lcd->setCursor(0, 1);
      lcd->printf("ID:%s %s", PROBE_ID.c_str(), AREA_LABEL.c_str());
      Serial.printf("[LCD] Initialized successfully at I2C address 0x%02X\n", lcdAddress);
      delay(1500);
    } else {
      Serial.println("[WARN] 16x2 LCD not detected on I2C bus (Check SDA=D21, SCL=D22).");
    }

    // Step 2: Initialize MAX485 Control & Serial2 Port
    pinMode(RS485_DE_PIN, OUTPUT);
    pinMode(RS485_RE_PIN, OUTPUT);
    rs485RxMode(); // Default to receive mode

    Serial.printf("[MAX485] Direction pins ready (DE=D%d, RE=D%d)\n", RS485_DE_PIN, RS485_RE_PIN);

    // Step 3: Connect to NPK Sensor via Modbus RTU
    Serial.print("[NPK] Connecting to RS-485 NPK Sensor... ");
    if (autoConnectNPK()) {
      Serial.printf("ONLINE (Baud: %lu, Register: 0x0000)\n", npkBaudRate);
      Serial.printf("[NPK Initial] N=%d, P=%d, K=%d mg/kg\n", nitrogenVal, phosphorusVal, potassiumVal);
      if (lcd != nullptr) {
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->print("NPK SENSOR: OK!");
        lcd->setCursor(0, 1);
        lcd->printf("N:%d P:%d K:%d", nitrogenVal, phosphorusVal, potassiumVal);
        delay(1200);
      }
    } else {
      Serial.println("WAITING FOR POWER / SOIL CONTACT");
      if (lcd != nullptr) {
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->print("NPK: SEARCHING..");
        lcd->setCursor(0, 1);
        lcd->print("Awaiting Sensor");
        delay(1200);
      }
    }

    // Step 4: Initialize LoRa SX1278 (Ra-02 433MHz)
    Serial.println("[LoRa] Initializing Ai-Thinker Ra-02 SX1278 (433 MHz)...");
    
    // Hardware Reset Pulse to wake the radio
    pinMode(LORA_RST, OUTPUT);
    digitalWrite(LORA_RST, LOW);
    delay(20);
    digitalWrite(LORA_RST, HIGH);
    delay(30);

    // Initialize SPI bus with custom LoRa pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

    // Test SPI communication by reading silicon version register (0x42)
    pinMode(LORA_SS, OUTPUT);
    digitalWrite(LORA_SS, HIGH);
    delay(5);
    digitalWrite(LORA_SS, LOW);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(0x42 & 0x7F); // Read RegVersion
    uint8_t chipVersion = SPI.transfer(0x00);
    SPI.endTransaction();
    digitalWrite(LORA_SS, HIGH);
    Serial.printf("[LoRa] SPI Chip Version read: 0x%02X (Expected: 0x12)\n", chipVersion);

    // Attempt LoRa radio startup
    if (LoRa.begin(LORA_BAND)) {
      loraOnline = true;
      LoRa.setTxPower(17);          // 17 dBm transmit power (strong range)
      LoRa.setSpreadingFactor(7);   // Standard SF7 for balanced speed/range
      LoRa.setSignalBandwidth(125E3);
      LoRa.setCodingRate4(5);
      Serial.println("[LoRa] SUCCESS! Radio is ONLINE at 433 MHz.");
      if (lcd != nullptr) {
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->print("LoRa 433MHz OK!");
        lcd->setCursor(0, 1);
        lcd->print("Radio: ONLINE");
        delay(1200);
      }
    } else {
      loraOnline = false;
      Serial.println("[LoRa] Module not responding on SPI. Continuing in Local-Only mode.");
      if (lcd != nullptr) {
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->print("LoRa: OFFLINE");
        lcd->setCursor(0, 1);
        lcd->printf("ChipVer: 0x%02X", chipVersion);
        delay(1200);
      }
    }

    Serial.println("\n[SYSTEM] Initialization complete. Entering operational loop.\n");
  }

  // ==========================================
  // 7. OPERATIONAL LOOP
  // ==========================================
  unsigned long lastCycleTime = 0;

  void loop() {
    unsigned long now = millis();

    // Run telemetry cycle every 3.0 seconds
    if (now - lastCycleTime >= 3000) {
      lastCycleTime = now;
      cycleCount++;

      // ------------------------------------------
      // A. READ SOIL MOISTURE (STRICT 0% IN AIR, STEADY IN SOIL)
      // ------------------------------------------
      int rawMoistureADC = readSoilMoistureADC();

      static int lockedSoilPercent = 0;
      static bool wasInSoil = false;
      int moisturePercent = 0;

      // In open air: raw ADC is >= 1950 (air baseline ~2300) -> STRICTLY 0%
      // In soil: raw ADC drops below 1950 (down to 700-1400)
      bool inSoil = (rawMoistureADC < MOISTURE_DRY_ADC && rawMoistureADC >= 400);

      if (inSoil) {
        int liveCalculated = calculateMoisturePercent(rawMoistureADC);

        if (!wasInSoil) {
          // Just entered soil: lock reading immediately!
          lockedSoilPercent = liveCalculated;
          wasInSoil = true;
        } else {
          // While resting in soil: ignore minor +/- 3% noise to keep display steady
          // Only update if probe is moved to a new soil spot or watered (diff > 3%)
          int change = abs(liveCalculated - lockedSoilPercent);
          if (change > 3) {
            lockedSoilPercent = liveCalculated;
          }
        }
        moisturePercent = lockedSoilPercent;
      } else {
        // Probe is in OPEN AIR / HINUGOT SA LUPA: STRICTLY 0%!
        wasInSoil = false;
        lockedSoilPercent = 0;
        moisturePercent = 0;
      }

      // ------------------------------------------
      // B. READ SOIL NPK NUTRIENTS
      // ------------------------------------------
      if (readNPKSensor()) {
        npkValid     = true;
        npkFailCount = 0;
      } else {
        npkFailCount++;
        if (npkFailCount >= 3) {
          npkValid = false; // Mark offline if 3 consecutive queries fail
          autoConnectNPK();
        }
      }

      // ------------------------------------------
      // C. PROCESS: THRESHOLD DECISION LOGIC
      // ------------------------------------------
      bool isAlert    = false;
      String alertMsg = "NONE";

      // Soil Moisture Classification
      if (moisturePercent < MOISTURE_MIN_THRESHOLD) {
        isAlert  = true;
        alertMsg = "DRY";
      } else if (moisturePercent > MOISTURE_MAX_THRESHOLD) {
        isAlert  = true;
        alertMsg = "WET";
      }

      // Nutrient Depletion Classification
      if (npkValid) {
        if (nitrogenVal   < NITROGEN_MIN_THRESHOLD ||
            phosphorusVal < PHOSPHORUS_MIN_THRESHOLD ||
            potassiumVal  < POTASSIUM_MIN_THRESHOLD) {
          isAlert  = true;
          alertMsg = (alertMsg == "NONE") ? "LOW_NPK" : alertMsg + "+LOW_NPK";
        }
      }

      String statusLabel = isAlert ? alertMsg : "OPTIMAL";

      // ------------------------------------------
      // D. OUTPUT: 16x2 LCD DISPLAY (PERMANENT LIVE READINGS)
      // ------------------------------------------
      if (lcd != nullptr) {
        lcd->clear();

        // Line 1: Live Soil Moisture Percentage + LoRa indicator
        lcd->setCursor(0, 0);
        lcd->printf("Moist:%2d%% %s%s", moisturePercent, isAlert ? "[!]" : "[OK]", loraOnline ? "TX" : "  ");

        // Line 2: Live Nitrogen, Phosphorus, Potassium
        lcd->setCursor(0, 1);
        if (npkValid) {
          lcd->printf("N:%-3d P:%-3d K:%-3d", nitrogenVal, phosphorusVal, potassiumVal);
        } else {
          lcd->print("NPK: SEARCHING..");
        }
      }

      // ------------------------------------------
      // E. OUTPUT: WIRELESS LORA TRANSMISSION
      // ------------------------------------------
      String loraPayload = "ID:" + PROBE_ID +
                          ",AREA:" + AREA_LABEL +
                          ",M:" + String(moisturePercent) +
                          ",N:" + String(nitrogenVal) +
                          ",P:" + String(phosphorusVal) +
                          ",K:" + String(potassiumVal) +
                          ",ALERT:" + alertMsg +
                          ",PKT:" + String(cycleCount);

      if (loraOnline) {
        LoRa.beginPacket();
        LoRa.print(loraPayload);
        LoRa.endPacket();
        Serial.printf(" [LoRa TX #%lu] %s\n", cycleCount, loraPayload.c_str());
      }

      // ------------------------------------------
      // F. OUTPUT: FORMATTED SERIAL TELEMETRY LOG
      // ------------------------------------------
      Serial.println("========================================================");
      Serial.printf(" [CYCLE #%lu] Probe: %s | Location: %s | LoRa: %s\n", 
        cycleCount, PROBE_ID.c_str(), AREA_LABEL.c_str(), loraOnline ? "ONLINE" : "OFFLINE");
      Serial.printf(" Soil Moisture : %3d%%  (Raw ADC: %d on GPIO 35) -> [%s] Status: %s\n", 
                    moisturePercent, rawMoistureADC,
                    inSoil ? "IN SOIL" : "OPEN AIR (0%)",
                    (moisturePercent < MOISTURE_MIN_THRESHOLD) ? "DRY" : 
                    (moisturePercent > MOISTURE_MAX_THRESHOLD) ? "WET" : "OPTIMAL");

      if (npkValid) {
        Serial.printf(" Soil Nutrients: Nitrogen   = %3d mg/kg  (Threshold >= %d)\n", nitrogenVal, NITROGEN_MIN_THRESHOLD);
        Serial.printf("                 Phosphorus = %3d mg/kg  (Threshold >= %d)\n", phosphorusVal, PHOSPHORUS_MIN_THRESHOLD);
        Serial.printf("                 Potassium  = %3d mg/kg  (Threshold >= %d)\n", potassiumVal, POTASSIUM_MIN_THRESHOLD);
      } else {
        Serial.println(" Soil Nutrients: [OFFLINE] NPK sensor not responding");
      }

      Serial.printf(" System Status : %s | Alert Flag: %s\n", statusLabel.c_str(), isAlert ? "YES" : "NO");
      Serial.println("========================================================");
    }
  }
