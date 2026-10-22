#include <ChronosESP32.h>
#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include "gif1.h"
#include "gif2.h"
#include "gif3.h"

// Cấu trúc quản lý danh sách GIF ngẫu nhiên
struct GifItem {
    const uint8_t *data;
    size_t size;
};

const GifItem gif_list[] = {
    { (const uint8_t *)gif1_data, sizeof(gif1_data) },
    { (const uint8_t *)gif2_data, sizeof(gif2_data) },
    { (const uint8_t *)gif3_data, sizeof(gif3_data) }
};
const int NUM_GIFS = sizeof(gif_list) / sizeof(gif_list[0]);
int currentGifIndex = 0;

//================================================================================
// 1. CẤU HÌNH MÀU SẮC & GIAO DIỆN (TỐI ƯU NGOÀI TRỜI & CẢNH BÁO)
//================================================================================
// Chế độ bình thường: Nền sáng tương phản cao ngoài nắng
#define BG_NORMAL            TFT_WHITE
#define COLOR_TIME           TFT_BLACK
#define COLOR_BATTERY        TFT_DARKGREEN
#define COLOR_NAV_ICON       TFT_NAVY
#define COLOR_DISTANCE       TFT_DARKGREEN
#define COLOR_DIRECTIONS     TFT_BLACK

// Chế độ Cuộc gọi đến: Đổi toàn bộ nền sang màu cảnh báo rực rỡ (Đỏ/Cam)
#define BG_ALERT             TFT_RED
#define COLOR_CALL_ALERT     TFT_YELLOW
#define COLOR_CALLER         TFT_WHITE
#define COLOR_NAV_ALERT      TFT_WHITE
#define COLOR_DIST_ALERT     TFT_YELLOW
#define COLOR_DIR_ALERT      TFT_WHITE

// Tọa độ an toàn cho màn hình tròn 240x240 (Phân chia các Zone độc lập, không đè lấn)
#define SCREEN_CENTER_X      120
#define SCREEN_CENTER_Y      120

// Zone 1: Icon dẫn đường 96x96 (Y: 36 -> 131)
#define NAV_ICON_X           72
#define NAV_ICON_Y           36

// Zone 2: Khúc rẽ tiếp theo / khoảng cách rẽ (nav.title - Y: 133 -> 163)
#define NAV_TURN_Y           148
#define NAV_TURN_H           30
#define NAV_TURN_W           200

// Zone 3: Tên đường rẽ / chỉ dẫn phụ (nav.directions - Y: 164 -> 202)
#define NAV_ROAD_Y           183
#define NAV_ROAD_H           38
#define NAV_ROAD_W           186

// Zone 4: Quãng đường tổng còn lại đến đích (nav.distance - Y: 203 -> 225)
#define NAV_DEST_Y           214
#define NAV_DEST_H           22
#define NAV_DEST_W           144

//================================================================================
// 2. KHAI BÁO BIẾN TOÀN CỤC & ĐỐI TƯỢNG
//================================================================================
ChronosESP32 watch("SmartWatch-ESP32");
TFT_eSPI tft = TFT_eSPI();
AnimatedGIF gif;

// Quản lý trạng thái
bool isNavigating        = false;
bool isRinging           = false;
String currentCaller     = "";

// Bộ nhớ đệm tránh vẽ lại gây giật/chớp màn hình (Flicker-free)
uint32_t last_icon_crc   = 0xFFFFFFFF;
String   last_turn_info  = "";
String   last_directions = "";
String   last_dest_dist  = "";
int      last_minute     = -1;
int      last_battery    = -1;
unsigned long lastTimeUpdate     = 0;
unsigned long lastKeepAliveTime  = 0;
bool isDisconnectedScreenInited  = false;

// Prototype hàm chuyển trạng thái
void switchToNavigation();
void switchToClock();

//================================================================================
// 3. TIỆN ÍCH TỐI ƯU HỆ THỐNG & ĐỒ HỌA
//================================================================================

// Lấy màu nền theo trạng thái (Bình thường vs Cuộc gọi)
inline uint16_t getScreenBg() {
    return isRinging ? BG_ALERT : BG_NORMAL;
}

