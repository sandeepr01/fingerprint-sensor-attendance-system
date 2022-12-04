
/*
 * Finger - 
 * green - 3v
 * yello - ground
 * orange - D5
 * red - D6
 * 
 * OLED
 * 1 - Ground
 * 2 - 3v
 * 3 - D1
 * 4 - D2
 */
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <Arduino_JSON.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <SH1106.h>

#include "images.h"

#ifndef APSSID
#define APSSID "SSR wifi"
#define APPSK  "mater@332"
#endif


#define ledBlue 6
#define ledRed 7

SH1106 display(0x3C,D1,D2);

ESP8266WiFiMulti WiFiMulti;

WiFiServer server(80);  // open port 80 for server connection



const char* host = "http://sandeeptechpr.000webhostapp.com/attendance/attendance.php";
String get_host = "http://sandeeptechpr.000webhostapp.com/attendance/attendance.php";

String mac_address;
String update_file;
String user_id, user_name, user_value, rstatus, rmsg;
String token = "lsadfhgasdkfbsvdfhvvfd";
String welcome;
bool finger_sensor = 0;
uint8_t id;
int Status = 0;
int reg = 0;
int x = -1;

#define Finger_Rx D5
#define Finger_Tx D6
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
void setup() {

  Serial.begin(115200);

  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(64, 20, "SSR");
  display.display();
  delay(3000);
  
  // starting the server
  server.begin();

  // connect wifi
  connect_wifi();

//  disconnect_wifi();
  delay(100);  
  Serial.println("\n\nAdafruit finger detect test");  
  
  finger.begin(57600);  


  while(!finger_sensor){
    if (finger.verifyPassword()) {
      finger_sensor = 1;
      Serial.println("Found fingerprint sensor!");  
      display.clear();
      display.drawXbm( 34, 0, FinPr_valid_width, FinPr_valid_height, FinPr_valid_bits);
      display.display();
    } else {  
      Serial.println("Did not find fingerprint sensor :(");
      display.clear();
      display.drawXbm( 34, 0, FinPr_failed_width, FinPr_failed_height, FinPr_failed_bits);
      display.display();
       while (1) { delay(1); }
    }
  }

  finger.getTemplateCount();  
  Serial.print("Sensor contains "); 
  Serial.print(finger.templateCount); 
  Serial.println(" templates");
  
//  finger.emptyDatabase();

}

//////////////////////print data on oled///////////////////
void display_data(String line1, String line2, String line3, bool clear = 0)
{
  if(clear){
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64,0, "SSR"); 
  }

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  
  if( line1 != "" ){
    display.drawString(0,15, line1);
  }
  if( line2 != "" ){
    display.drawString(0,30, line2);
  }
  if( line3 != "" ){
    display.drawString(0,45, line3);
  }
  display.display();
}
//////////////////////print data on oled///////////////////


void loop() {
  if(reg == 1){
    display_data("Name: "+user_name, "Place Your Finger", "", 1);
    digitalWrite(ledBlue,HIGH);
    digitalWrite(ledRed,HIGH);
    Serial.println("Registeration process");
    finger.getTemplateCount();  
    Serial.print("Sensor contains "); 
    Serial.print(finger.templateCount); 
    Serial.println(" templates");  
    id = finger.templateCount + 1;
    if(id==12){
      id++;
    }
    Serial.println(id);
    int s = 0;
    s = getFingerprintEnroll();
    if(s == 1){
      JSONVar myObject;
      myObject["id"] = user_id;
      myObject["name"] = user_name;
      myObject["fingerid"] = id;
      String data = JSON.stringify(myObject);
      
      check_data("reg_update", data);
    }

    

  }
  else{
    uint8_t userId = -1;
    digitalWrite(ledBlue,LOW);
    digitalWrite(ledRed,LOW);

    Serial.println("Waiting for valid finger...");
    display.clear();
    display.drawXbm( 34, 0, FinPr_scan_width, FinPr_scan_height, FinPr_scan_bits);
    display.display();
    for(int i=0;i<1;i++)
    {
      Status = 0;
      delay(2000);

      userId = getFingerprintIDez();
      if(userId >=1 && Status != 1)
      {
        display.clear();
        display.drawXbm( 34, 0, FinPr_valid_width, FinPr_valid_height, FinPr_valid_bits);
        display.display();
        digitalWrite(ledBlue,HIGH);
        delay(2000);
        
        JSONVar myObject;
        myObject["fingerid"] = userId; 
        String data = JSON.stringify(myObject);

        if(userId == 12){
          check_data("reg_query", "data");
        }
        else{
          check_data("attendance", data);
        }
        
      } 
    }
  }

}

