#include <SPI.h>
#include <WiFiNINA.h>
//#include <WiFi101.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "MQ131.h"

#include <ArduinoJson.h>
#include <string.h> 
#include "DustSensor.h"

//--------------------- Define for the dust_sensor (PM2.5) ------------------------

#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000    


//--------------------------------- Pin definition ---------------------------------
#define        DUST_ILED_PIN                   7 
#define        DUST_DATA_PIN                   A0

#define        MQ131_D_PIN                     0
#define        MQ131_A_PIN                     A1

#define        MQ7_D_PIN                       1
#define        MQ7_A_PIN                       A2

#define        CJMCU_6814_CO                   A3
#define        CJMCU_6814_NH3                  A4
#define        CJMCU_6814_NO2                  A5

#define        FAN_PIN                         2 
#define        BUTTON_PIN                      6

#define        ledPin                          LED_BUILTIN

const float MAX_VOLTS = 5.0;
const float MAX_ANALOG_STEPS = 1023.0;

Adafruit_BMP280 bmp; // I2C
unsigned bmp_status;

char ssid[] = "UNIFI_IDO2";           // Remplacez par le SSID de votre réseau
char pass[] = "99Bidules!";        // Remplacez par le mot de passe de votre réseau
/*char ssid[] = "238lavigne";           
char pass[] = "238lavigne";*/      
int wifi_status = WL_IDLE_STATUS;

const char server[] = "192.168.20.139";  // IP Adresse du serveur de l'API school
//const char server[] = "192.168.2.23";  // IP Adresse du serveur de l'API Home
int port = 8001;                            // Port par défaut pour HTTP

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server, port);

struct Data {
    String device_name;
    int quantityCO2;
    int quantityCO;
    int quantityNO2;
    int quantityO3;
    int fine_particle;
    float temperature;
};

int ledState = LOW;
unsigned long previousMillis = 0;
const long interval = 500;

