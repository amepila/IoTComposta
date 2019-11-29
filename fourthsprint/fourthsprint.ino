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
#include "Wire.h"                               /* Libreria para el I2C del acelerometro  */

/* Defines del programa*/
#define   SAMPLING      210000                  /* Valor de muestreo donde 1000 unidades son 1 segundo            */
#define   SEND_DATA     30000                   /* 30 segundos para subir datos a ThingSpeak con el protocolo WiFi*/
#define   ONE_WIRE_BUS  D3                      /* Pin dedicado al bus de los sensores de temperatura             */
#define   INPUT_D4      2                       /* Entrada digital 1 para sensor de humedad 1                     */
#define   INPUT_D5      14                      /* Entrada digital 2 para sensor de humedad 2                     */

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

/* Constantes del acelerometro y giroscopio*/
const uint8_t MPU_addr          = 0x68;         /* Direccion I2C del acelerometro */
const float MPU_GYRO_250_SCALE  = 131.0;        /* Escala de giroscopio en 250U   */
const float MPU_GYRO_500_SCALE  = 65.5;         /* Escala de giroscopio en 500U   */
const float MPU_GYRO_1000_SCALE = 32.8;         /* Escala de giroscopio en 1000U  */
const float MPU_GYRO_2000_SCALE = 16.4;         /* Escala de giroscopio en 2000U  */
const float MPU_ACCL_2_SCALE    = 16384.0;      /* Escala de acelerometro en 2U   */
const float MPU_ACCL_4_SCALE    = 8192.0;       /* Escala de acelerometro en 4U   */
const float MPU_ACCL_8_SCALE    = 4096.0;       /* Escala de acelerometro en 8U   */
const float MPU_ACCL_16_SCALE   = 2048.0;       /* Escala de acelerometro en 16U  */

/* Variables Globales del sensor de temperatura*/
float TemperatureSensor1  = 0;                  /* Variable del sensor temperatura 1*/
float TemperatureSensor2  = 0;                  /* Variable del sensor temperatura 2*/
float TemperatureInside   = 0;                  /* Temperatura dentro de la composta*/
float TemperatureOutside  = 0;                  /* Temperatura fuera de la composta */
uint8_t address_tempsensor1[8] = { 0x28, 0x4E, 0x57, 0x79, 0x97, 0x11, 0x03, 0x1F  };   /* Direccion del sensor temperatura 1*/
uint8_t address_tempsensor2[8] = { 0x28, 0xFF, 0x44, 0x36, 0xA2, 0x16, 0x05, 0x3F  };   /* Direccion del sensor temperatura 2*/

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

/* Estructura de datos de acelerometro/giroscopio sin mapeo*/
struct rawdata
{
  int16_t AcX;        /* Eje X del acelerometro */
  int16_t AcY;        /* Eje Y del acelerometro */
  int16_t AcZ;        /* Eje Z del acelerometro */
  int16_t Tmp;        /* Variable temporal      */
  int16_t GyX;        /* Eje X del giroscopio   */
  int16_t GyY;        /* Eje Y del giroscopio   */
  int16_t GyZ;        /* Eje Z del giroscopio   */
};

/* Estructura de datos de acelerometro/giroscopio con mapeo*/
struct scaleddata
{
  float AcX;          /* Eje X del acelerometro escalado  */
  float AcY;          /* Eje Y del acelerometro escalado  */
  float AcZ;          /* Eje Z del acelerometro escalado  */
  float Tmp;          /* Variable temporal escalada       */
  float GyX;          /* Eje X del giroscopio escalado    */
  float GyY;          /* Eje Y del giroscopio escalado    */
  float GyZ;          /* Eje Z del giroscopio escalado    */
};