///////////////////////////////////////Connect WiFi///////////////////////////////////////
void connect_wifi(){
//  WiFi.forceSleepWake();
  Serial.println("Connecting to WiFi : "+(String)APSSID);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(APSSID, APPSK);
  WiFiMulti.addAP("SSR","master@332");

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 21, "Connecting to");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 38, APSSID);
  display.drawXbm( 73, 15, Wifi_start_width, Wifi_start_height, Wifi_start_bits);
  display.display();
    
  while((WiFiMulti.run() != WL_CONNECTED)){
    Serial.print(".");
    delay(500);
  }
//  display.clear();
//  display.setTextAlignment(TEXT_ALIGN_CENTER);
//  display.setFont(ArialMT_Plain_16);
//  display.drawString(64, 5, "connected to 2nd wifi");
//  delay(1000);
//  display.display();
  
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 5, "Connected");
  display.drawXbm( 32, 18, Wifi_connected_width, Wifi_connected_height, Wifi_connected_bits);
  display.display();
  
}
void check_data(String action, String data){
//  connect_wifi();
  WiFiClient client = server.available();
  HTTPClient http;
  String url = get_host+"?action="+action+"&token="+token+"&data="+data;
  Serial.println(url);
  http.begin(client, url);
 
  //GET method
  int httpCode = http.GET();
  String payload;
  if(httpCode>0){
    payload = http.getString();
    Serial.println(payload);
  }
  else{
    payload = "";                             
  }

  if(payload == ""){
    Serial.println("NO DATA");
  }
  else if(payload == "NO_DATA"){
    Serial.println("NO DATA");
  }
  else{
    JSONVar webdata = JSON.parse(payload);
    rstatus = (const char*)webdata["status"];

    if(rstatus == "success"){
      user_id = (const char*)webdata["id"];
      user_name = (const char*)webdata["name"];
      user_value = (const char*)webdata["value"];
      
      if(user_value == "YES"){
        Serial.println("User registeration process start");
        display_data("welcome admin", "reg data found", "starting process...", 1);
        delay(2000);
        reg = 1;
      }
      else if(user_value == "NO"){
        reg = 0;
        display_data("welcome admin", "no reg data", "", 1);
        delay(2000);
      }
      else if(user_value == "SUCCESS"){
        Serial.println("User registered successfully");
        display_data("Name: "+user_name, "finger registered", "success", 1);
        delay(2000);
        reg = 0;
        
      }
      else if(user_value == "FAILED"){
        Serial.println("User registeration fialed");
        Serial.println("Try again");
        display_data("Name: "+user_name, "registration failed", "try again", 1);
        delay(2000);
        reg = 1;
      }
      else{
        Serial.println("User ID : "+user_id);
        Serial.println("User Name : "+user_name);
        Serial.println("Status : "+user_value);
        if(user_value == "IN"){
          welcome = "Welcome!!";
        }
        else{
          welcome = "Bye Bye !!";
        }
        display_data("Name: "+user_name, "Status: "+user_value, welcome, 1);
        delay(2000);

        if(user_value == "OUT"){
        display_data("Attendance done","", "", 1);
        delay(2000);
        } 
      }
    }
    else{
      rmsg = (const char*)webdata["status"];
      Serial.println(rmsg);
    }
  }

  http.end();

//  disconnect_wifi();
}
///////////////////////////////////////Check data ///////////////////////////////////////
///////////////////////////////////////Add Finger ///////////////////////////////////////
uint8_t getFingerprintEnroll() {
 
  int p = -1;
  int s = 0;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  display_data("Name: "+user_name, "remove finger", "", 1);
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  display_data("Name: "+user_name, "place Your finger", "again", 1);
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    
    s = 1;
    return s;
    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}
///////////////////////////////////////Add Finger///////////////////////////////////////
///////////////////////////////////////Check Finger ///////////////////////////////////////
int getFingerprintIDez() {
    
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK){
      delay(2000);
      Status = 1;
      return -1; 
    }
        

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK){
      delay(2000);      
      Status = 1;
      return -1;
    }

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK){
      display.clear();
      display.drawXbm( 34, 0, FinPr_invalid_width, FinPr_invalid_height, FinPr_invalid_bits);
      display.display();
      Serial.println("Error Finger");
      Serial.println("User not found");
      digitalWrite(ledBlue,LOW);
      digitalWrite(ledRed,HIGH);
      delay(2000);
      Status = 1;
      return -1;
      
    }

  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}
///////////////////////////////////////Check Finger ///////////////////////////////////////