/*
  Pin initialization
*/
void pinInitilization(){
  pinMode(DUST_ILED_PIN, OUTPUT);
  pinMode(MQ7_D_PIN, OUTPUT);
  pinMode(MQ7_A_PIN, INPUT);
  pinMode(CJMCU_6814_CO, INPUT);
  pinMode(CJMCU_6814_NH3, INPUT);
  pinMode(CJMCU_6814_NO2, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(DUST_ILED_PIN, LOW);
}

void MQ131_Calibration(){
  MQ131.begin(MQ131_D_PIN, MQ131_A_PIN, LOW_CONCENTRATION, 1000000);  

  Serial.println("Calibration in progress...");
  
  MQ131.calibrate();
  
  Serial.println("Calibration done!");
  Serial.print("R0 = ");
  Serial.print(MQ131.getR0());
  Serial.println(" Ohms");
  Serial.print("Time to heat = ");
  Serial.print(MQ131.getTimeToRead());
  Serial.println(" s");}

void setup() {

  pinInitilization();

  Serial.begin(9600);
  delay(5000);

  //-------------------------------- Connexion au WiFi ------------------------------

  while (wifi_status != WL_CONNECTED) {
    Serial.print("Tentative de connexion au réseau: ");
    Serial.println(ssid);
    wifi_status = WiFi.begin(ssid, pass);
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connecté au réseau WiFi");
  
  //-------------------------------- I2C BMP280 communication ------------------------

  /*bmp_status = bmp.begin(0x76);
  while (!bmp_status) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring");
    delay(3000);
  }*/
  
  //---------------------------- MQ131 (Ozone sensor) Calibration --------------------

  //MQ131_Calibration();
}

void loop() {

  // if the LED is off turn it on and vice-versa:
  if (ledState == LOW) {
    ledState = HIGH;
  } else {
    ledState = LOW;
  }

  // set the LED with the ledState of the variable:
  digitalWrite(ledPin, ledState);

  fanManagement(getTemperaturedata());

  // Construire les données JSON à envoyer
  StaticJsonDocument<100> jsonBuffer;
  jsonBuffer["device_name"] = "MKR1010";
  jsonBuffer["quantityCO2"] = getData(50, 100);
  jsonBuffer["quantityCO"] = getData(0, 40);
  jsonBuffer["quantityNO2"] = getData(0, 40);
  jsonBuffer["quantityO3"] = getData(0, 40);
  jsonBuffer["fine_particle"] = getData(0, 40);
  jsonBuffer["temperature"] = getData(0, 40);

  /*jsonBuffer["device_name"] = "MKR1010";
  jsonBuffer["quantityCO2"] = getCO_CJMCU_6814_Data();
  jsonBuffer["quantityCO"] = getCarbonMonoxideData();
  jsonBuffer["quantityNO2"] = getNO2_CJMCU_6814_Data();
  jsonBuffer["quantityO3"] = getData(0, 40);
  jsonBuffer["fine_particle"] = getFineParticleData();
  jsonBuffer["temperature"] = getTemperaturedata();*/
  String postData;
  serializeJson(jsonBuffer, postData);
  //Serial.println(jsonString);

Serial.print("Connexion au serveur... ");

if (client.connect(server, port)) {
    Serial.println("réussie !");
    client.beginRequest();
    client.post("/api/data/");   
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", postData.length());
    client.beginBody();
    client.print(postData);
    client.endRequest();

    // Réponse du serveur
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Statut de la requête: ");
    Serial.println(statusCode);
    Serial.print("Réponse de l'API: ");
    Serial.println(response);

  } else {
    Serial.println("Échec de connexion au serveur.");
  }  

  delay(2000);
}


int getData(int min, int max) {
    return min + (rand() % (max - min + 1));
}

/* 
  Dust sensor ou PM2.5 
  Get fine particle data 
*/
float getFineParticleData(){

  float density, voltage;
  int   adcvalue;
  
  digitalWrite(DUST_ILED_PIN, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(DUST_DATA_PIN);
  digitalWrite(DUST_ILED_PIN, LOW);
  
  adcvalue = filter(adcvalue);

  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11; //covert voltage (mv)

  if(voltage >= NO_DUST_VOLTAGE) //voltage to density
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
  }
  else
    density = 0;
 
  /*Serial.print("The current dust concentration is: ");
  Serial.print(density);
  Serial.print(" ug/m3\n");  */

  return density;
}

/* 
  BMP280 sensor 
  Get temperature
*/
float getTemperaturedata(){
  return bmp.readTemperature();
}

/* 
  MQ131 sensor
  Get ozone data
*/
float getOzoneData(){
  /*Serial.println("Sampling...");
  MQ131.sample();
  Serial.print("Concentration O3 : ");
  Serial.print(MQ131.getO3(PPM));
  Serial.println(" ppm");
  Serial.print("Concentration O3 : ");
  Serial.print(MQ131.getO3(PPB));
  Serial.println(" ppb");
  Serial.print("Concentration O3 : ");
  Serial.print(MQ131.getO3(MG_M3));
  Serial.println(" mg/m3");
  Serial.print("Concentration O3 : ");
  Serial.print(MQ131.getO3(UG_M3));
  Serial.println(" ug/m3");*/

  return MQ131.getO3(PPM);
}

/* 
  MQ7 sensor
  Get carbon monoxide data
*/
float getCarbonMonoxideData(){
  // A) preparation
  // turn the heater fully on
  analogWrite(MQ7_A_PIN, HIGH); // HIGH = 255
  // heat for 1 min
  delay(2000);
  // now reducing the heating power: turn the heater to approx 1,4V
  analogWrite(MQ7_A_PIN, 71.4); // 255x1400/5000
  // heat for 90 sec
  delay(5000);
  
  // B) reading    
  // CO2 via MQ7: we need to read the sensor at 5V, but must not let it heat up. So hurry!
  analogWrite(MQ7_A_PIN, HIGH); 

  return analogRead(MQ7_A_PIN);
}

/*
  CJMCU_6814 sensor
  Get carbon monoxide data
*/
float getCO_CJMCU_6814_Data(){
  return analogRead(CJMCU_6814_CO)* (MAX_VOLTS / MAX_ANALOG_STEPS);
}

/*
  CJMCU_6814 sensor
  Get carbon monoxide data
*/
float getNH3_CJMCU_6814_Data(){
  return analogRead(CJMCU_6814_NH3)* (MAX_VOLTS / MAX_ANALOG_STEPS);
}

/*
  CJMCU_6814 sensor
  Get carbon monoxide data
*/
float getNO2_CJMCU_6814_Data(){
  return analogRead(CJMCU_6814_NO2)* (MAX_VOLTS / MAX_ANALOG_STEPS);
}

void fanManagement(float temperature){
  if (temperature > 25 && temperature <= 30)
    analogWrite(FAN_PIN, 85);
  else if (temperature > 30 && temperature <= 40) 
    analogWrite(FAN_PIN, 170);
  else if (temperature > 40)
    analogWrite(FAN_PIN, 255);
  else
    analogWrite(FAN_PIN, 0);
}
