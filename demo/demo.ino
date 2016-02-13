#include <SPI.h>
#include <UIPEthernet.h>

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

IPAddress server(192,168,0,13);

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
  // start the Ethernet connection using a fixed IP address and DNS server:
  //if (Ethernet.begin(mac) == 0) {
  //Serial.println("Failed to configure Ethernet using DHCP");
  // no point in carrying on, so do nothing forevermore:
  // try to congifure using IP address instead of DHCP:
  //Ethernet.begin(mac, ip);
  //}
  Ethernet.begin(mac, ip, myDns);
  Serial.println(Ethernet.localIP());

  registerDevice();
  client.stop();
  Serial.println("Completed registration");
}

void loop() {

  delay(10000);
  postState();
}

void postState() {
  if (client.connect(server, 5555)) {
    Serial.println("connecting to post state");
    int temp = getTemp();


    //String tempstring = dtostrf(temp,4,1,buffer);


    String PostData = "temp="+String(temp)+"&ledstatus="+String(ledState)+" ";
    client.println("POST /Devices/"+devid+"/state HTTP/1.1");
    //client.println("Host: devhost01.anetwork.local");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();

    client.println(PostData);
    client.println();
    
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
  if (client.connect(server, 5555)) {
    Serial.println("connecting...");

    client.println("POST /Devices/"+devid+" HTTP/1.1");
    //client.println("Host: devhost01.anetwork.local");
    client.println("User-Agent: arduino-ethernet");
    client.println("Content-Length: 0");
    client.println();
    
    delay(50);
    while(client.connected()) {
      char c = client.read();
      if (c == '<') 
      {
        char d = client.read();
        if (d == '0')
        {
          Serial.println("new device registered");
          while(client.available()) { 
            client.read();
          }
          break;
        }
        else if (d == '1') {
          Serial.println("retreiving last device state");
          char e = client.read();
          if (e == '|')
          {
            char f = client.read();
            if (f == '1')
              ledState = 1;
            else
              ledState = 0;
            while(client.available()) { 
              client.read();
            }
            break;
          }
        }

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






