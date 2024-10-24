#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LoRa.h>

// Paramètres WiFi
const char* ssid = "*";
const char* password = "*";

// Paramètres MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "tp_popo/BIPBOUP";

// Paramètres LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DI0 26

union pack {
    uint8_t frame[16];
    float data[4];
} rdp;

struct lora_config {
    long freq;
    int sf;
    long sb;
};

WiFiClient espClient;
PubSubClient client(espClient);

lora_config mqtt_message;

void setup() {
    Serial.begin(9600);
    delay(1000);

    mqtt_message.freq = 8686E5;
    mqtt_message.sf = 8;
    mqtt_message.sb = 125E3;

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

    if (client.connect("theoetthomas")) {  // L'ID de client peut être n'importe quoi de unique
        Serial.println("Connected to MQTT broker");

        if (client.publish(mqtt_topic, (uint8_t*)&mqtt_message, sizeof(lora_config))) {
            Serial.println("Message sent successfully");
        }
        else {
            Serial.println("Failed to send message");
        }
    }
    else {
        Serial.println("Failed to connect to MQTT broker");
    }

    // LoRa
    pinMode(DI0, INPUT);

    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setPins(SS, RST, DI0);

    if (!LoRa.begin(mqtt_message.freq)) {
        Serial.println("Starting LoRa failed !");
        while (1);
    }

    LoRa.setSpreadingFactor(mqtt_message.sf);
    LoRa.setSignalBandwidth(mqtt_message.sb);

    Serial.println("Started LoRa !");
}

float d1, d2;
int i = 0;

void loop() {
    // mqtt
    // Pour maintenir la connexion MQTT si besoin
    if (!client.connected()) {
        // Essayer de se reconnecter au broker si nécessaire
        if (client.connect("theoetthomas")) {
            Serial.println("Reconnected to MQTT broker");
        }
    }
    client.loop();  // Garder la connexion MQTT active

    // LoRa
    int packetLen;
    packetLen = LoRa.parsePacket();

    if (packetLen != 0) {
        Serial.println("Reception !");

        i = 0;
        while (LoRa.available()) {
            rdp.frame[i] = LoRa.read();
            Serial.print(rdp.frame[i]);
            Serial.print(" ");
            i++;
        }
        Serial.println();

        d1 = rdp.data[0];
        d2 = rdp.data[1];

        int rssi = LoRa.packetRssi();

        Serial.print("d1 : ");
        Serial.println(d1);

        Serial.print("rssi : ");
        Serial.println(rssi);
        Serial.println();
    }
}