#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

#define HOST "soldier.cloudmqtt.com"
#define PORT 16925
#define HOSTUSER "inkphijn"
#define HOSTPASSWORD "F1QexKFsttXE"
#define PUBLISH_TOPIC "fiap/threerocks/sensor/flow"
#define TIMEOUT 5000 //milliseconds - conection timeout
#define REFRESH 1000 //milliseconds - refresh rate for the led and sensors data
#define ID 1 //ID SENSOR

//CONFIGS
WiFiServer server(80);
WiFiClient espClient;
PubSubClient mqtt(espClient);

int soundPin = A0;
int sensorValue = 0;

void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  wifiManager.autoConnect("AutoConnectAP");
  wifiManager.setMinimumSignalQuality(4);
  Serial.println("Connected.");
  mqtt.setServer(HOST, PORT);
  mqtt.setCallback(callback);
  server.begin();

}

void loop() {
  delay (1000);
  sensorValue = analogRead (soundPin);
  Serial.println (sensorValue);
  enviarMqttResponse(sensorValue);
  Serial.println ("--------");
  //int frequency = map(sensorValue, 0, 1023, 10, 1000);   // Convert analog value to frequency.
  //Serial.println (frequency);
}

void enviarMqttResponse(int value){
   if (WiFi.status() == WL_CONNECTED) { 
        reconn();
          //Checa mensagens - deve ser chamado de forma recorrente
          mqtt.loop();
          char texto[200];
          StaticJsonBuffer<200> jsonBuffer;
          JsonObject& json = jsonBuffer.createObject();
          json["sensorId"] = ID;
          if (!isnan(value)) {
            json["analogValue"] = value;
          }
          //Criando a string a ser enviada via MQTT publish
          char buffer[256];
          json.printTo(buffer, sizeof(buffer));
          Serial.print("[MQTT] Publish...\n");
          mqtt.publish(PUBLISH_TOPIC, buffer);;
      }else {
        Serial.println("Não conectado ao AP");
        delay(REFRESH);
      }
}

//Função de callback que deve ser executada quando chega uma mensagem
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(String("Message arrived [") + topic + "] ");
  payload[length] = '\0'; //com sorte há espaço para mais um caractere...
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(payload);
  //PENSAR EM LÓGICA AQUI
  if (json.success() && json.containsKey("value")) {

  }
  delay(10);
}

//Função que verifica a conexão do cliente MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT client connected");
      // Once connected, resubscribe to LED topic
      // mqtt.subscribe(LED_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(String(" try again in ") + TIMEOUT + " milliseconds");
      // Wait TIMEOUT seconds before retrying
      delay(TIMEOUT);
    }
  }
}

void reconn(){
  if(!mqtt.connected()){
    Serial.print("Attempting MQTT connection...");
    if(mqtt.connect("ESP8266Client", HOSTUSER, HOSTPASSWORD)){
      Serial.println("Connected MQTT");
    }else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt.state());
    }
  }

}
