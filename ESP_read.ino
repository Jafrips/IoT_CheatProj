#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <SPI.h>

// Пин для вибромотора
#define D 4

// Пины для дисплея
#define TFT_CS 15
#define TFT_RST 0
#define TFT_DC 2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

const char* ssid = "TTK";
const char* password = "Qazedc14";
const char* mqtt_server = "cscheatreciever.cloud.shiftr.io";
const char* mqtt_username = "cscheatreciever";   //Shiftr.io login
const char* mqtt_password = "bpOX0ZqZfzYCywVM";  // Shiftr.io password

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int x = 10;
  int y = 15;

  if (strcmp(topic, "ServerData") == 0) {

    // Цвет дисплея
    tft.fillScreen(ST77XX_BLACK);



    // Создаем буфер для хранения пришедших данных
    char message[length + 1];
    for (int i = 0; i < length; i++) {
      message[i] = (char)payload[i];
    }
    message[length] = '\0';

    // Десериализуем JSON
    DynamicJsonDocument doc(1024);  //размер буфера JSON
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Получаем значения из JSON
    JsonArray entities = doc.as<JsonArray>();
    for (JsonVariant entity : entities) {
      String name = entity["Name"];
      int health = entity["Health"];

      // Точка старта печати текста
      tft.setCursor(x, y);

      y += 20;
      // Вывод строки на экран
      tft.print(name + " " + String(health));
    }
  } else if (strcmp(topic, "DangerData") == 0) {
    String messageString = "";
    for (int i = 0; i < length; i++) {
      messageString += (char)payload[i];
    }

    // Пример: преобразование строки в число и использование данных
    int dangerLevel = messageString.toInt();
    Serial.println("danger? " + String(dangerLevel));
    // Делайте что-то с полученным уровнем опасности

    // Если противник рядом, включается вибромотор, иначе отключается
    if (dangerLevel) {
      analogWrite(D, 255);
    } else {
      analogWrite(D, 0);
    }
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client_1", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("ServerData");
      client.subscribe("DangerData");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Пин вибромотора
  pinMode(D, OUTPUT);

  // Init ST7735S chip, black tab
  tft.initR(INITR_BLACKTAB);

  //Перенос текста
  tft.setTextWrap(false);

  // Размер текста
  tft.setTextSize(2);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
