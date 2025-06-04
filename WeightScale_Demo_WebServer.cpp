

// libraries 
#include<WiFiMulti.h>   // wifi 
#include<WiFi.h>       // wifi 
#include <TM1637.h>    // Display 
#include "HX711.h"     // Load cell 
#include "soc/rtc.h"


// Load cell reader 
HX711 scaleReader; 
// HX711 circuit wiring
const int DOUT_PIN = 21;           // GPIO PIN 21 from the ESP32 MCU 
const int SCK_PIN  = 20;           // GPIO PIN 20 from the ESP32 MCU
float calibration_scale = -19749/50; // Calibration factor to estimate the 
                                    // right value of the scale. Calibration factor = (reading \ known weight). 
                                   // In my case, well known weight is 50 grams. 
                                   // -19749 is the number I get when calibration factor was not set.
                                   // More details about this process 
                                   // is given in the documentation



// Web server 
 WiFiServer server(80);
// Variable to store the HTTP request
String header;
long  currentWeight = 0;
// Auxiliar variables to store the current output state
bool taingScale   = true; 

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 10000; // 10 seconds


//Display circuit wiring 
#define CLK_PIN    48     // GPIO PIN 48 from the ESP32 MCU 
#define DIO_PIN    47     // GPIO PIN 47 from the ESP32 MCU 
#define maxScale 9999     // 4 digits = xxxxxxx = 0000 to 9999

// Access Provider (AP) details
#define AP_WIFI  "VM1080293"       // AP name 
#define AP_PASS "Omidmuhsin2015"  // AP password 

// display provider 
TM1637 displayScale(CLK_PIN,DIO_PIN);
WiFiMulti wifiMulti;  // wifi access



//******************************************************************************************** Help functions *********************************************************************
/**
 * @Desc: This function lood weignt to the scale and return it. Max value is expected to be between 
 *        0 and 9999, as this is the maximun value 4 didgits can have. 
 * @return:  - If reading > max, error and return 0 
 *           - if reading <= 0, return 0 
 *           - return reading otherwise. 
 */

long  getWeight()
{
  
    if (scaleReader.is_ready()) 
    {
       scaleReader.set_scale(calibration_scale);    
      Serial.println("Please remove any weights on the scale, taring");
    //  delay(5000); // 5 seconds delay before measuring and after to get accurate value from the scale. 
      scaleReader.tare();
      Serial.println("Taring completed...");
      Serial.print("Place a known weight on the scale(in grams(g) or kilo grams(kg))...");
     // delay(5000); // to let us put well know weight over the scale. 
      long reading  = scaleReader.get_units(10);
      long reading2 = scaleReader.read();
      long reading3 = scaleReader.read_average(10);
      long reading4 = scaleReader.get_value(10);
      Serial.println("read1:");
      Serial.println(reading2);

      Serial.println("read2:");
      Serial.println(reading3);

      Serial.println("read3:");
      Serial.println(reading4);
      
     // delay(5000);  // to let the scale measure the eight. 
      return reading;  // return the reading 
  } 
  else 
  {
    Serial.println("HX711 not found.");
    return 0;
  }
 // delay(5000);
}


/**
 * Desc: Reseting the display with  whenever needed.
 */

void resetDisplay()
{
    displayScale.display("____"); // reset display
}

/**
 * Desc: Display the given weight to the 4 digits 7-segment display 
 * @para  weightVal a value to be displayed 
 */
void displayWeight(long weightVal)
{ 
  // reset the display first so that we can have a clear display.
  // when digit is not displayed, it is assumed to be "-". 

  if (weightVal<=0)
    {
       displayScale.display("0000");
    }
    else
    {
        resetDisplay();
        displayScale.display(weightVal);
    }
  
}





//*************************************************************************** Web Server ****************************************************************************
/**
 * @Desc:  Responsing incoming request from clients. 
 * @para client  a client to be responded. 
 */
void runClientRequest(WiFiClient client)
{
   
   // get current time 
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";  
    String action; 
    if (client) {   // if these is any requests
    Serial.print("New Client: "); 
    Serial.println(client); 
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // actions to be done remotely. 
            // display LED or taring the scale 
            if (header.indexOf("GET /display") >= 0) 
            {
              Serial.println("Displaying");
              displayScale.display(currentWeight);
              delay(500);
              Serial.println("Displaying done");
            } 
            else 
            {
              Serial.println("Taring");
              displayScale.display("tare");
              scaleReader.tare();
              Serial.println("Taring done");
            } 

             unsigned long elapsedTime = (currentTime - previousTime);
             
             Serial.println(currentWeight);
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the display/taring buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #4CAF50;}</style></head>");
            //client.println(".button3 {background-color: #4CAF50;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Weight Measuring Web Server</h1>");
            // display the LED or taring the scale
             client.println("<p> Display </p>");
             client.println("<p><a href=\"/display\"><button class=\"button\">Display</button></a></p>");
             client.println("<p> Taring </p>");
             client.println("<p><a href=\"/taring\"><button class=\"button button2\">Taring</button></a></p>");
             client.println("<p> Current Weight </p>");
             client.println("<p><a href=\"><button class=\"button button3\"> ${currentWeight} </button></a></p>");            
           
             client.println("</body></html>");            
            // The HTTP response ends with another blank line
             client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  } 
}


//****************************************************************************** Set up *****************************************************************************

// set up function
// initilaize all chips such as load cell and display  
void setup() 
{
  // put your setup code here, to run once:
  // conenct to the available wifi using your AP name and password. 
  WiFi.begin(AP_WIFI,AP_PASS); // access wife 
  
  // set the speed to transfer data in the serial communication. 
  Serial.begin(115200);   // speed 
  
  // 1- Initializing the display (4 digits 7 segment display) 
  Serial.println("Initializing the LED display");
  displayScale.init();
  displayScale.setBrightness(7); // set the brightness (0:dimmest, 7:brightest)

  //2- load cell setting and initilization 
   Serial.println("Initializing the scale");
   scaleReader.begin(DOUT_PIN,SCK_PIN);
   //3 start server to response requests from any clients. 
   server.begin();
  // start to measure the scale and display it. 
  Serial.println("....Starting .... \n");  
}

//**************************************************************************** main function (loop) *****************************************************

// loop forever 
void loop() 
{
  // if Wifi not connected, do nothing. 
  while(WiFi.status() == WL_CONNECTED) 
  {           
       // get the scale readings
       displayScale.display("0000");
       long reading = getWeight(); 
       currentWeight = reading;
       // print the reading 
       Serial.print("Result(in grams): ");
        delay(1000);
       Serial.println(reading);
       // display the reading on the LED 
       displayWeight(reading); 
       delay(1000);
      // Print local IP address and start web server
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      
   // check if there is any request from any clients. 
    WiFiClient client = server.available();   // Listen for incoming clients
    if (client) 
    {  
       Serial.println("Connected Clients: ");
       runClientRequest(client);
    }       
  }

    // check if wifi is still connected
   if(WiFi.status() != WL_CONNECTED) 
   {
      Serial.println("..... Connecting \n");
      delay(1000); 
  } 
 // end of the code
}