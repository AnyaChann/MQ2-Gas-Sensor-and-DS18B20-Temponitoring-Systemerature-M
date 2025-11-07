# MQ-2 Gas Sensor & DS18B20 Temperature Monitoring System

## 📋 Mô tả dự án

Hệ thống giám sát khí gas và nhiệt độ sử dụng Arduino Uno với các cảm biến MQ-2 và DS18B20. Hệ thống có khả năng phát hiện khói, khí độc và nhiệt độ cao, kích hoạt cảnh báo âm thanh và đèn LED khi có nguy hiểm.

## 🔧 Phần cứng

### Vi điều khiển

- **Arduino Uno**

### Cảm biến

- **MQ-2 Gas Sensor**: Phát hiện khói và khí gas
- **DS18B20**: Cảm biến nhiệt độ chống nước

### Thiết bị xuất

- **Speaker/Buzzer**: Cảnh báo âm thanh
- **4x LED**: 1 LED cảnh báo + 3 LED báo mode

### Thiết bị nhập

- **Push Button**: Chuyển đổi mode hoạt động

## 📍 Sơ đồ kết nối

| Arduino Pin | Thiết bị | Mô tả |
|-------------|----------|-------|
| **A0** | MQ-2 (Analog) | Đọc giá trị analog khí gas |
| **A1** | DS18B20 (Data) | Dây dữ liệu cảm biến nhiệt độ |
| **D0** | MQ-2 (Digital) | Tín hiệu digital khí gas |
| **D3** | LED Alarm | Đèn LED cảnh báo (đỏ) |
| **D5** | Speaker/Buzzer | Loa cảnh báo |
| **D6** | Push Button | Nút chuyển đổi mode |
| **D9** | LED Mode 1 | LED báo TEST mode (đỏ) |
| **D10** | LED Mode 2 | LED báo NORMAL mode (vàng) |
| **D11** | LED Mode 3 | LED báo HIGH mode (xanh) |
| **VCC** | 5V | Nguồn cung cấp |
| **GND** | Ground | Mass chung |

### Kết nối DS18B20

```
DS18B20    Arduino
VCC   -->  3.3V hoặc 5V
GND   -->  GND
Data  -->  A1 (với điện trở pull-up 4.7kΩ)
```

### Kết nối Button

```
Button    Arduino
Pin 1 --> D6
Pin 2 --> GND
(Sử dụng INPUT_PULLUP - không cần điện trở ngoài)
```

## ⚙️ Chế độ hoạt động

Hệ thống có 3 chế độ hoạt động với **dual-threshold** (ngưỡng kép) cho cả khói và nhiệt độ:

| Mode | LED | Threshold Khói | Threshold Nhiệt độ | Mô tả |
|------|-----|----------------|---------------------|-------|
| **TEST** | D9 (Đỏ) | 50 | 10°C | Chế độ kiểm tra, dễ kích hoạt |
| **NORMAL** | D10 (Vàng) | 300 | 50°C | Chế độ bình thường |
| **HIGH** | D11 (Xanh) | 600 | 100°C | Chế độ nghiêm ngặt, khó kích hoạt |

### 🎯 Tính năng Dual-Threshold

- **Mỗi mode** có 2 threshold riêng biệt: **khói** và **nhiệt độ**
- **Tự động điều chỉnh** cả 2 threshold khi chuyển mode
- **Linh hoạt** cho các môi trường khác nhau (nhà bếp, kho hàng, phòng máy...)

### Chuyển đổi mode

- Nhấn nút tại **D6** để chuyển đổi: TEST → NORMAL → HIGH → TEST...
- LED tương ứng sẽ sáng để báo mode hiện tại
- LED cảnh báo (D3) nháy 1 lần khi đổi mode
- Mỗi mode có threshold riêng cho cả khói và nhiệt độ

## 🚨 Điều kiện cảnh báo

Hệ thống sẽ kích hoạt cảnh báo khi:

1. **Phát hiện khói**: Giá trị analog MQ-2 > threshold khói theo mode
2. **Nhiệt độ cao**: DS18B20 > threshold nhiệt độ theo mode
3. **Khí độc**: Tín hiệu digital MQ-2 = LOW

### Hiệu ứng cảnh báo

- 🔔 **Speaker**: Phát tiếng bíp liên tục (bật/tắt mỗi 100ms)
- 💡 **LED Alarm**: Nháy liên tục (bật/tắt mỗi 100ms)
- 📱 **Serial Monitor**: Hiển thị thông báo cảnh báo chi tiết

