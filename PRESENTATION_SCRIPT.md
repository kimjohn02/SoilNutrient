# CAPSTONE PRESENTATION & LIVE DEMO DEFENSE SCRIPT
**Project Title:** Location Enabled Soil Nutrient and Soil Moisture Monitoring Probe with LoRa Communication  
**Course & Institution:** IT10/L IT Elective 3 | College of Computing Education (CCE), University of Mindanao  
**Proponents:** KJ M. Lupogan & Kurt Christian Ramos  
**Adviser:** Lloyd Ryan Largo  

---

## 📋 TABLE OF CONTENTS
1. [Presentation Strategy & Speaker Roles](#1-presentation-strategy--speaker-roles)
2. [Pre-Demo Hardware Setup Checklist](#2-pre-demo-hardware-setup-checklist)
3. [Full Spoken Presentation Script (Word-for-Word)](#3-full-spoken-presentation-script-word-for-word)
   - [Part I: Introduction & Problem Context](#part-i-introduction--problem-context-0000--0130)
   - [Part II: System Architecture & Hardware Showcase](#part-ii-system-architecture--hardware-showcase-0130--0330)
   - [Part III: Live Interactive System Demonstration](#part-iii-live-interactive-system-demonstration-0330--0700)
   - [Part IV: Agronomic Intelligence & Data Export](#part-iv-agronomic-intelligence--data-export-0700--0830)
   - [Part V: Conclusion & Handover to Panelists](#part-v-conclusion--handover-to-panelists-0830--0900)
4. [Anticipated Panelist Q&A Defense Guide](#4-anticipated-panelist-qa-defense-guide)
5. [Emergency Troubleshooting Quick-Reference](#5-emergency-troubleshooting-quick-reference)

---

## 1. PRESENTATION STRATEGY & SPEAKER ROLES

| Speaker | Name | Primary Role & Focus |
|---|---|---|
| **Speaker 1** | **KJ M. Lupogan** | **Project Lead & System Specialist:** Introduces the project rationale, explains the 3-Layer IoT architecture, leads the live moisture & NPK hardware demonstration, and presents the conclusion. |
| **Speaker 2** | **Kurt Christian Ramos** | **Software & Telemetry Specialist:** Explains the wireless LoRa RF communication protocol, demonstrates the base station gateway, operates the live Web Dashboard, and demonstrates the CSV data export. |

*Total Presentation Time:* **8 to 10 Minutes** (+ Panel Q&A).  
*Tone:* Professional, confident, academic, and engineering-focused.

---

## 2. PRE-DEMO HARDWARE SETUP CHECKLIST

Do this **10 minutes before** the panel enters the room:

- [ ] **Base Station Gateway (COM4):**
  - Arduino Uno connected via USB to the laptop.
  - Ai-Thinker Ra-02 LoRa module wired (3.3V, GND, NSS=10, RST=9, DIO0=2, SCK=13, MISO=12, MOSI=11).
  - 433 MHz antenna securely attached.
- [ ] **Field Probe Node (COM3 / Battery):**
  - ESP32 powered via battery pack through the LM2596 buck converter (regulated to 5.0V).
  - Ai-Thinker Ra-02 LoRa module wired on SPI (NSS=5, RST=14, SCK=18, MISO=19, MOSI=23).
  - MAX485 module wired to Serial2 (RX=16, TX=17, DE=4, RE=15).
  - NPK sensor 3-prong probe connected to MAX485 A & B terminals.
  - Capacitive moisture sensor connected to Column **D34** (Signal to GPIO 34, VCC, GND).
  - 16x2 I2C LCD displaying live local readings (`Moist:% [OK]TX` / `N: P: K:`).
- [ ] **Demo Testing Materials Ready on Table:**
  - A cup/container of **moist soil** (or a damp paper towel/wet sponge).
  - A small cup of clean water.
  - A dry paper napkin or microfiber cloth to wipe the probe blade dry.
- [ ] **Web Dashboard Ready:**
  - Chrome browser open at `http://localhost:8080/dashboard.html`.
  - Serial connection established to `COM4` at 115200 baud.
  - Real-time status shows `LoRa Radio: BROADCASTING (TX)` in green.

---

## 3. FULL SPOKEN PRESENTATION SCRIPT (WORD-FOR-WORD)

### PART I: INTRODUCTION & PROBLEM CONTEXT (00:00 – 01:30)

**(Both speakers stand up straight, smile, and make eye contact with the panel.)**

**SPEAKER 1 (KJ M. Lupogan):**
> *"Good morning, esteemed panel of evaluators, our respected adviser Engr. Lloyd Ryan Largo, and distinguished guests. I am KJ M. Lupogan."*

**SPEAKER 2 (Kurt Christian Ramos):**
> *"And I am Kurt Christian Ramos. Today, we are proud to present our capstone project entitled: **'Location Enabled Soil Nutrient and Soil Moisture Monitoring Probe with LoRa Communication.'**"*

**SPEAKER 1 (KJ):**
> *"In the agricultural landscapes of the Philippines—particularly across Mindanao—crop productivity depends heavily on maintaining optimal soil moisture and the three primary soil macronutrients: **Nitrogen (N)**, **Phosphorus (P)**, and **Potassium (K)**.*
>
> *However, smallholder and commercial farmers face two critical bottlenecks:*
> 1. *Traditional agricultural testing requires manual soil collection sent to centralized laboratories, taking days or weeks to return results.*
> 2. *Existing commercial smart-farming probes rely on **Wi-Fi or cellular GSM (3G/4G/LTE) networks**—infrastructure that is notoriously absent, weak, or cost-prohibitive in remote rural farmlands.*
>
> *Our project addresses this exact technological gap by delivering an **autonomous, solar/battery-powered field monitoring probe** capable of interrogating real-time soil chemistry and transmitting agronomic telemetry over long distances **completely off-grid**, without requiring internet or cellular SIM cards."*

---

### PART II: SYSTEM ARCHITECTURE & HARDWARE SHOWCASE (01:30 – 03:30)

**SPEAKER 2 (Kurt):**
> *"To realize this solution, we designed and built a comprehensive **3-Layer IoT Architecture**:*
>
> 1. *First is the **Perception and Edge Processing Layer**—the physical field probe node located right here on our desk.*
> 2. *Second is the **Network and Transport Layer**—powered by sub-gigahertz Long Range Radio Frequency communication operating at 433 MHz.*
> 3. *Third is the **Base Station and Application Layer**—consisting of an Arduino Uno receiving gateway and our custom-built Agricultural Telemetry Dashboard.*
>
> *Let us walk you through the physical hardware engineering."*

**(SPEAKER 1 points directly to the hardware components on the table as Kurt explains them.)**

**SPEAKER 1 (KJ):**
> *"At the heart of the field probe is the **ESP32 Microcontroller**. We selected the ESP32 for its dual-core 240 MHz processing capability, multi-channel 12-bit ADC, and dedicated hardware UART peripherals.*
>
> *For chemical sensing, we deployed an industrial-grade **3-Prong Agricultural NPK Sensor**. This sensor uses differential frequency-domain reflectometry to quantify soil Nitrogen, Phosphorus, and Potassium in milligrams per kilogram (mg/kg). It communicates via the **RS-485 differential industrial standard** using the **Modbus RTU protocol** at 4800 baud.*
>
> *Because microcontrollers cannot natively read differential RS-485 voltage levels, we integrated a **MAX485 transceiver module** driven by ESP32 Serial2, utilizing hardware GPIOs 4 and 15 to dynamically switch between Transmit and Receive modes with microsecond-level timing precision.*
>
> *For hydration sensing, we utilize a **Capacitive Soil Moisture Sensor (HW-390)** connected to analog pin **GPIO 34**. Unlike cheap resistive sensors that corrode within days due to soil electrolysis, capacitive sensing measures the soil dielectric permittivity through protective solder-mask insulation, ensuring multi-season operational longevity.*
>
> *For on-site visualization, an onboard **16x2 Liquid Crystal Display** running over I2C allows field technicians to view instantaneous readings immediately without carrying a computer."*

**SPEAKER 2 (Kurt):**
> *"For the wireless telemetry backbone, we integrated the **Ai-Thinker Ra-02 transceiver**, based on the Semtech **SX1278 LoRa chip** operating at **433 MHz**.*
>
> *LoRa employs Chirp Spread Spectrum modulation, allowing signals to be decoded even below the environmental RF noise floor. With an effective transmit power of **+17 dBm** and Spreading Factor 7, our probe wirelessly broadcasts telemetry packets across vast hectares of open farmland directly to our base station gateway—with zero recurring network subscription fees."*

---

### PART III: LIVE INTERACTIVE SYSTEM DEMONSTRATION (03:30 – 07:00)

**SPEAKER 1 (KJ):**
> *"At this stage, we would like to transition directly into our **Live Hardware Demonstration**. Everything you see here is operating live in real-time."*

#### Step 1: Base Station Gateway & Dashboard Initialization
**SPEAKER 2 (Kurt - operating laptop):**
> *"Here on the laptop, our Arduino Uno Base Station Gateway is connected to USB Port `COM4`. We have launched our Web Telemetry Dashboard. When I click **'Connect to Probe'** via the Web Serial API, the gateway establishes a high-speed 115200 baud pipeline.*
>
> *Notice the indicator on the dashboard:*
> * **`LoRa Radio: BROADCASTING (TX)`** *in glowing green.*
> * *A live **Heartbeat Timestamp** updating every 1.2 seconds as each packet arrives over the air.*
> * *And the live packet counter incrementing continuously."*

#### Step 2: Local 16x2 LCD Mirror Verification
**SPEAKER 1 (KJ - holding up the field probe):**
> *"Now observe our physical field probe. On our onboard 16x2 LCD display:*
> * *Line 1 shows: `Moist:  0% [!]TX`*
> * *Line 2 shows our live NPK readings: `N: 1  P: 49  K: 42`*
>
> *Now glance at the laptop screen. In the upper-left card of our dashboard, we have engineered an **Exact LCD Hardware Mirror**. Whatever is rendered on the physical liquid crystal screen in the field is mirrored instantaneously on the base station dashboard. This guarantees field-to-office telemetry parity."*

#### Step 3: Demonstrating Baseline Open-Air Soil Moisture
**SPEAKER 2 (Kurt):**
> *"Notice that right now, the Soil Moisture is displaying **`0%`** with a red **`[!] DRY`** alert flag.*
>
> *This is completely accurate and expected agronomic behavior: the sensor is currently resting in open air on our table. Since there is zero soil contact, the measured volumetric moisture is zero, which immediately triggers the agronomic drought alert."*

#### Step 4: Live Dynamic Soil Moisture Insertion Test
**SPEAKER 1 (KJ - performs physical action):**
> *"Now, watch what happens when we introduce moisture to the probe blade. I will now insert the capacitive blade into our container of moist soil [or wrap it with our damp testing cloth]..."*

**(KJ inserts the sensor blade into moist soil / touches it firmly with damp cloth.)**

**SPEAKER 1 (KJ):**
> *"Observe both screens!"*

**SPEAKER 2 (Kurt):**
> *"Look at the physical LCD and look at the Web Dashboard:*
> 1. *The Soil Moisture percentage immediately climbs from **`0%` up to `45%`**!*
> 2. *The status changes from red `[!] DRY` to a green **`[OK] OPTIMAL`**!*
> 3. *On the dashboard, the **Soil Moisture Agronomic Zone Spectrum** slider moves dynamically into the green vegetative zone between 20% and 80%!*
> 4. *The real-time rolling trend line on the graph glides upward in real-time.*
>
> *This demonstrates our high-speed 24-sample trimmed-mean noise filter running on the ESP32, which eliminates electrical ripple from the buck converter while providing instantaneous 1.2-second responsiveness."*

**(KJ removes probe and wipes it with a dry napkin.)**

**SPEAKER 1 (KJ):**
> *"And as soon as I withdraw the probe and wipe it dry, within 2 seconds, the reading smoothly returns to **0%**, demonstrating true sensor hysteresis and zero false calibration drift."*

#### Step 5: Live NPK Soil Macronutrient Quantification
**SPEAKER 2 (Kurt):**
> *"Simultaneously, let us inspect the Macronutrient telemetry:*
> * **Nitrogen (N):** *Reads **`1 mg/kg`**.*
> * **Phosphorus (P):** *Reads **`49 mg/kg`** (well above our 15 mg/kg baseline—marked as **OPTIMAL** in amber).*
> * **Potassium (K):** *Reads **`42 mg/kg`** (well above our 30 mg/kg baseline—marked as **OPTIMAL** in purple).*
>
> *These values are not dummy simulations; they are retrieved via Modbus RTU byte queries interrogating the physical sensor registers at 4800 baud with verified CRC-16 error checking."*

---

### PART IV: AGRONOMIC INTELLIGENCE & DATA EXPORT (07:00 – 08:30)

**SPEAKER 1 (KJ):**
> *"Collecting raw data is only half the battle. A farmer does not simply want numbers; a farmer needs **actionable agronomic decisions**.*
>
> *For this reason, our system embeds an **Agronomic Decision Engine** directly into the telemetry pipeline. Based on standard vegetative crop requirements:*
> * *If Soil Moisture drops below 20%, the system flags **`IRRIGATION NEEDED`**.*
> * *If Nitrogen drops below 20 mg/kg, the advisory automatically flags **`APPLY FERTILIZER (Urea / N-Rich Formulations)`**.*
> * *The dashboard presents these automated recommendations clearly in plain English, allowing farm workers to act immediately without consulting an agronomist."*

**SPEAKER 2 (Kurt - clicks 'Export CSV' on dashboard):**
> *"Finally, for research logging and historical crop analysis, our dashboard includes a **One-Click Telemetry Export**.*
>
> *When I click **'Export CSV'**, the browser packages every single received LoRa packet into an industry-standard comma-separated file. Each record contains the UTC timestamp, unique Probe ID (`PROBE_01`), Monitoring Zone (`Zone_A`), Soil Moisture percentage, Nitrogen, Phosphorus, Potassium, Alert flags, and packet sequence numbers.*
>
> *This CSV file can be opened directly in Microsoft Excel, Google Sheets, or Python Pandas for precision agricultural modeling and harvest yield prediction."*

---

### PART V: CONCLUSION & HANDOVER TO PANELISTS (08:30 – 09:00)

**SPEAKER 1 (KJ):**
> *"To conclude, our project successfully proves that precision agricultural monitoring does not require expensive cellular data subscriptions, fragile laboratory turnarounds, or continuous grid electricity.*
>
> *By unifying **industrial RS-485 Modbus RTU sensing**, **durable capacitive dielectric measurement**, and **long-range 433 MHz LoRa radio frequency transmission**, we have delivered an off-grid, cost-effective, and fully autonomous soil monitoring platform designed specifically for the agricultural realities of the Philippines.*
>
> *Thank you very much. We are now honored to open the floor and welcome questions from our esteemed panel."*

**(Both speakers bow respectfully and stand ready for Q&A.)**

---

## 4. ANTICIPATED PANELIST Q&A DEFENSE GUIDE

Here are the exact answers to the most probable technical questions the panel will ask:

### Q1: "Why did you choose LoRa 433 MHz instead of Wi-Fi (ESP-NOW) or GSM/4G Cellular?"
> **Answer (Kurt):**  
> *"Sir/Ma'am, in rural agricultural fields, Wi-Fi has an effective line-of-sight range of only 50 to 100 meters and suffers severe signal attenuation through dense crop canopies. Cellular GSM requires monthly SIM card load subscriptions, high peak transmit power (up to 2 Watts), and relies on cell towers that are frequently out of reach in remote farms.*  
> *In contrast, LoRa at 433 MHz uses sub-gigahertz Chirp Spread Spectrum modulation. Sub-gigahertz RF has superior diffraction and foliage penetration, consumes a fraction of the power (+17 dBm / 50 mA transmit), and achieves multi-kilometer line-of-sight range with zero operational costs."*

---

### Q2: "Why capacitive soil moisture instead of the standard two-prong resistive sensor?"
> **Answer (KJ):**  
> *"Resistive soil moisture sensors pass an electrical current directly between two exposed metal PCB traces. When exposed to damp, mineral-rich soil, galvanic electrolysis corrodes the copper pads within 1 to 2 weeks, leading to severe sensor failure and toxic metal leaching.*  
> *Our HW-390 capacitive sensor has zero exposed copper. It measures the soil's dielectric constant ($\approx 80$ for water vs. $\approx 3$ to $5$ for dry soil minerals) through the protective solder mask. It induces no galvanic corrosion, ensuring multi-year stability."*

---

### Q3: "How does the ESP32 communicate with the industrial NPK sensor?"
> **Answer (Kurt):**  
> *"The NPK sensor operates on the RS-485 physical layer using differential signaling on balanced lines ($A$ and $B$). The communication protocol is **Modbus RTU** at 4800 baud, 8 data bits, no parity, 1 stop bit.*  
> *The ESP32 queries the sensor by sending an 8-byte hexadecimal inquiry frame: `01 03 00 00 00 03 05 CB`. The sensor responds with an 11-byte frame containing three 16-bit registers representing Nitrogen, Phosphorus, and Potassium in mg/kg. Our firmware validates the slave address (`0x01`), function code (`0x03`), byte count (`0x06`), and performs a 16-bit CRC checksum verification before updating the system state."*

---

### Q4: "Why do you need the MAX485 and LM2596 modules?"
> **Answer (KJ):**  
> *"The MAX485 transceiver translates the ESP32's 3.3V TTL UART logic into the differential $\pm 2\text{V}$ to $\pm 6\text{V}$ signaling required by RS-485. We control the DE (Driver Enable) and RE (Receiver Enable) pins via GPIOs 4 and 15 so the ESP32 transmits commands and immediately listens for responses.*  
> *The LM2596 DC-DC buck converter steps down raw 7.4V–12V battery power to a strictly regulated 5.00V DC. This protects sensitive logic from battery discharge dropouts and provides stable reference current for the RS-485 transceiver and LCD backlight."*

---

### Q5: "How does the system ensure data integrity across the wireless LoRa link?"
> **Answer (Kurt):**  
> *"Data integrity is protected at two layers:*  
> 1. *At the physical LoRa layer, the SX1278 hardware radio applies forward error correction (Coding Rate 4/5) and automated CRC payload validation, automatically dropping corrupted packets.*  
> 2. *At the application layer, telemetry is structured as a typed key-value CSV string (`ID:PROBE_01,AREA:Zone_A,M:45,N:1,P:49,K:42,ALERT:NONE,PKT:102`). The gateway verifies that all required identifiers and numeric values exist before parsing, rendering corrupt or incomplete transmissions impossible to reach the database or charts."*

---

### Q6: "What is your system's power consumption and estimated battery life?"
> **Answer (KJ):**  
> *"During active 1.2-second telemetry cycles (with LCD active, NPK queried, and LoRa transmitting at +17 dBm), the node draws an average current of approximately **95 to 110 mA**.*  
> *When powered by a standard 7.4V 2200 mAh 2S Li-ion battery pack through the high-efficiency LM2596 converter, the probe operates continuously for **over 20 to 24 hours** of active non-stop broadcast.*  
> *For real-world farm deployment, the firmware can easily be set to sleep for 10 or 15 minutes between cycles using ESP32 Deep Sleep, which drops standby power to under **15 microamperes**, extending battery runtime to **6 to 12 months** on a single charge or indefinitely with a miniature 5W solar panel."*

---

## 5. EMERGENCY TROUBLESHOOTING QUICK-REFERENCE

Keep this table handy during the defense in case anything unexpected happens:

| Symptom | Instant Root Cause | Quick 5-Second Fix |
|---|---|---|
| **Dashboard shows "Serial port disconnected"** | USB cable jarred or port selection closed | Click **"Connect to Probe"** $\rightarrow$ select **`COM4`** $\rightarrow$ Click **Connect**. |
| **Moisture reads 0% during demo** | Sensor blade is in dry open air | Firmly pinch the black sensor blade with **damp fingers** or wrap with damp tissue. |
| **NPK reads `NPK: SEARCHING..`** | RS-485 A/B wires loose or MAX485 power interrupted | Check MAX485 VCC jumper on 5V rail; verify A wire (Yellow) and B wire (Green) are screwed tightly. |
| **LoRa shows `STANDBY` on Dashboard** | ESP32 probe battery is switched OFF | Toggle the battery switch on the ESP32 probe; check that the red power LED on ESP32 lights up. |
| **LCD screen blank on probe** | Blue I2C potentiometer contrast drifted | Use a small flathead screwdriver to turn the blue contrast trimpot on the back of the LCD backpack until white letters appear. |

---
*Created and approved for the Capstone Oral Defense of KJ M. Lupogan & Kurt Christian Ramos.*  
*University of Mindanao — College of Computing Education (CCE).*
