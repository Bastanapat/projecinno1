#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
WiFiClient client;
PubSubClient mqtt(client);
#define WIFI_STA_NAME "105/712"
#define WIFI_STA_PASS "0934649179"
#define MQTT_SERVER   "electsut.trueddns.com"
#define MQTT_PORT     27860
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_NAME     "B6222932"
#define LED_PIN 23
#define sw 5
/////////////////////////temp//////////////////
#include <Wire.h>
#define si7021 0x40
bool WireEnd(){
  unsigned char err;
  err = Wire.endTransmission();
  if(err){
    Serial.print("Error: ");
    Serial.println(err);
         }
  return !err;       
              }
/////////////////////////////////////////////////
DynamicJsonDocument docd(1024);
DynamicJsonDocument docs1(1024);
DynamicJsonDocument docs2(1024);
char input1[256];
char output1[256];
char output2[256];
String st_sp = "";
int delay1 = 1000;
void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Topic = ");
  Serial.println(topic); 
  deserializeJson(docd,payload,length);
  serializeJson(docd,input1);
  std::string tp = topic;
  if(tp == "innoproject/group2/control1"){
    int control_start_stop = docd["control1"];
//    Serial.println(control_start_stop);
    if(control_start_stop == 1){
      Serial.println("1");
      st_sp = "on";
                               }
    else if(control_start_stop == 0){
      Serial.println("0");
      st_sp = "off";
                                    }
                                        }
  if(tp == "innoproject/group2/delay1"){
     int de = docd["delay1"];
     Serial.println(de);
     if (de == 1){
      delay1 = 1000;
      Serial.println(delay1);
                 }
     else if (de == 5){
      delay1 = 5000;
      Serial.println(delay1);
                       }
     else if (de == 10){
      delay1 = 10000;
      Serial.println(delay1);
                       }
     
                                        }
     
      
                                                              }                                                            
void setup() {
  //////////////////////temp///////////////////////////////
  bool success;
  unsigned int data;
  Serial.begin(115200);
  delay(1000);
  Wire.begin();
  //reset chip
  Serial.println("starting up si7021 sensor...");
  Wire.beginTransmission(si7021);
  Wire.write(0xFE);
  WireEnd();
  delay(100);

  // Get firmware Revision
  Serial.print("Firmware Revision");
  Wire.beginTransmission(si7021);
  Wire.write(0x84);
  Wire.write(0xB8);
  success = WireEnd();
  if(success){
    Wire.requestFrom(si7021,1);
    if(Wire.available()==1){
      data = Wire.read();
      Serial.print(data,HEX);//0xFF v1.0, 0x20 v2.0
      Serial.println();
                           }
             }              
     else{
      Serial.println("Firmware version command not supported!");
         }                      
             

  //////////////////////////////////////////////////////
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
}
void loop() {
  if (mqtt.connected() == false) {
    Serial.print("MQTT connecting... ");
    if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) 
    {
      Serial.println("connected");
      mqtt.subscribe("innoproject/group2/control1");
      mqtt.subscribe("innoproject/group2/delay1");
    } 
    else {
      Serial.println("failed");
      delay(1000);
          }
                                  } 
  else {
       ///////////////////////temp///////////////////////
        unsigned int data[2];
        // Humidity measurement
        Wire.beginTransmission(si7021);
        Wire.write(0xF5);
        WireEnd();
        delay(100);
        Wire.requestFrom(si7021,2); //read 2 byte
        if(Wire.available()==2){
          data[0] = Wire.read();
          data[1] = Wire.read();
                               }
        unsigned int tmp = (data[0] << 8) + data[1];
        float humidity = ((125.0 * tmp) / 65536.0) -6;
        // Temperture measure
        Wire.beginTransmission(si7021);
        Wire.write(0xF3);
        WireEnd();
        delay(100);
        Wire.requestFrom(si7021,2); //read 2 byte
        if(Wire.available()==2){
          data[0] = Wire.read();
          data[1] = Wire.read();
                               }
        tmp = (data[0] << 8) + data[1];  
        float celsiusTemp = ((175.72 * tmp) / 65536.0) - 46.85;
//        Serial.print("Humidity (% RH) ");
//        Serial.print(humidity);  
//        Serial.print("Temperture (C) ");  
//        Serial.println(celsiusTemp);                 
       /////////////////////////////////////////////////
          mqtt.loop();
          docs1["Humidity1"] = humidity;
          docs1["temperature1"] = celsiusTemp;
          serializeJson(docs1,output1);
          if(st_sp == "on"){
          mqtt.publish("innoproject/group2/measure1",output1);
                           }
          else if(st_sp == "off"){
          mqtt.publish("innoproject/group2/measure1",".......");
                           }
          delay(delay1);  
       }
            }
