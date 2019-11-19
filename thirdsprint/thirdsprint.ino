#include <ESP8266WiFi.h>
#include <ThingSpeak.h>;
#include <OneWire.h>
#include <DallasTemperature.h>

#define   SAMPLING  210000                  /* Value of the sampling where 1000 is equal to 1 second*/
#define   SEND_DATA 30000                   /* 30 Secons to upload to ThingSpeak with a standard Wifi */
#define   ONE_WIRE_BUS D2                   /* Bus de los 4 sensores */

const char * apiKey = "VJ3NDGMHL5LEM9M3";   // Enter your Write API key from ThingSpeak
const char *ssid =  "AndresWifi";                  // replace with your wifi ssid and wpa2 key
const char *pass =  "amepila77";
const char* server = "api.thingspeak.com";
unsigned long myChannelNumber = 879714;
const int FieldNumber1 = 1;                 // The field you wish to read
const int FieldNumber2 = 2;                 // The field you wish to read
int deviceCount = 0;
float tempSensor1, tempSensor2, tempSensor3, tempSensor4;

uint8_t sensor1[8] = { 0x28, 0xEE, 0xD5, 0x64, 0x1A, 0x16, 0x02, 0xEC  };
uint8_t sensor2[8] = { 0x28, 0x61, 0x64, 0x12, 0x3C, 0x7C, 0x2F, 0x27  };
uint8_t sensor3[8] = { 0x28, 0x61, 0x64, 0x12, 0x3F, 0xFD, 0x80, 0xC6  };
uint8_t sensor4[8] = { 0x28, 0x61, 0x64, 0x12, 0x3F, 0xFD, 0x80, 0xC6  };

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// variable to hold device addresses
DeviceAddress Thermometer;

WiFiClient client;

void setup() 
{  
  Serial.begin(9600);                       /* initialize serial communication at 9600 bits per second*/
  delay(100);
  sensors.begin();
  locateDevices();
  wifiSetup();
  ThingSpeak.begin(client);
}

void loop() 
{
  /** READING SENSORS */
  
  //int moisture = analogRead(A0);                  /* Read the input on analog pin 0*/  
  //htSensor();
 
  /** VALUES PRINTED IN THE TERMINAL */
  
  //delayTime(500);
  //Serial.println();
  //moistureFunction(moisture);                     /* Moisture function prints the moisture level*/
  //temperatureFunction();               /* Temperature funcion prints the value in Celsius*/
  //Serial.println();

  /** UPLOAD TO THINGSPEAK */
  
  //ThingSpeak.writeField(myChannelNumber, 1, moisture, apiKey);    /* Update in ThingSpeak*/
  //delayTime(SEND_DATA);
  //ThingSpeak.writeField(myChannelNumber, 2, tempSensor1, apiKey); /* Update in ThingSpeak*/
  //delayTime(SEND_DATA);

  //delayTime(SAMPLING);                                            /* Sampling of data*/
}

void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

void locateDevices(void)
{
  // locate devices on the bus
  Serial.println("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
  Serial.println("");
  
  Serial.println("Printing addresses...");
  for (int i = 0;  i < deviceCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" : ");
    sensors.getAddress(Thermometer, i);
    printAddress(Thermometer);
  }
}

void moistureFunction(int moisture)
{
  
}

void htSensor(void)
{
  sensors.requestTemperatures();
  tempSensor1 = sensors.getTempC(sensor1);
  tempSensor2 = sensors.getTempC(sensor2); 
  tempSensor3 = sensors.getTempC(sensor3);
  tempSensor4 = sensors.getTempC(sensor4);
}
void temperatureFunction(void)
{  
  Serial.print("Temperature Sensor 1(C): ");          /* Print the temperature*/     
  Serial.println(tempSensor1); 

  Serial.print("Temperature Sensor 2(C): ");          /* Print the temperature*/     
  Serial.println(tempSensor2); 

  Serial.print("Temperature Sensor 3(C): ");          /* Print the temperature*/     
  Serial.println(tempSensor3); 

  Serial.print("Temperature Sensor 4(C): ");          /* Print the temperature*/     
  Serial.println(tempSensor4); 
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

