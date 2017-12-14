#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "HX711.h"
#include "SSD1306.h"

// Map the NodeMCU pins to the ESP8266 GPIO

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

// Wifi Login Details
const char* ssid     = "SSID";
const char* password = "password";

// IFTTT Webhook Details
const char* host = "maker.ifttt.com";
const char* streamId   = "streamID";
const char* privateKey = "User specific string";
// 'User specific string' is the long string after maker.ifttt.com/use/'User specific string'
// Head to https://ifttt.com/services/maker_webhooks/settings whilst logged in to find it

// Initialize empty string for sending to IFTTT
String valueToSend = "";
String urlForMaker = "";

// Initalise variabes for scale
float weightFromScale = 0;
float truncatedWeight = 0;

// Setup timer for display refresh
unsigned long previousMillis = 0;
const long refreshTime = 2000;

// Define Pins
#define redLed D7
#define buttonPin  D1

// Data and Clock setup for the HX711
#define scaleDT D3
#define scaleSCK D2
HX711 scale(scaleDT, scaleSCK);
// Adjust calibration factor as required  
float calibration_factor = 22150;

// Initalise OLED Display
#define oledSCL D6
#define oledSDA D5
SSD1306  display(0x3c, oledSDA, oledSCL);

// Initalise timers
unsigned long previousMillis1 = 0;  
const long interval1 = 1000;           // Delay for screen refresh rate 
bool updateDisplay = 1;
void setup() {

  // Set Inputs / Outputs
  pinMode(redLed, OUTPUT);
  pinMode(buttonPin, INPUT);

  // Initialize display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  
  // Tare Scale
  scale.set_scale();
  scale.tare();

  Serial.begin(115200);
  delay(10);

  // Connect to a WiFi network

  display.clear();
  display.display();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 0, "CONNECTING");

  // write the buffer to the display
  display.display();
  delay(100);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  // Attempt to connect to WiFi, if not connected 
  // it keeps trying forever
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  // Update Display that WiFi is connected for one second
  display.clear();
  display.display();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(8, 0, "WiFi Connected!");
  // write the buffer to the display
  display.display();
  delay(2000);
  display.clear();
  display.display();



}

void loop(){

  // Get Weight from scale
    weightFromScale = getWeightFromScale();
    if (updateDisplay == 1){
  // Display weight on screen
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(30, 0, "WEIGHT:");
    display.setFont(ArialMT_Plain_24);
    display.drawString(37, 20, String(weightFromScale));
    display.setFont(ArialMT_Plain_10);
    display.drawString(30, 50, "Science Units"); // Alter depending on units
    // write the buffer to the display
    display.display();
    updateDisplay = 0;
  }
    // Delay Timer
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis;
    display.clear();
    display.display();
    updateDisplay = 1;
  }
  
    // When button is pressed, find weight and send to IFTTT
    // Also flashes LED if connected to D7
    int buttonState = digitalRead(buttonPin);
    if (buttonState == HIGH){
      digitalWrite(redLed, HIGH);
      weightFromScale = getWeightFromScale();
      sendToIFTTT(weightFromScale);
    display.clear();
    display.display();
    delay(100);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(4, 20, "Weight Tweeted!");
    // write the buffer to the display
    display.display();
    delay(1000);
    display.clear();

  }
  else{
      digitalWrite(redLed, LOW);
    }

  
}


float getWeightFromScale(){
 
  scale.set_scale(calibration_factor); // Adjusts calibration based on above defined calibration number

  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1);
  Serial.print(" Kg"); // Alter as required
  Serial.println("");
  weightFromScale = (scale.get_units());
  return weightFromScale;
}



void sendToIFTTT(float weight){
  // A messy attempt to get the weight into the JSON string
  // I have since found a library 'arduino-ifttt-maker' which makes this redundant
  
  // The JSON Value to Send
  valueToSend = "{\"value1\" : \"";
  valueToSend.concat(weight);
  valueToSend.concat("""\", \"value2\" : \"\", \"value3\" : \"\"}""");

  // The URL to POST to
  urlForMaker = "http://maker.ifttt.com/trigger/";
  urlForMaker.concat(streamId);
  urlForMaker.concat("/with/key/");
  urlForMaker.concat(privateKey);
  
 if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
 
   HTTPClient http;    //Declare object of class HTTPClient
 
   http.begin(urlForMaker); 
   http.addHeader("Content-Type", "application/json");
   http.POST(valueToSend);
   http.writeToStream(&Serial);
   http.end();
   Serial.println("");
 
  }
 
 else{
     Serial.println("Error in WiFi connection");   

  }
  
}


  