// Xử lý chuỗi: Bỏ dấu tiếng Việt chuẩn xác 100% bằng bảng tra nguyên gốc, lọc sạch Emoji
String cleanVietnamese(String str) {
    if (str.length() == 0) return "";

    // Kiểm tra nhanh: Nếu chuỗi là ASCII thuần túy (không có byte >= 128) thì bỏ qua tra bảng
    bool hasNonAscii = false;
    for (size_t i = 0; i < str.length(); i++) {
        if ((uint8_t)str[i] >= 128) {
            hasNonAscii = true;
            break;
        }
    }
    if (!hasNonAscii) return str;

    // 1. Bảng tra thay thế toàn bộ ký tự tiếng Việt có dấu chuẩn UTF-8
    static const char* const vn_map[][2] = {
        {"á","a"},{"à","a"},{"ả","a"},{"ã","a"},{"ạ","a"},
        {"ă","a"},{"ắ","a"},{"ằ","a"},{"ẳ","a"},{"ẵ","a"},{"ặ","a"},
        {"â","a"},{"ấ","a"},{"ầ","a"},{"ẩ","a"},{"ẫ","a"},{"ậ","a"},
        {"é","e"},{"è","e"},{"ẻ","e"},{"ẽ","e"},{"ẹ","e"},
        {"ê","e"},{"ế","e"},{"ề","e"},{"ể","e"},{"ễ","e"},{"ệ","e"},
        {"í","i"},{"ì","i"},{"ỉ","i"},{"ĩ","i"},{"ị","i"},
        {"ó","o"},{"ò","o"},{"ỏ","o"},{"õ","o"},{"ọ","o"},
        {"ô","o"},{"ố","o"},{"ồ","o"},{"ổ","o"},{"ỗ","o"},{"ộ","o"},
        {"ơ","o"},{"ớ","o"},{"ờ","o"},{"ở","o"},{"ỡ","o"},{"ợ","o"},
        {"ú","u"},{"ù","u"},{"ủ","u"},{"ũ","u"},{"ụ","u"},
        {"ư","u"},{"ứ","u"},{"ừ","u"},{"ử","u"},{"ữ","u"},{"ự","u"},
        {"ý","y"},{"ỳ","y"},{"ỷ","y"},{"ỹ","y"},{"ỵ","y"},
        {"đ","d"},{"Đ","D"},
        {"Á","A"},{"À","A"},{"Ả","A"},{"Ã","A"},{"Ạ","A"},
        {"Ă","A"},{"Ắ","A"},{"Ằ","A"},{"Ẳ","A"},{"Ẵ","A"},{"Ặ","A"},
        {"Â","A"},{"Ấ","A"},{"Ầ","A"},{"Ẩ","A"},{"Ẫ","A"},{"Ậ","A"},
        {"É","E"},{"È","E"},{"Ẻ","E"},{"Ẽ","E"},{"Ẹ","E"},
        {"Ê","E"},{"Ế","E"},{"Ề","E"},{"Ể","E"},{"Ễ","E"},{"Ệ","E"},
        {"Í","I"},{"Ì","I"},{"Ỉ","I"},{"Ĩ","I"},{"Ị","I"},
        {"Ó","O"},{"Ò","O"},{"Ỏ","O"},{"Õ","O"},{"Ọ","O"},
        {"Ô","O"},{"Ố","O"},{"Ồ","O"},{"Ổ","O"},{"Ỗ","O"},{"Ộ","O"},
        {"Ơ","O"},{"Ớ","O"},{"Ờ","O"},{"Ở","O"},{"Ỡ","O"},{"Ợ","O"},
        {"Ú","U"},{"Ù","U"},{"Ủ","U"},{"Ũ","U"},{"Ụ","U"},
        {"Ư","U"},{"Ứ","U"},{"Ừ","U"},{"Ử","U"},{"Ữ","U"},{"Ự","U"},
        {"Ý","Y"},{"Ỳ","Y"},{"Ỷ","Y"},{"Ỹ","Y"},{"Ỵ","Y"}
    };

    // Chỉ replace khi chuỗi thực sự chứa ký tự đó (tránh phân mảnh RAM)
    const size_t total = sizeof(vn_map) / sizeof(vn_map[0]);
    for (size_t i = 0; i < total; i++) {
        if (str.indexOf(vn_map[i][0]) >= 0) {
            str.replace(vn_map[i][0], vn_map[i][1]);
        }
    }

    // 2. Lọc sạch toàn bộ Emoji và ký tự ngoài dải ASCII chuẩn (byte >= 128)
    String clean = "";
    clean.reserve(str.length());
    bool lastWasSpace = false;
    for (size_t i = 0; i < str.length(); i++) {
        uint8_t c = (uint8_t)str[i];
        if (c >= 32 && c <= 126) {
            if (c == ' ') {
                if (!lastWasSpace && clean.length() > 0) {
                    clean += ' ';
                    lastWasSpace = true;
                }
            } else {
                clean += (char)c;
                lastWasSpace = false;
            }
        }
    }
    clean.trim();
    return clean;
}

