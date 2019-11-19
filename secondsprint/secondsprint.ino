#include <ESP8266WiFi.h>
#include <ThingSpeak.h>;
#include "DHTesp.h"                         /* Library to use the temperature sensor*/
#include <OneWire.h>
#include <DallasTemperature.h>

DHTesp    dht;                              /* Definiton of the temperature sensor*/
#define   SAMPLING  210000                  /* Value of the sampling where 1000 is equal to 1 second*/
#define   SEND_DATA 30000                   /* 30 Secons to upload to ThingSpeak with a standard Wifi */
#define   ONE_WIRE_BUS D2                   /* Bus de los 4 sensores */
const int DryLimit = 430;                   /* Dry limit of the moisture sensor*/
const int WaterLimit = 350;                 /* Water limit of the moisture sensor*/

const char * apiKey = "VJ3NDGMHL5LEM9M3";   // Enter your Write API key from ThingSpeak
const char *ssid =  "AndresWifi";                  // replace with your wifi ssid and wpa2 key
const char *pass =  "amepila77";
const char* server = "api.thingspeak.com";
unsigned long myChannelNumber = 879714;
const int FieldNumber1 = 1;                 // The field you wish to read
const int FieldNumber2 = 2;                 // The field you wish to read

WiFiClient client;

const int led = D0;                         // ESP8266 Pin to which onboard LED is connected

void setup() 
{  
  Serial.begin(9600);                       /* initialize serial communication at 9600 bits per second*/
  dht.setup(4, DHTesp::DHT11);              /* Set up of the temperature object where 4 is the GPIO4*/ 
  wifiSetup();
  ThingSpeak.begin(client);
}

void loop() 
{
  /** SENSOR VARIABLES */
  
  int moisture = analogRead(A0);                  /* Read the input on analog pin 0*/  
  float temperature = dht.getTemperature();       /* Read the temperature*/
 
  /** VALUES PRINTED IN THE TERMINAL */
  
  delayTime(500);
  Serial.println();
  moistureFunction(moisture);                     /* Moisture function prints the moisture level*/
  temperatureFunction(temperature);               /* Temperature funcion prints the value in Celsius*/
  Serial.println();

  /** UPLOAD TO THINGSPEAK */
  
  ThingSpeak.writeField(myChannelNumber, 1, moisture, apiKey);    /* Update in ThingSpeak*/
  delayTime(SEND_DATA);
  ThingSpeak.writeField(myChannelNumber, 2, temperature, apiKey); /* Update in ThingSpeak*/
  delayTime(SEND_DATA);

  delayTime(SAMPLING);                                            /* Sampling of data*/
}

void moistureFunction(int moisture)
{
  if(moisture <= WaterLimit)
  {
    Serial.print("Moisture: Very Wet ");      /* Print the moisture level*/
    Serial.print("(");
    Serial.print(moisture);                   /* Print out the value*/
    Serial.print(")");
    Serial.println();                     
  }
  else if((moisture > WaterLimit) && (moisture <= DryLimit))
  {
    Serial.print("Moisture: Wet ");           /* Print the moisture level*/
    Serial.print("(");
    Serial.print(moisture);                   /* Print out the value*/
    Serial.print(")");
    Serial.println();
  }
  else if(moisture > DryLimit)
  {
    Serial.print("Moisture: Dry ");           /* Print the moisture level*/
    Serial.print("(");
    Serial.print(moisture);                   /* Print out the value*/
    Serial.print(")");
    Serial.println();
  }    
}

void temperatureFunction(float temperature)
{
  Serial.print("Temperature (C): ");          /* Print the temperature*/     
  Serial.println(temperature); 
}

void wifiSetup(void)
{
  delay(10);                                  /* Small delay before connecting to Wi-Fi*/
  Serial.println("Connecting to ");
  Serial.println(ssid); 
 
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected"); 
}

void delayTime(long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  
  if((currentMillis - previousMillis) >= interval) 
    previousMillis = currentMillis;
}

