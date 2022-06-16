#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h> 
#include <ArduinoJson.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>
#include <SoftwareSerial.h>
//inisialisasi relay
const int relay_phup = 2;
const int relay_phdown = 4;
const int relay_water = 12;
const int relay_nutrisi = 0;
const int relay_lampu = 16;

String id = "vdxa52";
/* 1. Define the WiFi credentials */
//#define WIFI_SSID "Pucang Adi"
//#define WIFI_PASSWORD "niar1970"
#define WIFI_SSID "AndroidAP4C2B"
#define WIFI_PASSWORD "vdxa5227"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
/* 2. Define the API Key */
//#define API_KEY "AIzaSyDUKeNvkdJu6VPP5q7EN7bt4IoMcDIxg54"
#define API_KEY "AIzaSyA-_OvIHm9wekiFJl0RRDJ5ib5qWloa5Wg"
//String tempVal,humiVal,ldrVal,tdsVal,ecVal,token,pH;
//String tempVal,humiVal,ldrVal,tdsVal,ecVal,pH;
//long token;
int tempVal,  humiVal,ldrVal;
float tdsVal,ecVal;
long token;
String ECVAL;
float PH;
int8_t indexOfA,indexOfB,indexOfC,indexOfD,indexOfE,indexOfF,indexOfG;
//char c;
String c;
String label;
String dataIn;
/* 3. Define the project ID */
//#define FIREBASE_PROJECT_ID "green-device-351102"
#define FIREBASE_PROJECT_ID "hydromon-test-351509"
SoftwareSerial Arduino_serial(10,11);
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "fariedsyahrizal@gmail.com"
#define USER_PASSWORD "testing123"
String Value;
String Sensor_data;
// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

bool taskcomplete = false;

void setup()
{
    timeClient.begin();
    Serial.begin(115200);
    
    pinMode(relay_phup, OUTPUT);
    pinMode(relay_phdown, OUTPUT);
    pinMode(relay_water, OUTPUT);
    pinMode(relay_nutrisi, OUTPUT);
    pinMode(relay_lampu, OUTPUT);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    timeClient.setTimeOffset(25200); // Set offset time in seconds to adjust for your timezone, for example:
    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

#if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{
  get_data();
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  //Print complete date:
  String currentDate = String(currentYear) + "." + String(currentMonth) + "." + String(monthDay);
  String TDS = String(tdsVal);
  String EC = String(ecVal);
  String TEM = String(tempVal);
  String HUM = String(humiVal);
  String LDR = String(ldrVal);
  String TOKEN = String(token);
  String pH = String(PH);  
  sendData(TOKEN,TEM,HUM,TDS,EC,LDR,pH,formattedTime,currentDate);
}

