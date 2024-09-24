#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ESP32Servo.h> // Library untuk Servo
#include <DHT.h>        // Library untuk DHT22

// Wifi network station credentials
#define WIFI_SSID "BOLA"
#define WIFI_PASSWORD "00000000"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "7136033211:AAGwVJOhsWnXvB6gOaNYaGLIaTnQqGGSoKA"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

// Servo
Servo myServo;
const int servoPin = 18; 
int servoAngle = 0; 

// DHT22
#define DHTPIN 15 // Pin yang terhubung ke data DHT22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/buka")
    {
      servoAngle = 180; // Atur servo ke sudut 180
      myServo.write(servoAngle);
      bot.sendMessage(chat_id, "Servo terbuka (180°)", "");
    }

    if (text == "/tutup")
    {
      servoAngle = 0; // Atur servo ke sudut 0
      myServo.write(servoAngle);
      bot.sendMessage(chat_id, "Servo tertutup (0°)", "");
    }

    if (text == "/status")
    {
      bot.sendMessage(chat_id, "Sudut servo saat ini: " + String(servoAngle) + "°", "");
    }

    if (text == "/suhu")
    {
      float suhu = dht.readTemperature(); // Membaca suhu
      if (isnan(suhu))
      {
        bot.sendMessage(chat_id, "Gagal membaca suhu!", "");
      }
      else
      {
        bot.sendMessage(chat_id, "Suhu saat ini: " + String(suhu) + "°C", "");
      }
    }

    if (text == "/kelembapan")
    {
      float kelembapan = dht.readHumidity(); // Membaca kelembapan
      if (isnan(kelembapan))
      {
        bot.sendMessage(chat_id, "Gagal membaca kelembapan!", "");
      }
      else
      {
        bot.sendMessage(chat_id, "Kelembapan saat ini: " + String(kelembapan) + "%", "");
      }
    }

    if (text == "/start")
    {
      String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
      welcome += "Ini adalah bot untuk mengontrol servo dan menampilkan data DHT22.\n\n";
      welcome += "/buka : untuk membuka servo (180°)\n";
      welcome += "/tutup : untuk menutup servo (0°)\n";
      welcome += "/status : Menampilkan sudut servo saat ini\n";
      welcome += "/suhu : Menampilkan suhu saat ini dari DHT22\n";
      welcome += "/kelembapan : Menampilkan kelembapan saat ini dari DHT22\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // Servo
  myServo.attach(servoPin); // Pasang servo ke pin yang ditentukan
  myServo.write(servoAngle); // Set posisi awal servo ke 0°

  // DHT22
  dht.begin(); // Inisialisasi DHT22

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
