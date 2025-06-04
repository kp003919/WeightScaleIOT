

// libraries 
#include<WiFiMulti.h>   // wifi 
#include <TM1637.h>    // Display 
#include "HX711.h"     // Load cell 
#include "soc/rtc.h"


// Load cell reader 
HX711 scaleReader; 
// HX711 circuit wiring
const int DOUT_PIN = 21;           // GPIO PIN 21 from the ESM32 MCU 
const int SCK_PIN  = 20;           // GPIO PIN 20 from the ESM32 MCU
float calibration_scale = -19749/50; // Calibration factor to estimate the 
                                    // right value of the scale. Calibration factor = (reading \ known weight). 
                                   // In my case, well known weight is 50 grams. 
                                   // -19749 is the number I get when calibration factor was not set.
                                   // More details about this process 
                                   // is given in the documentation
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
      delay(5000); // 5 seconds delay before measuring and after to get accurate value from the scale. 
      scaleReader.tare();
      Serial.println("Taring completed...");
      Serial.print("Place a known weight on the scale(in grams(g) or kilo grams(kg))...");
      delay(5000); // to let us put well know weight over the scale. 
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
      
      delay(5000);  // to let the scale measure the eight. 
      return reading;  // return the reading 
  } 
  else 
  {
    Serial.println("HX711 not found.");
    return 0;
  }
  delay(5000);
}


/**
 * Desc: Reseting the display with 4 Os (0000), whenever needed.
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
  

   ////
  //rtc_cpu_freq_config_t config;
  //rtc_clk_cpu_freq_get_config(&config);
  

   ///
    
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
       // print the reading 
       Serial.print("Result(in grams): ");
       Serial.println(reading);
       // display the reading on the LED 
       displayWeight(reading); 
       delay(5000);
  }

    // check if wifi is still connected
   if(WiFi.status() != WL_CONNECTED) 
   {
      Serial.println("..... Connecting \n");
      delay(1000); 
  } 
 // end of the code
}