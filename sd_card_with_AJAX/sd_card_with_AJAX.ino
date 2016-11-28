/*
  Web Server - SD Card

  A simple web server that shows the value of the analog input pins.
  using an Arduino Wiznet Ethernet shield. Also makes use of AJAX
  to keep elements updated on the webpage.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13
   LED connected to pin 8 and GND

  Last Modified:         27/11/2016

  Author:       Gus @ ArduinoMyLifeUp.com
  --------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// (port 80 is default for HTTP):
EthernetServer server(80);

File webPage;
int RED = 8;              //LED to turn on/off
String HTTP_req;          // stores the HTTP request

void setup()
{
  Ethernet.begin(mac, ip);  // initialize Ethernet device
  server.begin();           // start to listen for clients
  Serial.begin(9600);       // for debugging
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // initialize SD card
  Serial.println("Checking SD card is accessible...");
  if (!SD.begin(4)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
  Serial.println("SUCCESS - SD card initialized.");
  HTTP_req = "";
  //Set LED to output
  pinMode(RED, OUTPUT);
}

void loop()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read(); // read 1 byte (character) from client
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if ( HTTP_req.length() < 80)
          HTTP_req += c;  // save the HTTP request 1 char at a time
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          // send web page
          if (HTTP_req.indexOf("ajaxrefresh") >= 0 ) {
            ajaxRequest(client);  //update the analog values
            break;
          }
          else if (HTTP_req.indexOf("ledstatus") >= 0 ) {
            ledChangeStatus(client); //change the LED state
            break;
          }
          else {
            webPage = SD.open("index.htm");        // open web page file
            if (webPage) {
              while (webPage.available()) {
                client.write(webPage.read()); // send web page to client
              }
              webPage.close();
            }
            break;
          }
          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          } else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    HTTP_req = "";
    Serial.println("client disconnected");
  } // end if (client)
}

// send the state of the switch to the web browser
void ajaxRequest(EthernetClient client)
{
  for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
    int sensorReading = analogRead(analogChannel);
    client.print("analog input ");
    client.print(analogChannel);
    client.print(" is ");
    client.print(sensorReading);
    client.println("<br />");
  }
}

void ledChangeStatus(EthernetClient client)
{
  int state = digitalRead(RED);
  Serial.println(state);
  if (state == 1) {
    digitalWrite(RED, LOW);
    client.print("OFF");
  }
  else {
    digitalWrite(RED, HIGH);
    client.print("ON");
  }
}


