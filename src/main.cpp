#include <ChronosESP32.h>
#include <TFT_eSPI.h> 
#include <malloc.h>

//======================Prototype Declarations==========================================
void displayText(const char* text, int x, int y, uint32_t nail, int textSize, uint32_t textColor);
void configCallback(Config config, uint32_t a, uint32_t b);
void drawBitmapScaled(int x, int y, const uint8_t *bitmap, int w, int h, int scale, uint32_t color, uint32_t bgcolor); 
String cleanVietnamese(String str);
void displayTextSplit(const char* text, int x, int y, int textSize, uint32_t textColor);
void drawDateTime(int x, int y, uint32_t textColor, uint32_t bgColor);
void ringerCallback(String caller, bool active) ;
//======================Define Variables==========================================
#define navigation_icon_color  TFT_NAVY

#define background_color  TFT_WHITE
#define navigation_text_color  TFT_BLACK
#define distance_color  TFT_DARKGREEN
#define title_color  TFT_RED
//======================Global Variables==========================================
ChronosESP32 watch("SmartWatch-ESP32");
TFT_eSPI tft = TFT_eSPI(); 
uint32_t last_icon_crc = 0xFFFFFFFF;
String last_distance = "";
String last_directions = "";
String last_title = "";
String last_battery = "";
int last_minute = -1;
String last_time_str = "";
unsigned long lastTimeUpdate = 0;
bool isRinging = false;
String last_caller = "";

//======================Setup==================================
void setup() {
    Serial.begin(115200);
    tft.init();    // Khởi tạo phần cứng màn hình TFT
    tft.setRotation(2); // Chỉnh hướng xoay màn hình phù hợp
    tft.fillScreen(background_color); // Xóa sạch màn hình về màu trắng, mai mốt thử bằng restart bằng GPIO
    watch.setConfigurationCallback(configCallback);// Đăng ký callback và chạy BLE
    watch.begin();
    watch.setRingerCallback(ringerCallback);
    Serial.println("Hệ thống sẵn sàng. Chờ dữ liệu Maps...");
}
 //=====================Loop==================================
void loop() {
    watch.loop(); // Duy trì BLE ngầm liên tục
    if (millis() - lastTimeUpdate >= 5000) {
        lastTimeUpdate = millis();
        drawDateTime(120, 15, TFT_BLACK, TFT_WHITE);
    }
}

//======================Functions=================================================================================================================
void configCallback(Config config, uint32_t a, uint32_t b)
{
    if (config == CF_NAV_ICON && a == 2) // Khi nhận đủ dữ liệu Icon Maps (a == 2 = Đủ byte vẽ)
    {
        Navigation nav = watch.getNavigation();
        if (last_icon_crc != nav.iconCRC) // Chỉ vẽ lại khi mũi tên đổi hướng (CRC thay đổi) để tránh chớp màn hình
        {
            last_icon_crc = nav.iconCRC;
            drawBitmapScaled(0, 65, nav.icon, 48, 48, 2, navigation_icon_color, background_color);
            Serial.println("[MÀN HÌNH ĐÃ VẼ BIỂU TƯỢNG] !");
        }
    }
    if (config == CF_NAV_DATA) // Nhận thông tin văn bản từ map
    {
        Navigation nav = watch.getNavigation();
        
        // Serial.println("====================== ");
        // Serial.print("Khoảng cách còn: ");
        // Serial.println(nav.distance); 
        // Serial.print("Thời gian còn: ");
        // Serial.println(nav.duration);
        // Serial.print("Tên đường/Hướng đi: ");
        // Serial.println(nav.directions); 
        // Serial.print("Quẹo: ");
        // Serial.println(nav.title); 
        //---------------------------------------------------------------------------------------------------------------------------------
        displayText(last_title.c_str(), 98, 90, CL_DATUM, 4, background_color);
        displayText(nav.title.c_str(), 98, 90, CL_DATUM, 4, title_color);
        last_title = nav.title;
        displayText(last_distance.c_str(), 98, 130, CL_DATUM, 3, background_color);
        displayText(nav.distance.c_str(), 98, 130, CL_DATUM, 3, distance_color);
        last_distance = nav.distance;
        String cleanText = cleanVietnamese(nav.directions);
        displayText(last_directions.c_str(), 120, 180, MC_DATUM, 2, background_color);
        displayText(cleanText.c_str(), 120, 180, MC_DATUM, 2, navigation_text_color);
        last_directions = cleanText;
    }
    
    if (config == CF_PBAT) 
    {
        String current_battery = String(b) + "%";
        displayText(last_battery.c_str(), 185, 40, MC_DATUM, 2, background_color);
        displayText(current_battery.c_str(), 185, 40, MC_DATUM, 2, TFT_DARKGREEN);
        last_battery = current_battery;
    }

    malloc_trim(0);
}

