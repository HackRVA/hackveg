
/*
HackVeg Automated Gardening System
Author: Jamie Duncan
hackveg.org

# Copyright (C) 2013  Jamie Duncan (jamie.e.duncan@gmail.com)

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
#include <EthernetUdp.h>

//a version number
float ver = 0.3;

//*************************************************************
// LCD SETUP
//*************************************************************

//delay between lcd screens
int d = 3000; //delay value in ms

//define the lcd pins
LiquidCrystal lcd(5,6,7,8,9,10);

//*************************************************************
// SOCIAL MEDIA SETUP
//*************************************************************

//Twitter auth secret
Twitter twitter("14778397-pQbwMzp5tBtpqyBnZtpyNVrsUGYkZKK6s29eB8ojt");

//*************************************************************
// NTP SETUP
//*************************************************************

unsigned int localPort = 8888; 
const int NTP_PACKET_SIZE= 48;

EthernetUDP Udp;

byte SNTP_server_IP[]    = { 192, 43, 244, 18}; // time.nist.gov
//byte SNTP_server_IP[] = { 130,149,17,21};    // ntps1-0.cs.tu-berlin.de
//byte SNTP_server_IP[] = { 192,53,103,108};   // ptbtime1.ptb.de

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

//**************************************************************
// NETWOKRKING
//**************************************************************

//define the networking bits
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x2D, 0xB7};
/*
byte ip[] = {192,168,1,117};
byte subnet[] = {255,255,255,0};
byte gateway[] = {192,168,1,1};
byte dnsserver[] = {192,168,1,1};
*/
//**************************************************************
// I2C
//**************************************************************

Adafruit_BMP085 bmp;

//**************************************************************
// SETUP
//**************************************************************

void setup() {

  lcd.begin(16,2);
  lcd.clear();
  //Ethernet.begin(mac, ip, dnsserver, gateway, subnet);
  Ethernet.begin(mac);
  bmp.begin();
  Serial.begin(57600);
  Udp.begin(localPort);

  for (int pinNum = 30; pinNum < 35; pinNum++) {
    pinMode(pinNum, OUTPUT); 
  }
}

//**************************************************************
// LOOP
//**************************************************************

void loop() {

  boolean debug = false;
  boolean send_tweet = false;
  boolean send_water = false;
  boolean lcd_on = true;
  boolean temp_c = false: //if true, displays temp in celsius

  /*-------- NTP code ----------*/
  sendNTPpacket(timeServer); // send an NTP packet to a time server

  // wait to see if a reply is available
  delay(1000);  
  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
  
    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:
  
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    if (debug) {
      Serial.print("Seconds since Jan 1 1900 = " );
      Serial.println(secsSince1900);               
    
      // now convert NTP time into everyday time:
      Serial.print("Unix time = ");
    }
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;     
  // subtract seventy years:
  unsigned long epoch = secsSince1900 - seventyYears;
  
  int hour = (epoch  % 86400L) / 3600;
  int minutes = (epoch  % 3600) / 60;
  
  if (debug) {  
    // print Unix time:
    Serial.println(epoch);                               
  
  
    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print(hour); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( minutes < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
  
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':'); 
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    
    Serial.println(epoch %60); // print the second
  }
  // wait 30 seconds before asking for the time again
  delay(30000); 
  }
  
  // send an NTP request to the time server at the given address 
  unsigned long sendNTPpacket(IPAddress& address) {
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE); 
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49; 
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:         
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer,NTP_PACKET_SIZE);
    Udp.endPacket(); 
  }
  
  //only do the time-based stuff if we're not debugging
  if (debug == false) {
    // Check the time
    time_t t = now();
  
    //we will potentially water between 3am and 4am
    if ((hour(t) > 2) && (hour(t) < 5)) {
      send_water = true;  
    }
  
    //if it's the top of the hour we'll send a tweet
    if (minute(t) == 0) {
      send_tweet = true; 
    }
  }
  
  if (lcd_on) {
    //Welcome Screen
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("HackVeg ver. ");
    lcd.print(ver, DEC);
    lcd.setCursor(0,2);
    lcd.print("hackveg.org");
    delay(d);

    if (debug == true) {  
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
    }
   
    //print the IP address 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("IP Address");
    lcd.setCursor(0,2);
    printIP(ip);
    delay(d);
  
    /*
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
    */
  
    //Probe Temp 
    char poll_temp = getTemp(bmp.readTemperature(), send_tweet);
    if (lcd_on) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Temperature");
      lcd.setCursor(0,2);
      lcd.print(poll_temp)
      delay(d);
    }
      
    //Probe Pressure
    char poll_press = getPress(bmp.readPressure(), send_tweet);
    if (lcd_on) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Pressure");
      lcd.setCursor(0,2);
      lcd.print(poll_press);
      delay(d);
    }
    
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
  
  } //end lcd_on check
} //end loop()

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

void getMoistureLevel( int m, boolean tweet_check ) {
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
  //test to see if we want to tweet or not[/color]
  if (tweet_check == true) {
  //and prep the tweet
  char pres_c[4];
    dtostrf(pres,1,1,pres_c);
  
    char twt[140];
  
    String twt_s = "@bohenderson -the #hackveg air pressure is " + String(pres_c) + " mb";
    twt_s.toCharArray(twt,139);
  
    tweetMsg(twt);
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



