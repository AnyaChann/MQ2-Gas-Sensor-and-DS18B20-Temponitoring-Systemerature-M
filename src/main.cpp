#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Định nghĩa chân kết nối với các cảm biến và thiết bị
#define MQ2_ANALOG_PIN A0       // Chân A0: Giá trị analog từ MQ-2
#define TEMP_SENSOR_PIN A1      // Chân A1: Cảm biến nhiệt độ DS18B20
#define MQ2_DIGITAL_PIN 0       // Chân D0: Tín hiệu digital từ MQ-2
#define SPEAKER_PIN 5           // Chân D5: Loa cảnh báo
#define PUMP_RELAY_PIN 6        // Chân D6: Relay điều khiển bơm
#define LED_PIN 3               // Chân D3: Đèn LED khẩn cấp
#define BUTTON_PIN 7            // Chân D7: Button để đổi threshold
#define LED_MODE1 9             // Chân D9: LED báo Mode 1 (Test)
#define LED_MODE2 10            // Chân D10: LED báo Mode 2 (Normal)
#define LED_MODE3 11            // Chân D11: LED báo Mode 3 (High)

// Cài đặt cơ sở cho cảnh báo
#define SPEAKER_VOLUME 100       // Âm lượng loa (0-255), 60 = ~24% công suất
#define BEEP_DURATION 100        // Thời gian bíp (ms)
#define BEEP_INTERVAL 100        // Khoảng cách giữa các bíp (ms)
#define LED_BLINK_DURATION 100  // Thời gian sáng của đèn (ms)
#define LED_BLINK_INTERVAL 100  // Khoảng cách giữa các nháy (ms)
// 3 mode threshold cho smoke và temperature
int smokeThresholds[] = {50, 300, 600};     // Test, Normal, High
float tempThresholds[] = {10.0, 50.0, 100.0}; // Test, Normal, High (°C)
String modeNames[] = {"TEST", "NORMAL", "HIGH"};
int currentThresholdIndex = 1;  // Bắt đầu với Normal mode
int currentSmokeThreshold = smokeThresholds[currentThresholdIndex];
float currentTempThreshold = tempThresholds[currentThresholdIndex];

// Biến để lưu trữ dữ liệu sensor
int sensorAnalogValue = 0;
int sensorDigitalValue = 0;
unsigned long lastBeepTime = 0;  // Lưu thời gian bíp cuối cùng
boolean isBeeping = false;       // Trạng thái bíp
unsigned long lastBlinkTime = 0; // Lưu thời gian nháy đèn cuối cùng
boolean isLedOn = false;         // Trạng thái đèn

// Setup OneWire cho DS18B20
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
float currentTemp = 0.0;         // Lưu nhiệt độ hiện tại
unsigned long lastTempReadTime = 0;  // Lưu thời gian đọc nhiệt độ lần cuối
#define TEMP_READ_INTERVAL 2000  // Đọc nhiệt độ mỗi 2 giây

// Biến cảnh báo
boolean smokeDetected = false;   // Phát hiện khói
boolean highTempDetected = false; // Phát hiện nhiệt độ cao

// Biến cho button
boolean lastButtonState = HIGH;
boolean currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Prototype for function used before its definition
void triggerAlarmBeep();
void triggerLedBlink();
void handleButton();
void updateModeLEDs();

void setup() {
  // Khởi tạo Serial với baud rate 115200
  Serial.begin(115200);
  
  // Thiết lập chế độ cho các chân
  pinMode(MQ2_ANALOG_PIN, INPUT);     // A0 là input
  pinMode(TEMP_SENSOR_PIN, INPUT);    // A1 là input (DS18B20)
  pinMode(MQ2_DIGITAL_PIN, INPUT);    // D0 là input (MQ-2 digital)
  pinMode(SPEAKER_PIN, OUTPUT);       // D5 là output cho loa
  pinMode(PUMP_RELAY_PIN, OUTPUT);    // D6 là output cho relay bơm
  pinMode(LED_PIN, OUTPUT);           // D3 là output cho đèn LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // D7 là input với pull-up resistor
  pinMode(LED_MODE1, OUTPUT);         // D9 là output cho LED mode 1
  pinMode(LED_MODE2, OUTPUT);         // D10 là output cho LED mode 2
  pinMode(LED_MODE3, OUTPUT);         // D11 là output cho LED mode 3
  analogWrite(SPEAKER_PIN, 0);        // Tắt loa ban đầu
  digitalWrite(PUMP_RELAY_PIN, HIGH);  // Tắt bơm ban đầu (relay OFF)
  digitalWrite(LED_PIN, LOW);         // Tắt đèn ban đầu
  
  // Khởi tạo cảm biến nhiệt độ DS18B20
  sensors.begin();
  
  // Chờ để serial sẵn sàng (có thể bỏ qua nếu không cần)
  delay(1000);
  
  Serial.println("==== MQ2 Gas Sensor + DS18B20 Temperature Sensor ====");
  Serial.println("System is ready!");
  Serial.print("Current Mode: ");
  Serial.print(modeNames[currentThresholdIndex]);
  Serial.print(" (Smoke: ");
  Serial.print(currentSmokeThreshold);
  Serial.print(", Temp: ");
  Serial.print(currentTempThreshold);
  Serial.println("C)");
  
  // Hiển thị LED mode ban đầu
  updateModeLEDs();
  Serial.println("");
}

