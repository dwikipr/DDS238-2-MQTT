/* Tested on ArduinoMega 2560 */

#include <SPI.h>
#include <EthernetENC.h>
#include <PubSubClient.h>

#define AUTHENTICATE
//#define DHCP
#define DEBUG
#define ARDUINO_UNO

#define ARDUINO_CLIENT_ID "powerMeter1"                     // Client ID for Arduino pub/sub

#ifdef AUTHENTICATE
const char user[] = "powermeter"; // broker key  
const char pass[] = "p0w3rm3t3r"; // broker secret
const char exportedenergyAddress[] = "/powermeter/1/exportedenergy";
const char importedenergyAddress[] = "/powermeter/1/importedenergy";
const char voltageAddress[] = "/powermeter/1/voltage";
const char currentAddress[] = "/powermeter/1/current";
const char activepowerAddress[] = "/powermeter/1/activepower";
const char reactivepowerAddress[] = "/powermeter/1/reactivepower";
const char powerfactorAddress[] = "/powermeter/1/powerfactor";
const char frequencyAddress[] = "/powermeter/1/frequency";
#endif

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
const PROGMEM byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress mqttIP(192, 168, 87, 174); // MQTT server IP address
#ifndef DHCP
IPAddress myIP(192, 168, 87, 13); // Arduino IP address
#endif

byte data[] = {0x1, 0x3, 0x0, 0x8, 0x0, 0xA, 0x44, 0xF};
char buff[6];
byte received[25], byteRead, bytesRead;
uint32_t tmpInt32, exportedenergy, importedenergy;
uint16_t tmpInt16, voltage, current, activepower, reactivepower, powerfactor, frequency;
int j, i = 0;


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void setup() { 
  Serial.begin(115200);
  Serial.println("Get Data Modbus");
  Serial.println("");

  clearValues();

#ifdef DHCP
  if (Ethernet.begin(mac) == 0) {
#ifdef DEBUG
    Serial.println("Failed to configure Ethernet using DHCP");
#endif
    // no point in carrying on, so do nothing forevermore:
    while(true);
  }
#else
  Ethernet.begin(mac, myIP);
#endif
  
  // MTTQ parameters
  client.setServer(mqttIP, 1883);
  client.setCallback(callback);

  if (Ethernet.linkStatus() == LinkOFF) {
    delay(500);
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    } 
    else{
      Serial.println(Ethernet.localIP());
    }
  }
  Serial1.begin(9600);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  refreshData();
  #ifdef DEBUG
  Serial.print((String)exportedenergy + " " + (String)importedenergy + " " + (String)voltage + " " + (String)current + " " + (String)activepower + " " + (String)reactivepower + " " + (String)powerfactor + " " + (String)frequency);
  Serial.println(" ");
  #endif
  
  client.loop();
  delay(200);
}

void clearValues() {
  exportedenergy = 0;
  importedenergy = 0;
  voltage = 0;
  current = 0;
  activepower = 0;
  reactivepower = 0;
  powerfactor = 0;
  frequency = 0;
}

uint16_t returnInt16(byte offset){
  tmpInt16 = received[offset];
  tmpInt16 <<= 8;
  tmpInt16 += received[offset+1];
  return tmpInt16;
}

uint32_t returnInt32(byte offset){
  tmpInt32 = returnInt16(offset);
  tmpInt32 <<= 16;
  tmpInt32 += returnInt16(offset+2);
  return tmpInt32;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.print("Attempting connection...");
    #endif
    // Attempt to connect
    if (client.connect("arduinoClient", "powermeter", "p0w3rm3t3r")) {
      #ifdef DEBUG
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("/powermeter/1/voltage","2200");
      #endif
    } else {
      #ifdef DEBUG
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      #endif
    }
  }
}

void refreshData(){
  Serial1.write(data, sizeof(data));
  delay(50);
  itoa(0, buff, 10);
  if (Serial1.available()) {
    i = 0;
    byteRead = Serial1.read();
    received[i++] = byteRead;
    
    if (byteRead == 0x01) {
      byteRead = Serial1.read();
      received[i++] = byteRead;
      if (byteRead == 0x03) {
        bytesRead = Serial1.read() * 2 + 2;
        received[i++] = byteRead;
        for (j=0; j<bytesRead; j++) {
          byteRead = Serial1.read();
          received[i++] = byteRead;
        }

        if (returnInt32(3) != exportedenergy) {
          exportedenergy = returnInt16(11);
          itoa(exportedenergy, buff, 10);
          client.publish(exportedenergyAddress, buff);
        }

        if (returnInt32(7) != importedenergy) {
          importedenergy = returnInt16(11);
          itoa(importedenergy, buff, 10);
          client.publish(importedenergyAddress, buff);
        }

        if (returnInt16(11) != voltage) {
          voltage = returnInt16(11);
          itoa(voltage, buff, 10);
          client.publish(voltageAddress, buff);
        }

        if (returnInt16(13) != current) {
          current = returnInt16(13);
          itoa(current, buff, 10);
          client.publish(currentAddress, buff);
        }
        
        if (returnInt16(15) != activepower) {
          activepower = returnInt16(13);
          itoa(activepower, buff, 10);
          client.publish(activepowerAddress, buff);
        }

        if (returnInt16(17) != reactivepower) {
          reactivepower = returnInt16(13);
          itoa(reactivepower, buff, 10);
          client.publish(reactivepowerAddress, buff);
        }

        if (returnInt16(19) != powerfactor) {
          powerfactor = returnInt16(13);
          itoa(powerfactor, buff, 10);
          client.publish(powerfactorAddress, buff);
        }

        if (returnInt16(21) != frequency) {
          frequency = returnInt16(21);
          itoa(frequency, buff, 10);
          client.publish(frequencyAddress, buff);
        }
      } else {
        clearValues();
        client.publish(exportedenergyAddress, buff);
        client.publish(importedenergyAddress, buff);
        client.publish(voltageAddress, buff);
        client.publish(currentAddress, buff);
        client.publish(activepowerAddress, buff);
        client.publish(reactivepowerAddress, buff);
        client.publish(powerfactorAddress, buff);
        client.publish(frequencyAddress, buff);
      }
    } else {
      clearValues();
        client.publish(exportedenergyAddress, buff);
        client.publish(importedenergyAddress, buff);
        client.publish(voltageAddress, buff);
        client.publish(currentAddress, buff);
        client.publish(activepowerAddress, buff);
        client.publish(reactivepowerAddress, buff);
        client.publish(powerfactorAddress, buff);
        client.publish(frequencyAddress, buff);
    }
  } else {
    clearValues();
    client.publish(exportedenergyAddress, buff);
    client.publish(importedenergyAddress, buff);
    client.publish(voltageAddress, buff);
    client.publish(currentAddress, buff);
    client.publish(activepowerAddress, buff);
    client.publish(reactivepowerAddress, buff);
    client.publish(powerfactorAddress, buff);
    client.publish(frequencyAddress, buff);
  }
}