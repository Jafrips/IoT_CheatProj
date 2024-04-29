#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "DIR-615-019a";
const char* password = "33612423";
const char* mqtt_server = "cheatreciever.cloud.shiftr.io";
const char* mqtt_username = "cheatreciever"; //Shiftr.io login
const char* mqtt_password = "Slsk3E6qcEI0mM84"; // Shiftr.io password

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

void callback(char* topic, byte* message, unsigned int length)
{
  if (strcmp(topic, "Danger-data") == 0)
  {
    Serial.print("Danger-data recieved: ");
    String dangerData = "";
    for (int i=0; i<length; i++) {
      dangerData += (char)message[i];
    } Serial.println(dangerData);
  }
  else if (strcmp(topic, "Server-data") == 0)
  {
    Serial.print("Server-data recieved: ");

    // Преобразуем полученные данные в строку
    String jsonString = "";
    for (int i = 0; i < length; i++) {
      jsonString += (char)message[i];
    }

    // Создаем объект JsonDocument с достаточным размером буфера
    StaticJsonDocument<2000> jsonDocument;
    DeserializationError error = deserializeJson(jsonDocument, jsonString);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.f_str());
      return;
    }

    // Получаем массив enemyDataTotal
    JsonArray enemyDataTotal = jsonDocument["enemyDataTotal"];

    // Итерируемся по массиву enemyDataTotal
    for (JsonVariant enemyData : enemyDataTotal) {
      // Проверяем, является ли элемент массивом строк
      if (enemyData.is<JsonArray>()) {
        // Разбираем массив строк
        JsonArray enemyDataArray = enemyData.as<JsonArray>();
        // Проверяем, имеет ли массив нужный размер
        if (enemyDataArray.size() == 3) {
          // Получаем элементы массива строк
          String name = enemyDataArray[0];
          int health = enemyDataArray[1].as<int>();
          int distance = enemyDataArray[2].as<int>();
          // Выводим значения
          Serial.print("Name: ");
          Serial.println(name);
          Serial.print("Health: ");
          Serial.println(health);
          Serial.print("Distance: ");
          Serial.println(distance);
        }
      }
    }
  } 
  else
    Serial.println("Unknown topic");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      //client.publish("ReadyToRead", "True");
	  client.subscribe("Danger-data");
      client.subscribe("Server-data");
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
