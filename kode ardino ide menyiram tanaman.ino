#include <ESP8266WiFi.h>

// Konfigurasi WiFi
const char *ssid = "KOST_2";
const char *password = "tanyoadam";

// IP Address Server yang terpasang usbserver
const char *host = "192.168.1.11";

// Pin yang terhubung ke sensor kelembapan tanah
const int soilMoisturePin = A0;

// Pin yang terhubung ke relay untuk mengontrol pompa
const int pumpRelayPin = D8;

// Ambang batas kelembapan tanah untuk mengaktifkan pompa
const int moistureThreshold = 500;

void setup() {
  // Inisialisasi pin sebagai input atau output
  pinMode(soilMoisturePin, INPUT);
  pinMode(pumpRelayPin, OUTPUT);

  // Matikan pompa saat awalnya
  digitalWrite(pumpRelayPin, LOW);

  // Mulai serial communication untuk debugging
  Serial.begin(19200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Jika koneksi berhasil, maka akan muncul address di serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Membaca nilai kelembapan tanah
  int soilMoistureValue = analogRead(soilMoisturePin);

  // Debugging: Tampilkan nilai kelembapan tanah pada Serial Monitor
  Serial.print("Kelembapan Tanah: ");
  Serial.println(soilMoistureValue);

  // Periksa kelembapan tanah terhadap ambang batas
  if (soilMoistureValue < moistureThreshold) {
    // Jika kelembapan tanah di bawah ambang batas, aktifkan pompa
    digitalWrite(pumpRelayPin, HIGH);
    Serial.println("Aktifkan Pompa!");

    // Kirim data ke server
    sendSensorData(soilMoistureValue);
  } else {
    // Jika kelembapan tanah di atas ambang batas, matikan pompa
    digitalWrite(pumpRelayPin, LOW);
    Serial.println("Matikan Pompa!");
  }

  // Tunda sebentar sebelum membaca kelembapan tanah kembali
  delay(1000);
}

// Fungsi untuk mengirim data sensor ke server
void sendSensorData(int data) {
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // Isi Konten yang dikirim adalah alamat ip si esp
  String url = "/soilmoisture/simpan-data.php?data=";
  url += data;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // Mengirimkan Request ke Server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 1000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}
