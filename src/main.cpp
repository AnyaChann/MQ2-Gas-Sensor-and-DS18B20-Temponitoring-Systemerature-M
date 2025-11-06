#include <Arduino.h>

// Định nghĩa chân kết nối với sensor MQ2
#define MQ2_ANALOG_PIN A0  // Chân analog để đọc giá trị analog
#define MQ2_DIGITAL_PIN 6  // Chân digital để đọc tín hiệu digital (D6)

// Biến để lưu trữ dữ liệu sensor
int sensorAnalogValue = 0;
int sensorDigitalValue = 0;

void setup() {
  // Khởi tạo Serial với baud rate 9600
  Serial.begin(9600);
  
  // Thiết lập chế độ cho các chân
  pinMode(MQ2_ANALOG_PIN, INPUT);    // A0 là input
  pinMode(MQ2_DIGITAL_PIN, INPUT);   // D6 là input
  
  // Chờ để serial sẵn sàng (có thể bỏ qua nếu không cần)
  delay(2000);
  
  Serial.println("==== MQ2 Gas Sensor ====");
  Serial.println("Hệ thống đã sẵn sàng!");
  Serial.println("");
}

void loop() {
  // Đọc giá trị analog từ A0
  sensorAnalogValue = analogRead(MQ2_ANALOG_PIN);
  
  // Đọc giá trị digital từ D6
  sensorDigitalValue = digitalRead(MQ2_DIGITAL_PIN);
  
  // Hiển thị dữ liệu qua Serial Monitor
  Serial.print("Analog Value (A0): ");
  Serial.print(sensorAnalogValue);
  Serial.print("  |  Digital Value (D6): ");
  Serial.println(sensorDigitalValue);
  
  // Kiểm tra nếu có khí độc được phát hiện (D6 = LOW)
  if (sensorDigitalValue == LOW) {
    Serial.println("⚠️ CẢNH BÁO: Phát hiện khí độc!");
  }
  
  // Tạm dừng 1 giây trước khi đọc lần tiếp theo
  delay(1000);
}