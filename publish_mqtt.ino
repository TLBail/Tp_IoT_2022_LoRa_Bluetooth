#include <WiFi.h>
#include <PubSubClient.h>

// Paramètres WiFi
const char* ssid = "";
const char* password = "";

// Paramètres MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "tp_popo/alban";
const char* mqtt_message = "Hello from ESP32!";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Connexion au réseau WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

    // Connexion au serveur MQTT
    client.setServer(mqtt_server, mqtt_port);

    if (client.connect("ESP32Client")) {  // L'ID de client peut être n'importe quoi de unique
        Serial.println("Connected to MQTT broker");
        // Publier un message sur le topic
        if (client.publish(mqtt_topic, mqtt_message)) {
            Serial.println("Message sent successfully");
        }
        else {
            Serial.println("Failed to send message");
        }
    }
    else {
        Serial.println("Failed to connect to MQTT broker");
    }
}

void loop() {
    // Pour maintenir la connexion MQTT si besoin
    if (!client.connected()) {
        // Essayer de se reconnecter au broker si nécessaire
        if (client.connect("ESP32Client")) {
            Serial.println("Reconnected to MQTT broker");
        }
    }
    client.loop();  // Garder la connexion MQTT active
}
