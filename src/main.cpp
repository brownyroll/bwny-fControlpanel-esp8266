#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

#define WIFI_SSID         "Brownyrollz_IoT"
#define WIFI_PASS         "Kittisak20z@"
#define APP_KEY           "6c354318-e37c-4093-bbec-48bb940b48f1"
#define APP_SECRET        "50ec4cd4-d04f-4efa-84b2-b655cabda4bc-60d8f1be-3eb3-41ef-8421-be7079d63c91"
#define RESET_SWITCH_ID   "6a23b0e5977a0619a7540e67"
#define POWER_SWITCH_ID   "6a23b0ba977a0619a7540e3a"
#define RESET_PIN         D5 
#define POWER_PIN         D6 
#define WIFI_LED_PIN      2

unsigned long lastWiFiCheckTime = 0;
unsigned long lastBlinkTime = 0;
const unsigned long wifiCheckInterval = 10000; 

// --------------------------------------------------------
// ฟังก์ชันสำหรับกดปุ่ม Reset (D5)
bool onResetState(const String &deviceId, bool &state) {
  if (state) { 
    Serial.println("Pressing RESET button...");
    digitalWrite(RESET_PIN, LOW);
    delay(500);
    digitalWrite(RESET_PIN, HIGH);
    state = false; 
    return true; 
  }
  return true;
}

// --------------------------------------------------------
// ฟังก์ชันสำหรับกดปุ่ม Power (D6)
bool onPowerState(const String &deviceId, bool &state) {
  if (state) { // ถ้ากด "เปิด" ในแอป
    Serial.println("Pressing POWER button...");
    digitalWrite(POWER_PIN, LOW);  // สั่ง LOW ให้ PNP ทำงาน (Relay ON)
    delay(500);                    // หน่วงเวลา 0.5 วินาที (จำลองการเอานิ้วกด)
    digitalWrite(POWER_PIN, HIGH); // สั่ง HIGH ให้ PNP หยุด (Relay OFF)
    
    state = false; // เด้งปุ่มในแอปกลับไปเป็น OFF
    return true; 
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F(R"EOF(
 _______   __        __  __    __  __      __        ________   ______                        __                          __                                        __ 
|       \ |  \  _  |  \|  \  |  \|  \    /  \      |        \ /      \                      |  \                        |  \                                       |  \
| $$$$$$$\| $$ / \ | $$| $$\ | $$ \$$\  /  $$      | $$$$$$$$|  $$$$$$\  ______   _______  _| $$_      ______    ______ | $$  ______    ______   _______    ______ |  $$
| $$__/ $$| $$/  $\| $$| $$$\| $$  \$$\/  $$       | $$__    | $$   \$$ /      \ |       \|   $$ \    /      \  /      \| $$ /      \  |      \ |       \  /      \|  $$
| $$    $$| $$ $$$\  $$| $$$$\ $$   \$$  $$        | $$  \   | $$      |  $$$$$$\| $$$$$$$\\$$$$$$  |  $$$$$$\|  $$$$$$\| $$|  $$$$$$\ \$$$$$$\| $$$$$$$\|  $$$$$$\|  $$
| $$$$$$$\| $$ $$\$$\$$| $$\$$ $$    \$$$$         | $$$$$   | $$   __ | $$  | $$| $$  | $$ | $$ __ | $$   \$$| $$  | $$| $$| $$  | $$ /      $$| $$  | $$| $$    $$| $$
| $$__/ $$| $$$$  \$$$$| $$ \$$$$    | $$          | $$      | $$__/  \| $$__/ $$| $$  | $$ | $$|  \| $$      | $$__/ $$| $$| $$__/ $$|  $$$$$$$| $$  | $$| $$$$$$$$| $$
| $$    $$| $$$    \$$$| $$  \$$$    | $$          | $$       \$$    $$ \$$    $$| $$  | $$  \$$  $$| $$       \$$    $$| $$| $$    $$ \$$    $$| $$  | $$ \$$     \| $$
 \$$$$$$$  \$$      \$$ \$$   \$$     \$$           \$$        \$$$$$$   \$$$$$$  \$$   \$$   \$$$$  \$$        \$$$$$$  \$$| $$$$$$$   \$$$$$$$ \$$   \$$  \$$$$$$$ \$$
                                                                                                                            | $$                                        
                                                                                                                            | $$                                        
                                                                                                                             \$$                                        
  )EOF"));
  
  Serial.println("\n=========================================");
  Serial.println(" Brownyrollz System is Booting...  ");
  Serial.println(" Initializing Server Control Panel...  ");
  Serial.println("=========================================\n");
  
  pinMode(RESET_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(WIFI_LED_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
  digitalWrite(POWER_PIN, LOW);

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Bwny ESP Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    digitalWrite(WIFI_LED_PIN, !digitalRead(WIFI_LED_PIN)); // กระพริบ LED ขณะรอการเชื่อมต่อครั้งแรก
  }

  Serial.println("\nBwny ESP WiFi connected!");
  Serial.println("IP address: " + WiFi.localIP().toString());
  Serial.println("MAC address: " + WiFi.macAddress());
  Serial.println("WiFi signal strength (RSSI): " + String(WiFi.RSSI()) + " dBm");
  Serial.println("WiFi channel: " + String(WiFi.channel()));
  Serial.println("WiFi encryption type: " + String(WiFi.encryptionType(WiFi.scanNetworks())));
  Serial.println("Status: " + String(WiFi.status()));
  Serial.println("System FControlPanel is Ready!\n");

  SinricProSwitch& resetSwitch = SinricPro[RESET_SWITCH_ID];
  resetSwitch.onPowerState(onResetState);

  SinricProSwitch& powerSwitch = SinricPro[POWER_SWITCH_ID];
  powerSwitch.onPowerState(onPowerState);

  SinricPro.begin(APP_KEY, APP_SECRET);
}

void loop() {
  unsigned long currentMillis = millis();

  // ตรวจสอบสถานะ WiFi
  if (WiFi.status() != WL_CONNECTED) {
    if (currentMillis - lastBlinkTime >= 500) {
      digitalWrite(WIFI_LED_PIN, !digitalRead(WIFI_LED_PIN));
      lastBlinkTime = currentMillis;
    }

    if (currentMillis - lastWiFiCheckTime >= wifiCheckInterval) {
      Serial.println("WiFi connection lost. Attempting to reconnect...");
      WiFi.reconnect(); // สั่งให้เชื่อมต่อใหม่
      lastWiFiCheckTime = currentMillis;
    }
  } else {
    digitalWrite(WIFI_LED_PIN, LOW);
    SinricPro.handle();
  }
}