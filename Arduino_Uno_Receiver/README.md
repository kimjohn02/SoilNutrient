# Arduino Uno LoRa Receiver (Base Station Gateway)
**Authors:** KJ M. Lupogan & Kurt Christian Ramos (IT10 - 3454)  
**Target:** Arduino Uno + Ai-Thinker Ra-02 (SX1278 433 MHz)

---

## 1. Wiring Checklist (One Wire at a Time)

> ⚠️ **CRITICAL WARNING:** Ra-02 is **3.3V ONLY**. Never connect to the 5V pin!

| Ra-02 Pin | Wire Color | Arduino Uno Pin | Notes |
| :--- | :---: | :--- | :--- |
| **`3.3V`** | 🟥 Red | **`3.3V` Pin** | **3.3V ONLY! (Never use 5V)** |
| **`GND`** | ⬛ Black | **`GND` Pin** | Common Ground |
| **`NSS`** (CS) | 🟧 Orange | **Pin `10`** | SPI Chip Select |
| **`RST`** | 🔘 Grey | **Pin `9`** | Radio Reset |
| **`DIO0`** | 🟪 Purple | **Pin `2`** | Packet Interrupt |
| **`SCK`** | 🟨 Yellow | **Pin `13`** | SPI Clock (Also flashes onboard LED) |
| **`MISO`** | 🟩 Green | **Pin `12`** | SPI Master-In |
| **`MOSI`** | 🟦 Blue | **Pin `11`** | SPI Master-Out |
| **`ANT`** | Coax | **433 MHz Antenna** | Attach antenna before powering on! |

---

## 2. How to Upload Tomorrow:

1. Double-click **`Arduino_Uno_Receiver.ino`** (it will open automatically in Arduino IDE).
2. Go to **Sketch** -> **Include Library** -> **Manage Libraries...** -> install **`LoRa` by Sandeep Mistry** (if not yet installed).
3. Connect your **Arduino Uno** via USB.
4. Go to **Tools** -> **Board** -> select **Arduino Uno**.
5. Go to **Tools** -> **Port** -> select your Arduino COM port.
6. Click the **Upload** arrow (`Ctrl + U`).

---

## 3. How to Connect to Web Dashboard:

1. Make sure the Arduino IDE Serial Monitor is **CLOSED**.
2. Turn on your **ESP32 Soil Probe** using the 12V battery pack.
3. Open `dashboard.html` in Chrome or Edge.
4. Click **Connect to Probe** and select the **Arduino Uno's COM port**.
5. Watch the live soil telemetry update wirelessly over the air!
