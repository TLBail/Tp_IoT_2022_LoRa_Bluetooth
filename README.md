
# Tp_IoT_2022_LoRa_Bluetooth

### Autheurs :

- Théo LE BAIL
- Thomas LEBRETON
- Corto BEAU
- Lilian FORGET

## Mise en place

### Linux:

1. Ajouter dans le groupe dialout pour l'utilisateur courant
```bash
sudo usermod -a -G dialout $USER
```

2. Changer les droits sur le port série
```bash
sudo chmod a+rw /dev/ttyUSB0
```

Nous avons ensuite suivi le document du tp d'installation.
[Document d'installation](http://www.smartcomputerlab.org/m5/IoT.Labs.base.fr.2020.jpg.pdf)

### Windows:

1. Installation du driver pour la carte Heltec Wifi LoRa 32 V2
[Drivers Windows](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads)

2. Installation de l'IDE Arduino via le site officiel
[Logiciel](https://www.arduino.cc/en/software)

Nous avons ensuite suivi le document du tp d'installation.
[Document d'installation](http://www.smartcomputerlab.org/m5/IoT.Labs.base.fr.2020.jpg.pdf)

## Publication de données sur un serveur MQTT

publish_mqtt.ino:
Se connect au réseau wifi local et
permet de publier sur le serveur test.mosquitto.org, topic tp_popo/alban
un message sous forme de texte.



## Etablissement des fréquences LoRa via un message MQTT
- Terminal 1 avec mqtt_sender_lora_receiver.ino:
Se connect au réseau wifi local.
Publie sur le serveur test.mosquitto.org, topic tp_popo/BIPBOUP le struct suivant:
```c
struct lora_config {
    long freq;
    int sf;
    long sb;
};
``` 
avec les valeurs de freq, sf et sb.
8686E5
8
125E3

- Terminal 2 avec mqtt_receiver_lora_sender.ino:
Se connect au réseau wifi local.
Souscrit au serveur test.mosquitto.org, topic tp_popo/BIPBOUP.
Récupère la struct lora_config avec les valeurs de freq, sf et sb.
Envoie des données de test sur le protocol Lora avec les valeurs de freq, sf et sb. (Valeurs de 1 à 16).


- Terminal 1 avec mqtt_sender_lora_receiver.ino:
Ecoute avec le protocol Lora sur la fréquences envoyé.
Affiche les données reçu. (normalement les valeurs de 1 à 16). 


## Echange Lora / MQTT et inversement entre 2 terminals
- Terminal 1 : switcher.ino

