#define BLYNK_TEMPLATE_ID "TMPL68A3k_vYX"
#define BLYNK_TEMPLATE_NAME "Kerja Praktik"
#define BLYNK_AUTH_TOKEN "anbtsGZm6LL80zBOUqTH1YHWuHB0VQ42"
char auth[] = BLYNK_AUTH_TOKEN;

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

//dht11
#define DHTPIN D7         // pin DHT11 
#define DHTTYPE DHT11     // TYPE DHT 11  
DHT dht(DHTPIN, DHTTYPE);

//konfigurasi wifi
char ssid[] = "Nikma2";  // nama wifi
char pass[] = "12345678";  // password wifi

//variabel untuk menampung URL
String url;

//siapkan variabel untuk WifiClient
WiFiClient client;

//blynk
BlynkTimer timer;

void setup() {
  Serial.begin(9600);
  pinMode(DHTPIN, INPUT);
  dht.begin();
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
}

void loop() {
  //sensor DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  Serial.print("|| Suhu : ");
  Serial.print(t);
  Serial.print("|| Kelembaban : ");
  Serial.println(h);

  if (t <= 32) {
    Serial.print("Suhu Ruangan Aman");
  } else {
    Serial.print("Suhu Ruangan Tinggi");
    kirim_wa("Waspada suhu meningkat, silahkan cek kondisi sekitar!!");
  }
  delay(2000);
}

void kirim_wa(String pesan) {
  url = "http://api.callmebot.com/whatsapp.php?phone=+6281903969383&text="+ urlencode(pesan) +"&apikey=5175219";
  //kirim pesan
  postData();
}

void postData() {
  //siapkan variabel untuk menampung status pesan terkirim atau tidak
  int httpCode;
  //siapkan variabel untuk protokol http yang akan terkoneksi ke server callmebot.com
  HTTPClient http;
  //eksekusi link URL
  http.begin(client, url);
  httpCode = http.POST(url);
  //uji nilai variabel httpCode
  if(httpCode==200) {
    Serial.println("Notifikasi WhatsApp Berhasil Terkirim");
  } else {
    Serial.println("Notifikasi WhatsApp Gagal Terkirim");
  }
  http.end();
}

String urlencode(String str) {
  String encodedString="";
  char c;
  char code0, code1, code2;
  for(int i=0; i<str.length(); i++) {
    c = str.charAt(i);
    //jika ada spasi kosong diantara pesan yg akan dikirim, maka ganti dengan tanda +
    if(c == ' ') {
      encodedString += '+';
    } else if(isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c>>4)&0xf;
      code0 = c+'0';
      if(c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }
  Serial.println(encodedString);
  return encodedString;
}