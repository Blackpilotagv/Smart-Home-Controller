#include <WiFi.h>
#include <PubSubClient.h>

// WiFi & MQTT
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASS";
const char* mqtt_server = "YOUR_VPS_IP";

WiFiClient espClient;
PubSubClient client(espClient);

// Relay config (ACTIVE LOW)
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// Pins
int pins[] = {4, 5, 13, 14, 16, 17, 18, 19, 23, 27};
int numPins = sizeof(pins) / sizeof(pins[0]);

// Topics (better structure)
String subTopics[] = {
  "home/relay/1/set",
  "home/relay/2/set",
  "home/relay/3/set",
  "home/relay/4/set",
  "home/relay/5/set",
  "home/relay/6/set",
  "home/relay/7/set",
  "home/relay/8/set",
  "home/relay/9/set",
  "home/relay/10/set"
};

String pubTopics[] = {
  "home/relay/1/state",
  "home/relay/2/state",
  "home/relay/3/state",
  "home/relay/4/state",
  "home/relay/5/state",
  "home/relay/6/state",
  "home/relay/7/state",
  "home/relay/8/state",
  "home/relay/9/state",
  "home/relay/10/state"
};

// Unique client ID using MAC
String clientId;

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");

      // Subscribe to topics
      for (int i = 0; i < numPins; i++) {
        client.subscribe(subTopics[i].c_str());
      }

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 2 sec");
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;

  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(msg);

  for (int i = 0; i < numPins; i++) {
    if (String(topic) == subTopics[i]) {

      if (msg == "ON") {
        digitalWrite(pins[i], RELAY_ON);
        client.publish(pubTopics[i].c_str(), "ON", true);
      } 
      else if (msg == "OFF") {
        digitalWrite(pins[i], RELAY_OFF);
        client.publish(pubTopics[i].c_str(), "OFF", true);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Generate unique client ID
  clientId = "ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);

  // Setup pins safely
  for (int i = 0; i < numPins; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], RELAY_OFF); // Prevent startup flicker
  }

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // WiFi reconnect
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  // MQTT reconnect
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}