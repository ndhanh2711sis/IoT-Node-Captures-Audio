#include <Arduino.h>
#include "App.h"
#include <WiFi.h>
#include <WiFiClient.h>
// ⚙️ Bước 1: Nhập thông tin Wi-Fi của bạn
const char* ssid = "Quang Hai T3";      // <-- đổi tên Wi-Fi thật của bạn
const char* password = "19741975";  // <-- đổi mật khẩu Wi-Fi thật

// ⚙️ Bước 2: Nhập IP của máy tính chạy Mosquitto (broker)
const char* brokerHost = "192.168.88.113";  // <-- đổi theo IP máy thật
const uint16_t brokerPort = 1883;         // 1883 = cổng MQTT mặc định
// ⚙️ Hàm in thông tin mạng
void net_diag() {
  Serial.println("=== NET INFO ===");
  Serial.printf("WiFi.status = %d (3 = WL_CONNECTED)\n", WiFi.status());
  Serial.printf("ESP IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
  Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
}

// ⚙️ Hàm kiểm tra kết nối TCP (probe)
bool tcp_probe(const char* host, uint16_t port, uint32_t timeoutMs = 3000) {
  WiFiClient client; // đối tượng WiFi TCP
  Serial.printf("TCP probe %s:%u ...\n", host, port);
  
  if (!client.connect(host, port, timeoutMs)) {
    Serial.printf("[FAIL] errno=%d\n", errno);
    return false;
  }

  Serial.println("[OK] TCP connected!");
  client.stop();
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ⚙️ Kết nối Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to Wi-Fi %s", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWi-Fi connected!");

  // ⚙️ In thông tin mạng
  net_diag();

  // ⚙️ Kiểm tra kết nối đến broker
  tcp_probe(brokerHost, brokerPort);
}

void loop() {
  // không làm gì thêm
}