// In chữ căn giữa màn tròn, tự động xuống dòng nếu vượt quá chiều rộng an toàn
// clearH > 0: Chủ động xóa sạch toàn bộ vùng chữ nhật chỉ định trước khi vẽ, chống đọng pixel cũ
void drawCenteredText(const String& str, int y, int size, uint16_t color, uint16_t bg, int maxW, int clearH = 0) {
    tft.setTextSize(size);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(color, bg);
    
    // Nếu có clearH, xóa sạch toàn bộ vùng an toàn chỉ định trước
    if (clearH > 0) {
        tft.fillRect(SCREEN_CENTER_X - maxW / 2, y - clearH / 2, maxW, clearH, bg);
    }
    if (str.length() == 0) return;

    int totalW = tft.textWidth(str);
    if (totalW <= maxW) {
        if (clearH == 0) {
            int boxH = 8 * size + 4;
            tft.fillRect(SCREEN_CENTER_X - maxW / 2, y - boxH / 2, maxW, boxH, bg);
        }
        tft.drawString(str, SCREEN_CENTER_X, y);
    } else {
        // Tách 2 dòng tại khoảng trắng gần trung tâm nhất
        int mid = str.length() / 2;
        int split = -1;
        int minDiff = 999;
        for (int i = 0; i < str.length(); i++) {
            if (str[i] == ' ') {
                int diff = abs(i - mid);
                if (diff < minDiff) {
                    minDiff = diff;
                    split = i;
                }
            }
        }
        if (split == -1) split = mid;

        String line1 = str.substring(0, split);
        String line2 = str.substring(split + 1);
        line1.trim();
        line2.trim();

        int lineSpacing = 8 * size + 1; // size 2: lineSpacing = 17
        if (clearH == 0) {
            int boxH = lineSpacing * 2 + 6;
            tft.fillRect(SCREEN_CENTER_X - maxW / 2, y - boxH / 2, maxW, boxH, bg);
        }
        tft.drawString(line1, SCREEN_CENTER_X, y - lineSpacing / 2);
        tft.drawString(line2, SCREEN_CENTER_X, y + lineSpacing / 2);
    }
}

// Vẽ Icon mũi tên điều hướng (48x48 scale 2 = 96x96):
// Tự động nhận diện pixel mũi tên và nền icon để hòa vào màu nền màn hình, KHÔNG BỊ Ô VUÔNG NỀN XANH
void drawNavIconScaled(int x, int y, const uint8_t *bitmap, uint16_t arrowColor, uint16_t bgColor) {
    if (!bitmap) return;
    
    // Pixel góc (0,0) luôn là màu nền bao quanh của ảnh icon từ Google Maps
    bool bgBit = (bitmap[0] >> 7) & 0x01;
    
    uint16_t lineBuf[96];
    for (int j = 0; j < 48; j++) {
        for (int i = 0; i < 48; i++) {
            int byteIdx = (i + j * 48) / 8;
            int bitShift = 7 - ((i + j * 48) % 8);
            bool bit = (bitmap[byteIdx] >> bitShift) & 0x01;
            
            // Nếu bit khác với bit nền góc (0,0) thì đó chính là hình MŨI TÊN
            uint16_t pix = (bit != bgBit) ? arrowColor : bgColor;
            
            lineBuf[i * 2]     = pix;
            lineBuf[i * 2 + 1] = pix;
        }
        tft.pushImage(x, y + j * 2,     96, 1, lineBuf);
        tft.pushImage(x, y + j * 2 + 1, 96, 1, lineBuf);
    }
}

