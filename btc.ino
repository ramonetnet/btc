//=============================================================================
// Track Bitcoin prices automatically with an ESP32 and an OLED display
// Created on 19 August 2024
// Created by Lucas Fernando (https://www.youtube.com/@lucasfernandochannel)
//
// v 1.0 - 20241214 - 
//   1.1 -          - use MarketData key
//   1.2            - use TFT_eSPI.h instead of Adafruit_ST7789.h
//   1.3            - save keys in external file and dont send it to GihHub 
//
//=============================================================================

#include <WiFi.h>
#include <HTTPClient.h>

#include <ArduinoJson.h>   // per "DynamicJsonDocument doc(1024) ;"

#include <TFT_eSPI.h>
#include <SPI.h>

int cnt = 0 ;

#include <credentials.h>      
const char* ssid     = STASSID ;
const char* password = STAPSK ;
const char* api_key = STA_API_KEY ;
const char* api_url = "https://rest.coinapi.io/v1/exchangerate/BTC/USD";

// Define the waiting time between the API calls
const int waiting_time = 900000; // Wait for 15 minutes

// Initialize the ST7789 display

TFT_eSPI tft = TFT_eSPI();

// +++ code

void InitSerial(void) 
{

  Serial.begin(115200) ;  // init Serial 
  while (!Serial) ;       // wait for Serial to become available
  delay(4000) ;           // to be able to connect Serial Monitor after reset or power up and before first print out

} // InitSerial() 


void setup() {

  InitSerial()  ;  // Initialize serial communication
  Serial.println( "*** btc.ino v 1.3 ***" ) ; 

  tft.init() ;                             // Initialize the display
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setFreeFont(&Orbitron_Light_24);

  Serial.print  ( "X max " ) ; 
  Serial.print  ( tft.width() ) ; 
  Serial.print  ( ", Y max " ) ; 
  Serial.println( tft.height() ) ; 

  connectToWiFi() ;                        // Connect to Wi-Fi

  // Make API call and display the result
  displayBitcoinPrice();

} ; // setup() 

void loop() {

  cnt = cnt + 1 ;
  Serial.print  ( cnt ) ;
  Serial.println(" +++ get BTC v 1.3 +++");

  delay(waiting_time);
  displayBitcoinPrice();  

}

void connectToWiFi() {

  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_RED); 
  tft.setCursor(10, 30);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setFreeFont(&Orbitron_Light_24);
  tft.print(">>> connecting wifi"); 

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("wait 1 sg ...");  
  }

  Serial.println("+++ Connected to Wi-Fi");

}

String formatTime(String time) {

  // Extract date and time components
  String date = time.substring(0, 10);
  String timeOfDay = time.substring(11, 16);

  // Convert date components
  String year  = date.substring(0, 4);  // Year
  String month = date.substring(5, 7);  // Month
  String day  = date.substring(8, 10);  // Day

  // Convert the month number to the month name
  String monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  int monthIndex = month.toInt() - 1;
  String monthName = monthNames[monthIndex];

  // Convert time from 24-hour to 12-hour format
  int hour = timeOfDay.substring(0, 2).toInt();
  String minute = timeOfDay.substring(3, 5);  // Minute
  String period = "AM";

  if (hour >= 12) {
    period = "PM";
    if (hour > 12) hour -= 12;
  } else if (hour == 0) {
    hour = 12; // Midnight case
  }

//  String formattedTime = monthName + " " + day + ", " + year + ", ";
//  formattedTime += String(hour) + ":" + minute + " " + period + " UTC";
  String formattedTime = String(hour) + ":" + minute + " " + period + " UTC";

  return formattedTime;

} // formatTime() 


void displayBitcoinPrice() {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    http.begin(api_url);
    http.addHeader("X-CoinAPI-Key", api_key);

    int httpCode = http.GET(); // Make the request
    Serial.print  (">>> http.GET rc is ... "); 
    Serial.println(httpCode); 

    if (httpCode > 0) {

      String payload = http.getString();

      Serial.print  (">>> payload is ... "); 
      Serial.println(payload);

      DynamicJsonDocument doc(1024) ; 
      deserializeJson(doc, payload);

      float price = doc["rate"];
      String date = doc["time"];
      String formatted_date = formatTime(date);

      Serial.print  (">>> timestamp is ... {"); 
      Serial.print  (formatted_date);
      Serial.print  ("}, price is ... "); 
      Serial.println(price); 

      // Display the price on the OLED
      tft.fillScreen(TFT_BLACK);
      tft.drawRect(0, 0, tft.width(), tft.height(), TFT_RED); 
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setFreeFont(&Orbitron_Light_24);

      tft.setCursor(10, 30);
      tft.print("Bitcoin Price");
      tft.setCursor(10, 60);
      tft.print("USD ");
      tft.setCursor(80, 60);
      tft.print(price, 2);

      tft.setTextColor(TFT_BLUE, TFT_BLACK);
      tft.setFreeFont(&Orbitron_Light_24);
      tft.setCursor(10, 115);
      tft.print(formatted_date);

    } else {

      Serial.println("--- Error on HTTP request"); 

    } // if httpCode 

    http.end(); // End the request

  } // if CONNECTED

} ; // displayBitcoinPrice() 
