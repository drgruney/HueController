
// All SmartThings code comes from Daniel Ogorchock's ST_Anything project. 
// https://github.com/DanielOgorchock/ST_Anything
// ST_Anything is used in this project to allow a double throw toggle switch to control a SmartThings dimmer switch.

// Hue related code was written by myself with guidance from Philip's SDK documentation.

// ESP8266 code comes from Sparkfun and Adafruit exmaples and official libraries.


// library used for I2C device communication.
#include <Wire.h> 

// SmartThings Library for ESP8266WiFi
#include <SmartThingsESP8266WiFi.h> 

// ST_Anything Libraries
#include <Constants.h>       //Constants.h is designed to be modified by the end user to adjust behavior of the ST_Anything library
#include <Device.h>          //Generic Device Class, inherited by Sensor and Executor classes
#include <Sensor.h>          //Generic Sensor Class, typically provides data to ST Cloud (e.g. Temperature, Motion, etc...)
#include <Executor.h>        //Generic Executor Class, typically receives data from ST Cloud (e.g. Switch)
#include <InterruptSensor.h> //Generic Interrupt "Sensor" Class, waits for change of state on digital input 
#include <PollingSensor.h>   //Generic Polling "Sensor" Class, polls Arduino pins periodically
#include <Everything.h>      //Master Brain of ST_Anything library that ties everything together and performs ST Shield communications

#include <PS_Illuminance.h>  //Implements a Polling Sensor (PS) to measure light levels via a photo resistor

#include <PS_TemperatureHumidity.h>  //Implements a Polling Sensor (PS) to measure Temperature and Humidity via DHT library
#include <PS_DS18B20_Temperature.h>  //Implements a Polling Sesnor (PS) to measure Temperature via DS18B20 libraries 
#include <PS_Water.h>        //Implements a Polling Sensor (PS) to measure presence of water (i.e. leak detector)
#include <IS_Motion.h>       //Implements an Interrupt Sensor (IS) to detect motion via a PIR sensor
#include <IS_Contact.h>      //Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin
#include <IS_Smoke.h>        //Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin
#include <IS_DoorControl.h>  //Implements an Interrupt Sensor (IS) and Executor to monitor the status of a digital input pin and control a digital output pin
#include <IS_Button.h>       //Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin for button presses
#include <EX_Switch.h>       //Implements an Executor (EX) via a digital output to a relay
#include <EX_Alarm.h>        //Implements Executor (EX)as an Alarm Siren capability via a digital output to a relay
#include <S_TimedRelay.h>    //Implements a Sensor to control a digital output pin with timing capabilities

// library for ESP8266 microcontroller
#include <ESP8266WiFi.h>

// ssid must be for b/g/n network, cannot be a/ac
// password can be passphrase or hex, wpa, wpa2, wep
const char* ssid     = "--------";
const char* password = "--------";

const unsigned int serverPort = 8090; // port to run http server on

// hueHubIP can be found in home router DHCP table,
// or by disabling DHCP in Hue mobile app. App holds on to assigned IP
// hueID is unique user number, get yours at http://developers.meethue.com
const char* hueHubIP = "192.168.1.10";
const char* hueID = "----------------------------------------";
                     
//Smartthings Hub info
IPAddress hubIp(192, 168, 1, 130); // smartthings hub ip
const unsigned int hubPort = 39500; // smartthings hub port


WiFiClient client;

byte inputA = 0; // variable to store output of MCP23017
byte inputB = 0; // variable to store output of MCP23017


// serial output for debug
/*void callback(const String &msg)
{
  Serial.print(F("ST_Anything Callback: Sniffed data = "));
  Serial.println(msg);
}
*/

void setup() {

//Interrupt Sensors
static st::IS_Button  sensor1(F("button1"), 13, 2000, LOW, true, 500);
static st::IS_Button  sensor2(F("button2"), 14, 2000, LOW, true, 500);

st::Everything::debug=false;
st::Device::debug=false;
st::Executor::debug=false;
st::PollingSensor::debug=false;
st::InterruptSensor::debug=false;


//Initialize the optional local callback routine (safe to comment out if not desired)
st::Everything::callOnMsgSend = callback;

//Create the SmartThings ESP8266WiFi Communications Object
//DHCP IP Assigment - Must set your router's DHCP server to provice a static IP address for this device's MAC address
st::Everything::SmartThing = new st::SmartThingsESP8266WiFi(ssid, password, serverPort, hubIp, hubPort, st::receiveSmartString);

//Run the Everything class' init() routine which establishes WiFi communications with SmartThings Hub
st::Everything::init();

  //*****************************************************************************
  //Add each sensor to the "Everything" Class
  //*****************************************************************************
  st::Everything::addSensor(&sensor1);
  st::Everything::addSensor(&sensor2);

  //*****************************************************************************
  //Initialize each of the devices which were added to the Everything Class
  //*****************************************************************************
  st::Everything::initDevices();

Wire.begin(); // wake up I2C bus
Wire.write((byte)0x01); // set all of bank B to inputs
Wire.write((byte)0x00);
pinMode(13, INPUT);
pinMode(14, INPUT);

WiFi.mode(WIFI_STA);                        // sets WiFi module to be a client (station)
WiFi.begin(ssid, password);                 // uses entered variables to log in to network
}