void drawBitmapScaled(int x, int y, const uint8_t *bitmap, int w, int h, int scale, uint32_t color, uint32_t bgcolor) {
    tft.fillRect(x, y, w * scale, h * scale, bgcolor);
    // Duyệt qua từng pixel của ảnh gốc
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int byteIdx = (i + j * w) / 8;
            int bitShift = 7 - ((i + j * w) % 8);
            bool pixel = (bitmap[byteIdx] >> bitShift) & 0x01;
            if (pixel) {
                tft.fillRect(x + (i * scale), y + (j * scale), scale, scale, color);
            }
        }
    }
}

String cleanVietnamese(String str) {
    str.replace("á", "a"); str.replace("à", "a"); str.replace("ả", "a"); str.replace("ã", "a"); str.replace("ạ", "a");
    str.replace("ă", "a"); str.replace("ắ", "a"); str.replace("ằ", "a"); str.replace("ẳ", "a"); str.replace("ẵ", "a"); str.replace("ặ", "a");
    str.replace("â", "a"); str.replace("ấ", "a"); str.replace("ầ", "a"); str.replace("ẩ", "a"); str.replace("ẫ", "a"); str.replace("ậ", "a");
    
    str.replace("é", "e"); str.replace("è", "e"); str.replace("ẻ", "e"); str.replace("ẽ", "e"); str.replace("ẹ", "e");
    str.replace("ê", "e"); str.replace("ế", "e"); str.replace("ề", "e"); str.replace("ể", "e"); str.replace("ễ", "e"); str.replace("ệ", "e");
    
    str.replace("í", "i"); str.replace("ì", "i"); str.replace("ỉ", "i"); str.replace("ĩ", "i"); str.replace("ị", "i");
    
    str.replace("ó", "o"); str.replace("ò", "o"); str.replace("ỏ", "o"); str.replace("õ", "o"); str.replace("ọ", "o");
    str.replace("ô", "o"); str.replace("ố", "o"); str.replace("ồ", "o"); str.replace("ổ", "o"); str.replace("ỗ", "o"); str.replace("ộ", "o");
    str.replace("ơ", "o"); str.replace("ớ", "o"); str.replace("ờ", "o"); str.replace("ở", "o"); str.replace("ỡ", "o"); str.replace("ợ", "o");
    
    str.replace("ú", "u"); str.replace("ù", "u"); str.replace("ủ", "u"); str.replace("ũ", "u"); str.replace("ụ", "u");
    str.replace("ư", "u"); str.replace("ứ", "u"); str.replace("ừ", "u"); str.replace("ử", "u"); str.replace("ữ", "u"); str.replace("ự", "u");
    
    str.replace("ý", "y"); str.replace("ỳ", "y"); str.replace("ỷ", "y"); str.replace("ỹ", "y"); str.replace("ỵ", "y");
    
    str.replace("đ", "d"); str.replace("Đ", "D");
    
    // Thêm chữ hoa nếu cần
    str.replace("Á", "A"); str.replace("À", "A"); str.replace("Â", "A");
    str.replace("É", "E"); str.replace("È", "E"); str.replace("Ê", "E");
    str.replace("Ó", "O"); str.replace("Ò", "O"); str.replace("Ô", "O");
    
    return str;
}

