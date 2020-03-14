#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <OneWire.h>                            /* Libreria del sensor temperatura        */
#include <DallasTemperature.h>                  /* Libreria de temperatura                */
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

/* Defines del programa*/
#define   SAMPLING      1000                   /* Valor de muestreo donde 1000 unidades son 1 segundo            */
#define   SEND_DATA     30000                   /* 30 segundos para subir datos a ThingSpeak con el protocolo WiFi*/
#define   ONE_WIRE_BUS  D3                      /* Pin dedicado al bus de los sensores de temperatura             */
#define   DRY_LIMIT     430
#define   WATER_LIMIT   350

/* Muestreo*/
unsigned int FlagSampling;                      /* Bandera de muestreo*/
unsigned int CounterSamples = 0;                /* Contador de muestras para realizar el promedio*/
#define   MAX_COUNT     1

const int DryLimit = 430;                   /* Dry limit of the moisture sensor*/
const int WaterLimit = 350;                 /* Water limit of the moisture sensor*/

/* Constantes de la plataforma ThingSpeak*/
const char * apiKey = "X3YGLGST69JA5RUM";       /* Write API key de ThingSpeak    */
const char *ssid    = "AndresWifi";             /* SSID de la red a utilizar      */
const char *pass    = "amepila77";              /* Password de la red a utilizar  */
const char* server  = "api.thingspeak.com";     /* Servidor de la ThingSpeak      */
unsigned long myChannelNumber = 879714;         /* Numero de canal de ThingSpeak  */
const int     FieldNumber1    = 1;              /* Numero de campo 1              */
const int     FieldNumber2    = 2;              /* Numero de campo 2 */

/* Variables Globales del sensor de temperatura*/
float TemperatureSensor  = 0;                  /* Variable del sensor temperatura 1*/
uint8_t address_tempsensor[8] = { 0x28, 0xFF, 0x36, 0x3C, 0xA2, 0x16, 0x05, 0xCA  };   /* Direccion del sensor temperatura 1*/

/* Variables Globales del sensor de humedad*/
float MoistureSensor = 0;                      /* Variable del sensor humedad 1  */

/* Instancias del programa*/
OneWire           oneWire(ONE_WIRE_BUS);        /* Configuracion del bus de sensores de temperatura           */     
DallasTemperature sensors(&oneWire);            /* Pase por referencia de la instancia a la API del fabricante*/
DeviceAddress     Thermometer;                  /* Variable para mantener la direccion de los dispositivos    */
WiFiClient        client;                       /* Creacion de la instancia de cliente en WiFi                */
Adafruit_PCD8544 display = Adafruit_PCD8544(D8, D7, D6, D5, D4);

void setup() 
{
  pinMode(D3, INPUT);                       /* Sets the digital PIN 3 as input*/
  Serial.begin(9600);                       /* initialize serial communication at 9600 bits per second*/
  
  display.begin();
  display.setContrast(57);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Hello world!");
  display.display();
  delay(2000);
  display.clearDisplay();
  
  sensors.begin();                          /* Inicializacion de los sensores de temperatura/humedad              */
  wifiSetup();                              /* Configuracion inicial del modulo WiFi del NodeMCU                  */
  ThingSpeak.begin(client);                 /* Inicializacion del cliente WiFi en la plataforma ThingSpeak        */
}

void loop() 
{
  FlagSampling = delayTime(SAMPLING);   /* Se obtiene la bandera de frecuencia de muestreo*/
  if(FlagSampling)
  {
    moistureFunction();            /* Lectura de sensores de humedad                     */
    temperatureFunction();         /* Lectura de sensores de temperatura                 */
    printSensors();                     /* Impresion del valor de los sensores en la terminal */
    displaytext();
    //ThingSpeakUpload();
  }
}

void moistureFunction(void)
{
  MoistureSensor = analogRead(A0);
}

void temperatureFunction(void)
{
  sensors.requestTemperatures();                                /* Se realiza un request para leer el bus de sensores*/
  TemperatureSensor = sensors.getTempC(address_tempsensor);   /* Se lee el sensor de temperatura 1*/
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
  Serial.println(MoistureSensor);                     /* Dato de humedad interior de la composta                */

  Serial.print("Temperature Inside Sensor (C): ");    /* Impresion de temperatura en el interior de la composta */    
  Serial.println(TemperatureSensor);                  /* Dato de temperatura interior de la composta            */
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
  \return     flagCount
 */
unsigned int delayTime(unsigned long interval)
{
  static unsigned long previousMillis = 0;          /* Variable que guarda el estado del tiempo               */
  unsigned long currentMillis         = millis();   /* Variable que se actualiza con el valor del tiempo      */
  unsigned int flagCount = 0;
  
  if((currentMillis - previousMillis) >= interval)  /* Mientras el valor actualizable sea mayor al intervalo  */
  {
    flagCount = 1;                                  /* Si termina la cuenta se activa la bandera              */
    previousMillis = currentMillis;                 /* El valor actualizable se guarda como valor previo      */
  }else
  {
    flagCount = 0;                                  /* Mientras no alcance el valor se mantiene en 0          */
  }
  return flagCount;
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que sube los datos a la nube de ThingSpeak
  \param[in]  void
  \return     void
 */
void ThingSpeakUpload(void)
{
  /* Subida de datos a ThingSpeak*/
  ThingSpeak.writeField(myChannelNumber, 1, TemperatureSensor, apiKey);      /* Muestreo de humedad dentro de la composta en ThingSpeak    */
  delay(SEND_DATA);                                                       /* Tiempo de espera de subida del dato a ThingSpeak           */
  ThingSpeak.writeField(myChannelNumber, 2, MoistureSensor, apiKey);   /* Muestreo de temperatura dentro de la composta en ThingSpeak*/
  delay(SEND_DATA);                                                       /* Tiempo de espera de subida del dato a ThingSpeak           */
}

void displaytext(void)
{
  
  // Display Text
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Hello world!");
  display.display();
  delay(2000);
  //display.clearDisplay();
}

