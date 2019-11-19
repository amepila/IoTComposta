#include <ESP8266WiFi.h>
#include <ThingSpeak.h>;
#include "DHTesp.h"                         /* Library to use the temperature sensor*/

DHTesp    dht;                              /* Definiton of the temperature sensor*/
#define   SAMPLING  15000                    /* Value of the sampling where 1000 is equal to 1 second*/
#define   SEND_DATA 20000
const int DryLimit = 430;                   /* Dry limit of the moisture sensor*/
const int WaterLimit = 350;                 /* Water limit of the moisture sensor*/

const char * apiKey = "VJ3NDGMHL5LEM9M3"; // Enter your Write API key from ThingSpeak
const char *ssid =  "AndresWifi";     // replace with your wifi ssid and wpa2 key
const char *pass =  "amepila77";
const char* server = "api.thingspeak.com";
unsigned long myChannelNumber = 879714;
const int FieldNumber1 = 1;  // The field you wish to read
const int FieldNumber2 = 2;  // The field you wish to read
const int FieldNumber3 = 3;  // The field you wish to read

WiFiClient client;

void setup() 
{
  pinMode(D3, INPUT);                       /* Sets the digital PIN 3 as input*/
  Serial.begin(9600);                       /* initialize serial communication at 9600 bits per second*/
  dht.setup(4, DHTesp::DHT11);              /* Set up of the temperature object where 4 is the GPIO4*/ 
  wifiSetup();
  ThingSpeak.begin(client);
}

void loop() 
{
  int moisture = analogRead(A0);                  /* Read the input on analog pin 0*/  
  float temperature = dht.getTemperature();       /* Read the temperature*/
  int gas = digitalRead(D3);                      /* Read the gas sensor from digital pin*/

  delay(500);
  Serial.println();
  moistureFunction(moisture);                 /* Moisture function prints the moisture level*/
  temperatureFunction(temperature);           /* Temperature funcion prints the value in Celsius*/
  gas = gasFunction(gas);                     /* Gas function prints the presence of gas*/
  Serial.println();

  ThingSpeak.writeField(myChannelNumber, 1, moisture, apiKey); //Update in ThingSpeak
  delay(SEND_DATA);
  ThingSpeak.writeField(myChannelNumber, 2, temperature, apiKey); //Update in ThingSpeak
  delay(SEND_DATA);
  ThingSpeak.writeField(myChannelNumber, 3, gas, apiKey); //Update in ThingSpeak
  delay(SEND_DATA);

  delay(SAMPLING);                            /* Sampling of data*/
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

int gasFunction(int gas)
{
  int presence = 0;
  
  if(gas == HIGH)
  {
    Serial.println("Absence of gas");        /*Print the presence or absence of gas*/
    presence = LOW;
  }
  else
  {
    Serial.println("Presence of gas");
    presence = HIGH;
  }
  return presence;
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


