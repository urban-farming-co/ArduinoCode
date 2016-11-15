#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>         

// Arduino WiFi shield SD card: pin 4

#define chipSelect 4




int checkCard() {
  #if !defined(SOFTWARE_SPI)
    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
      if(chipSelect != 53) {
            pinMode(53, OUTPUT); // SS on Mega
            digitalWrite(53, HIGH);
      }
    #else
       if(chipSelect != 10) {
         pinMode(10, OUTPUT); // SS on Uno, etc.
         digitalWrite(10, HIGH);
       }
    #endif
  #endif
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return -1;
  }
  Serial.println("Found Card");
  return 0; 
}

int checkCamera(Adafruit_VC0706 * cam) {
  // Try to locate the camera

  if (cam->begin()) {
    Serial.println("Camera Found:");
    return 0;
  } else {
    Serial.println("Camera not found?");
    return -1;
  }
}

void printVersionInfo (Adafruit_VC0706 * cam) {
  char *reply = cam->getVersion();
  if (reply == 0) {
    Serial.println("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
  return;
}

int setImageSize(Adafruit_VC0706 * cam) {

  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
  // Remember that bigger pictures take longer to transmit!
  //cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  cam->setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam->getImageSize();  
  Serial.println("Image size: ");
  if (imgsize == VC0706_640x480) {
    Serial.println("640x480");
  }
  else { 
    if (imgsize == VC0706_320x240) {
      Serial.println("320x240");
    }
    else{
      if (imgsize == VC0706_160x120) {
        Serial.println("160x120");
      }else {
        Serial.println(imgsize);
        return -1;
      }
    }
  }
  return 0;
}
void takeAPicture(Adafruit_VC0706 * cam) {
  bool b = cam->takePicture();
  Serial.println("Snap in 3 secs...");
  Serial.println(b);
  delay(3000);
  b = cam->takePicture();
  if (!b) {
    Serial.println("Camera Failed to Snap!");
    return -1;
  }
  else { 
    Serial.println("Picture taken!");
    return 0;
  }
}


void saveImage(Adafruit_VC0706 * cam, char * f){
  char filename[9];
  Serial.println(f);
  strcpy(filename, (const) f);
  Serial.println(filename);

  
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);
  Serial.println("Opened file for writing");
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam->frameLength();
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
    buffer = cam->readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
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

  return;
}

void setup() {

  // When using hardware SPI, the SS pin MUST be set to an
  // output (even if not connected or used).  If left as a
  // floating input w/SPI on, this can cause lockuppage.
  // Using SoftwareSerial (Arduino 1.0+) or NewSoftSerial (Arduino 0023 & prior):
  #if ARDUINO >= 100
    // On Uno: camera TX connected to pin 2, camera RX to pin 3:
    // Camera TX is gray
    // Camera RX is purple
    SoftwareSerial cameraconnection = SoftwareSerial(6, 7);
    SoftwareSerial cameraconnection1 = SoftwareSerial(2, 3);

  #else
    NewSoftSerial cameraconnection = NewSoftSerial(6, 7);
    NewSoftSerial cameraconnection1 = NewSoftSerial(2, 3);
  #endif

  Adafruit_VC0706 camL = Adafruit_VC0706(&cameraconnection1);
  Adafruit_VC0706 camR = Adafruit_VC0706(&cameraconnection);
  //Adafruit_VC0706 camR = Adafruit_VC0706(&Serial);

  

  Serial.begin(9600);
  Serial.println("VC0706 Camera snapshot test");

  Serial.println("checking the card");
  int i = checkCard();
  Serial.println(i);
  if (i!=0){
    Serial.println("Sadly, I haven't found the SD card.");
    return;
  }
  Serial.println("checked the card");
  Serial.println("checking the left camera");
  i = checkCamera(&camL);
  if (i!=0){
    Serial.println("Sadly, I haven't found that camera.");
    return;
  }
  Serial.println("checked the left camera");
  Serial.println("checking the right camera");
  i = checkCamera(&camR);
  if (i!=0){
    Serial.println("Sadly, I haven't found that camera.");
    return;
  }
  Serial.println("checked the right camera");
  // Print out the camera version information (optional)
  printVersionInfo(&camL);
  printVersionInfo(&camR);
  Serial.println("Printed version information");
  Serial.println("Setting image size.");
  setImageSize(&camL); 
  setImageSize(&camR);
  Serial.println("Set image size.");
  
  Serial.println("=======");

  char * filename = (char* )calloc(9, sizeof(char));

  Serial.println("Taking right picture");
  delay(1000);
  takeAPicture(&camR);
   
  Serial.println("Saving right image");
  delay(1000);
  filename = "RIGHT.JPG";
  saveImage(&camR, filename);


  filename = (char* )realloc(filename, sizeof(char)*8);
  Serial.println("Taking left picture");
  delay(1000);
  takeAPicture(&camL);

  Serial.println("Saving left image");
  delay(1000);
  filename = "LEFT.JPG";
  saveImage(&camL, filename);

  
  free (filename);
  Serial.println("Done.");

  
}

void loop() {
}