// Reset các biến nhớ để ép vẽ lại toàn bộ
void resetUICache() {
    last_icon_crc   = 0xFFFFFFFF;
    last_turn_info  = "";
    last_directions = "";
    last_dest_dist  = "";
    last_minute     = -1;
}

//================================================================================
// 4. CÁC HÀM VẼ GIAO DIỆN (UI RENDERERS CHO MÀN TRÒN)
//================================================================================

// Thanh trạng thái đỉnh màn tròn (Giờ & Pin hoặc Banner Cuộc gọi)
void drawTopBar() {
    uint16_t bg = getScreenBg();
    if (isRinging) {
        drawCenteredText("CUOC GOI DEN!", 18, 2, COLOR_CALL_ALERT, bg, 130);
        drawCenteredText(currentCaller.length() ? currentCaller : "Khach", 34, 2, COLOR_CALLER, bg, 160);
    } else {
        if (isNavigating) {
            String timeStr = watch.getHourZ() + ":" + ((watch.getMinute() < 10) ? "0" : "") + String(watch.getMinute());
            String batStr = (last_battery >= 0) ? (String(last_battery) + "%") : "";
            String status = timeStr + (batStr.length() ? ("   " + batStr) : "");
            drawCenteredText(status, 18, 2, COLOR_TIME, bg, 140);
        }
    }
}

// Cập nhật dữ liệu dẫn đường:
// 1. Chỉ dẫn rẽ / Khoảng cách khúc rẽ tiếp theo (nav.title)
// 2. Tên đường rẽ (nav.directions)
// 3. Quãng đường tổng còn lại đến đích (nav.distance)
void updateNavDisplay(const Navigation &nav, bool force = false) {
    uint16_t bg = getScreenBg();
    uint16_t turnColor = isRinging ? COLOR_DIST_ALERT : COLOR_DISTANCE;
    uint16_t dirColor  = isRinging ? COLOR_DIR_ALERT  : COLOR_DIRECTIONS;
    uint16_t destColor = isRinging ? COLOR_CALL_ALERT : TFT_DARKGREEN;

    // 1. HƯỚNG RẼ / KHOẢNG CÁCH RẼ TIẾP THEO (nav.title - ví dụ: "Ve huong D. Ng. Si Lien" hoặc "200 m")
    String turnInfo = cleanVietnamese(nav.title);
    if (turnInfo == "Chronos" || turnInfo.length() == 0) {
        turnInfo = cleanVietnamese(nav.directions);
    }

    if (force || turnInfo != last_turn_info) {
        last_turn_info = turnInfo;
        
        // Nếu là số đo ngắn (như "200 m", "500 m") dùng Font 4 to; nếu là chuỗi dài dùng Font 2 rõ ràng
        if (last_turn_info.length() <= 8 && (last_turn_info.indexOf("m") >= 0 || last_turn_info.indexOf("km") >= 0)) {
            tft.fillRect(SCREEN_CENTER_X - NAV_TURN_W / 2, NAV_TURN_Y - NAV_TURN_H / 2, NAV_TURN_W, NAV_TURN_H, bg);
            tft.setTextSize(1);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(turnColor, bg);
            tft.drawString(last_turn_info, SCREEN_CENTER_X, NAV_TURN_Y, 4);
        } else {
            drawCenteredText(last_turn_info, NAV_TURN_Y, 2, turnColor, bg, NAV_TURN_W, NAV_TURN_H);
        }
    }

    // 2. TÊN ĐƯỜNG TIẾP THEO (nav.directions - nếu khác nav.title)
    String cleanDir = cleanVietnamese(nav.directions);
    if (cleanDir == "Start navigation on Google maps" || cleanDir == last_turn_info) {
        cleanDir = "";
    }
    if (force || cleanDir != last_directions) {
        last_directions = cleanDir;
        // Xóa sạch toàn bộ khối tên đường (NAV_ROAD_H = 38px) trước khi vẽ -> TRIỆT TIÊU HOÀN TOÀN CHỮ CŨ Ở RÌA TRÊN/DƯỚI
        drawCenteredText(last_directions, NAV_ROAD_Y, 2, dirColor, bg, NAV_ROAD_W, NAV_ROAD_H);
    }

    // 3. QUÃNG ĐƯỜNG TỔNG CÒN LẠI ĐẾN ĐÍCH (nav.distance - ví dụ: "Con 12 km - 18 ph")
    String destDist = "";
    if (nav.distance.length() > 0) {
        String distStr = cleanVietnamese(nav.distance);
        String durStr  = cleanVietnamese(nav.duration);
        if (durStr == "Inactive" || durStr == "Disabled") durStr = "";

        if (distStr.startsWith("Con ") || distStr.startsWith("con ")) {
            destDist = distStr;
        } else {
            destDist = "Con " + distStr;
        }

        if (durStr.length() > 0) {
            destDist += " - " + durStr;
        }
    }
    if (force || destDist != last_dest_dist) {
        last_dest_dist = destDist;
        tft.fillRect(SCREEN_CENTER_X - NAV_DEST_W / 2, NAV_DEST_Y - NAV_DEST_H / 2, NAV_DEST_W, NAV_DEST_H, bg);
        if (last_dest_dist.length() > 0) {
            tft.setTextSize(1);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(destColor, bg);
            // Giữ chữ nằm gọn trong vùng an toàn NAV_DEST_W của màn tròn
            if (tft.textWidth(last_dest_dist, 2) > NAV_DEST_W && last_dest_dist.indexOf(" - ") > 0) {
                String shortDist = last_dest_dist.substring(0, last_dest_dist.indexOf(" - "));
                tft.drawString(shortDist, SCREEN_CENTER_X, NAV_DEST_Y, 2);
            } else {
                tft.drawString(last_dest_dist, SCREEN_CENTER_X, NAV_DEST_Y, 2);
            }
        }
    }
}

