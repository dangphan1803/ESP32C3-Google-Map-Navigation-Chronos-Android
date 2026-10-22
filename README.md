# 🧭 ESP32-C3 Round Smart Display: Google Maps Navigation with Chronos

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange?logo=platformio)](https://platformio.org/)
[![ESP32-C3](https://img.shields.io/badge/Chip-ESP32--C3-red?logo=espressif)](https://www.espressif.com/)
[![Display](https://img.shields.io/badge/Display-GC9A01_240x240_Round-blue)](#hardware-wiring)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

A smart, wearable/motorcycle round navigation display powered by an **ESP32-C3** and a **1.28-inch GC9A01 round TFT LCD (240x240)**. It pairs with an Android smartphone via **BLE** using the **Chronos** app to provide real-time, turn-by-turn Google Maps navigation, incoming call alerts, and animated GIF standby screens.

---

## ✨ Key Features

- 🗺️ **Turn-by-Turn Navigation Interface**:
  - 96x96 pixel scaled turn maneuver arrow (adaptive color blending, no box artifact).
  - Next maneuver / distance to next turn (`nav.title`, e.g., `200 m`, `Re phai`).
  - Next street name (`nav.directions`, with auto multi-line text wrapping).
  - Total remaining distance & travel time to destination (`nav.distance` & `nav.duration`).
- ⭕ **Circular Screen UI Layout (240x240)**:
  - Strict zone-based coordinate allocation prevents text from clipping outside the glass bezel.
  - Dedicated wipe boundaries eliminate lingering old text fragments when street names change.
- 🔤 **Vietnamese Transliteration & Emoji Sanitizer**:
  - Full 134-character lookup table converts Vietnamese accented text to clean ASCII without font corruption.
  - Fast-path ASCII check (`hasNonAscii`) for zero overhead on numeric & standard English data.
  - Strips non-printable symbols & emojis automatically.
- 📞 **High-Contrast Incoming Call Alert**:
  - Full-screen high-contrast red alert (`TFT_RED`) with caller name.
  - Automatically inverts navigation arrow to bright white on red, keeping live navigation visible while ringing.
- 🎞️ **Multi-GIF Standby Mode (Randomized)**:
  - Extended partition table (`3.875 MB app0`) accommodates 3 full 240x240 GIF animations in Flash memory.
  - Uses ESP32 hardware True Random Number Generator (`esp_random()`) to select a random GIF on boot and on every BLE disconnect.
- ⚡ **BLE Keep-Alive Ping**:
  - Periodic ping keeps the BLE link active and prevents Android OS battery optimization from killing background navigation updates.

---

## 🛠️ Hardware & Pinout

### Components:
- **MCU**: ESP32-C3 DevKit (4MB Flash, RISC-V)
- **Display**: 1.28-inch Round TFT LCD, 240x240, GC9A01 SPI Driver

### Wiring Table:
| Display Pin | ESP32-C3 GPIO | Description |
| :--- | :--- | :--- |
| **VCC** | `3.3V` | Power Supply |
| **GND** | `GND` | Ground |
| **SCL / SCLK** | `GPIO 6` | SPI Clock |
| **SDA / MOSI** | `GPIO 7` | SPI MOSI |
| **CS** | `GPIO 10` | Chip Select |
| **DC** | `GPIO 5` | Data / Command |
| **RST / RES** | `GPIO 4` | Hardware Reset |
| **BLK / BL** | `3.3V` (or GPIO) | Backlight LED |

---

## 📱 Mobile App Configuration (Chronos)

1. Download and install **Chronos** on your Android smartphone ([Chronos on GitHub](https://github.com/fbiego/chronos-esp32) or Google Play Store).
2. Open Chronos and grant:
   - **Notification Access** (required to read Google Maps navigation notifications).
   - **Bluetooth & Location** permissions.
3. In Android Settings:
   - **Disable Battery Optimization** for Chronos (set to *Unrestricted*).
   - Lock Chronos in the recent apps menu so Android won't close it in the background.
4. Scan and connect to `SmartWatch-ESP32`.
5. Start navigation on **Google Maps** — the display will instantly switch from GIF standby to live navigation mode!

---

## 💻 Software & Compilation (PlatformIO)

### Dependencies (automatically handled by PlatformIO):
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- [AnimatedGIF](https://github.com/bitbank2/AnimatedGIF)
- [ChronosESP32](https://github.com/fbiego/chronos-esp32)

### Partition Setup:
This project uses a custom partition scheme (`partitions_custom.csv`) to provide `3.875 MB` of application Flash, allowing all 3 high-resolution GIF animations to fit inside the internal 4MB Flash:

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x3E0000,
```

### Build and Flash:

```bash
# Clone the repository
git clone https://github.com/<your-username>/ESP32C3-Google-Map-Navigation-Chronos.git
cd ESP32C3-Google-Map-Navigation-Chronos

# Compile and upload via PlatformIO
pio run -t upload --upload-port <YOUR_COM_PORT>
```

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
