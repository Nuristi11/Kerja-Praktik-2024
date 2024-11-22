#define BLYNK_TEMPLATE_ID "TMPL68A3k_vYX"
#define BLYNK_TEMPLATE_NAME "Kerja Praktik"
#define BLYNK_AUTH_TOKEN "anbtsGZm6LL80zBOUqTH1YHWuHB0VQ42"
char auth[] = BLYNK_AUTH_TOKEN;

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>

//include library untuk WiFi
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

//konfigurasi wifi
const char* ssid = "Nikma2";
const char* password = "12345678";

//variabel sensor
#define sensor_api 0 //D3 - GPIO0
#define pin_led 2 //D4 = GPIO2
#define pin_buzzer 15 //D5 = GPIO15

//variabel untuk menampung URL
String url;

//siapkan variabel untuk WifiClient
WiFiClient client;

//blynk
BlynkTimer timer;

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, password, "blynk.cloud", 80);

  pinMode(sensor_api, INPUT);
  pinMode(pin_led, OUTPUT);
  pinMode(pin_buzzer, OUTPUT);

  //koneksi ke wifi
  WiFi.hostname("NodeMCU");
  WiFi.begin(ssid, password);

  //uji koneksi wifi
  while(WiFi.status() != WL_CONNECTED) {
    digitalWrite(pin_led, LOW);
    delay(500);
  }

  //apabila terkoneksi
  digitalWrite(pin_led, HIGH);
}

void loop() {
  //baca nilai sensor api
  int api = digitalRead(sensor_api);
  Serial.print(F("Api : "));
  Serial.print(api);
  if(api==1) {
    //tidak ada api, pesan whatsapp tidak dikirim
    Serial.println("Tidak Ada Api");
    digitalWrite(pin_buzzer, LOW);
    Blynk.virtualWrite(V2, api);

  } else {
    Serial.println("Ada Titik Api");
    digitalWrite(pin_buzzer, HIGH);
    Blynk.virtualWrite(V2, api);
    //kirim pesan whatsapp (WA) ke WhatsApp Bot
    kirim_wa("Awas ada api, segera selamatkan diri anda!!");
  }
  delay(1000);
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