void sendData(String token,String tem, String hum,String tds,String Ec,String ldr,String pH,String formattedTime,String currentDate) {
  String string_token = String(token);
    if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();
        FirebaseJson content;
        // aa is the collection id, bb is the document id.
        //String documentPath = "test/123/c1/d3";
        String documentPath = "data_hidroponik/13789112";


        if (!taskcomplete)
        {
            taskcomplete = true;

            content.clear();
            
        
        //content.set("fields/count/stringValue", String(count).c_str());
        content.set("fields/time/stringValue", formattedTime);
        content.set("fields/date/stringValue", currentDate);
        content.set("fields/temperature/stringValue", tem);
        content.set("fields/humidity/stringValue", hum);
        content.set("fields/tds/stringValue", tds);
        content.set("fields/ec/stringValue", Ec);
        content.set("fields/ldr/stringValue", ldr);
        content.set("fields/ph/stringValue", pH);
        content.set("fields/id_hidroponik/stringValue", id);
        //content.set("fields/status/stringValue", count % 2 == 0);


            Serial.print("Create a document... ");

            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            else
                Serial.println(fbdo.errorReason());
        }

        //count++;
        content.clear();
        //content.set("fields/count/stringValue", String(count).c_str());
        content.set("fields/time/stringValue", formattedTime);
        content.set("fields/date/stringValue", currentDate);
        content.set("fields/temperature/stringValue", tem);
        content.set("fields/humidity/stringValue", hum);
        content.set("fields/tds/stringValue", tds);
        content.set("fields/ec/stringValue", Ec);
        content.set("fields/ldr/stringValue", ldr);
        content.set("fields/ph/stringValue", pH);
        //content.set("fields/id_hidroponik/stringValue", id);
        //content.set("fields/status/booleanValue", count % 2 == 0);

        Serial.print("Update a document... ");

        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(), "count,ph,ldr,ec,tds,humidity,temperature,time,date.id" /* updateMask */)){
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            get_doc();}
        else{
            Serial.println(fbdo.errorReason());}

    }
}
void get_data(){
    bool StringReady;
  String json;

  while (Serial.available()){
  json=Serial.readString();
  StringReady = true;
  }
  //infoAtm = json;
  if (StringReady){
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(json);
  //StaticJsonDocument<1000> data;
  //deserializeJson(data, json);

      tdsVal = data["Tds"];      
      ecVal = data["Ec"];     
      humiVal = data["humidity"];
      tempVal = data["temperature"];
      ldrVal = data["ldr"];
      token = data["token"];
      PH = data["pH"];
      delay(10000);
  }
}

void action(){
  if (label == "0") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    }
   else if (label == "1") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_water, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    }
       else if (label == "2") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_lampu, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
       else if (label =="3") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_nutrisi, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    }
       else if (label == "4") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_phup, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    }
       else if (label == "5") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_phdown, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    }
       else if (label == "6") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_water, LOW);
    digitalWrite(relay_lampu, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
       else if (label == "7") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_water, LOW);
    digitalWrite(relay_phup, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    }
       else if (label == "8") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_water, LOW);
    digitalWrite(relay_phdown, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    }
       else if (label == "9") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_water, LOW);
    digitalWrite(relay_lampu, LOW);
    digitalWrite(relay_phup, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
       else if (label == "10") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_water, LOW);
    digitalWrite(relay_lampu, LOW);
    digitalWrite(relay_phdown, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
       else if (label =="11") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_nutrisi, LOW);
    digitalWrite(relay_lampu, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
       else if (label== "12") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_phup, LOW);
    digitalWrite(relay_lampu, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
       else if (label == "13") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_phdown, LOW);
    digitalWrite(relay_lampu, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
       else if (label == "14") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_nutrisi, LOW);
    digitalWrite(relay_lampu, LOW);
    digitalWrite(relay_phup, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
       else if (label == "15") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_nutrisi, LOW);
    digitalWrite(relay_lampu, LOW);
    digitalWrite(relay_phdown, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, LOW);
    }
        else if (label == "16") {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_nutrisi, LOW);
    digitalWrite(relay_phup, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    }
   else {
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
    delay(500);
    digitalWrite(relay_nutrisi, LOW);
    digitalWrite(relay_phdown, LOW);
    delay(2000);
    digitalWrite(relay_phup, HIGH);
    digitalWrite(relay_phdown, HIGH);
    digitalWrite(relay_water, HIGH);
    digitalWrite(relay_nutrisi, HIGH);
    digitalWrite(relay_lampu, HIGH);
  }
}


void get_doc(){
        delay(20000);
        Serial.print("Get a document... ");
        String documentPath = "data_hidroponik/13789112";
        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), "")) {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

    // Create a FirebaseJson object and set content with received payload
             FirebaseJson payload;
             payload.setJsonData(fbdo.payload().c_str());

    // Get the data from FirebaseJson object 
             FirebaseJsonData jsonData;
             payload.get(jsonData, "fields/label/stringValue", true);
             //Serial.println(jsonData.stringValue);
             label = jsonData.stringValue;
             Serial.println("label :");
             Serial.println(label);
             action();
}
  
}
