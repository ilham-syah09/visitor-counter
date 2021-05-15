//library lcd i2c
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//library Wifi
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

//Alamat pengirim data
String simpan = "http://192.168.43.35/mmcall/Data/save?ket=";
String respon, tb_masuk = "tb_masuk", tb_keluar = "tb_keluar";

//Sensor Infrared
#define in D6
#define out D7

int count = 0, st_masuk = 0, st_keluar = 0;

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
  WiFiMulti.addAP("password?","oreosan09");

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

  
  // put your setup code here, to run once:
  lcd.begin();
  delay(1000);
  pinMode(in, INPUT);
  pinMode(out, INPUT);
  lcd.print("Jumlah Orang");
  lcd.setCursor(0,1);
  lcd.print(count);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(in) == LOW){
    if (st_masuk == 0) {
      if (count <= 30) {
        IN();
  
        kirim_db(1, tb_masuk);
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
    if (count > 0) {
      OUT();

      kirim_db(1, tb_keluar);
    }
    else {
      Serial.println("Sorry !!");
    }
  }

  if(count == 30){
    lcd.clear();
    lcd.print("RUANGAN PENUH");
    lcd.setCursor(0,1);
    lcd.print(count);
    delay(300);
  }
}

void IN()
{
  count++;
  lcd.clear();
  lcd.print("JUMLAH ORANG");
  lcd.setCursor(0,1);
  lcd.print(count);
  delay(300);
}

void OUT()
{
  count--;
  lcd.clear();
  lcd.print("JUMLAH ORANG");
  lcd.setCursor(0,1);
  lcd.print(count);
  delay(300);
}

void kirim_db(int ket, String table_insert) {
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
        USE_SERIAL.println("Respon : " + respon);
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