// Giao diện Mặt Đồng hồ chờ (Fix lỗi số giờ quá to bị lẹm rìa phải)
void drawClockScreen() {
    uint16_t bg = getScreenBg();
    if (isRinging) {
        drawCenteredText("CUOC GOI DEN!", 65, 4, COLOR_CALL_ALERT, bg, 200);
        drawCenteredText(currentCaller.length() ? currentCaller : "Khach", 115, 3, COLOR_CALLER, bg, 200);
        drawCenteredText("Dang do chuong...", 165, 2, COLOR_CALLER, bg, 170);
        return;
    }

    int curMin = watch.getMinute();
    if (curMin != last_minute) {
        last_minute = curMin;
        String curTime = watch.getHourZ() + ":" + ((curMin < 10) ? "0" : "") + String(curMin);
        
        tft.setTextSize(1);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_TIME, bg);
        
        int timeW = tft.textWidth(curTime, 7);
        tft.fillRect(SCREEN_CENTER_X - timeW / 2 - 6, 85, timeW + 12, 52, bg);
        tft.drawString(curTime, SCREEN_CENTER_X, 110, 7);
    }

    if (last_battery >= 0) {
        String bat = "PIN: " + String(last_battery) + "%";
        drawCenteredText(bat, 32, 2, COLOR_BATTERY, bg, 140);
    }

    drawCenteredText("Chronos BLE", 168, 2, TFT_DARKCYAN, bg, 160);
}

// Chuyển sang giao diện Dẫn đường
void switchToNavigation() {
    isNavigating = true;
    tft.fillScreen(getScreenBg());
    resetUICache();

    Navigation &nav = watch.getNavigation();
    last_icon_crc = 0xFFFFFFFF; // Cho phép vẽ lại icon mới ngay khi gói icon tới

    uint16_t arrowColor = isRinging ? COLOR_NAV_ALERT : COLOR_NAV_ICON;
    drawNavIconScaled(NAV_ICON_X, NAV_ICON_Y, nav.icon, arrowColor, getScreenBg());

    updateNavDisplay(nav, true);
    drawTopBar();
    Serial.println("[UI] >> Da CHUYEN sang giao dien DAN DUONG thanh cong!");
}

// Chuyển về giao diện Đồng hồ chờ
void switchToClock() {
    isNavigating = false;
    tft.fillScreen(getScreenBg());
    resetUICache();

    drawClockScreen();
    Serial.println("[UI] >> Da CHUYEN ve giao dien DONG HO CHO!");
}