/* Prototipos de funciones del acelerometro/giroscopio*/
bool checkI2c(byte addr);                                               /* Funcion que verifica el estado del dispositivo en I2C*/
void mpu6050Begin(byte addr);                                           /* Inicializacion del sensor                            */
rawdata mpu6050Read(byte addr, bool Debug);                             /* Lectura de datos sin mapear del sensor               */
void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl);             /* Establecer escala de los latos leidos                */
void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl);           /* Conseguir datos escalados del sensor                 */
scaleddata convertRawToScaled(byte addr, rawdata data_in,bool Debug);   /* Conversion de datos sin mapear a escalados           */
 
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
  Wire.begin();                             /* Inicializacion del protocolo I2C para el acelerometro/giroscopio   */
  Serial.begin(9600);                       /* Inicializacion de la comunicacion serial a 9600 bits por segundo   */
  mpu6050Begin(MPU_addr);                   /* Inicializacion del acelerometro/giroscopio con su direccion        */
  delay(100);                               /* Espera de 100ms para terminar de configurar la comunicacion serial */
  dht1.setup(INPUT_D4, DHTesp::DHT11);      /* Configuracion del sensor de humedad 1 en la entrada D4             */ 
  dht2.setup(INPUT_D5, DHTesp::DHT11);      /* Configuracion del sensor de humedad 2 en la entrada D5             */ 
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
  bool north = positionSensor();  /* Position del norte con el giroscopio               */
  moistureFunction(north);        /* Lectura de sensores de humedad                     */
  temperatureFunction(north);     /* Lectura de sensores de temperatura                 */
  printSensors();                 /* Impresion del valor de los sensores en la terminal */
  ThingSpeakUpload();             /* Subida de informacion a la nube de ThingSpeak      */
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que inicializa el integrado o sensor, que verifica la direccion correcta
  \param[in]  addr
  \return     void
 */
