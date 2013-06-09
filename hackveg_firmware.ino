
/*
HackVeg Automated Gardening System
Author: Jamie Duncan
hackveg.org
*/

#include <Ethernet.h>
#include <Dns.h>
#include <util.h>
#include <Dhcp.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Twitter.h>
#include <Time.h>

//define the lcd pins
LiquidCrystal lcd(4,5,6,7,8,9);

//Twitter auth secret
Twitter twitter("14778397-pQbwMzp5tBtpqyBnZtpyNVrsUGYkZKK6s29eB8ojt");

//a version number
float ver = 0.2;

//do you prefer Farenheit or Celsius?
const char* temp_pref = "F";

//delay between lcd screens
int d = 3000; //delay value in ms

//
//*************************************************************
// PIN ASSIGNMENTS PIN ASSIGNMENTS PIN ASSIGNMENTS
//*************************************************************

//set the moisture sensor analog pins
int z1sensor = 0;
int z2sensor = 1;
int z3sensor = 2;
int z4sensor = 3;
int z5sensor = 4;

//set  the zone power pins
int z1power = 30;
int z2power = 31;
int z3power = 32;
int z4power = 33;
int z5power = 34;

//define the networking bits
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x2D, 0xB7};
byte ip[] = {192,168,1,117};
byte subnet[] = {255,255,255,0};
byte gateway[] = {192,168,1,1};
byte dnsserver[] = {192,168,1,1};

//create the I2C sensor instance
Adafruit_BMP085 bmp;

void setup() {

  lcd.begin(16,2);
  lcd.clear();
  Ethernet.begin(mac, ip, dnsserver, gateway, subnet);
  bmp.begin();
  Serial.begin(57600);
  
  for (int pinNum = 30; pinNum < 35; pinNum++) {
    pinMode(pinNum, OUTPUT); 
  }
}

int sensor_loop_count = 0;  

void loop() {
  /*
  let's decide if this is a "special" loop
  where we want to send out tweets, etc.
  */
  
  //by default we don't tweet
  boolean send_tweet = false;
  
  if (sensor_loop_count == 10) { //divisible by 10
   //we tweet!
   send_tweet = true;
   //and we reset the counter
   sensor_loop_count = 0;
  }

  /*
  //debugging
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("sensorloopcount");
  lcd.setCursor(0,2);
  lcd.print(sensor_loop_count);
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("send_tweet");
  lcd.setCursor(0,2);
  lcd.print(send_tweet);
  delay(1000);  
  */
  
  //Welcome Screen
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HackVeg ver. ");
  lcd.print(ver, DEC);
  lcd.setCursor(0,2);
  lcd.print("hackveg.org");
  delay(d);
 
  //print the IP address 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("IP Address");
  lcd.setCursor(0,2);
  printIP(ip);
  delay(d);

  //print the Subnet Mask
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Subnet Mask");
  lcd.setCursor(0,2);
  printIP(subnet);
  delay(d);  
  
  //print the GW address 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Gateway");
  lcd.setCursor(0,2);
  printIP(gateway);
  delay(d);
  
  //print the DNS Server 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DNS Server");
  lcd.setCursor(0,2);
  printIP(dnsserver);
  delay(d);
  
  //print the Temp 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperature");
  lcd.setCursor(0,2);
  getTemp(bmp.readTemperature(), send_tweet);
  delay(d);
  
  
  //print the Pressure
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pressure");
  lcd.setCursor(0,2);
  getPress(bmp.readPressure(), send_tweet);
  delay(d);
  
  //print the elevation
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Altitude");
  lcd.setCursor(0,2);
  getAlt(bmp.readAltitude(), send_tweet);
  delay(d);
  
  //poll each moisture sensor
  lcd.clear();
  lcd.setCursor(0,0);
  for (int pinNum = z1sensor; pinNum < z5sensor; pinNum++) {
    getMoisture(pinNum); 
  }
  
  sensor_loop_count++; 
}

