#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

//Hardware configuration
const int sensorOpen =  D1;
const int sensorClose =  D3;
const int buttonPin = D5;
const int ledPin =  D4;

int flag=0;
int buttonState = 0; 

//USER CONFIGURED SECTION START//
const char* ssid = "YOUR_SSID_NETWORK";
const char* password = "YOUR_NETWORK_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_SERVER_ADDRESS";
const int mqtt_port = 1883; //YOUR_MQTT_SERVER_PORT DEFAULT:1883
const char *mqtt_user = "YOUR_MQTT_USER";
const char *mqtt_pass = "YOUR_MQTT_PASSWORD";
const char *mqtt_client_name = "UNIQUE_NAME"; // Client connections cant have the same connection name
const char *mqtt_stttopic = "YOUR_STATE_TOPIC";
const char *mqtt_cmdtopic = "YOUR_COMAND_TOPIC";
//USER CONFIGURED SECTION END//

WiFiClient espClient;
PubSubClient client(espClient); 

void setup_wifi() {
   delay(100);
  // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  /// Debug console
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
// Setup notification button on pin D1
  pinMode(sensorOpen,INPUT_PULLUP);
  pinMode(sensorClose,INPUT_PULLUP);
  pinMode(ledPin,OUTPUT);
  pinMode(buttonPin, INPUT);
}

void callback(char* topic, byte* payload, unsigned int length) 
{
    String newTopic = topic;
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  if (newTopic == mqtt_cmdtopic) 
  {
    if (newPayload == "push")
    {
      digitalWrite(ledPin,HIGH);
      delay(1000);
      digitalWrite(ledPin,LOW);
    }
  }
} //end callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    
    if (client.connect((char*) clientId.c_str(),mqtt_user,mqtt_pass))
    //if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe(mqtt_cmdtopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} //end reconnect()

void loop() {
 if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  
  //Read magnet sensors
  int openState = digitalRead(sensorOpen);
  int closeState = digitalRead(closeState);

  buttonState = digitalRead(buttonPin);
  
  if (openState==1 && closeState==0) {
    client.publish(mqtt_stttopic, "Open");
    Serial.println("Gate is open");
    digitalWrite(ledPin,HIGH);
    flag=0;
  }
  if (openState==0 && closeState==1)
  {
    client.publish(mqtt_stttopic, "Closed");
    Serial.println("Gate is closed");
    digitalWrite(ledPin,LOW);
    flag=1;
  }
  if (openState==1 && closeState==1 && flag==0){
    //client.publish(mqtt_stttopic, "Closing");
    Serial.println("Gate is closing");
  }
  if (openState==1 && closeState==1 && flag==1){
    client.publish(mqtt_stttopic, "Opening");
    Serial.println("Gate is opening ");
  }
    if (buttonState == HIGH) {
    // turn LED on:
  digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
  
  delay(2000);
}