void loop() {
  //*****************************************************************************
  //Execute the Everything run method which takes care of "Everything"
  //*****************************************************************************

/* Serial.begin(9600);
Serial.print("A:");
Serial.println(inputA, DEC); // display the contents of the GPIOB register in binary
Serial.print("B:" );
Serial.println(inputB, DEC);
*/
  st::Everything::run();

// pins 15~17 of MCP23017 to ground, I2C bus address is 0x20
// ESP8266 I2C is handled through bit-banging, not GPIO expansion. So, IO is read as bytes converted to dec.
  Wire.beginTransmission(0x20);
  Wire.write(0x12); //
  Wire.endTransmission();
  Wire.requestFrom(0x20, 1); // request one byte of data from MCP20317
  inputB = Wire.read(); // store the incoming byte into "inputs"
  /*
    if (inputB>0) {
     digitalWrite(2, LOW);
    }
    else {
     digitalWrite(2, HIGH);
    }
  */
  Wire.beginTransmission(0x20);
  Wire.write(0x13); // set MCP23017 memory pointer to GPIOB address
  Wire.endTransmission();
  Wire.requestFrom(0x20, 1); // request one byte of data from MCP20317
  inputA = Wire.read(); // store the incoming byte into "inputs"

  // allows use of LED lights on Adafruit Feather Huzzah as debug to ensure button presses are being read.
  /*
    if (inputA>0) {
     digitalWrite(2, LOW);
    }
    else {
     digitalWrite(2, HIGH);
    }
    if (inputB>0) {
     digitalWrite(0, LOW);
    }
    else {
     digitalWrite(0, HIGH);
    }
  */  

// following code interprets data captured in button presses and pushes commands to the Philips Hue Hub.
  if (inputB == 32) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/lights/18/state HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.print("Content-Length:30");
      client.println("\r\n");
      client.print("{\"on\":true,\"bri\":254,\"ct\":443}");
    }
  }

  if (inputB == 16) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/lights/18/state HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.print("Content-Length:12");
      client.println("\r\n");
      client.print("{\"on\":false}");
    }
  }

  if (inputA == 1) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/lights/11/state HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.print("Content-Length:30");
      client.println("\r\n");
      client.print("{\"on\":true,\"bri\":254,\"ct\":443}");
    }
  }

  if (inputA == 2) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/lights/11/state HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.print("Content-Length:12");
      client.println("\r\n");
      client.print("{\"on\":false}");
    }
  }

  if (inputA == 4) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/groups/2/action HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.print("Content-Length:30");
      client.println("\r\n");
      client.print("{\"on\":true,\"bri\":254,\"ct\":346}");
    }
  }
  
  if (inputA == 8) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/groups/2/action HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.print("Content-Length:12");
      client.println("\r\n");
      client.print("{\"on\":false}");
    }
  }

if (inputB == 8) {
  if (client.connect(hueHubIP, 80)) {
    client.print("PUT /api/");
    client.print(hueID);
    client.println("/groups/2/action HTTP/1.1");
    client.print("Host:");
    client.println(hueHubIP);
    client.print("Content-Length: 27");
    client.println("\r\n");
    client.print("{\"scene\":\"s4bhoGejvMmhE6d\"}");
  
    }
  }

if (inputB == 4) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/groups/2/action HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.println("Content-Length: 12\r\n");
      client.print("{\"on\":false}");
    }
  }

if (inputB == 2) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/groups/1/action HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.print("Content-Length:30");
      client.println("\r\n");
      client.print("{\"on\":true,\"bri\":254,\"ct\":346}");
    }
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/groups/2/action HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.print("Content-Length:30");
      client.println("\r\n");
      client.print("{\"on\":true,\"bri\":254,\"ct\":346}");
    }
  }

if (inputB == 1) {
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/groups/2/action HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.println("Content-Length: 12\r\n");
      client.print("{\"on\":false}");
    }
    if (client.connect(hueHubIP, 80)) {
      client.print("PUT /api/");
      client.print(hueID);
      client.println("/groups/1/action HTTP/1.1");
      client.print("Host:");
      client.println(hueHubIP);
      client.println("Content-Length: 12\r\n");
      client.print("{\"on\":false}");
    
  }
}
}