void loop() {
  // Xử lý button để đổi threshold
  handleButton();
  
  // Đọc giá trị analog từ A0
  sensorAnalogValue = analogRead(MQ2_ANALOG_PIN);
  
  // Đọc giá trị digital từ D0
  sensorDigitalValue = digitalRead(MQ2_DIGITAL_PIN);
  
  // Đọc nhiệt độ từ DS18B20 (mỗi TEMP_READ_INTERVAL ms)
  unsigned long currentTime = millis();
  if (currentTime - lastTempReadTime >= TEMP_READ_INTERVAL) {
    sensors.requestTemperatures();
    currentTemp = sensors.getTempCByIndex(0);
    lastTempReadTime = currentTime;
  }
  
  // Hiển thị dữ liệu qua Serial Monitor
  Serial.print("Analog Value (A0): ");
  Serial.print(sensorAnalogValue);
  Serial.print("  |  Digital Value (D0): ");
  Serial.print(sensorDigitalValue);
  Serial.print("  |  Temperature (A1): ");
  Serial.print(currentTemp);
  Serial.print("C  |  Pump: ");
  Serial.println(digitalRead(PUMP_RELAY_PIN) ? "ON" : "OFF");
  
  // Kiểm tra điều kiện cảnh báo
  smokeDetected = (sensorAnalogValue > currentSmokeThreshold);
  highTempDetected = (currentTemp > currentTempThreshold);
  
  // Kích hoạt cảnh báo nếu có khói hoặc nhiệt độ cao
  if (smokeDetected || highTempDetected || sensorDigitalValue == LOW) {
    if (smokeDetected) {
      Serial.print("FIRE ALARM - SMOKE DETECTED: Level = ");
      Serial.println(sensorAnalogValue);
    }
    if (highTempDetected) {
      Serial.print("TEMPERATURE ALARM - HIGH TEMP: ");
      Serial.print(currentTemp);
      Serial.println(" C");
    }
    if (sensorDigitalValue == LOW) {
      Serial.println("WARNING - TOXIC GAS DETECTED!");
    }
    
    Serial.println("*** PUMP ACTIVATED - FIRE SUPPRESSION ***");
    
    // Kích hoạt speaker, LED và bơm
    triggerAlarmBeep();
    triggerLedBlink();
    digitalWrite(PUMP_RELAY_PIN, LOW);  // Bật bơm khi có cảnh báo
    
  } else {
    // Tắt speaker, LED và bơm khi an toàn
    analogWrite(SPEAKER_PIN, 0);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(PUMP_RELAY_PIN, HIGH);   // Tắt bơm khi an toàn
    isBeeping = false;
    isLedOn = false;
  }
  
  delay(1000);  // Delay 1 second - update once per second
}

// Function to trigger beep effect continuously
void triggerAlarmBeep() {
  unsigned long currentTime = millis();
  unsigned long timeSinceLastBeep = currentTime - lastBeepTime;
  
  // Simple beep: alternate ON and OFF every 100ms
  if (timeSinceLastBeep >= BEEP_DURATION) {
    if (isBeeping) {
      analogWrite(SPEAKER_PIN, 0);  // Turn off
      isBeeping = false;
    } else {
      analogWrite(SPEAKER_PIN, SPEAKER_VOLUME);  // Turn on
      isBeeping = true;
    }
    lastBeepTime = currentTime;
  }
}

// Function to trigger LED blink effect
void triggerLedBlink() {
  unsigned long currentTime = millis();
  unsigned long timeSinceLastBlink = currentTime - lastBlinkTime;
  
  // Simple blink: alternate ON and OFF every 100ms
  if (timeSinceLastBlink >= LED_BLINK_DURATION) {
    if (isLedOn) {
      digitalWrite(LED_PIN, LOW);  // Turn off
      isLedOn = false;
    } else {
      digitalWrite(LED_PIN, HIGH);  // Turn on
      isLedOn = true;
    }
    lastBlinkTime = currentTime;
  }
}

// Function to handle button press and change smoke threshold
void handleButton() {
  // Read the button state
  int reading = digitalRead(BUTTON_PIN);
  
  // Check if button state changed (for debouncing)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If button state has changed
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      // If button is pressed (LOW because of INPUT_PULLUP)
      if (currentButtonState == LOW) {
        // Change to next mode (3 modes only)
        currentThresholdIndex = (currentThresholdIndex + 1) % 3;
        currentSmokeThreshold = smokeThresholds[currentThresholdIndex];
        currentTempThreshold = tempThresholds[currentThresholdIndex];
        
        // Update mode LEDs
        updateModeLEDs();
        
        // Print new mode
        Serial.print("Mode Changed to: ");
        Serial.print(modeNames[currentThresholdIndex]);
        Serial.print(" (Smoke: ");
        Serial.print(currentSmokeThreshold);
        Serial.print(", Temp: ");
        Serial.print(currentTempThreshold);
        Serial.println("C)");
        
        // Blink alarm LED to indicate change
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
  
  lastButtonState = reading;
}

// Function to update mode indicator LEDs
void updateModeLEDs() {
  // Turn off all mode LEDs first
  digitalWrite(LED_MODE1, LOW);
  digitalWrite(LED_MODE2, LOW);
  digitalWrite(LED_MODE3, LOW);
  
  // Turn on the current mode LED
  switch(currentThresholdIndex) {
    case 0: // TEST mode
      digitalWrite(LED_MODE1, HIGH);
      break;
    case 1: // NORMAL mode
      digitalWrite(LED_MODE2, HIGH);
      break;
    case 2: // HIGH mode
      digitalWrite(LED_MODE3, HIGH);
      break;
  }
}