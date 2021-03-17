/* 
LorentzFactr's Temperature and Humidity Data Acquisition Unit (DAQ)
Overview: this will take a temperature and humidity reading every x seconds (see TIME_TO_SLEEP) OR take a temp/hum reading whenever
the PIR sensor detects motion. It has the ability to know if the wake up was due to the timer or if motion was detected using the A0
input. It reports the data to IFTTT and IFTTT creates a new row in a spread sheet with the following info in each cell:
Day/Time, Event Name, Humidity, Temperature, Motion?


With borrowed code from:
https://github.com/MKme/ESP8266-WIFI-Humidity-Monitor


DHT Signal> D3 or GPIO0, also connect 10k res from DHT sig to 3.3V
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


String result;
float h = 111;
float t = 222;
String motion;



#include "DHT.h"
#define DHTPIN 0                          // GPIO 0 is labelled pin D3 
#define DHTTYPE DHT11    
#define uS_TO_S_FACTOR 1000000            // Conversion factor for micro seconds to seconds - LF
#define TIME_TO_SLEEP 1200                //Edit this to adjust the amount of seconds to sleep - LF


// Initialize DHT sensor.

DHT dht(DHTPIN, DHTTYPE);
void setup() 
{
  pinMode(D0, WAKEUP_PULLUP);           //Likely not needed, but there for safety. - LF
  int PIRval = analogRead(A0);          //Read the analog pin. - LF
  Serial.begin(9600);
  Serial.setTimeout(2000);
  

  Serial.println("Hi I'm awake now. ");

  
  while(!Serial) { }
  WiFi.hostname("Temp-Hum-DAQ");      //This changes the hostname of the ESP8266 to display neatly on the network esp on router.
  WiFi.begin(ssid, password);
  Serial.println("LorentzFactr's Temp & Humidity DAQ");
  dht.begin();
  delay(2000);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  
   h = dht.readHumidity();
   t = dht.readTemperature();

   //logic to discern if the wakeup was caused by motion or if was by the leep timer -LF
   if (PIRval > 100){
    motion = "TRUE";
   }
   else{motion = "FALSE";}
   delay(1000);


  //Check if Temp/Hum returns are valid, if they are NaN (not a number) then something went wrong. 
  //Otherwise, convert to freedom units. - LF
 if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    t= 1.8*t+32; 
  }

 
  
  const char trigger[ ]     = "small_PP_pics";          //name of the trigger you would like to send to IFTTT
                                                        //S/O to Yiren for this mature request to name the event "small_PP_pics" - LF
                                                        
  Serial.print("Connect to: ");                       
  Serial.println(host);
  
  //WiFiClient to make HTTP connection
  WiFiClient client;
  delay(2000);                                          //give IFTTT 2 seconds to respond before checking the connection -LF
  
  if (!client.connect(host, 80)) {
    Serial.println("Host connection failed");
    Serial.println("Going back to sleep for 30 seconds and trying again..");
    Serial.println();
    ESP.deepSleep(30e6);                                //Don't "return;" here... this will get ESP stuck in setup never to be 
    }                                                   //woken again unless manually reset. D0 has no power here. -LF

// Build URL for IFTTT host
  String url = String("/trigger/") + trigger + String("/with/key/") + APIKey + String("?value1=") + h + String("&value2=") + t + String("&value3=") + motion;
  Serial.print("Requesting URL: ");
  Serial.println(url);

// Send request to IFTTT
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n"); 
  delay(20);

// Read all the lines of the reply from server and print them to Serial
// Personally I haven't seen the server respond with anything ever. I just left this in because reasons. - LF
  Serial.println("Respond:");
 while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
    }
  Serial.println();
  Serial.println("closing connection");
  Serial.println();                                         //space things our in serial monitor for purdy
  client.stop();                                           
  
  Serial.println("Info Sent to Host:");                     //Send things to serial for handy dandy info, but not required.
  Serial.print("humidity ");
  Serial.println(h);
  Serial.print("temp ");
  Serial.println(t);
  Serial.println("Motion? ");
  Serial.println(motion);
  Serial.println("Deep sleep mode entered");
  Serial.println("******************************************"); 
  Serial.println();
  ESP.deepSleep(uS_TO_S_FACTOR * TIME_TO_SLEEP);            //Adjust sleep time using #define TIME_TO_SLEEP at the top.
 }
 
void loop() //Where nothing happens because esp8266 deep sleep mode
{ 
   
}


  
