#include <SPI.h>
#include <Ethernet.h>
//#include <Time.h>


const int ledOutput = 7;
String devid = "ARDUOBJ";
const int tempinput = A0;
int ledState = 1;
static char buffer[10];
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xFD};
IPAddress ip(192,168,254,70);
IPAddress snip(255,255,255,0);
IPAddress gw(192,168,254,254);
IPAddress myDns(8,8,8,8);
// initialize the library instance:
EthernetClient client;

//char server[] = "devhost01.anetwork.local";
//char server[] = "iot0917.azurewebsites.net";
//IPAddress server(192,168,254,89);
IPAddress server(40,83,124,55);
const int PORT = 5555;

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
   //   Serial.println("Failed to configure Ethernet using DHCP");
      // no point in carrying on, so do nothing forevermore:
      // try to congifure using IP address instead of DHCP:
  #if defined(WIZ550io_WITH_MACADDRESS)
      Ethernet.begin(ip,myDns,gw,snip);
  #else
      Ethernet.begin(mac, ip,myDns,gw,snip);
  #endif  
  }
  Serial.println(Ethernet.localIP());
  
  registerDevice();
  client.stop();
  Serial.println("Completed registration");
  

}

void loop() {

  delay(1000);
  postState();
}

void postState() {
  if (client.connect(server, PORT)) {
    
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
    
    delay(100);
    while(client.available()) {
      
      char c = client.read();
      if (c == '<') {
        c = client.read();
      if(c == '0') {
        ledState = 0;
      } else if (c == '1') {
        ledState = 1;
      } }
      if(c == '>')
        break;
    }
    digitalWrite(ledOutput, ledState);
    Serial.println(ledState);
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
  if (client.connect(server, PORT)) {
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
  return millivolts;
}


