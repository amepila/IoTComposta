#include "DHTesp.h"

DHTesp dht1;
DHTesp dht2;

unsigned int flag;
void setup()
{
Serial.begin(9600);
Serial.println();
dht1.setup(2, DHTesp::DHT11); // GPIO4 
dht2.setup(14, DHTesp::DHT11); // GPIO4 

}

void loop()
{
//delay(dht.getMinimumSamplingPeriod() + 100);
  flag = delayTime(2000);
  if(flag)
  {
    float humidity1 = dht1.getHumidity();
    float temperature1 = dht1.getTemperature();
    float humidity2 = dht2.getHumidity();
    float temperature2 = dht2.getTemperature();
    //Serial.print("Status: ");
    //Serial.print(dht.getStatusString());
    //Serial.print("\t");
    Serial.print("Humidity 1(%): ");
    Serial.print(humidity1, 1);
    Serial.print("\t");
    Serial.print("Temperature 1(C): ");
    Serial.print(temperature1, 1);
    Serial.print("\t");
    Serial.println();
    
    Serial.print("Humidity 2(%): ");
    Serial.print(humidity2, 1);
    Serial.print("\t");
    Serial.print("Temperature 2(C): ");
    Serial.print(temperature2, 1);
    Serial.print("\t");
    Serial.println();
  }
}
unsigned int delayTime(unsigned long interval)
{
  static unsigned long previousMillis = 0;          /* Variable que guarda el estado del tiempo               */
  unsigned long currentMillis         = millis();   /* Variable que se actualiza con el valor del tiempo      */
  unsigned int flag = 0;
  
  if((currentMillis - previousMillis) >= interval)  /* Mientras el valor actualizable sea mayor al intervalo  */
  {
    flag = 1;
    previousMillis = currentMillis;
  }else
  {
    flag = 0;
  }
  return flag;
}

