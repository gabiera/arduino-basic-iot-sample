#include <SPI.h>
#include <Ethernet.h>
#include <Time.h>


IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov


const int timeZone = 0;     // UTC
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)


EthernetUDP Udp;
unsigned int udpPort = 51242;  // local port to listen for UDP packets




const int ledOutput = 7;
String devid = "ARDUOBJ";
const int tempinput = A0;
static int ledState = 0;
static char buffer[10];
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xFD};
IPAddress ip(192,168,0,240);

IPAddress myDns(192,168,0,253);

// initialize the library instance:
EthernetClient client;

//char server[] = "devhost01.anetwork.local";
char server[] = "iot0917.azurewebsites.net";
//IPAddress server(192,168,0,13);

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000;  // delay between updates, in milliseconds

void setup() {
  // start serial port:
  Serial.begin(9600);
  pinMode(ledOutput, OUTPUT);
  digitalWrite(ledOutput, ledState);
  // give the ethernet module time to boot up:
  Serial.println("booting");
  delay(5000);
    // start the Ethernet connection:
  #if defined(WIZ550io_WITH_MACADDRESS)
    if (Ethernet.begin() == 0) {
  #else
    if (Ethernet.begin(mac) == 0) {
  #endif  
      Serial.println("Failed to configure Ethernet using DHCP");
      // no point in carrying on, so do nothing forevermore:
      // try to congifure using IP address instead of DHCP:
  #if defined(WIZ550io_WITH_MACADDRESS)
      Ethernet.begin(ip);
  #else
      Ethernet.begin(mac, ip);
  #endif  
  }
  Serial.println(Ethernet.localIP());
  
  Udp.begin(udpPort);
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  
  registerDevice();
  client.stop();
  Serial.println("Completed registration");
  

}

void loop() {

  delay(1000);
  postState();
}

void postState() {
  if (client.connect(server, 80)) {
    Serial.println("connecting to post state");
    int temp = getTemp();
    //String tempstring = dtostrf(temp,4,1,buffer);
    String PostData = "temp="+String(temp);
    client.println("POST /Devices/"+devid+"/state HTTP/1.1");
    client.println("Host: iot0917.azurewebsites.net");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.print("\r\n");

    client.println(PostData);
    client.print("\r\n");
    
    while(client.connected()) {
      char c = client.read();
      if(c == '>')
        break;
    }
    client.stop();
  } 
  else {

    Serial.println("connection failed to post state");

  }
  Serial.println("disconnecting from posting state.");
  client.stop();
}


void registerDevice() {
  Serial.println("Beginning registration");
  if (client.connect(server, 80)) {
    Serial.println("connecting...");

    client.println("POST /Devices/"+devid+" HTTP/1.1");
    client.println("User-Agent: arduino-ethernet");
    client.println("Host: iot0917.azurewebsites.net");
    client.println("Connection: close");
    client.println("Content-Length: 0");
    client.print("\r\n");
    
    delay(50);
    while(client.connected()) {
      char c = client.read();
      if (c == '<') 
      {
        char d = client.read();
        if (d == '0')
        {
          Serial.println("new device registered");
        }
        else if (d == '1') {
          Serial.println("device already exist");
        }
          while(client.available()) { 
            client.read();
          }
          break;
      }  
    } 


  } 
  else {
    Serial.println("connection failed");
    while(true);
  }
  Serial.println("disconnecting.");
  client.stop();

}

int getTemp() {
  int  sensorValue = analogRead(tempinput);
  float millivolts = (sensorValue/1024.0) * 5000;
  float farenheit =  millivolts/10;
  float celsius = (farenheit * 5.0 / 9.0) - 32;
  return celsius;
}


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
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
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