void displayText(const char* text, int x, int y, uint32_t nail, int textSize, uint32_t textColor) {
    String str = String(text);
    int maxWidth = 220; // Chiều rộng tối đa cho phép trên 1 dòng
    
    tft.setTextSize(textSize);
    int totalWidth = tft.textWidth(str);

    if (totalWidth <= maxWidth) {
        // --- CHỮ NGẮN: Vẽ 1 dòng bình thường ---
        tft.setTextColor(textColor); 
        tft.setTextDatum(nail);
        tft.drawString(str.c_str(), x, y);
    } 
    else {
        // --- CHỮ DÀI: Tách làm 2 dòng ---
        int midIndex = str.length() / 2;
        int splitIndex = -1;
        int minDistance = 999;
        // Tìm khoảng trắng gần giữa nhất để ngắt dòng
        for (int i = 0; i < str.length(); i++) {
            if (str[i] == ' ') {
                int distance = abs(i - midIndex);
                if (distance < minDistance) {
                    minDistance = distance;
                    splitIndex = i;
                }
            }
        }
        
        if (splitIndex == -1) {
            splitIndex = midIndex;
        }
        String line1 = str.substring(0, splitIndex);
        String line2 = str.substring(splitIndex);
        line2.trim();
        int lineSpacing = 8 * textSize + 2; // Khoảng cách dịch chuyển dòng theo trục Y
        tft.setTextColor(textColor); 
        tft.setTextDatum(nail); // Giữ nguyên kiểu căn lề bạn truyền vào (ví dụ MC_DATUM hoặc CL_DATUM)
        // Vẽ 2 dòng lệch nhau theo trục Y quanh tâm y ban đầu
        tft.drawString(line1.c_str(), x, y - (lineSpacing / 2));
        tft.drawString(line2.c_str(), x, y + (lineSpacing / 2));
    }
}
void drawDateTime(int x, int y, uint32_t textColor, uint32_t bgColor) {
    int current_min = watch.getMinute();
    
    // Chỉ vẽ lại khi số phút thực sự thay đổi để tránh chớp màn hình vô ích
    if (current_min != last_minute) {
        last_minute = current_min;

        // 1. Định dạng Giờ và Phút dạng 24h (luôn có 2 chữ số)
        String hourStr = watch.getHourZ(); // Trả về giờ 24h định dạng "00"-"23"
        String minStr = (current_min < 10) ? "0" + String(current_min) : String(current_min);
        String currentTime = hourStr + ":" + minStr;

        // 2. Xóa chữ cũ bằng cách vẽ đè màu nền
        if (last_time_str != "") {
            displayText(last_time_str.c_str(), x, y, MC_DATUM, 2, bgColor);
        }

        // 3. Vẽ giờ mới nét căng lên màn hình
        displayText(currentTime.c_str(), x, y, MC_DATUM, 2, textColor);

        // 4. Lưu lại chuỗi vừa vẽ để xóa cho lần sau
        last_time_str = currentTime;
    }
}
void ringerCallback(String caller, bool active) {
    isRinging = active; // Cập nhật trạng thái cuộc gọi để tạm dừng các hình vẽ khác nếu cần

    if (active) {
        // --- 1. CÓ CUỘC GỌI ĐẾN ---
        Serial.println("Cuộc gọi đến từ: " + caller);
        String cleanCaller = cleanVietnamese(caller);// Lọc tiếng Việt cho tên người gọi để không lỗi font
        // Xóa tên người gọi cũ (nếu có) trước khi vẽ đè
        if (last_caller != "") {
            displayText(last_caller.c_str(), 37, 40, MC_DATUM, 2, background_color); // Vẽ đè màu trắng
        }
        // Vẽ tiêu đề "CUỘC GỌI ĐẾN" màu đỏ cực nổi bật
        displayText("CUOC GOI DEN!", 37, 60, CL_DATUM, 2, TFT_RED);
        // Vẽ tên người gọi (Y = 140)
        displayText(cleanCaller.c_str(), 37, 40, CL_DATUM, 2, TFT_BLACK);
        last_caller = cleanCaller;
    } 
    else {
        // --- 2. CUỘC GỌI KẾT THÚC (Cúp máy / Bắt máy) ---
        Serial.println("Cuộc gọi kết thúc");

        // Xóa sạch thông báo cuộc gọi bằng cách vẽ đè màu nền (TFT_WHITE)
        displayText("CUOC GOI DEN!", 37, 60, CL_DATUM, 2, background_color);
        if (last_caller != "") {
            displayText(last_caller.c_str(), 37, 40, CL_DATUM, 2, TFT_WHITE);
            last_caller = ""; // Reset
        }

        // Mẹo: Sau khi xóa xong, bạn có thể gọi hàm vẽ lại giao diện chỉ đường/giờ giấc tại đây
    }
}