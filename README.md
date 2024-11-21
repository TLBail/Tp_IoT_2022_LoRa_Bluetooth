# Tp_IoT_2022_LoRa_Bluetooth
### Auteurs :

- Théo LE BAIL
- Thomas LEBRETON
- Corto BEAU
- Lilian FORGET

## Mise en place
### Linux :

1. Ajouter dans le groupe dialout pour l'utilisateur courant
```bash
sudo usermod -a -G dialout $USER
```

2. Changer les droits sur le port série
```bash
sudo chmod a+rw /dev/ttyUSB0
```

Nous avons ensuite suivi le document du tp d'installation.
[Document d’installation](http://www.smartcomputerlab.org/m5/IoT.Labs.base.fr.2020.jpg.pdf)

### Windows :

1. Installation du driver pour la carte Heltec Wifi LoRa 32 V2
[Drivers Windows](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads)

2. Installation de l'IDE Arduino via le site officiel
[Logiciel](https://www.arduino.cc/en/software)

Nous avons ensuite suivi le document du tp d'installation.
[Document d'installation](http://www.smartcomputerlab.org/m5/IoT.Labs.base.fr.2020.jpg.pdf)

## Publication de données sur un serveur MQTT

publish_mqtt.ino:
On se connecte au réseau WiFi avec l’instance WiFiClient.

Cela n’est pas forcément immédiat, donc on délaie le reste du setup tant que la connexion n’est pas établie.

On connecte ensuite le client MQTT au broker à l’adresse test.mosquitto.org:1883.

Si la connexion est réussie, on publie un message sur le sujet tp_popo/alban.

Ensuite dans la loop, on se reconnecte au broker MQTT si le client n’est plus connecté pour maintenir la connexion.

## Etablissement des fréquences LoRa via un message MQTT
- Terminal 1 avec mqtt_sender_lora_receiver.ino:
Même setup que publish_mqtt ; cependant au lieu de publier un message Hello World, on publie un buffer ayant la structure suivante :
```c
struct lora_config {
    long freq;
    int sf;
    long sb;
};
``` 
avec les valeurs de freq, sf et sb. 8686E5 8 125E3

Ce même buffer est utilisé pour configurer l’antenne LoRa :

```c
// mqtt_sender_lora_receiver.ino
if (LoRa.begin(mqtt_message.freq)) {
	LoRa.setSpreadingFactor(mqtt_message.sf);
	LoRa.setSignalBandwidth(mqtt_message.sb);
}
```

Ensuite sur l’autre client, on peut recevoir cette configuration et l’utiliser pour paramétrer l’antenne LoRa :
```c
// mqtt_receiver_lora_sender.ino
void onMqttReceive(char* topic, uint8_t* payload, unsigned int length) {
lora_config conf;
memcpy(&conf, payload, sizeof(lora_config)); LoRa.begin(conf.freq);
...
lora_configured = true;
}
```

Dans la loop on peut ensuite émettre un paquet avec l’antenne lora si elle est configurée :

```c
// mqtt_receiver_lora_sender.ino
void loop() {
	...
	if (lora_configured) {
		LoRa.beginPacket();
		uint8_t packet[16] = { ... };
		LoRa.write(packet, 16°);
		LoRa.endPacket();
	}
}
```
Retour au premier client, où on peut maintenant recevoir le message envoyé via LoRa :

```c
void loop() {
	...
	int packetLen = LoRa.parsePacket();
	if (packetLen > 0) {
		...
	}
}
```
## Echange Lora / MQTT et inversement entre 2 terminaux

Cette fois, on utilise un client unique capable d’alterner entre les deux modes. Par simplicité ou modifie simplement un booléen quand on le démarre une deuxième fois pour qu’il se lance directement dans le bon mode.

Si le booléen est true, ont créé une configuration LoRa aléatoire, on l’utilise pour initialiser l’antenne LoRa puis on la publie sur le sujet MQTT.

À chaque fois qu’un des deux clients reçoit une configuration MQTT, il l’utilise pour configurer sa propre antenne LoRa, puis émet un message via l’antenne LoRa.

Ensuite, on attend 10s pour publier une config LoRa aléatoire, ce qui permet d’inverser le fonctionnement des deux.

Cependant, cela pose un problème : en effet, un client peut recevoir sa propre configuration LoRa sur le sujet MQTT puisqu’il est à la fois émetteur et receveur. Dans certains cas, un client peut n’être jamais amené à communiquer avec l’autre s’il reçoit systématiquement sa propre configuration qui de fait ne change jamais.

La solution est donc d’utiliser des sujets différents pour chaque client, ce qui évite à un même client de recevoir ses propres paquets.

```c
// switcher.ino
#define MY_TOPIC “tp_popo/client1”
#define THEIR_TOPIC “tp_popo/client2”
```

On inverse donc simplement les deux valeurs sur l’autre client. On fait ensuite en sorte d’uniquement publier sur son propre topic et de s’inscrire au topic de l’autre :
```c
// switcher.ino
mqttClient.subscribe(THEIR_TOPIC);
...
mqttClient.publish(MY_TOPIC, ...);
```
