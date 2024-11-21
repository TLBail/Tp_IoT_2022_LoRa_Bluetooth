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

#define MY_TOPIC "tp_popo/client2"
#define THEIR_TOPIC "tp_popo/client1"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

struct lora_config {
    long freq;
    int sf;
    long sb;
};

void setup() {
    Serial.begin(115200);

    pinMode(DI0, INPUT);

    SPI.begin(SCK, MISO, MOSI, SS);

    LoRa.setPins(SS, RST, DI0);

    LoRa.onReceive(onLoraReceive);

    LoRa.receive();

    WiFi.begin("xxxx", "xxxxxx");

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

    mqttClient.subscribe(THEIR_TOPIC);

    lora_config conf;

    conf.freq = 8686E5;
    conf.sf = 8;
    conf.sb = 125E3;

    setup_lora(conf);
}

void onLoraReceive(int packetSize) {
    uint8_t buf[24];

    Serial.println("LoRa Receive");

    for (int i = 0; i < packetSize; i++) {
        uint8_t c = LoRa.read();

        buf[i++] = c;

        Serial.print((int)c);
    }

    Serial.println();
}

bool lora_configured = false;

bool setup_lora(const lora_config& conf) {
    Serial.print("conf.freq ");
    Serial.println(conf.freq);

    Serial.print("conf.sf ");
    Serial.println(conf.sf);

    Serial.print("conf.sb ");
    Serial.println(conf.sb);

    if (LoRa.begin(conf.freq)) {
        LoRa.setSpreadingFactor(conf.sf);
        LoRa.setSignalBandwidth(conf.sb);

        Serial.println("Setup OK");

        return true;
    }
    else {

        Serial.println("Setup KO");
        return false;
    }
}

void onMqttReceive(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println("onMqttReceive");
    lora_config conf;
    memcpy(&conf, payload, sizeof(lora_config));

    setup_lora(conf);

    LoRa.beginPacket();
    uint8_t packet[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
    LoRa.write(packet, 16);
    LoRa.endPacket();
    LoRa.end();

    delay(10000);

    conf.freq = 868E6;
    setup_lora(conf);

    mqttClient.publish(MY_TOPIC, (uint8_t*)&conf, sizeof(lora_config));
}

void loop() {
    mqttClient.loop();

    delay(2000);
}
