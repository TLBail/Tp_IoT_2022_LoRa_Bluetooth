#include "WiFi.h"
#include "PubSubClient.h"
#include "LoRa.h"
#include "SPI.h"

#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DI0 26
/*
#define freq 868.5E6
#define sf 8
#define sb 125E3
*/

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

struct lora_config {
    long freq;
    int sf;
    long sb;
};

void setup() {
    Serial.begin(9600);

    pinMode(DI0, INPUT);

    SPI.begin(SCK, MISO, MOSI, SS);

    LoRa.setPins(SS, RST, DI0);

    LoRa.onReceive(onLoraReceive);

    LoRa.receive();

    WiFi.begin("*", "*");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');

        delay(500);
    }

    Serial.println("WiFi connected");

    mqttClient.setServer("test.mosquitto.org", 1883);
    mqttClient.setCallback(onMqttReceive);

    while (!mqttClient.connected()) {
        if (mqttClient.connect("esp32-corto-lilian")) {
            Serial.println("MQTT client connected");
        }
        else {
            Serial.print("failed with state ");

            Serial.println(mqttClient.state());

            delay(2000);
        }
    }

    mqttClient.publish("tp_popo/alban", "mqtt client connected corto/lilian");

    mqttClient.subscribe("tp_popo/BIPBOUP");
}

void onLoraReceive(int packetSize) {
    uint8_t buf[24];

    for (int i = 0; i < packetSize; i++) {
        uint8_t c = LoRa.read();

        buf[i++] = c;

        Serial.print(c);
    }

    Serial.println();
}

bool lora_configured = false;

void onMqttReceive(char* topic, uint8_t* payload, unsigned int length) {
    lora_config conf;

    memcpy(&conf, payload, sizeof(lora_config));

    if (lora_configured) {
        LoRa.end();

        lora_configured = false;
    }

    if (!LoRa.begin(conf.freq)) {
        Serial.println("LoRa failed");
    }
    else {
        Serial.println("LoRa setup done");

        lora_configured = true;

        LoRa.setSpreadingFactor(conf.sf);
        LoRa.setSignalBandwidth(conf.sb);
    }
}

void loop() {
    mqttClient.loop();

    if (lora_configured) {
        LoRa.beginPacket();

        uint8_t packet[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };

        LoRa.write(packet, 16);

        LoRa.endPacket();
    }

    delay(2000);
}