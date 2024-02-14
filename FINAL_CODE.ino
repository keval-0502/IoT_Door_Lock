#include <WiFi.h> // include WIFI module for ESP32
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_Fingerprint.h>   
#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

const char *wifi_ssid =  "ONEPLUS 9RT";     // Enter your WiFi Name
const char *wifi_pass =  "1234567890"; // Enter your WiFi Password
WiFiServer server(80);    // Server will be at port 80
WiFiClient client;

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "keval0502" 
#define MQTT_PASS "aio_Mqio33UMqjUArFjZsJJfOs9NCVMA" // Enter the API key that you copied from your adafrui IO account

#define relay 4
#define buzzer 18

Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);
//Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe Lock = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/Lock");

void setup()
{
  Serial.begin(115200);
  Serial2.begin(57600);
  delay(10);
  mqtt.subscribe(&Lock);
  pinMode(relay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  digitalWrite(relay, LOW); // keep motor off initally 
  digitalWrite(buzzer, LOW);
 Serial.println("Fingerprint Door Lock");
 delay(3000);

 
 // set the data rate for the sensor serial port
 finger.begin(57600);
 
 if (finger.verifyPassword()) 
 {
   Serial.println("Fingerprint Sensor Connected");
   delay(3000); 
 }
 
 else  
 {
   Serial.println("Unable to find Sensor");
   delay(3000);
   Serial.println("Check Connections");
 
   while (1) {
     delay(1);
   }
 }
  
  Serial.println("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_pass);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");              // print ... till not connected
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());           // Getting the IP address
  Serial.println("Type the above IP address into browser search bar"); 
  server.begin();                           // Starting the server
}
 
void loop()
{
  WiFiClient client = server.available();     //Checking if any client request is available or not
  if (client)
  {
    Serial.println("New Client is requesting web page"); 
    boolean currentLineIsBlank = true;
    String buffer = "";  
    while (client.connected())
    {
      if (client.available())                    // if there is some client data available
      {
        char c = client.read(); 
        buffer+=c;                              // read a byte
        if (c == '\n' && currentLineIsBlank)    // check for newline character, 
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();    
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          //client.println("<link rel=\"icon\" href=\"data:,\">");
          //client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
          //client.println(".button { background-color: #4CAF50; border: 2px solid #4CAF50;; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
          //client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"); 
          client.println("</head>");
          client.print("<body><center><h1>IOT BASED SMART DOOR LOCK</h1></center>");
          //client.println("<form><center>");
          client.print("<center><p>Relay Control</p></center>");
          client.print("<center><a href=\"/?relayon\"\"><button>DOOR UNLOCK</button></a></center>");
          client.print("<center><a href=\"/?relayoff\"\"><button>DOOR LOCK</button></a></center>");
          client.print("</body></HTML>");
          break;        // break out of the while loop:
        }
        if (c == '\n') { 
          currentLineIsBlank = true;
          buffer="";       
        } 
        else 
          if (c == '\r') {     
          if(buffer.indexOf("GET /?relayon")>=0)
          {
            digitalWrite(relay, LOW);
            digitalWrite(buzzer, HIGH);
            delay(5000);
            digitalWrite(buzzer, LOW);
          }
          if(buffer.indexOf("GET /?relayoff")>=0)
          {
            digitalWrite(relay, HIGH); 
            digitalWrite(buzzer, HIGH);
            delay(5000);
            digitalWrite(buzzer, LOW);  
          }
        }
        else {
          currentLineIsBlank = false;
        }  
      }
    }
    client.stop();
    Serial.println("Client disconnected.");
  }
  
   MQTT_connect();  
Adafruit_MQTT_Subscribe * subscription;
while ((subscription = mqtt.readSubscription(5000)))
     {   
   if (subscription == &Lock)
     {
      //Print the new value to the serial monitor
      Serial.println((char*) Lock.lastread);     
   if (!strcmp((char*) Lock.lastread, "Close"))
      {
        digitalWrite(relay, LOW);
        Serial.print("Door Unlocked - ");
        digitalWrite(buzzer, HIGH);
        delay(2000);
        digitalWrite(buzzer, LOW);
    }
    if (!strcmp((char*) Lock.lastread, "Open"))
      {
        digitalWrite(relay, HIGH);
        Serial.print("Door Closed - ");
        digitalWrite(buzzer, HIGH);
        delay(2000);
        digitalWrite(buzzer, LOW);
    }
 }  
     } 

  getFingerprintIDez();
 delay(50);            //don't need to run this at full speed.
}
 
// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() 
{
 uint8_t p = finger.getImage();
 if (p != FINGERPRINT_OK)  
 {
   Serial.println("Waiting For Valid Finger");
   return -1;
 }
 
 p = finger.image2Tz();
 if (p != FINGERPRINT_OK)  
 {   
   Serial.println("Messy Image Try Again");
   delay(3000);
   return -1;
 }
 
 p = finger.fingerFastSearch();
 if (p != FINGERPRINT_OK)  {
 
   Serial.println("Not Valid Finger");
   delay(3000);
   return -1;
 }
 
 // found a match!
   Serial.println(("Door Unlocked"));
   digitalWrite(relay, LOW);
   digitalWrite(buzzer, HIGH);
   delay(5000);
  
   Serial.println(("Closing the Door"));
   digitalWrite(relay, HIGH);
   digitalWrite(buzzer, LOW);
   delay(5000);
   
   return finger.fingerID;
}
void MQTT_connect() 
{
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  {       
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
}
