//library lcd i2c
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//library sensor suhu
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//library Wifi
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

//Alamat pengirim data
String jumlah_awal = "http://mmctegal.xyz/data/get_jumlah?ket=1";
String simpan = "http://mmctegal.xyz/data/save?ket=";
String respon, tb_masuk = "tb_masuk", tb_keluar = "tb_keluar";

String jumlah;

//Sensor Infrared
#define in D6
#define out D7

float suhu = 0;

int count = 0, st_masuk = 0, st_keluar = 0;

//Buzzer
#define buzzer D8

void setup() {
  Serial.begin(115200);
  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(false);


  for(uint8_t t = 3; t > 0; t--){
    USE_SERIAL.printf("[SETUP] Tunggu %d...\n",t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("password?", "12345678");

//cek koneksi wifi
  for (int u = 1; u <= 5; u++)
  {
    if((WiFiMulti.run() == WL_CONNECTED))
    {
      USE_SERIAL.println("Wifi Connected");
      USE_SERIAL.flush();
      delay(1000);
    }
    else
    {
      Serial.println("Wifi Disconnected");
      delay(1000);
    }
  }

  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());

  mlx.begin(); 
 
  // put your setup code here, to run once:
  lcd.begin();
  lcd.backlight();
  delay(1000);

  //  Inisialisasi infrared sebagai inputan
  pinMode(in, INPUT);
  pinMode(out, INPUT);
  
  lcd.print("Jumlah Orang");
  lcd.setCursor(0,1);
  
  get_jumlah();
  lcd.print(count);
  pinMode(buzzer, OUTPUT);
}

void loop() {
  suhu = mlx.readObjectTempC();
  
  // put your main code here, to run repeatedly:
  if(digitalRead(in) == LOW){
    if (st_masuk == 0) {
      if (count < 30) {
        if (suhu >= 34 && suhu <= 38) {
          IN();
          buzzerNormal();
  
          kirim_db(suhu, tb_masuk);
        } else {
          Serial.println("Maaf suhu anda tidak normal");
          Serial.print("Suhu : ");
          Serial.println(suhu);
          lcd.clear();
          lcd.print("SUHU UPNORMAL !!");
          lcd.setCursor(0,1);
          lcd.print("SUHU : ");
          lcd.setCursor(7,1);
          lcd.print(suhu);
          buzzerUpnormal();
          delay(2000);
        }
      }
      else {
        Serial.println("Sudah 30 orang");
      }

      st_masuk = 1;
    }
    else {
      Serial.println("Anda sudah dihitung !!");
    }
  }
  else {
    st_masuk = 0;
  }

  if(digitalRead(out) == LOW){
    if (st_keluar == 0) {
      if (count > 0) {
        OUT();
  
        kirim_db(1, tb_keluar);
      }
      else {
        Serial.println("Sorry !!");
      }

      st_keluar = 1;
    } 
    else {
      Serial.println("Anda sudah dihitung !!");
    }
  }
  else {
    st_keluar = 0;
  }

  if(count == 30){
    lcd.clear();
    lcd.print("RUANGAN PENUH");
    lcd.setCursor(0,1);
    lcd.print(count);
    delay(300);
  } else {
    lcd.clear();
    lcd.print("JML PENGUNJUNG");
    lcd.setCursor(0,1);
    lcd.print(count);
    delay(300);
  }

  delay(1000);
}

void IN()
{
  count++;
  lcd.clear();
  lcd.print("JML PENGUNJUNG");
  lcd.setCursor(0,1);
  lcd.print(count);
  lcd.setCursor(4,1);
  lcd.print("SUHU: ");
  lcd.setCursor(10,1);
  lcd.print(suhu);

  Serial.println("Pengunjung masuk");
  Serial.print("Jumlah orang : ");
  Serial.println(count);
  Serial.print("Suhu badan : ");
  Serial.println(suhu);
  
  delay(2000);
}

void OUT()
{
  count--;
  lcd.clear();
  lcd.print("JML PENGUNJUNG");
  lcd.setCursor(0,1);
  lcd.print(count);

  Serial.println("Pengunjung keluar");
  Serial.print("Jumlah orang : ");
  Serial.println(count);
  
  delay(2000);
}

void kirim_db(float ket, String table_insert) {
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    http.begin(simpan + (String) ket +"&table_insert=" + (String) table_insert);

    USE_SERIAL.print("[HTTP] menyimpan ke database ...\n");
    int httpCode = http.GET();

    if(httpCode > 0)
    {
      USE_SERIAL.printf("[HTTP] kode response GET : %d\n", httpCode);

      if(httpCode == HTTP_CODE_OK)
      {
        respon = http.getString();
        USE_SERIAL.println("Jumlah pengunjung : " + String(count));
        delay(200);
      }
    }
    else
    {
      USE_SERIAL.printf("[HTTP] Simpan data gagal, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

// FUNGSI get_jumlah() untuk mengirim data ke dalam website
void get_jumlah() {
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    http.begin(jumlah_awal);

    USE_SERIAL.print("[HTTP] get jumlah dari database ...\n");
    int httpCode = http.GET();

    if(httpCode > 0)
    {
      USE_SERIAL.printf("[HTTP] kode response GET : %d\n", httpCode);

      if(httpCode == HTTP_CODE_OK)
      {
        jumlah = http.getString();
        count = jumlah.toInt();
        USE_SERIAL.println("Jumlah pengunjung : " + jumlah);
        delay(200);
      }
    }
    else
    {
      USE_SERIAL.printf("[HTTP] get jumlah gagal, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

void buzzerNormal(){
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(1000);
}

void buzzerUpnormal(){
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(1000);
}
