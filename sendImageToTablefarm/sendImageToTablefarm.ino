
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include<stdlib.h>
#include <SD.h>

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ 3
#define ADAFRUIT_CC3000_VBAT 5
#define ADAFRUIT_CC3000_CS 10

// WiFi network (change with your settings !)
#define WLAN_SSID "UrbanFarmingTest"
#define WLAN_PASS "JohnAnderson13.30-04/05/16"
#define WLAN_SECURITY WLAN_SEC_WPA2


// Create CC3000 instances
// WIFi setup
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,SPI_CLOCK_DIVIDER);
Adafruit_CC3000_Client client;

char filename[14] = "CAM1.jpg";
int i = 0;
int keyIndex = 1;
int port = 80; //Server Port 
uint32_t MANip = 0;

void setup() {

  Serial.begin(115200);

  // Initialise the module
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin()){
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }  /* Delete any old connection data on the module */
  Serial.println(F("\nDeleting old connection profiles"));
  if (!cc3000.deleteProfiles()) {
    Serial.println(F("Failed!"));
    return ;
  }
  Serial.println("Found WiFi");


  // Connect to WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println(F("Connected!"));

  // Display connection details
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()){
    delay(100);
  }
  Serial.println("Ready");
 Serial.println("-------");

 //Get MAN IP address

  getMANip();


  // Prepare HTTP request
  String start_request = "";
  String end_request = "";
  start_request = start_request + "\n" + "--AaB03x" + "\n" + "Content-Disposition: form-data; name=\"fileToUpload\"; filename="+filename+"\n" + "Content-Type: file" + "\n" + "Content-Transfer-Encoding: binary" + "\n" + "\n";
  end_request = end_request + "\n" + "--AaB03x--" + "\n";



  //Initialise SD Card
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  } 
  Serial.println("initialization of SD done.");
 
 File myFile = SD.open("CAM1.jpg");
  uint16_t jpglen = myFile.size();
  uint16_t extra_length;
  extra_length = start_request.length() + end_request.length();
  uint16_t len = jpglen + extra_length;

  // Set up TCP connection with web server
  client = cc3000.connectTCP(MANip, port);


  
  if (client.connected()) {
    Serial.println("Start uploading...");

    client.println(F("POST /~modelavi/plugins/upload.php HTTP/1.1"));

    client.println(F("Host: www.modelaviationnorthland.co.nz"));
    client.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
    client.print(F("Content-Length: "));
    client.println(len);
    client.println(start_request);

    Serial.println(F("Host: www.modelaviationnorthland.co.nz"));
    Serial.println(F("Content-Type: multipart/form-data; boundary=AaB03x"));
    Serial.print(F("Content-Length: "));
    Serial.println(len);
    Serial.println(start_request);
  
 if (myFile) {
  byte clientBuf[32];
  int clientCount = 0;

  while(myFile.available())
  {
    clientBuf[clientCount] = myFile.read();
    clientCount++;

    if(clientCount > 31)
    {
      client.write(clientBuf,32);
      clientCount = 0;
    }
  }
  if(clientCount > 0) client.write(clientBuf,clientCount);

  client.print(end_request);
  client.println();
  }
  else{
       Serial.println("File not found");
      }
  }    
  else{
    Serial.println("Web server connection failed");
  }
 
  myFile.close();  
  client.close();
  
  Serial.println("done...");

 
}

void loop() {



delay(10000);

}


void getMANip(){
#ifndef CC3000_TINY_DRIVER
  /* Try looking up www.modelaviationnorthland.co.nz */
    //Serial.print(F("www.modelaviationnorthland.co.nz -> "));
  while  (MANip  ==  0)  {
    if  (!  cc3000.getHostByName("www.modelaviationnorthland.co.nz", &MANip))  {
      //Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }  
  //cc3000.printIPdotsRev(MANip);
#endif
}