void mpu6050Begin(byte addr)
{
  if(checkI2c(addr))                    /* Si la direccion del dispositivo es correcta se realiza la transmision*/
  {
    Wire.beginTransmission(MPU_addr);   /* Se realiza la transmision en el bus de I2C para encontrar al sensor  */
    Wire.write(0x6B);                   /* Se escribe el registro PWR_MGMT_1                                    */
    Wire.write(0);                      /* Se setea en cero para desepertar al dispositivo                      */
    Wire.endTransmission(true);         /* Termina la transmision de inicializacion                             */
    delay(30);                          /* Este delay da tiempo para que el giroscopio despierte                */
  }
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que inicializa el integrado o sensor, que verifica la direccion correcta
  \param[in]  addr
  \return     void
 */
bool checkI2c(byte addr)
{
  Serial.println(" ");                      /* Impresion de salto de linea              */
  Wire.beginTransmission(addr);             /* Se realiza una transmision con el sensor */
   
  if (Wire.endTransmission() == 0)          /* Si se recibe un acknowledge              */
  {
    Serial.print(" Device Found at 0x");    /* Se imprime la direccion del dispositivo  */
    Serial.println(addr,HEX);               /* La direccion del sensor acelerometro     */
    return true;                            /* Se retorna un valor en true              */
  }
  else                                      /* Si no se recibe el acknowledge           */
  {
    Serial.print(" No Device Found at 0x"); /* Se imprime el aviso de que fue encontrado*/
    Serial.println(addr,HEX);               /* Se imprime la direccion del sensor       */
    return false;                           /* Se retorna un valor en falso             */
  }
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que lee los datos de 16-bits del sensor
  \param[in]  addr, Debug
  \return     rawdata
 */
rawdata mpu6050Read(byte addr, bool Debug)
{
  rawdata values;                           /* Declaracion de los datos a leer          */
   
  Wire.beginTransmission(addr);             /* Se comienza la transmision en I2C        */
  Wire.write(0x3B);                         /* Inicia con el registro de ACCEL_XOUT_H   */
  Wire.endTransmission(false);              /* Se finaliza la transmisio                */
  Wire.requestFrom(addr,14,true);           /* Se piden los valores de 14 registros     */
  
  values.AcX = Wire.read()<<8|Wire.read();  /* Lectura de 0x3B (ACCEL_XOUT_H) y 0x3C (ACCEL_XOUT_L)   */
  values.AcY = Wire.read()<<8|Wire.read();  /* Lectura de 0x3D (ACCEL_YOUT_H) y 0x3E (ACCEL_YOUT_L)   */
  values.AcZ = Wire.read()<<8|Wire.read();  /* Lectura de 0x3F (ACCEL_ZOUT_H) y 0x40 (ACCEL_ZOUT_L)   */
  values.Tmp = Wire.read()<<8|Wire.read();  /* Lectura de 0x41 (TEMP_OUT_H) y 0x42 (TEMP_OUT_L)       */
  values.GyX = Wire.read()<<8|Wire.read();  /* Lectura de 0x43 (GYRO_XOUT_H)) y 0x44 (GYRO_XOUT_L)    */ 
  values.GyY = Wire.read()<<8|Wire.read();  /* Lectura de 0x45 (GYRO_YOUT_H) y 0x46 (GYRO_YOUT_L)     */
  values.GyZ = Wire.read()<<8|Wire.read();  /* Lectura de 0x47 (GYRO_ZOUT_H) y 0x48 (GYRO_ZOUT_L)     */
    
  if(Debug)                                                 /* Si el modo debugueo esta activado    */
  {
    Serial.print(" GyX = "); Serial.print(values.GyX);      /* Imprime el valor del giroscopio eje X    */
    Serial.print(" | GyY = "); Serial.print(values.GyY);    /* Imprime el valor del giroscopio eje Y    */
    Serial.print(" | GyZ = "); Serial.print(values.GyZ);    /* Imprime el valor del giroscopio eje Z    */
    Serial.print(" | Tmp = "); Serial.print(values.Tmp);    /* Imprime el valor tempoeral del giroscopio*/
    Serial.print(" | AcX = "); Serial.print(values.AcX);    /* Imprime el valor del acelerometro eje X  */
    Serial.print(" | AcY = "); Serial.print(values.AcY);    /* Imprime el valor del acelerometro eje Y  */
    Serial.print(" | AcZ = "); Serial.println(values.AcZ);  /* Imprime el valor del acelerometro eje Z  */
  }
   
  return values;
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que escala los valores del acelerometro y giroscopio
  \param[in]  addr, Gyro, Accl
  \return     void
 */
void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl)
{
  Wire.beginTransmission(addr);   /* Se inicia la transmision por I2C         */
  Wire.write(0x1B);               /* Escribe el registro de incializacion 0x1B*/
  Wire.write(Gyro);               /* Se avisa al sensor giroscopio            */
  Wire.write(Accl);               /* Se avisa al sensor acelerometro          */
  Wire.endTransmission(true);     /* Se finaliza la transmision por I2C       */
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que consigue los valores del acelerometro y giroscopio
  \param[in]  addr, &Gyro, &Accl
  \return     void
 */
void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl)
{
  Wire.beginTransmission(addr);             /* Se inicia la transmision por I2C         */
  Wire.write(0x1B);                         /* Escribe el registro de incializacion Ox1B*/
  Wire.endTransmission(false);              /* Se finaliza la transmision por I2C       */
  Wire.requestFrom(addr,2,true);            /* Se piden los valores de los 14 registros */
  Gyro = (Wire.read()&(bit(3)|bit(4)))>>3;  /* Se obtienen los valores normalizados     */
  Accl = (Wire.read()&(bit(3)|bit(4)))>>3;  /* Del sensor giroscopio y acelerometro     */
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que escala los valores del acelerometro y giroscopio
  \param[in]  addr, data_in, Debug
  \return     void
 */
scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug)
{
  scaleddata values;                          /* Declaracion de los valores escalados */
  float scale_value = 0.0;                    /* Definicion del valor base escalado   */
  byte Gyro, Accl;                            /* Declaracion de selector del sensor   */
   
  getMPU6050scales(MPU_addr, Gyro, Accl);     /* Se consiguen los datos escalados     */
   
  if(Debug)                                   /* Si esta en modo debug                */
  {
    Serial.print("Gyro Full-Scale = ");       /* Se imprime la escala completa        */
  }
   
  switch (Gyro)                               /* Selector de escala del giroscopio    */
  {
    case 0:
      scale_value = MPU_GYRO_250_SCALE;       /* 0: Escala de 250U                    */
      if(Debug)                               /* Si esta en modo debug                */
      {
        Serial.println("±250 °/s");           /* Se imprime la escala                 */
      }
    break;
    
    case 1:
      scale_value = MPU_GYRO_500_SCALE;       /* 1: Escala de 500U                    */
      if(Debug)                               /* Si esta en modo debug                */
      {
        Serial.println("±500 °/s");           /* Se imprime la escala                 */
      }
    break;
    
    case 2:                                   
      scale_value = MPU_GYRO_1000_SCALE;      /* 2: Escala de 1000U                   */
      if(Debug)                               /* Si esta en modo debug                */
      {
        Serial.println("±1000 °/s");          /* Se imprime la escala                 */
      }
    break;
    
    case 3:
      scale_value = MPU_GYRO_2000_SCALE;      /* 3: Escala de 2000U                   */
      if(Debug)                               /* Si esta en modo debug                */
      {
        Serial.println("±2000 °/s");          /* Se imprime la escala                 */
      }
    break;
    
    default:
    break;
  }
   
  values.GyX = (float) data_in.GyX / scale_value;   /* Se consigue el valor normalizado del giroscopio en eje X */
  values.GyY = (float) data_in.GyY / scale_value;   /* Se consigue el valor normalizado del giroscopio en eje Y */
  values.GyZ = (float) data_in.GyZ / scale_value;   /* Se consigue el valor normalizado del giroscopio en eje Z */
   
  scale_value = 0.0;                                /* Se reinicia el valor escala a 0                          */
  if(Debug)                                         /* Si esta en modo debug                                    */
  {
    Serial.print("Accl Full-Scale = ");             /* Se indica la escala total con el acelerometro            */
  }
  
  switch (Accl)                                     /* Selector de escala del acelerometro                      */
  {
    case 0:
      scale_value = MPU_ACCL_2_SCALE;               /* 0: Escala de 2G                                          */
      if(Debug)                                     /* Si esta en modo debug                                    */
      {
        Serial.println("±2 g");                     /* Se imprime la escala                                     */
      }
    break;
    
    case 1:
      scale_value = MPU_ACCL_4_SCALE;               /* 1: Escala de 4G                                          */
      if(Debug)                                     /* Si esta en modo debug                                    */
      {
      Serial.println("±4 g");                       /* Se imprime la escala                                     */
      }
    break;
    
    case 2:
      scale_value = MPU_ACCL_8_SCALE;               /* 2: Escala de 8G                                          */
      if(Debug)                                     /* Si esta en modo debug                                    */
      {
        Serial.println("±8 g");                     /* Se imprime la escala                                     */
      }
    break;
    
    case 3:
      scale_value = MPU_ACCL_16_SCALE;              /* 3: Escala de 16G                                         */
      if(Debug)                                     /* Si esta en modo debug                                    */
      {
        Serial.println("±16 g");                    /* Se imprime la escala                                     */
      }
    break;
    
    default:
    break;
  }
  values.AcX = (float) data_in.AcX / scale_value;   /* Se consigue el valor normalizado del acelerometro en eje X   */
  values.AcY = (float) data_in.AcY / scale_value;   /* Se consigue el valor normalizado del acelerometro en eje Y   */
  values.AcZ = (float) data_in.AcZ / scale_value;   /* Se consigue el valor normalizado del acelerometro en eje Z   */
  values.Tmp = (float) data_in.Tmp / 340.0 + 36.53; /* Se consigue el valor normalizado del acelerometro en eje TMP */
   
  if(Debug)                                                   /* Si esta en modo debud                    */
  {
    Serial.print(" GyX = "); Serial.print(values.GyX);        /* Se imprime el valor del giroscopio eje X     */
    Serial.print(" °/s| GyY = "); Serial.print(values.GyY);   /* Se imprime el valor del giroscopio eje Y     */
    Serial.print(" °/s| GyZ = "); Serial.print(values.GyZ);   /* Se imprime el valor del giroscopio eje Z     */
    Serial.print(" °/s| Tmp = "); Serial.print(values.Tmp);   /* Se imprime el valor del tempoera             */
    Serial.print(" °C| AcX = "); Serial.print(values.AcX);    /* Se imprime el valor del acelerometro eje X   */
    Serial.print(" g| AcY = "); Serial.print(values.AcY);     /* Se imprime el valor del acelerometro eje Y   */
    Serial.print(" g| AcZ = "); Serial.print(values.AcZ);     /* Se imprime el valor del acelerometro eje Z   */
    Serial.println(" g");
  }
  return values;                                              /* Se retornan los valores finales del sensor   */
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que utiliza el giroscopio para determinar si el norte esta fuera de su 
              posicion y si esta fuera, se mandara un FALSE sino un TRUE, en caso de que este
              en su posicion original o cercana dentro de sus 180 grados
  \param[in]  void
  \return     
 */
bool positionSensor(void)
{
  bool north;                 /* Booleano para saber el norte     */
  rawdata next_sample;        /* Muestra de los datos sin escalar */
  scaleddata sensor_values;   /* Valores escalados del sensor     */
  
  setMPU6050scales(MPU_addr,0b00000000,0b00010000);                 /* Se establece la escala del sensor*/
  next_sample = mpu6050Read(MPU_addr, true);                        /* Se realiza la lectura del sensor */
  sensor_values = convertRawToScaled(MPU_addr, next_sample,true);   /* Se escalan los valores leidos    */
  delay(1500);                                                  /* Retraso de 500 milisegundos      */

  if(sensor_values.AcZ >= 0)
    north = true;
  else
    north = false;
  return north;
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que lee los datos recolectados de los sensores de humedad, cada vez que
              el norte cambia, los sensores se cambian de rol de interior y externo
  \param[in]  north
  \return     void
 */
void moistureFunction(bool north)
{
  MoistureSensor1 = dht1.getHumidity();   /* Lectura de sensor de humedad 1*/   
  MoistureSensor2 = dht2.getHumidity();   /* Lectura de sensor de humedad 2*/
  
  if(north == true)                       /* Si el norte esta en su posicion original*/
  {
    MoistureInside  = MoistureSensor1;    /* El sensor de humedad interior es el sensor 1*/
    MoistureOutside = MoistureSensor2;    /* El sensor de humedad exterior es el sensor 2*/
  }
  else                                    /* Si el norte cambia de posicion original*/
  {
    MoistureInside  = MoistureSensor2;    /* El sensor de humedad interior es el sensor 2*/
    MoistureOutside = MoistureSensor1;    /* El sensor de humedad exterior es el sensor 1*/
  }        
}

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/*!
  \brief      Funcion que lee los datos recolectados de los sensores de temperatura, cada vez
              que el norte cambia, los sensores se cambian de rol de interior y externo
  \param[in]  north
  \return     void
 */
void temperatureFunction(bool north)
{
  sensors.requestTemperatures();                                /* Se realiza un request para leer el bus de sensores*/
  TemperatureSensor1 = sensors.getTempC(address_tempsensor1);   /* Se lee el sensor de temperatura 1*/
  TemperatureSensor2 = sensors.getTempC(address_tempsensor2);   /* Se lee el sensor de temperatura 2*/

  if(north == true)                                             /* Si el norte esta en su posicion original*/
  {
    TemperatureInside   = TemperatureSensor1;                   /* El sensor de temperatura interior es el sensor 1*/
    TemperatureOutside  = TemperatureSensor2;                   /* El sensor de temperatura exterior es el sensor 2*/
  }
  else                                                          /* Si el norte cambia de posicion original*/
  {
    TemperatureInside   = TemperatureSensor2;                   /* El sensor de temperatura interior es el sensor 2*/
    TemperatureOutside  = TemperatureSensor1;                   /* El sensor de temperatura exterior es el sensor 1*/
  }
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

