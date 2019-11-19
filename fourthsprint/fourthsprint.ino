/**
  \file   fourthsprint
  \brief
          Este codigo representa el firmware del sistema de
          monitoreo de composta, en el cual hay 4 variables
          que son la temperatura y humedad, dentro y fuera
          de la composta.
  \author Jose Andres Hernandez Hernandez ie704453@iteso.mx
  \date   20/Nov/19
 */

/* Librerias*/
#include <ESP8266WiFi.h>                        /* Libreria de NodeMCU                    */
#include <ThingSpeak.h>                         /* Libreria de la plataforma ThingSpeak   */
#include "DHTesp.h"                             /* Libraria para el sensor de humedad     */
#include <OneWire.h>                            /* Libreria del sensor temperatura        */
#include <DallasTemperature.h>                  /* Libreria de temperatura                */

/* Defines del programa*/
#define   SAMPLING      210000                  /* Valor de muestreo donde 1000 unidades son 1 segundo            */
#define   SEND_DATA     30000                   /* 30 segundos para subir datos a ThingSpeak con el protocolo WiFi*/
#define   ONE_WIRE_BUS  D2                      /* Pin dedicado al bus de los sensores de temperatura             */
#define   INPUT_D0      16                      /* Entrada digital 1 para sensor de humedad 1                     */
#define   INPUT_D1      5                       /* Entrada digital 2 para sensor de humedad 2                     */

/* Constantes de la plataforma ThingSpeak*/
const char * apiKey = "VJ3NDGMHL5LEM9M3";       /* Write API key de ThingSpeak    */
const char *ssid    = "AndresWifi";             /* SSID de la red a utilizar      */
const char *pass    = "amepila77";              /* Password de la red a utilizar  */
const char* server  = "api.thingspeak.com";     /* Servidor de la ThingSpeak      */
unsigned long myChannelNumber = 879714;         /* Numero de canal de ThingSpeak  */
const int     FieldNumber1    = 1;              /* Numero de campo 1              */
const int     FieldNumber2    = 2;              /* Numero de campo 2              */
const int     FieldNumber3    = 3;              /* Numero de campo 3              */
const int     FieldNumber4    = 4;              /* Numero de campo 4              */

/* Variables Globales del sensor de temperatura*/
float TemperatureSensor1  = 0;                  /* Variable del sensor temperatura 1*/
float TemperatureSensor2  = 0;                  /* Variable del sensor temperatura 2*/
float TemperatureInside   = 0;                  /* Temperatura dentro de la composta*/
float TemperatureOutside  = 0;                  /* Temperatura fuera de la composta */
uint8_t address_tempsensor1[8] = { 0x28, 0xEE, 0xD5, 0x64, 0x1A, 0x16, 0x02, 0xEC  };   /* Direccion del sensor temperatura 1*/
uint8_t address_tempsensor2[8] = { 0x28, 0x61, 0x64, 0x12, 0x3C, 0x7C, 0x2F, 0x27  };   /* Direccion del sensor temperatura 2*/

/* Variables Globales del sensor de humedad*/
float MoistureSensor1 = 0;                      /* Variable del sensor humedad 1  */
float MoistureSensor2 = 0;                      /* Variable del sensor humedad 2  */
float MoistureInside  = 0;                      /* Humedad dentro de la composta  */
float MoistureOutside = 0;                      /* Humedad fuera de la composta   */

/* Instancias del programa*/
OneWire           oneWire(ONE_WIRE_BUS);        /* Configuracion del bus de sensores de temperatura           */     
DallasTemperature sensors(&oneWire);            /* Pase por referencia de la instancia a la API del fabricante*/
DeviceAddress     Thermometer;                  /* Variable para mantener la direccion de los dispositivos    */
DHTesp            dht1;                         /* Instancia del sensor de humedad 1                          */
DHTesp            dht2;                         /* Instancia del sensor de humedad 2                          */
WiFiClient        client;                       /* Creacion de la instancia de cliente en WiFi                */


/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion de configuracion inicial del sistema
  \param[in]  void
  \return     void
 */
