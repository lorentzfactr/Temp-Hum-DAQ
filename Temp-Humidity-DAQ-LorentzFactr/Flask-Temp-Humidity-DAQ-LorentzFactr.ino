/* 
LorentzFactr's Temperature and Humidity Data Acquisition Unit (DAQ)
Overview: this will take a temperature and humidity reading every x seconds (see TIME_TO_SLEEP) OR take a temp/hum reading whenever
the PIR sensor detects motion. It has the ability to know if the wake up was due to the timer or if motion was detected using the A0
input. It reports the data to my Local Host creates a new row in a CSV with the following info in each cell:
Day, Time, Humidity, Temperature, Motion?

IMPORTANT: this code requires setting up your own local host to get the data. Otherwise Arduino will never connect and send
the info. I used Flask in python to setup mine. If you don't want to go down the path of creating your own server, I'd suggest 
sticking with the last version of this code that utilizes IFTTT to report the info from Arduino.


With borrowed code from:
https://github.com/MKme/ESP8266-WIFI-Humidity-Monitor


DHT Signal> D3 or GPIO 0, also connect 10k res from DHT sig to 3.3V
DHT VCC> 3.3V
DHT GND> GND
RST> D0 for wakeup (GPIO16/WAKE on NodeMCU)
PIR V+: 5V (because its power hungry)
PIR sig: Attached to NPN gate (logic inverter circuit) & Voltage Divider (for analog measuring the PIR sig)
PIR GND: GND

Logic Inverter: flips the logic for the RST line
Voltage divider: lowers the voltage to less than 1V for A0 to discern if the wake up was due to motion or the sleep timer. 


NOTE: RST line wakeup Jumper must be removed to program.
*/


#include "dontlook.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"

float h = 111;
float t = 222;
String motion;

#define DHTPIN 0                            // GPIO 0 is labelled pin D3 
#define DHTTYPE DHT11    
#define uS_TO_S_FACTOR 1000000              // Conversion factor for micro seconds to seconds - LF
#define TIME_TO_SLEEP 1200                  //Edit this to adjust the amount of seconds to sleep - LF


// Initialize DHT sensor.

DHT dht(DHTPIN, DHTTYPE);
void setup() 
{
  pinMode(D0, WAKEUP_PULLUP);               //Likely not needed, but there for safety. - LF
  int PIRval = analogRead(A0);              //Read the analog pin. - LF

  Serial.begin(9600);
  Serial.setTimeout(2000);
  
  Serial.println("Hi, I'm awake now.");

  while(!Serial) { }
  WiFi.hostname("Temp-Hum-DAQ");            //This changes the hostname of the ESP8266 to display neatly on the network esp on router.
  WiFi.begin(ssid, password);
  Serial.println("LorentzFactr's Temp & Humidity DAQ");
  dht.begin();
  delay(2000);
  
// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  
   h = dht.readHumidity();
   delay(2000);
   t = dht.readTemperature();


// Logic to discern if the wakeup was caused by motion or if was by the leep timer -LF

   if (PIRval > 50){
    motion = "TRUE";
   }
   else{motion = "FALSE";}
   
   Serial.println("Motion ADC Value: ");
   Serial.println(PIRval);
   
   Serial.println("Motion? ");
   Serial.println(motion);
  
   delay(1000);


// Check if Temp/Hum returns are valid, if they are NaN (not a number) then something went wrong. 
// Otherwise, convert to freedom units. - LF

 if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    t= 1.8*t+32; 
  }
                                                
  Serial.print("Connect to: ");                       
  Serial.println(host);
  
// WiFiClient to make HTTP connection

  WiFiClient client;
  delay(2000);                                           //give the host 2 seconds to respond before checking the connection -LF
  
  if (!client.connect(host, 5000)) {
    Serial.println("Host connection failed");
    Serial.println("Going back to sleep for 30 seconds and trying again..");
    Serial.println();
    ESP.deepSleep(120e6);                                //Don't "return;" here... this will get ESP stuck in setup never to be 
    }                                                    //woken again unless manually reset. D0 has no power here. -LF

// Build URL for Local Host
  String url = String("/") + String("?temp=") + t + String("&hum=") + h + String("&motion=") + motion;
  Serial.print("Requesting URL: ");
  Serial.println(url);

// Send request to Local Host
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n"); 
  delay(20);

// Read all the lines of the reply from server and print them to Serial
// This isn't required, I just left this in because debug. - LF

  Serial.println();
  Serial.println("closing connection");
  Serial.println();                                         //Space things our in serial monitor for purdy
  client.stop();                                           

// Not required, more of a debug thing if you want to monitor the readings in Serial port while getting it up and running.
// Obviously, while on battery power this really becomes useless...

/*
  Serial.println("Info Sent to Host:");                     
  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("Temp: ");
  Serial.println(t);
  Serial.println("Motion? ");
  Serial.println(motion);
  Serial.println("PIR ADC Value: ");
  Serial.println(PIRval);
  Serial.println();
  Serial.println("Deep sleep mode entered");
  Serial.println("******************************************"); 
  Serial.println();
  delay(200);
*/

//Put arduino to sleep to conserve battery power.
  ESP.deepSleep(uS_TO_S_FACTOR * TIME_TO_SLEEP);            //Adjust sleep time using #define TIME_TO_SLEEP at the top (in seconds).
}

 
void loop() //Where nothing happens because esp8266 deep sleep mode
{ 
   
}


  