## 📊 Dữ liệu Serial Monitor

### Định dạng output

```
Analog Value (A0): 156  |  Digital Value (D0): 1  |  Temperature (A1): 24.5C
Current Mode: NORMAL (Smoke: 300, Temp: 50.0C)
```

### Thông báo cảnh báo

```
FIRE ALARM - SMOKE DETECTED: Level = 450
TEMPERATURE ALARM - HIGH TEMP: 100.0 C
WARNING - TOXIC GAS DETECTED!
```

### Thông báo đổi mode

```
Mode Changed to: HIGH (Smoke: 600, Temp: 100.0C)
```

## 💻 Cấu hình phần mềm

### Thư viện cần thiết

```cpp
#include <OneWire.h>          // v2.3.7
#include <DallasTemperature.h> // v3.11.0
```

### Cấu hình Serial

- **Baud rate**: 115200
- **Tần suất cập nhật**: 1 giây
- **Tần suất đọc nhiệt độ**: 2 giây

### Tham số có thể điều chỉnh

```cpp
#define SPEAKER_VOLUME 100           // Âm lượng loa (0-255)
#define BEEP_DURATION 100            // Thời gian bíp (ms)
#define LED_BLINK_DURATION 100       // Thời gian nháy LED (ms)

// Threshold arrays cho 3 mode
int smokeThresholds[] = {50, 300, 600};      // Test, Normal, High
float tempThresholds[] = {10.0, 50.0, 100.0}; // Test, Normal, High (°C)
```

## 🚀 Hướng dẫn sử dụng

### 1. Cài đặt phần mềm

1. Cài đặt **PlatformIO IDE**
2. Clone hoặc download project
3. Mở project trong PlatformIO
4. Build và upload code lên Arduino Uno

### 2. Kết nối phần cứng

1. Kết nối các thiết bị theo sơ đồ trên
2. Đảm bảo nguồn cung cấp ổn định (5V)
3. Kiểm tra kết nối DS18B20 có điện trở pull-up

### 3. Vận hành

1. Mở **Serial Monitor** với baud rate **115200**
2. Quan sát dữ liệu cảm biến
3. Nhấn nút D6 để thay đổi mode
4. Test cảnh báo bằng cách tạo khói hoặc tăng nhiệt độ

### 4. Kiểm tra hoạt động

- **LED Mode**: Kiểm tra LED báo mode có sáng đúng
- **Cảm biến**: Quan sát giá trị trên Serial Monitor
- **Cảnh báo**: Test bằng mode TEST (threshold = 50)

## 🔍 Troubleshooting

### Speaker không kêu

- Kiểm tra kết nối D5
- Tăng `SPEAKER_VOLUME` lên 200-255
- Thử chuyển sang chân PWM khác (D3, D6, D9, D10, D11)

### Nhiệt độ hiển thị -127°C

- Kiểm tra kết nối DS18B20
- Đảm bảo có điện trở pull-up 4.7kΩ
- Kiểm tra nguồn cung cấp cho DS18B20

### Button không hoạt động

- Kiểm tra kết nối D6 và GND
- Đảm bảo sử dụng INPUT_PULLUP
- Tăng `debounceDelay` nếu button nhạy quá

### MQ-2 không chính xác

- Làm nóng cảm biến 24-48 giờ để ổn định
- Hiệu chỉnh threshold theo môi trường thực tế
- Đảm bảo cảm biến không bị che khuất

## 📈 Phát triển thêm

### Tính năng có thể bổ sung

- **WiFi Module**: Gửi cảnh báo qua internet
- **SD Card**: Lưu trữ dữ liệu lịch sử
- **LCD Display**: Hiển thị thông tin trực quan
- **RTC Module**: Thêm timestamp cho dữ liệu
- **Relay Module**: Điều khiển thiết bị ngoại vi

### Cải tiến code

- Thêm calibration tự động cho MQ-2
- Lưu cấu hình vào EEPROM
- Thêm chế độ tiết kiệm năng lượng
- Cải thiện thuật toán lọc nhiễu

## 🙏 Cảm ơn

- Cộng đồng Arduino
- Thư viện OneWire và DallasTemperature
- PlatformIO team

---

**Lưu ý**: Đây là hệ thống giám sát, không thay thế cho các thiết bị báo cháy chuyên nghiệp. Sử dụng với mục đích học tập và nghiên cứu.
