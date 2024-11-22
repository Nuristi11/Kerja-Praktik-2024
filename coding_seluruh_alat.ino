#define BLYNK_TEMPLATE_ID "TMPL68A3k_vYX"
#define BLYNK_TEMPLATE_NAME "Kerja Praktik"
#define BLYNK_AUTH_TOKEN "anbtsGZm6LL80zBOUqTH1YHWuHB0VQ42"
char auth[] = BLYNK_AUTH_TOKEN;

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include<LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//include library untuk WiFi
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

//konfigurasi wifi
const char* ssid = "Nuraini net";
const char* password = "pakbur1011";

//variabel sensor
#define sensor_api 0 //D3 - GPIO0

#include <MQ2.h> // MQ2 Gas Sensor Library
int pin_mq2 = A0;

#include <DHT.h>
//dht11
#define DHTPIN D7         // pin DHT11 
#define DHTTYPE DHT11     // TYPE DHT 11  
DHT dht(DHTPIN, DHTTYPE);

//led
#define led1 D4
#define led2 D6

//buzzer
const int buzzerPin = D5;

//URL WahtsApp
String url;

//blynk
BlynkTimer timer;

//siapkan variabel untuk WifiClient
WiFiClient client;

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, password, "blynk.cloud", 80);
  timer.setInterval(2500L, sendSensor);

  pinMode(sensor_api, INPUT);
  pinMode(pin_mq2, INPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  dht.begin();

  //lcd
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("WiFi...");

  //koneksi wifi
  WiFi.hostname("NodeMCU");
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); 
  Serial.println(" ...");
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led1, LOW);
    delay(1000);
    Serial.print(".");
  }
  digitalWrite(led1, HIGH);
  Serial.println('\n');
  Serial.print("Connect to ");
  Serial.println(ssid);
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connect to");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print(ssid);
  delay(5000);
}

void loop() {
  Blynk.run();
  timer.run();
}

void sendSensor() {
  //baca nilai sensor api
  int api = digitalRead(sensor_api);
  Serial.print(F("Api : "));
  Serial.print(api);
  Blynk.virtualWrite(V2, api);
  if (api == 1) {
    lcd.clear();
    lcd.setCursor(8, 1);
    lcd.print(",No Api");
  } else {
    lcd.clear();
    lcd.setCursor(8, 1);
    lcd.print(",Ada Api");
  }
  
  //sensor gas
  int asap = analogRead(pin_mq2);
  Serial.print("|| Asap : ");
  Serial.print(asap);
  Serial.print("|| Asap : ");
  Serial.println(asap);
  lcd.setCursor(0, 1);
  lcd.print("Asap:");
  lcd.print(asap);
  Blynk.virtualWrite(V3, asap);

  //sensor dht11
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  Blynk.virtualWrite(V0, t);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("|| Suhu : ");
  Serial.print(t);
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);
  lcd.print(",");
  Serial.print("|| Kelembaban : ");
  Serial.print(h);
  lcd.print(" H:");
  lcd.print(h);
  delay(1000);

  //kondisi
  //1 = tidak ada api ; 0 = ada api
  if(api == 1 && t < 32 && asap < 500) {
    //tidak ada api, pesan tidak dikirim ke wa
    Serial.println("Kondisi Aman, Tidak Ada Api");
    digitalWrite(led2, LOW);
    digitalWrite(buzzerPin, LOW);
  } else if(api == 1 && t > 32 && asap < 500) {
    Serial.println("Awas Suhu Meningkat");
    digitalWrite(led2, HIGH);
    digitalWrite(buzzerPin, LOW);
  } else if(api == 1 && t > 32 && asap > 500) {
    Serial.println("Awas Suhu dan Asap Meningkat");
    digitalWrite(led2, HIGH);
    digitalWrite(buzzerPin, LOW);
    kirim_wa("Suhu dan asap meningkat. WASPADA ADA API DISEKITARMU!!!"); 
  } else if(api == 1 && t < 32 && asap > 500) {
    Serial.println("Awas Asap Meningkat");
    digitalWrite(led2, HIGH);
    digitalWrite(buzzerPin, LOW);
  } else {
    Serial.println("Awas Ada Api");
    digitalWrite(buzzerPin, HIGH);
    kirim_wa("AWAS!!!\n\nTerdeteksi ada Api di sekitarmu. Segera selamatkan diri Anda!!!\n\nCEPATTT, BAHAYA!!!");
  }
}

void kirim_wa(String pesan) {
  url = "http://api.callmebot.com/whatsapp.php?phone=+6281903969383&text="+ urlencode(pesan) +"&apikey=1898819";
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
