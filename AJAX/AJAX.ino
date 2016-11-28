/*
  Web Server - AJAX

  A simple web server that shows the value of the analog input pins.
  using an Arduino Wiznet Ethernet shield. This example introduces
  using AJAX to both update the analog pins and control a
  LED.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13

  Last Modified:         27/11/2016

  Author:       Gus @ ArduinoMyLifeUp.com
  --------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

//LED to turn on/off
int RED = 8;
String HTTP_req;          // stores the HTTP request

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  HTTP_req = "";
  //Set LED to output
  pinMode(RED, OUTPUT);
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if( HTTP_req.length() < 120)
          HTTP_req += c;  // save the HTTP request 1 char at a time
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          Serial.println(HTTP_req);      
          if (HTTP_req.indexOf("ajaxrefresh") >= 0 ) {
            // read switch state and analog input
            ajaxRequest(client);
            break;
          }
          else if (HTTP_req.indexOf("ledstatus") >= 0 ) {
            // read switch state and analog input
            ledChangeStatus(client);
            break;
          }
          else {              
            client.println("<!DOCTYPE HTML>");
            client.println("<html lang=\"en\">");
            client.println("<script>window.setInterval(function(){");
            client.println("nocache = \"&nocache=\" + Math.random() * 10;");
            client.println("var request = new XMLHttpRequest();");
            client.println("request.onreadystatechange = function() {");
            client.println("if (this.readyState == 4) {");
            client.println("if (this.status == 200) {");
            client.println("if (this.responseText != null) {");
            client.println("document.getElementById(\"analoge_data\")\
.innerHTML = this.responseText;");
            client.println("}}}}");
            client.println(
              "request.open(\"GET\", \"ajaxrefresh\" + nocache, true);");
            client.println("request.send(null);");
            client.println("}, 5000);");
            client.println("function changeLEDStatus() {");
            client.println(
              "nocache = \"&nocache=\" + Math.random() * 10;");
            client.println("var request = new XMLHttpRequest();");
            client.println("request.onreadystatechange = function() {");
            client.println("if (this.readyState == 4) {");
            client.println("if (this.status == 200) {");
            client.println("if (this.responseText != null) {");
            client.println("document.getElementById(\"led_status\")\
.innerHTML = this.responseText;");
            client.println("}}}}");
            client.println("request.open(\"GET\", \"?ledstatus=1\" + nocache, true);");
            client.println("request.send(null);");
            client.println("}");
            client.println("</script></head>");
            // output the value of each analog input pin
            client.print("<h1>Analogue Values</h1>");
            client.println("<div id=\"analoge_data\">Arduino analog input values loading.....</div>");
            client.println("<h1>Arduino LED Status</h1>");
            client.println("<div><span id=\"led_status\">");
            if(digitalRead(RED) == 1)
             client.println("On");
            else
              client.println("Off");
            client.println("</span> | <button onclick=\"changeLEDStatus()\">Change Status</button> </div>");
            client.println("</html>");
            break;
          }
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
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    HTTP_req = "";
    Serial.println("client disconnected");
  }
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

