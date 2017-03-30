//This code is for the project listed at
//http://www.iliketomakestuff.com/make-large-bluetooth-scoreboard/

#include <CurieBLE.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN            3
#define NUMPIXELS      60

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);
int delayVal = 5000;
uint32_t colorList[] = { pixels.Color(0,0,255), pixels.Color(255,255,255)  };// blue , white

BLEPeripheral blePeripheral;       // BLE Peripheral Device (the board you're programming)
BLEService arduinoService("6fbee06f-b902-4ac1-bf99-8d4a9b76db8f"); // create service

// create switch characteristic and allow remote device to read and write

BLECharCharacteristic pinD6Char("DD06", BLERead | BLEWrite );
BLECharCharacteristic pinD7Char("DD07", BLERead | BLEWrite );
BLECharCharacteristic pinD8Char("DD08", BLERead | BLEWrite );
BLECharCharacteristic pinD9Char("DD09", BLERead | BLEWrite );
BLECharCharacteristic pinD10Char("DD10", BLERead | BLEWrite );
BLECharCharacteristic pinD11Char("DD11", BLERead | BLEWrite );

BLECharCharacteristic myDchars[6] = {pinD6Char,pinD7Char,pinD8Char,pinD9Char,pinD10Char,pinD11Char};
int score[2] = {0,0};

long previousMillis = 0;  // last time the battery level was checked, in ms

int oldDValues[4] = {0,0,0,0};  // previous values for each digital pin

const int pinD6 = 6; 
const int pinD7 = 7; 
const int pinD8 = 8; 
const int pinD9 = 9; 
const int pinD10 = 10; 
const int pinD11 = 11;

int myDPins[6] = {pinD6, pinD7, pinD8, pinD9, pinD10, pinD11};

const int characterOffset = 15;
/*
charLayout[0] = [1,1,1,
                 1,0,1,
                 1,0,1,
                 1,0,1,
                 1,1,1];
*/

  int charLayout[10][15] = {
      {1,1,1,1,0,1,1,0,1,1,0,1,1,1,1}, //0
      {1,1,0,0,1,0,0,1,0,0,1,0,1,1,1}, //1
      {1,1,1,1,0,0,1,1,1,0,0,1,1,1,1}, //2
      {1,1,1,1,0,0,1,1,1,1,0,0,1,1,1}, //3
      {1,0,1,1,0,1,1,1,1,1,0,0,0,0,1}, //4
      {1,1,1,0,0,1,1,1,1,1,0,0,1,1,1}, //5
      {1,1,1,0,0,1,1,1,1,1,0,1,1,1,1}, //6
      {1,1,1,1,0,0,0,0,1,1,0,0,0,0,1}, //7
      {1,1,1,1,0,1,1,1,1,1,0,1,1,1,1}, //8
      {1,1,1,1,0,1,1,1,1,1,0,0,0,0,1}  //9
  } ; 

void setup() {
  Serial.begin(9600);    // initialize serial communication
  pixels.begin(); // This initializes the NeoPixel library.
  for (int i=0; i < 6; i++){
    pinMode(myDPins[i], OUTPUT);
    digitalWrite(myDPins[i], LOW);
  } 
  
  /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but maybe be truncated based on space left in advertisement packet */
  blePeripheral.setLocalName("BLE101");  // must use this name for the BLE101 app to find it.
  blePeripheral.setAdvertisedServiceUuid(arduinoService.uuid());  // add the service UUID
  blePeripheral.addAttribute(arduinoService);   // Add the BLE Battery service

  // Configure digital ports
  for (int i=0; i < 6; i++) {
    blePeripheral.addAttribute(myDchars[i]);
    myDchars[i].setValue(0);
  } 

  myDchars[0].setEventHandler(BLEWritten, listener0);
  myDchars[1].setEventHandler(BLEWritten, listener1);
  myDchars[2].setEventHandler(BLEWritten, listener2);
  myDchars[3].setEventHandler(BLEWritten, listener3);
  myDchars[4].setEventHandler(BLEWritten, listener4);
  myDchars[5].setEventHandler(BLEWritten, listener5);  
  
  /* Now activate the BLE device.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */
  blePeripheral.begin();
  Serial.println("Bluetooth device active, waiting for connections...");
    setNumber(0,00);
  setNumber(1,00);
}

void loop() {
  // put your main code here, to run repeatedly:

BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    //blePeripheral.poll();
    
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(13, HIGH);
    
    // check the battery level every 200ms
    // as long as the central is still connected:
    while (central.connected()) {
      long currentMillis = millis();
      // if 200ms have passed, check the battery level:
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
      }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(13, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void setNumber(int side, int displayNum) {
  int sideOffset = side*characterOffset*2;
  int ones = (displayNum%10);
  int tens = ((displayNum/10)%10);
  for(int k=0;k<15;k++){
      toggleLocation(side, k+sideOffset, charLayout[tens][k]);     
      toggleLocation(side, k+characterOffset+sideOffset, charLayout[ones][k]); 
  }
  
  pixels.show();
  //delay(delayVal);
}
 
void toggleLocation(int side, int loc, int ledState){
  Serial.println(loc);
  if(ledState==0){
    
    pixels.setPixelColor(loc, pixels.Color(0,0,0)); //off
  } else{
    pixels.setPixelColor(loc, colorList[side]); // team color
  }
}

void sendDigitalValue(int pinNum) {
  if (digitalRead(myDPins[pinNum]) == LOW) {
    char lowValue = LOW;
    myDchars[pinNum].setValue(lowValue);
  } else {
    char highValue = HIGH;
    myDchars[pinNum].setValue(highValue);
  }
}

bool haschangedDigital(int pinNum) {
    char currentValue = digitalRead(myDPins[pinNum]);
    char previousValue = oldDValues[pinNum];

    if (currentValue != previousValue) {      
      oldDValues[pinNum] = currentValue;
      return true;
    } else {
      return false;
    }

}

void listener0(BLECentral& central, BLECharacteristic& characteristic) {
  setScore(0,-1);
}

void listener1(BLECentral& central, BLECharacteristic& characteristic) {
  setScore(0,0);
}

void listener2(BLECentral& central, BLECharacteristic& characteristic) {
  setScore(0,1);
}

void listener3(BLECentral& central, BLECharacteristic& characteristic) {
  setScore(1,-1);
}

void listener4(BLECentral& central, BLECharacteristic& characteristic) {
  setScore(1,0);
}

void listener5(BLECentral& central, BLECharacteristic& characteristic) {
  setScore(1,1);
}

void setScore(int team, int val){
  int teamScore = score[team];
  if(val==0){
    teamScore=0;
  } else {
    teamScore += val;
    if (teamScore<0){
        teamScore=0;
      }
  }
  score[team]=teamScore;
  setNumber(team,teamScore);
  Serial.print("Team 1: ");
  Serial.print(score[0]);
  Serial.print(" - Team 2: ");
  Serial.print(score[1]); 
  Serial.println(""); 
}