void getMoistureValue( int pin ) {
  //reports back one of the moisture zone readings
  int data = analogRead(pin);
  String data_s = String(data);
  String zoneName = getZoneNick(pin);
  
  lcd.print(
  
}

void waterOn(int pin) {
  //turns on a pin (high) to open up a solenoid for a given zone
  digitalWrite(pin, HIGH);
)  

void waterOff(int pin) {
  //turns off a pin (low) to close a solenoid for a given zone 
  digitalWrite(pin, LOW);
}

void getAlt( float alt ) {
  //the sensor returns altitude in meters,
  //we will optionally convert to feet
  if (alt < 1 ){
    //let's not live below sea level (can happen indoors, etc. since the altitude is inferred)
    //we also use the temperature option to decide to show it in meters or feet
    alt = 0;
    
    if (temp_pref == "F") {
    alt = alt / 3.28084;
    lcd.print (alt, 1);
    lcd.print (" feet");
   }
   else {
    lcd.print(alt,1);
    lcd.print(" meters");
   }
  }
  
}

void getZoneMoisture( int z) {
  
   //subtract 1 since we start at zero for the zone pins
   int pin = z - 1; 
}

void getZoneNick( int z) {
 //each zone has to be called something human-readable...
 const char* nick = "";
 if (z == 1) {
   nick = "Corn";
 } 
 else if (z == 2) {
   nick = "Rutabegas";
 }
 else if (z == 3) {
   nick = "Onions"; 
 }
 else if (z == 4) {
   nick = "Strawberries"; 
 }
 else if (z == 5) {
   nick = "Potatoes"; 
 }
 lcd.print(nick);
}

void getMoistureLevel( int m ) {
  //going off of the spec sheet, estimating the moisture of the soil
  // the sensor value description
  // 0  ~300     dry soil
  // 300~700     humid soil
  // 700~950     in water
  
  const char* soil = "";
  
  if (m < 300) {
    const char* soil = "Dry"; 
  }
  else if (m > 300 && m < 700) {
    const char* soil = "Moist"; 
  }
  else if (m > 700) {
    const char* soil = "Wet"; 
  }
  lcd.print(soil);

}
void getPress( float pres, boolean tweet_check ) {
  //the sensor delivers the pressure in Pascals by default. 
  //We will convert to millibars
  
  //present on the LCD
  pres = pres / 1000;
  lcd.print(pres, 1);
  lcd.print(" mb");
  
  //test to see if we want to tweet or not
  if (tweet_check == true) {
    //and prep the tweet
    char pres_c[4];
    dtostrf(pres,1,1,pres_c);
  
    char twt[140];
  
    String twt_s = "@bohenderson -the #hackveg air pressure is " + String(pres_c) + " mb";
    twt_s.toCharArray(twt,139);
  
    tweetMsg(twt);
  }
}

void getTemp( float temp ) {
  //the sensor delivers the temp in C by default
 
 if (temp_pref == "F") {
   temp = temp *1.8 + 32;
   lcd.print(temp,1);
   lcd.print(" *F");
 
  //test to see if we want to tweet or not
  if (tweet_check == true) {
    //and prep the tweet
    char temp_c[4];
    dtostrf(temp,1,1,temp_c);
    
    char twt[140];
    
    String twt_s = "Current Garden Temp: " + String(temp_c) + 
 }
 else {
   lcd.print(temp);
   lcd.print(" *C");
 } 
}

void printIP( byte bIP[] ) {
  //prints out a formatted IP address
  lcd.print(bIP[0],DEC);
  lcd.print(".");
  lcd.print(bIP[1],DEC);
  lcd.print(".");
  lcd.print(bIP[2],DEC);
  lcd.print(".");
  lcd.print(bIP[3],DEC);
}

void tweetMsg( char msg[]) {
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Twitter");
  lcd.setCursor(0,2);
  if (twitter.post(msg)) {
    int status = twitter.wait();
    if (status = 200) {
      lcd.print("Sent!");
    } else {
      lcd.print("Failed: code ");
      lcd.print(status);
    }
  } else {
    lcd.print("connection failed");
  }
}