// Callback giải mã AnimatedGIF
void GIFDraw(GIFDRAW *pDraw) {
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int iWidth = (pDraw->iWidth > 320) ? 320 : pDraw->iWidth;
    usPalette = pDraw->pPalette;
    int y = pDraw->iY + pDraw->y;
    s = pDraw->pPixels;

    if (pDraw->ucHasTransparency) {
        uint8_t *pEnd = s + iWidth, c, ucTransparent = pDraw->ucTransparent;
        int x = 0, xCount = 0;
        while (x < iWidth) {
            c = ucTransparent - 1;
            d = usTemp;
            while (c != ucTransparent && s < pEnd) {
                c = *s++;
                if (c == ucTransparent) s--;
                else { *d++ = usPalette[c]; xCount++; }
            }
            if (xCount) {
                tft.setSwapBytes(true);
                tft.pushImage(pDraw->iX + x, y, xCount, 1, usTemp);
                tft.setSwapBytes(false);
                x += xCount;
                xCount = 0;
            }
            c = ucTransparent;
            while (c == ucTransparent && s < pEnd) {
                c = *s++;
                if (c == ucTransparent) x++;
                else s--;
            }
        }
    } else {
        for (int x = 0; x < iWidth; x++) usTemp[x] = usPalette[*s++];
        tft.setSwapBytes(true);
        tft.pushImage(pDraw->iX, y, iWidth, 1, usTemp);
        tft.setSwapBytes(false);
    }
}

//================================================================================
// 5. CALLBACKS CHRONOS BLE
//================================================================================
void connectionCallback(bool state) {
    Serial.printf("[BLE] Trang thai ket noi: %s\n", state ? "CONNECTED" : "DISCONNECTED");
}

void notificationCallback(Notification notification) {
    Serial.printf("[NOTIF] App: %s | Title: %s | Msg: %s\n",
        notification.app.c_str(), notification.title.c_str(), notification.message.c_str());
}

void configCallback(Config config, uint32_t a, uint32_t b) {
    // 1. Nhận Icon dẫn đường từ Google Maps
    if (config == CF_NAV_ICON) {
        Serial.printf("[BLE NAV_ICON] pos=%d, CRC=0x%08X\n", a, b);
        if (a == 2) {
            Navigation &nav = watch.getNavigation();
            if (!isNavigating) {
                switchToNavigation();
            }
            if (last_icon_crc != nav.iconCRC) {
                last_icon_crc = nav.iconCRC;
                uint16_t arrowColor = isRinging ? COLOR_NAV_ALERT : COLOR_NAV_ICON;
                drawNavIconScaled(NAV_ICON_X, NAV_ICON_Y, nav.icon, arrowColor, getScreenBg());
                Serial.printf("[UI] Da ve icon moi (CRC: 0x%08X)\n", nav.iconCRC);
            }
        }
    }
    // 2. Nhận dữ liệu text dẫn đường
    else if (config == CF_NAV_DATA) {
        Navigation &nav = watch.getNavigation();
        Serial.printf("[BLE NAV_DATA] a=%d, act=%d, isNav=%d, dist='%s', title='%s', dir='%s', dur='%s'\n",
            a, nav.active, nav.isNavigation, nav.distance.c_str(), nav.title.c_str(), nav.directions.c_str(), nav.duration.c_str());

        bool isNavActive = (a == 1) || nav.active || nav.isNavigation;
        if (nav.duration == "Inactive" || nav.directions == "Start navigation on Google maps") {
            isNavActive = false;
        }

        if (isNavActive) {
            if (!isNavigating) {
                switchToNavigation();
            } else {
                updateNavDisplay(nav);
            }
        } else {
            if (isNavigating) {
                switchToClock();
            }
        }
    }
    // 3. Nhận phần trăm Pin điện thoại (phản hồi từ Keep-Alive)
    else if (config == CF_PBAT) {
        last_battery = (int)b;
        if (isNavigating) drawTopBar();
        else if (watch.isConnected()) drawClockScreen();
    }
}

