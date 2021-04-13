
/*
  SignalWire Simple LaML server and client example

  Simple webserver, and httpclient demonstrating how to send and receive calls and text messages.

  You can use this together with NGROK on a pc on your network, to allow inbound traffic from SignalWire.

  The phone numbers, when using this client need to be in E.164 format, which contains a plus sign i.e. +15553332222
  the plus sign needs to be url encoded, so you need to enter it as '%2b' which will translate to a plus on the other side.
 */
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

// SignalWire Settings
String sw_space = "<space>.signalwire.com";        // Your SignalWire Space Id
String sw_project = "*****************************";    // Your SignalWire Project Id
String sw_token = "PT*****************************";    // Your SignalWire Token
String sw_phone_number = "%2b15553332222"; // Remember the '%2b' prefix in place of the plus sign

// Wifi Settings
char ssid[] = "<ssid>";        // your network SSID (name) to connect to
char pass[] = "<wifi-password>";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

WiFiSSLClient ssl_client;

void setup() {
  Serial.begin(9600);      // initialize serial communication
  pinMode(LED_BUILTIN, OUTPUT);      // set the LED pin mode

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status
}

void sendPhoneCall(String To, String From, String Url){
  
  Serial.println("Making voice call request to SignalWire...");

  HttpClient sw_client = HttpClient(ssl_client, sw_space, 443);
  
  sw_client.beginRequest();

  String contentType = "application/x-www-form-urlencoded";
  String postData = "To=" + To + "&From=" + From + "&Url=" + Url;
  
  Serial.println(postData);

  sw_client.post("/api/laml/2010-04-01/Accounts/"+sw_project+"/Calls.json");
  sw_client.sendBasicAuth(sw_project, sw_token);
  sw_client.sendHeader("Content-Type", contentType);
  sw_client.sendHeader("Content-Length", postData.length()); 
  sw_client.print(postData); // if any postbody
  
  sw_client.endRequest();

  // read the status code and body of the response
  int statusCode = sw_client.responseStatusCode();
  String response = sw_client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

}

void sendSmsMessage(String To, String From, String Message){
  
  Serial.println("Making send sms request to SignalWire...");

  WiFiSSLClient wifi;
  HttpClient sw_client = HttpClient(ssl_client, sw_space, 443);
  
  sw_client.beginRequest();

  String contentType = "application/x-www-form-urlencoded";
  String postData = "To=" + To + "&From=" + From + "&Body=" + Message;
  
  Serial.println(postData);

  sw_client.post("/api/laml/2010-04-01/Accounts/"+sw_project+"/Messages.json");
  sw_client.sendBasicAuth(sw_project, sw_token);
  sw_client.sendHeader("Content-Type", contentType);
  sw_client.sendHeader("Content-Length", postData.length()); 
  sw_client.print(postData); // if any postbody
  
  sw_client.endRequest();

  // read the status code and body of the response
  int statusCode = sw_client.responseStatusCode();
  String response = sw_client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

}

void loop() {

  // Make simple web server ready to process requests
  WiFiClient client = server.available();   

  // If client is ready and not null
  if (client) {                             

    Serial.println("New Connection Request ...");           
    
    String endpoint = "";
    bool isHead = false;

    // Loop throught the request, digest and provide a response...
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,

        // Read each line of the request, 
        String line = client.readStringUntil('\n');
        Serial.println(line);

        // check to see if it was the first line, containing the requested resource
        if(isHead == false) {
          isHead = true;
          endpoint = line;
          endpoint.replace(" HTTP/1.1\r", "");
        }

        // Close request on \r by breaking while loop
        if(line == "\r"){
          break;
        }       
      }
    }

   // Print out to serial port the reqeusted endpoint
   Serial.println("endpoint is " + endpoint);
    
   if (endpoint.endsWith("GET /send-call")) { // GET - /send-voice

          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/xml");
          client.println();

          client.println("OK");
         
          // The HTTP response ends with another blank line:
          client.println();

          String to = "%2b15553337855" // remember the %2b;
          
          Serial.println("Sending voice call ...");
          sendPhoneCall(to, sw_phone_number, "https://<ngrok-address>/inbound-voice");
             
                    
    }else if (endpoint.endsWith("GET /send-text")) { // GET - /send-text

          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/xml");
          client.println();

          client.println("OK");

          String to = "%2b15553337855" // remember the %2b;
   
          Serial.println("Sending text message ...");
          sendSmsMessage(to, sw_phone_number, "This is your arduino with an important message!");
          
                    
    }else if (endpoint.endsWith("GET /inbound-voice")) { // GET - /inbound-voice
        
          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/xml");
          client.println();

          client.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
          client.println("<Response>");
          client.println("  <Say>Welcome to SignalWire!</Say>");
          client.println("</Response>");
          
          // The HTTP response ends with another blank line:
          client.println();
                    
     }else if (endpoint.endsWith("GET /inbound-sms")) { // GET - /inbound-sms
        
          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/xml");
          client.println();

          client.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
          client.println("<Response>");
          client.println("  <Say>Welcome to SignalWire!</Say>");
          client.println("</Response>");
          
          // The HTTP response ends with another blank line:
          client.println();
                    
     }else{ // root response  /

          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println();

          // the content of the HTTP response follows the header:
          client.print("This is the default document.");

          // The HTTP response ends with another blank line:
          client.println();
    }
    
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}