
/*
  WriteMultipleFields
  
  Description: Writes values to fields 1,2,3 and 4  in a single ThingSpeak update every 20 seconds.
  
  Hardware: Arduino Ethernet
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
 - Requires the Ethernet library
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2018, The MathWorks, Inc.
*/


#include "ThingSpeak.h"
#include <Ethernet.h>
#include <OneWire.h>
#include "secrets.h"
#include <DallasTemperature.h>


const byte mac[] = SECRET_MAC;

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);

EthernetClient client;

const unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

OneWire oneWire(9);  // on pin 9 (a 4.7K resistor is necessary)
DallasTemperature sensors(&oneWire);
#define LDRpin A0 // pin where we connected the LDR and the resistor

const uint8_t sensor1[8] = { 0x28, 0x5B, 0xFF, 0x50, 0x04, 0x00, 0x00, 0x47 }; //Onboard sensor
const uint8_t sensor2[8] = { 0x28, 0x3F, 0x7A, 0x51, 0x04, 0x00, 0x00, 0xCE }; //Brown(Pwr+), Brown-W (Pwr-), Green (Data+), Green-W (Data-)
const uint8_t sensor3[8] = { 0x28, 0xDD, 0x02, 0x51, 0x04, 0x00, 0x00, 0xDA }; //Orange(Pwr+), Orange-W (Pwr-), Blue (Data+), Blue-W (Data-)
//Grey Flat cable sensor : 0x28, 0x5C, 0xC1, 0x50, 0x04, 0x00, 0x00, 0xE4

float sensor1value = 0;
float sensor2value = 0;
float sensor3value = 0;

const int DI5Pin = 5;     // the number of the DI pin
const int DI6Pin = 6;     // the number of the DI pin
const int DI7Pin = 7;     // the number of the DI pin
int DI5State = 0;         // variable for reading the pushbutton status
int DI6State = 0;         // variable for reading the pushbutton status
int DI7State = 0;         // variable for reading the pushbutton status
const int ledPin =  8;      // the number of the LED pin
int LDRValue = 0;     // result of reading the analog pin

void setup() {
  Ethernet.init(10);  // Most Arduino Ethernet hardware
  Serial.begin(115200);  //Initialize serial
    
  // start the Ethernet connection:
  Serial.println("I: Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("W: Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("E: Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("E: Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("I: DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  sensors.begin();
  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(DI5Pin, INPUT_PULLUP);
  pinMode(DI6Pin, INPUT_PULLUP);
  pinMode(DI7Pin, INPUT_PULLUP);
  
}

void loop() {

  Serial.print("I: Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(" DONE");
  
  sensor1value = sensors.getTempC(sensor1); 
  sensor2value = sensors.getTempC(sensor2); 
  sensor3value = sensors.getTempC(sensor3); 
  LDRValue = analogRead(LDRpin); // read the value from the LDR
  
  Serial.print("I: Temperature values: || ");
  Serial.print(sensor1value);
  Serial.print(" °C | ");
  Serial.print(sensor2value);
  Serial.print(" °C | ");
  Serial.print(sensor3value);
  Serial.println(" °C ||");
  Serial.print("I: LDR value: ");
  Serial.println(LDRValue);
  
  // read the state of the pushbutton value:
  DI5State = digitalRead(DI5Pin);
  DI6State = digitalRead(DI6Pin);
  DI7State = digitalRead(DI7Pin);
    Serial.print("I: DI5-7 status = ");
    Serial.print(!DI5State);
    Serial.print(!DI6State);
    Serial.println(!DI7State);

  
  // set the fields with the values
  ThingSpeak.setField(1, sensor1value);
  ThingSpeak.setField(2, sensor2value);
  ThingSpeak.setField(3, sensor3value);
  ThingSpeak.setField(4, !DI5State);
  ThingSpeak.setField(5, !DI6State);
  ThingSpeak.setField(6, !DI7State);  
  ThingSpeak.setField(7, LDRValue);  


  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (DI5State == HIGH) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
  
  // write to the ThingSpeak channel 
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("I: Channel update successful.");
  }
  else{
    Serial.println("E: Problem updating channel. HTTP error code " + String(x));
  }
  // Wait 12x5 seconds to update the channel again
  Serial.print("I: Waiting");
  delay(5000); Serial.print(".");delay(5000); Serial.print(".");delay(5000); Serial.print(".");delay(5000);
  Serial.print("  new");
  delay(5000); Serial.print(".");delay(5000); Serial.print(".");delay(5000); Serial.print(".");delay(5000);
  Serial.print("  cycle");
  delay(5000); Serial.print(".");delay(5000); Serial.print(".");delay(5000); Serial.println(".");delay(5000);
}