void setup() 
{  
  Serial.begin(9600);                       /* Inicializacion de la comunicacion serial a 9600 bits por segundo   */
  delay(100);                               /* Espera de 100ms para terminar de configurar la comunicacion serial */
  dht1.setup(INPUT_D0, DHTesp::DHT11);      /* Configuracion del sensor de humedad 1 en la entrada D0             */ 
  dht2.setup(INPUT_D1, DHTesp::DHT11);      /* Configuracion del sensor de humedad 2 en la entrada D1             */ 
  sensors.begin();                          /* Inicializacion de los sensores de temperatura/humedad              */
  wifiSetup();                              /* Configuracion inicial del modulo WiFi del NodeMCU                  */
  ThingSpeak.begin(client);                 /* Inicializacion del cliente WiFi en la plataforma ThingSpeak        */
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion Main del programa
  \param[in]  void
  \return     void
 */
void loop() 
{
  /* Lectura de sensores*/
  moistureFunction();
  temperatureFunction();
 
  /* Impresion de valores en la terminal*/
  printSensors();                                                         /* Impresion del valor de los sensores                        */

  /* Subida de datos a ThingSpeak*/
  ThingSpeak.writeField(myChannelNumber, 1, MoistureInside, apiKey);      /* Muestreo de humedad dentro de la composta en ThingSpeak    */
  delayTime(SEND_DATA);                                                   /* Tiempo de espera de subida del dato a ThingSpeak           */
  ThingSpeak.writeField(myChannelNumber, 2, TemperatureInside, apiKey);   /* Muestreo de temperatura dentro de la composta en ThingSpeak*/
  delayTime(SEND_DATA);                                                   /* Tiempo de espera de subida del dato a ThingSpeak           */
  ThingSpeak.writeField(myChannelNumber, 3, MoistureOutside, apiKey);     /* Muestreo de humedad fuera de la composta en ThingSpeak     */
  delayTime(SEND_DATA);                                                   /* Tiempo de espera de subida del dato a ThingSpeak           */
  ThingSpeak.writeField(myChannelNumber, 4, TemperatureOutside, apiKey);  /* Muestreo de temperatura fuera de la composta en ThingSpeak */
  delayTime(SEND_DATA);                                                   /* Tiempo de espera de subida del dato a ThingSpeak           */
  delayTime(SAMPLING);                                                    /* Tiempo de muestreo de los datos                            */
}


void moistureFunction(void)
{
  MoistureSensor1 = dht1.getHumidity();      
  MoistureSensor2 = dht2.getHumidity();        
}

void temperatureSensor(void)
{
  sensors.requestTemperatures();
  TemperatureSensor1 = sensors.getTempC(address_tempsensor1);
  TemperatureSensor2 = sensors.getTempC(address_tempsensor2); 

}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que imprime los valores de los sensores de humedad y temperatura
              dentro y fuera de la composta en la terminal de Arduino
  \param[in]  void
  \return     void
 */
void printSensors(void)
{  
  delayTime(500);                                     /* Espera de medio segundo para poder imprimir datos      */  
  Serial.println();                                   /* Salto de linea de los datos                            */
  Serial.print("Moisture Inside Sensor (%RH): ");     /* Impresion de humedad en el interior de la composta     */     
  Serial.println(MoistureInside);                     /* Dato de humedad interior de la composta                */

  Serial.print("Temperature Inside Sensor (C): ");    /* Impresion de temperatura en el interior de la composta */    
  Serial.println(TemperatureInside);                  /* Dato de temperatura interior de la composta            */

  Serial.print("Moisture Outside Sensor (%RH): ");    /* Impresion de humedad en el exterior de la composta     */   
  Serial.println(MoistureOutside);                    /* Dato de humedad exterior de la composta                */

  Serial.print("Temperature Outside Sensor (C): ");   /* Impresion de temperatura en el exterior de la composta */   
  Serial.println(TemperatureOutside);                 /* Dato de temperatura exterior de la composta            */
  Serial.println();                                   /* Salto de linea de los datos                            */
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion de configuracion del WiFi del NodeMCU, en el cual espera la conexion 
              con la red que se establece en las constantes de red
  \param[in]  void
  \return     void
 */
void wifiSetup(void)
{
  delay(10);                              /* Pequeno delay para comenzar con la configuracion */
  Serial.println("Connecting to ");       /* Aviso en terminal mientras se conecta a la red   */
  Serial.println(ssid);                   /* Impresion en terminal del nombre de la red       */
 
  WiFi.begin(ssid, pass);                 /* Inicializacion de la red con su contrasena       */
  while (WiFi.status() != WL_CONNECTED)   /* Mientras se logra la conexion                    */
  {
    delay(500);                           /* Se espera cada medio segundo                     */
    Serial.print(".");                    /* Para verificar si se realizo la conexxion de red */
  }
  Serial.println("");                     /* Simple espacio para dar formato en terminal      */
  Serial.println("WiFi connected");       /* Aviso de que ya se logro conectar a la red       */
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion de espera no bloqueante parametrizable
  \param[in]  Intervalo en milisegundos
  \return     void
 */
void delayTime(long interval)
{
  static unsigned long previousMillis = 0;          /* Variable que guarda el estado del tiempo               */
  unsigned long currentMillis         = millis();   /* Variable que se actualiza con el valor del tiempo      */
  
  if((currentMillis - previousMillis) >= interval)  /* Mientras el valor actualizable sea mayor al intervalo  */
    previousMillis = currentMillis;                 /* El valor actualizable se guarda como valor previo      */
}

