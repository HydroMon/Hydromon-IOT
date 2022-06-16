#include <ArduinoJson.h>
#include <dht.h>
//#include <SoftwareSerial.h>
#include  <EEPROM.h>
//inisialisasi temp/humidity sensor
#define TdsSensorPin A1
#define VREF 5.0              
#define SCOUNT  30            
#define dht_apin A2 //sensor connect to pin analog 3
//inisialisasi temp/humidity sensor
dht DHT;
String Value;
String data_kirim;
float hum,temp,tds,ec;
//inisialisasi tds/Ec
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
String pH;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation
float ecCalibration = 1;
float ecValue = 0;
String letter;
byte randomValue = random(0, 26);
// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

//inisialisai relay 
const int RELAY_Pump1 = 6;
const int RELAY_Pump2 = 7;
//LDR sensor
//#define LDRPin A3
const int LDRPin=A3;
//inisialisasi PH
int pHSense = A0;
int samples = 10;
float adc_resolution = 1024.0;

//inisialisasi random token
long randnumber;
int alamat;
long token;
void setup() {
  Serial.begin(115200);
  //nodemcu.begin(9600);
  delay(500);//Delay to let system boot// put your setup code here, to run once:
  if(EEPROM.read(202) != 123){
    EEPROM.write(202,123);
    delay(500);
    int alamat = sizeof(long);
    randnumber = random(100000, 999999);
    delay(500);
    randomSeed(analogRead(0));
    randnumber = random(100000, 999999);
    EEPROM.put(alamat,randnumber);
    //Serial.println(randnumber);
    //Serial.println("done Generate Token");
  }
  else{
    int alamat = sizeof(long);
    EEPROM.get(alamat,randnumber);
    //Serial.println(randnumber);
    //Serial.println("done get Token");
    token = randnumber;
    //Serial.println(token);
  }  
  pinMode(TdsSensorPin,INPUT);
  pinMode(RELAY_Pump1, OUTPUT);
  pinMode(RELAY_Pump2, OUTPUT);
  digitalWrite(RELAY_Pump1, LOW);
  digitalWrite(RELAY_Pump2, LOW);
  pinMode(LDRPin, INPUT);

delay(1000);

}

void loop() {
  StaticJsonBuffer<1000> jsonBuffer;
  //DynamicJsonDocument data(1024);
  JsonObject& data = jsonBuffer.createObject();
  //DHT sensor read
  DHT.read11(dht_apin);
  int LDRValue = analogRead(LDRPin);
  //sensor TDS
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // dataset
      averageVoltage = analogRead(TdsSensorPin) * (float)VREF / 1024.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*((DHT.temperature)-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      ecValue = compensationVoltage * ecCalibration;
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
 
    }
  }

  //SENSOR PH 
    int measurings=0;

    for (int i = 0; i < samples; i++)
    {
        measurings += analogRead(pHSense);
        delay(10);
    }
    float voltage = 5 / adc_resolution * measurings/samples;
    pH = abs((ph(voltage)));

  //store to json then send to esp
  temp = DHT.temperature;
  hum = DHT.humidity;
  tds = tdsValue;
  ec = ecValue;
  //store data to json 
  data["token"] = token;
  data["humidity"] = hum;
  data["temperature"] = temp;
  data["Tds"] = tds; 
  data["Ec"] = ec;
  data["ldr"] = LDRValue; 
  data["pH"] = pH; 
  data.printTo(Serial);
  //serializeJson(data, Serial);
  Serial.println();
  jsonBuffer.clear();
  delay(10000);



}


float ph (float voltage) {
  return 7 + ((2.5 - voltage) / 0.18);} // KALIBRASI NILAI PH
