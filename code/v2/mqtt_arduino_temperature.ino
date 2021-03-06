/*****
 
 All the resources for this project:
 https://rntlab.com/
 
*****/

// Loading the libraries
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <RCSwitch.h>

// This MAC addres can remain the same
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };

/* 
 *  IMPORTANT!
 *  YOU MUST UPDATE THESE NEXT TWO VARIABLES WITH VALUES SUITABLE TO YOUR NETWORK!
 *  
*/

// Replace with an IP that is suitable for your network. I could use any IP in this range: 192.168.1.X
// I've used a tool http://angryip.org/ to see an available IP address and I ended up using 192.168.1.99
IPAddress ip(192, 168, 1, 99);

// Replace with your Raspberry Pi IP Address. In my case, the RPi IP Address is 192.168.1.76
IPAddress server(192, 168, 1, 76);

// Initializes the ethClient. You have to change the ethClient name if you have multiple ESPs running in your home automation system
EthernetClient ethClient;
PubSubClient client(ethClient);

RCSwitch mySwitch = RCSwitch();

// Defines the analog temperature pin for the LM335 temperature sensor
int tempAnalogPin = A0;

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Feel free to add more if statements to control more outlets with MQTT

  // If a message is received on the topic home/livingroom/arduino/outletX, 
  // you check if the message is either 1 or 0. Turns the outlet according to the message
  if(String(topic)=="home/livingroom/arduino/outlet1"){
      Serial.print("Changing outlet 1 to ");
      if(messageTemp == "1"){
        mySwitch.send("000101010101000101010101");
        Serial.print("On");
      }
      else if(messageTemp == "0"){
        mySwitch.send("000101010101000101010100");
        Serial.print("Off");
      }
  }
  if(String(topic)=="home/livingroom/arduino/outlet2"){
      Serial.print("Changing outlet 2 to ");
      if(messageTemp == "1"){
        mySwitch.send("000101010101010001010101");
        Serial.print("On");
      }
      else if(messageTemp == "0"){
        mySwitch.send("000101010101010001010100");
        Serial.print("Off");
      }
  }
  Serial.println();
}


// This functions reconnects your Arduino to your MQTT broker
// Change the function below if you want to subscribe to more topics with your Arduino 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("home/livingroom/arduino/outlet1");
      client.subscribe("home/livingroom/arduino/outlet2");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your Arduino pins as a transmitter, starts the serial communication at a baud rate of 57600
// Sets your mqtt broker and sets the callback function. The callback function is what receives messages and
// actually controls the LEDs. Finally starts the Ethernet communication
void setup()
{
  mySwitch.enableTransmit(6);
 
  // SET YOUR PROTOCOL (default is 1, will work for most outlets)
  mySwitch.setProtocol(REPLACE_WITH_YOUR_PROTOCOL);

   // SET YOUR PULSE LENGTH
  mySwitch.setPulseLength(REPLACE_WITH_YOUR_PULSE_LENGTH);
  
  // Set number of transmission repetitions.
  mySwitch.setRepeatTransmit(15);
  
  Serial.begin(57600);

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
}

// For this project, you don't need to change anything in the loop function. 
// Basically it ensures that you Arduino is connected to your broker
void loop(){
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("arduinoClient");
  now = millis();
  // Publishes new temperature and humidity every 10 seconds
  if (now - lastMeasure > 10000) {
    lastMeasure = now;
    
    // Reads analog pin and computes temperature values in Kelvin
    int rawVoltage= analogRead(tempAnalogPin);
    float millivolts= (rawVoltage/1024.0) * 5000;
    float kelvin= (millivolts/10);
    Serial.print(kelvin);
    Serial.println(" degrees Kelvin");
    
    // Converts temperature in Kelvin to Celsius, you can comment 
    // and uncomment the lines below if you prefer Fahrenheit temperature
    float celsius = kelvin - 273.15;
    Serial.print(celsius);
    Serial.println(" degrees Celsius");    
    static char temperatureTemp[7];
    dtostrf(celsius, 6, 2, temperatureTemp);
    
    // Uncomment to convert temperature in Kelvin to Fahrenheit 
    /*
    float fahrenheit = (((kelvin - 273.15) * 9)/5 +32);
    Serial.print(fahrenheit);
    Serial.println(" degrees Fahrenheit");
    static char temperatureTemp[7];
    dtostrf(fahrenheit, 6, 2, temperatureTemp);*/

    // Publishes a new Temperature value
    client.publish("home/livingroom/arduino/temperature", temperatureTemp);
  }
}
