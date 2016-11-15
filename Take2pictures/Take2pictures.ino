#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

// Arduino WiFi shield SD card: pin 4

#define chipSelect 4

// Initalize the camera variables.
#if ARDUINO >= 100
SoftwareSerial cameraconnection =  SoftwareSerial(6, 7);
SoftwareSerial cameraconnection1 = SoftwareSerial(2, 3);
#else
NewSoftSerial cameraconnection = NewSoftSerial(6, 7);
NewSoftSerial cameraconnection1 = NewSoftSerial(2, 3);
#endif
Adafruit_VC0706 camR = Adafruit_VC0706(&cameraconnection);
Adafruit_VC0706 camL = Adafruit_VC0706(&cameraconnection1);


void setup() {

  Serial.begin(9600);
  Serial.println("starting");

  Serial.println("=======================================");
  // Check on the card.
#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if (chipSelect != 53) {
    pinMode(53, OUTPUT); // SS on Mega
    digitalWrite(53, HIGH);
  }
#else
  if (chipSelect != 10) {
    pinMode(10, OUTPUT); // SS on Uno, etc.
    digitalWrite(10, HIGH);
  }
#endif
#endif

/* 
 *  
 *  Find the card, camera left and camera right
 */
  Serial.println("========================================");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return -1;
  }
  Serial.println("Found Card");


  delay(50);
  Serial.println("========================================");
  // Try to locate the right camera
  if (camR.begin()) {
    delay(1000);
    camR.reset();
    delay(1000);
    Serial.println("Camera Found:");
    char *reply = camR.getVersion();
    if (reply == 0) {
      Serial.println("Failed to get version");
    } else {
      Serial.println("-----------------");
      Serial.print(reply);
      Serial.println("-----------------");
    }
  } else {
    Serial.println("Camera on right not found?");
    return -1;
  }

  // Try to locate the left camera
  if (camL.begin()) {
    delay(1000);
    camL.reset();
    delay(1000);
    Serial.println("Camera Found:");
    char *reply = camL.getVersion();
    if (reply == 0) {
      Serial.println("Failed to get version");
    } else {
      Serial.println("-----------------");
      Serial.print(reply);
      Serial.println("-----------------");
    }
  } else {
    Serial.println("Camera on left not found?");
    return -1;
  }

/*
 * 
 * Set the image size to small
 */


  Serial.println("========================================");
  delay(1000);
  Serial.println(VC0706_160x120);
  delay(1000);
  // Set the left camera image size
  // camL.setImageSize(VC0706_160x120);          // small
  
  camL.setImageSize(0); 
  // You can read the size back from the camera (optional, but maybe useful?)
  delay(10);

  uint8_t imgsize = camL.getImageSize();
  Serial.println("Image size: ");
  if (imgsize == VC0706_160x120) {
    Serial.println("160x120");
  }
  else {
    Serial.println(imgsize);
  }
  delay(1000);
  // Set the left camera image size
  // camR.setImageSize(VC0706_160x120);          // small
  camR.setImageSize(0); 
  // You can read the size back from the camera (optional, but maybe useful?)
  delay(10);

  uint8_t imgsizer = camR.getImageSize();
  Serial.println("Image size: ");
  if (imgsizer == VC0706_160x120) {
    Serial.println("160x120");
  }
  else {
    Serial.println(imgsizer);
  }

  Serial.println("========================================");
  // Take right picture;
  Serial.println("Snap in 3 secs...");
  delay(3000);
  bool br = camR.takePicture();
  if (!br) {
    Serial.println("Camera Failed to Snap!");
  }
  else {
    Serial.println("Picture taken!");
  }
  // Take Left picture

  Serial.println("Snap in 3 secs...");
  delay(3000);
  bool b = camL.takePicture();
  if (!b) {
    Serial.println("Camera Failed to Snap!");
  }
  else {
    Serial.println("Picture taken!");
  }

  delay(1000);

  Serial.println("========================================");
  // Save Left Picture
  char filename[9] = "LEFT.JPG";
  Serial.println(filename);
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);
  Serial.println("Opened file for writing");
  // Get the size of the image (frame) taken
  uint16_t jpglen = camL.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");

  int32_t time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = camL.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if (++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();


  time = millis() - time;
  Serial.println("done saving!");
  Serial.print(time);
  Serial.println(" ms elapsed");


  Serial.println("========================================");
  delay(50);

  // Save Right image
  char frilenameR[10] = "RIGHT.JPG";
  Serial.println(frilenameR);
  Serial.println("========================================");
  delay(50);
  Serial.println(frilenameR);

  // Open the file for writing
  File imgFileR = SD.open(frilenameR, FILE_WRITE);
  Serial.println("Opened file for writing");
  // Get the size of the image (frame) taken
  uint16_t jpglenR = camR.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglenR, DEC);
  Serial.print(" byte image.");

  time = millis();
  pinMode(8, OUTPUT);
  // Read all the data up to # bytes!
  wCount = 0; // For counting # of writes
  while (jpglenR > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglenR); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = camR.readPicture(bytesToRead);
    imgFileR.write(buffer, bytesToRead);
    if (++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglenR -= bytesToRead;
  }
  imgFileR.close();

  time = millis() - time;
  Serial.println("done saving!");
  Serial.print(time);
  Serial.println(" ms elapsed");

  Serial.println("========================================");

}

void loop() {
  // put your main code here, to run repeatedly:

}
