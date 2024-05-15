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

const char* ssid = "DIR-615-019a";
const char* password = "33612423";
const char* mqtt_server = "cscheatreciever.cloud.shiftr.io";
const char* mqtt_username = "cscheatreciever";   //Shiftr.io login
const char* mqtt_password = "bpOX0ZqZfzYCywVM";  // Shiftr.io password

bool runMotor = true;
int MotorPower = 0;
bool IsDanger = false;
int x = 10;
int y = 15;

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
  Serial.print("]\n");

  if (strcmp(topic, "ServerData") == 0 || strcmp(topic, "MotorSetup") == 0) {

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
    
    if (strcmp(topic, "ServerData") == 0) {
      // Цвет дисплея
      tft.fillScreen(ST77XX_BLACK);    

      // Получаем значения из JSON
      JsonArray entities = doc.as<JsonArray>();
      for (JsonVariant entity : entities) {
        String name = entity["Name"];
        Serial.println(name);
        int health = entity["Health"];
        Serial.println(health);

        // Точка старта печати текста
        tft.setCursor(x, y);

        y += 20;
        // Вывод строки на экран
        tft.print(name + " " + String(health));
      }
    } else {
      // Получение значений из JSON
      runMotor = doc["isRun"];
      MotorPower = doc["pwr"];
    }
  } else if (strcmp(topic, "DangerData") == 0) {
    String messageString = "";
    for (int i = 0; i < length; i++) {
      messageString += (char)payload[i];
    }

    // Пример: преобразование строки в число и использование данных
    if (messageString == "True")
      IsDanger = true;

    // Если противник рядом, включается вибромотор, иначе отключается
    if (IsDanger && runMotor) {
      analogWrite(D, MotorPower);
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
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("ServerData");
      client.subscribe("DangerData");
      client.subscribe("MotorSetup");
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