void ringerCallback(String caller, bool active) {
    isRinging = active;
    if (active) {
        currentCaller = cleanVietnamese(caller);
        if (currentCaller.length() == 0) currentCaller = "Cuoc goi den";
        if (currentCaller.length() > 18) currentCaller = currentCaller.substring(0, 16) + "..";
        Serial.printf("[CUỘC GỌI ĐẾN]: %s\n", currentCaller.c_str());
    } else {
        currentCaller = "";
        Serial.println("[CUỘC GỌI KẾT THÚC]");
    }

    tft.fillScreen(getScreenBg());
    resetUICache();

    if (isNavigating) {
        Navigation &nav = watch.getNavigation();
        uint16_t arrowColor = isRinging ? COLOR_NAV_ALERT : COLOR_NAV_ICON;
        drawNavIconScaled(NAV_ICON_X, NAV_ICON_Y, nav.icon, arrowColor, getScreenBg());
        updateNavDisplay(nav, true);
        drawTopBar();
    } else {
        drawClockScreen();
    }
}

//================================================================================
// 6. SETUP & MAIN LOOP
//================================================================================
void setup() {
    Serial.begin(115200);
    
    // Khởi tạo màn hình TFT LCD tròn 240x240
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(BG_NORMAL);

    // Cấu hình màn hình tròn 240x240 1.28" cho Chronos
    watch.setScreen(CS_240x240_128_CTF);

    // Đăng ký toàn bộ Callbacks
    watch.setConnectionCallback(connectionCallback);
    watch.setNotificationCallback(notificationCallback);
    watch.setConfigurationCallback(configCallback);
    watch.setRingerCallback(ringerCallback);
    
    // Bắt đầu BLE
    watch.begin();

    // Khởi tạo bộ giải mã GIF và chọn ngẫu nhiên 1 GIF khi mở nguồn
    gif.begin(LITTLE_ENDIAN_PIXELS);
    currentGifIndex = esp_random() % NUM_GIFS;
    Serial.printf("[GIF] Khoi dong: Chon ngau nhien GIF so %d/%d\n", currentGifIndex + 1, NUM_GIFS);
}

void loop() {
    watch.loop(); // Duy trì BLE liên tục

    unsigned long now = millis();

    // --- A. CƠ CHẾ KEEP-ALIVE: Gửi gói kích hoạt lên điện thoại chống rớt kết nối ---
    if (watch.isConnected()) {
        unsigned long pingInterval = isNavigating ? 10000 : 30000;
        if (now - lastKeepAliveTime >= pingInterval) {
            lastKeepAliveTime = now;
            watch.setNotifyBattery(true);
            Serial.println("[BLE] >> Keep-Alive Ping sent to Phone");
        }
    }

    // --- B. XỬ LÝ CHẾ ĐỘ MẤT KẾT NỐI (HIỂN THỊ GIF) ---
    if (!watch.isConnected()) {
        if (!isDisconnectedScreenInited) {
            tft.fillScreen(BG_NORMAL);
            isDisconnectedScreenInited = true;
            resetUICache();
            // Đổi ngẫu nhiên GIF mỗi lần mất kết nối quay lại màn hình chờ
            currentGifIndex = esp_random() % NUM_GIFS;
            Serial.printf("[GIF] Disconnected: Phat ngau nhien GIF so %d/%d\n", currentGifIndex + 1, NUM_GIFS);
        }

        if (gif.open((uint8_t *)gif_list[currentGifIndex].data, gif_list[currentGifIndex].size, GIFDraw)) {
            while (gif.playFrame(true, NULL)) {
                watch.loop();
                if (watch.isConnected()) break; // Thoát ngay khi kết nối lại
            }
            gif.close();
        }
    } 
    else {
        // Vừa kết nối lại: Thoát cờ GIF và hiển thị màn hình tương ứng
        if (isDisconnectedScreenInited) {
            tft.fillScreen(getScreenBg());
            isDisconnectedScreenInited = false;
            resetUICache();
            if (isNavigating) switchToNavigation();
            else switchToClock();
        }

        // --- C. CẬP NHẬT CHU KỲ 1 GIÂY ---
        if (now - lastTimeUpdate >= 1000) {
            lastTimeUpdate = now;
            if (isNavigating) {
                drawTopBar(); // Cập nhật đồng hồ nhỏ phía trên
            } else {
                drawClockScreen(); // Cập nhật đồng hồ to màn hình chờ
            }
        }
    }
}
