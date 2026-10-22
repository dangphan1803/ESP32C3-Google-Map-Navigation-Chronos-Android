# 🧭 ESP32-C3 Round Smart Display: Google Maps Navigation with Chronos

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange?logo=platformio)](https://platformio.org/)
[![ESP32-C3](https://img.shields.io/badge/Chip-ESP32--C3-red?logo=espressif)](https://www.espressif.com/)
[![Display](https://img.shields.io/badge/Display-GC9A01_240x240_Round-blue)](#-sơ-đồ-nối-dây-phần-cứng--hardware-wiring)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

[🇻🇳 Tiếng Việt](#-tiếng-việt) | [🇬🇧 English](#-english)

---

# 🇻🇳 Tiếng Việt

Thiết bị hiển thị thông minh dạng màn hình tròn (đồng hồ đeo tay hoặc màn hình phụ gắn ghi-đông xe máy/xe đạp), sử dụng vi điều khiển **ESP32-C3** kết hợp với **màn hình tròn TFT LCD 1.28 inch (GC9A01, độ phân giải 240x240)**. 

Thiết bị giao tiếp không dây với điện thoại Android qua **Bluetooth Low Energy (BLE)** thông qua ứng dụng **Chronos**, hiển thị trực tiếp dữ liệu chỉ đường theo thời gian thực từ **Google Maps**, thông báo cuộc gọi đến nổi bật, và phát hoạt ảnh GIF sinh động khi ở chế độ chờ.

---

## ✨ Tính năng nổi bật

- 🗺️ **Giao diện Dẫn đường Google Maps thời gian thực**:
  - Mũi tên chỉ hướng rẽ kích thước lớn $96\times96\text{px}$ (thuật toán tự động nhận diện màu nền, hòa viền liền mạch, không bị ô vuông viền icon).
  - Khoảng cách đến khúc rẽ tiếp theo / Hành động rẽ (`nav.title`, ví dụ: `200 m`, `Rẽ phải`).
  - Tên tuyến đường tiếp theo (`nav.directions`, tự động ngắt dòng thông minh khi tên đường dài).
  - Quãng đường tổng còn lại & Thời gian di chuyển đến đích (`nav.distance` & `nav.duration`, ví dụ: `Còn 12 km - 18 ph`).
- ⭕ **Thiết kế tối ưu cho màn hình tròn (240x240)**:
  - Phân chia các Zone tọa độ hiển thị độc lập, không bị lẹm hay tràn ra ngoài viền kính cong.
  - Vùng quét xóa chữ (`fillRect`) kích thước cố định theo Zone, loại bỏ 100% hiện tượng đọng chữ cũ ở rìa trên/dưới khi đổi tên đường.
- 🔤 **Bộ khử dấu tiếng Việt & Lọc Emoji chuyên dụng**:
  - Bảng tra chuẩn 134 ký tự tiếng Việt nguyên bản, chuyển đổi chính xác mọi dấu câu sang ASCII chuẩn (ví dụ: `Về hướng Đ. Ng. Sĩ Liên` $\to$ `Ve huong D. Ng. Si Lien`).
  - Tích hợp kiểm tra nhanh `hasNonAscii`, bỏ qua tra bảng khi gặp chuỗi số/chữ tiếng Anh giúp tiết kiệm tối đa chu kỳ CPU.
  - Tự động lọc sạch Emoji và ký tự ngoài chuẩn ASCII, tránh tràn bộ đệm hiển thị.
- 📞 **Cảnh báo cuộc gọi đến rực rỡ**:
  - Tự động chuyển toàn bộ màn hình sang màu đỏ cảnh báo (`TFT_RED`) kèm tên người gọi đến.
  - Mũi tên chỉ đường tự động đảo màu sang trắng sáng (`TFT_WHITE`) trên nền đỏ, giúp người lái xe vừa biết có cuộc gọi vừa không bị lạc hướng.
- 🎞️ **Phát Random 3 GIF ở chế độ chờ (Standby)**:
  - Tận dụng bảng phân vùng mở rộng (`3.875 MB app0`), nạp sẵn cả 3 hình ảnh động GIF $240\times240$ vào bộ nhớ Flash.
  - Sử dụng bộ tạo số ngẫu nhiên phần cứng chân thực của ESP32 (`esp_random()`) để chọn ngẫu nhiên 1 GIF khi mở nguồn và mỗi lần mất kết nối BLE.
- ⚡ **Cơ chế BLE Keep-Alive**:
  - ESP32 chủ động gửi gói tin ping định kỳ lên điện thoại, ngăn chặn Android tắt tiến trình chạy ngầm của Chronos khi chạy đường dài.

---

## 🛠️ Sơ đồ nối dây phần cứng (Hardware Wiring)

### Linh kiện sử dụng:
- **Board điều khiển**: ESP32-C3 DevKit (Flash 4MB, kiến trúc RISC-V).
- **Màn hình**: TFT LCD tròn 1.28" GC9A01, độ phân giải 240x240, giao tiếp SPI.

### Bảng kết nối chân (Wiring Pinout):

| Chân Màn Hình (TFT) | Chân ESP32-C3 | Chức Năng |
| :--- | :--- | :--- |
| **VCC** | `3.3V` | Nguồn cấp 3.3V |
| **GND** | `GND` | Nối đất (Ground) |
| **SCL / SCLK** | `GPIO 6` | SPI Clock |
| **SDA / MOSI** | `GPIO 7` | SPI MOSI (Data In) |
| **CS** | `GPIO 10` | Chip Select |
| **DC** | `GPIO 5` | Data / Command |
| **RST / RES** | `GPIO 4` | Reset chân cứng |
| **BLK / BL** | `3.3V` (hoặc GPIO) | Đèn nền màn hình (Backlight) |

---

## 📱 Cài đặt & Cấu hình Ứng dụng Android (Chronos)

1. Tải và cài đặt ứng dụng **Chronos** trên điện thoại Android ([Tải từ GitHub fbiego/chronos-esp32](https://github.com/fbiego/chronos-esp32) hoặc CH Play).
2. Mở app Chronos và cấp các quyền sau:
   - **Quyền truy cập thông báo (Notification Access):** Bắt buộc để ứng dụng đọc được thông báo dẫn đường của Google Maps.
   - **Quyền Bluetooth & Vị trí (Location).**
3. Trong cài đặt điện thoại Android:
   - **Tắt tối ưu hóa pin** cho ứng dụng Chronos (chọn *Không hạn chế* / *Unrestricted*).
   - Khóa ứng dụng trong màn hình Đa nhiệm (Recent Apps) để tránh Android tự động tắt ứng dụng chạy ngầm.
4. Quét tìm và kết nối với thiết bị BLE tên: `SmartWatch-ESP32`.
5. Bật dẫn đường trên **Google Maps** — màn hình ESP32 sẽ lập tức chuyển từ GIF sang màn hình dẫn đường sống động!

---

## 💻 Biên dịch & Nạp Code (PlatformIO)

### Thư viện sử dụng (PlatformIO tự động tải):
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- [AnimatedGIF](https://github.com/bitbank2/AnimatedGIF)
- [ChronosESP32](https://github.com/fbiego/chronos-esp32)

### Bảng phân vùng (Partition Setup):
Dự án sử dụng file cấu hình phân vùng `partitions_custom.csv` mở rộng bộ nhớ Flash dành cho ứng dụng lên **3.875 MB**, cho phép chứa trọn vẹn cả 3 file GIF $240\times240$ trực tiếp trong Flash của ESP32-C3 (4MB):

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x3E0000,
```

### Lệnh nạp firmware:

```bash
# Clone repository về máy
git clone https://github.com/<your-username>/ESP32C3-Google-Map-Navigation-Chronos.git
cd ESP32C3-Google-Map-Navigation-Chronos

# Biên dịch và nạp trực tiếp qua cổng COM
pio run -t upload --upload-port <CỔNG_COM_CỦA_BẠN>
```

---
---

# 🇬🇧 English

A smart, wearable/motorcycle round navigation display powered by an **ESP32-C3** and a **1.28-inch GC9A01 round TFT LCD (240x240)**. It pairs with an Android smartphone via **BLE** using the **Chronos** app to provide real-time, turn-by-turn Google Maps navigation, high-contrast incoming call alerts, and random animated GIF standby screens.

---

## ✨ Features

- 🗺️ **Turn-by-Turn Navigation Interface**:
  - 96x96 pixel scaled turn maneuver arrow (adaptive corner pixel color blending, zero box artifact).
  - Next maneuver / distance to next turn (`nav.title`, e.g., `200 m`, `Turn right`).
  - Next street name (`nav.directions`, with auto multi-line text wrapping).
  - Total remaining distance & travel time to destination (`nav.distance` & `nav.duration`).
- ⭕ **Circular Screen UI Layout (240x240)**:
  - Strict zone-based coordinate allocation prevents text from clipping outside the circular bezel.
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
