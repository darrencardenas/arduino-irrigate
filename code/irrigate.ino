/*
 * Title:  Arduino Irrigation System 
 *
 * Version: 1.0
 * 
 * Author: Darren Cardenas
 * 
 * Date:   11 August 2019
 * 
 * Description: This program checks a moisture sensor for a minimum value. If the value is
 *              below the minimum, a water valve is opened for a period of time.  The 
 *              moisture readings are sent via email and text message.  The readings are also
 *              saved to a publicly viewable IoT website to store data and generate graphs.
 * 
 */

#include <SoftwareSerial.h>

#define AP_SSID             "SSID_NAME"          // Replace with your wifi SSID name
#define AP_PASSWORD         "SSID_PASSWORD"      // Replace with your wifi SSID password

#define IOT_WEBSITE         "api.thingspeak.com"
#define IOT_PORT            80
#define IOT_KEY             "ABCDEFGHIJKLMNOP"   // Replace with your ThingSpeak "Write API" key

#define SMTP_SERVER         "mail.smtp2go.com"
#define SMTP_PORT           2525
#define SMTP_USERNAME       "abcdefghijklmnopqrstuv0123456789"  // Replace with your SMTP2GO Base64-encoded username
#define SMTP_PASSWORD       "zyxwvutsrqponmlkjihgfe9876543210"  // Replace with your SMTP2GO Base64-encoded password

#define EMAIL_ADDR_SENDER   "your@email.com"         // Replace with sender email address
    
const char *RECIPIENTS[] = {
                            "your@email.com",        // Replace with recipient email address
                            "5551234567@vtext.com",  // Replace with mobile number email address (for SMS/text message)
                           };

const int NUM_RECIPIENTS = sizeof(RECIPIENTS) / sizeof(RECIPIENTS[0]);

// How often to check moisture
#define STATUS_CHECK_HRS          6 // Hours

// How long to allow the water valve to stay open
#define WATERING_PERIOD_MINS      5 // Minutes

// Moisture sensor maximum value
#define MOISTURE_MAX         640   // Take reading with moisture sensor blade fully submerged in water

// A moisture value less than or equal to this triggers watering
#define MOISTURE_THRESHOLD   300   // Modify with desired threshold

// Pin definitions
#define VALVE_RELAY_SIG  8  // Controls the valve
#define VALVE_RELAY_VCC  9  // Powers the relay
#define RX              10  // Receives from the wifi module
#define TX              11  // Transmits to the wifi module
#define MOISTURE_VCC     7  // Powers the moisture sensor
#define MOISTURE_SENSOR  0  // Analog, reads from the moisture sensor

// Delay values
#define RELAY_DELAY           500   // milliseconds
#define MOISTURE_DELAY        1000  // milliseconds
#define WIFI_CMD_DELAY        1000  // milliseconds
#define WIFI_CMD_LONG_DELAY   4000  // milliseconds

// Baud rates
#define BAUD_SERIAL    9600
#define BAUD_WIFI    115200


SoftwareSerial esp8266(RX,TX);


//-------------------------
// Arduino Functions
//-------------------------


// This function runs once at power-on or reset
void setup()
{   
  pinMode(VALVE_RELAY_VCC, OUTPUT);  // Powers the relay
  pinMode(VALVE_RELAY_SIG, OUTPUT);  // Controls the valve
  pinMode(MOISTURE_VCC, OUTPUT);     // Powers the moisture sensor

  Serial.begin(BAUD_SERIAL);         // Use Serial Monitor for debug statements
  Serial.println("Sketch started");  
  
  setupComm();
}


// This function runs after setup() and repeats forever
void loop()
{  
  int moisture_val_init = 0;   // Moisture value before watering
  int moisture_val_end = 0;    // Moisture value after watering

  String msg = "";             // String for email body
  
  // Get the moisture reading
  Serial.println("Checking moisture ...");
  moisture_val_init = getMoistureVal();
  Serial.println(String("Moisture value: ") + moisture_val_init);

  // Update channel feed with moisture level  
  updateChannelFeed(moisture_val_init);

  // Soil is too dry
  if (moisture_val_init <= MOISTURE_THRESHOLD)
  {
    unsigned long watering_time = WATERING_PERIOD_MINS*60000;  // Convert minutes to milliseconds   
    Serial.println(String("Soil is too dry. Watering for ") + WATERING_PERIOD_MINS + String(WATERING_PERIOD_MINS == 1 ? " minute." : " minutes."));

    digitalWrite(VALVE_RELAY_VCC, HIGH);  //  Apply power to water valve relay
    delay(RELAY_DELAY);
    digitalWrite(VALVE_RELAY_SIG, HIGH);  // Turn on water valve
    
    // Leave water valve open for a set amount of time
    delay(watering_time);  

    // Close water valve
    digitalWrite(VALVE_RELAY_SIG, LOW); 
    delay(RELAY_DELAY);
    digitalWrite(VALVE_RELAY_VCC, LOW);
     
    Serial.println("Watering is complete.");
    
    // Get the moisture reading after watering
    Serial.println("Checking moisture after watering ...");
    moisture_val_end = getMoistureVal();
    Serial.println(String("Moisture value after watering: ") + moisture_val_end);
    
    // Update channel feed with moisture level  
    updateChannelFeed(moisture_val_end);

    // Send status email
    msg = String("Moisture <= ") + MOISTURE_THRESHOLD + String(" triggers watering.\n\n") + 
          String("Before watering: ") + moisture_val_init + String("\n") +
          String("After watering: ") + moisture_val_end + String("\n\n") + 
          String("I watered the garden!");
    Serial.println(msg);
    sendEmail(msg);
  }
  // Soil moisture is good
  else
  {
    // Send status email
    msg = String("Moisture <= ") + MOISTURE_THRESHOLD + String(" triggers watering.\n\n") + 
          String("Moisture value: ") + moisture_val_init + String("\n\n") +
          String("The garden does not need watering!");
    Serial.println(msg);
    sendEmail(msg);            
  }

  // Wait for next status check
  unsigned long wait_time = STATUS_CHECK_HRS*3600000;   // Convert hours to milliseconds
  Serial.print(String("Waiting for ") + STATUS_CHECK_HRS + String(STATUS_CHECK_HRS == 1 ? " hour" : " hours") + String(" before checking again.\n"));
  delay(wait_time); 
  
}  // End of loop()


//-------------------------
// Utility Functions
//-------------------------

// This function gets a moisture value
int getMoistureVal (void)
{
  int val = 0;
  
  digitalWrite(MOISTURE_VCC, HIGH);   // Apply power to moisture sensor
  delay(MOISTURE_DELAY);              // Wait for stable output
  val = analogRead(MOISTURE_SENSOR);  // Read the moisture sensor
  digitalWrite(MOISTURE_VCC, LOW);    // Turn off to prolong life of sensor
  
  return val;
  
}  // End of getMoistureVal()


// Send email with username and password encoded in Base64
int sendEmail (String text)
{
  int i = 0;  // Loop iterator
  
  Serial.println("Connecting to SMTP server ...");
  esp8266.println(String("AT+CIPSTART=2,\"TCP\",\"") + String(SMTP_SERVER) + String("\",") + SMTP_PORT);
  delay(WIFI_CMD_LONG_DELAY);

  // Send commands to SMTP server
  Serial.println("Sending commands to SMTP server ...");
  Serial.println("EHLO arduino");
  sendWifiCmd("EHLO arduino");  
  Serial.println("AUTH LOGIN");
  sendWifiCmd("AUTH LOGIN");  
  Serial.println("Username");
  sendWifiCmd(SMTP_USERNAME);  
  Serial.println("Password");
  sendWifiCmd(SMTP_PASSWORD);    
  Serial.println("MAIL FROM");
  sendWifiCmd(String("MAIL FROM:<") + String(EMAIL_ADDR_SENDER) + String(">"));      
  Serial.println("RCPT TO");

  // Loop through all email recipients
  for (i=0; i<NUM_RECIPIENTS; i++)
  {
    Serial.println(String("Emailing ") + String(RECIPIENTS[i]));
    sendWifiCmd(String("RCPT TO:<") + String(RECIPIENTS[i]) + String(">"));
  }
  // End loop through all email recipients
  
  Serial.println("DATA");
  Serial.println(text);
  sendWifiCmd("DATA");
  sendWifiCmd(String("from: ") + String(EMAIL_ADDR_SENDER));
  sendWifiCmd("SUBJECT: From your arduino");  // Do not edit
  sendWifiCmd("");
  sendWifiCmd(text);
  sendWifiCmd(".");
  sendWifiCmd("QUIT");
  Serial.println("DONE");
  return 0;
  
}  // End of sendEmail()


// This function sends a command to the wifi module
int sendWifiCmd (String cmd)
{
  esp8266.println("AT+CIPSEND=2," + String(cmd.length()+2));
  delay(WIFI_CMD_DELAY);  
  esp8266.println(cmd);
  delay(WIFI_CMD_DELAY);  
  return 0;
  
}  // End of sendWifiCmd()


// This function sets up communications
void setupComm (void)
{
  esp8266.begin(BAUD_WIFI);  
  
  // Set WiFi mode 
  esp8266.println("AT+CWMODE=1");  

  Serial.print("Connecting to wifi ...");

  // Join access point
  esp8266.println(String("AT+CWJAP=\"") + String(AP_SSID) + String("\",\"") + String(AP_PASSWORD) + String("\""));  
  delay(WIFI_CMD_LONG_DELAY*3);  // Handle potentially slow router responses

  // Allow multiple TCP connections
  esp8266.println("AT+CIPMUX=1");
  delay(WIFI_CMD_LONG_DELAY*2);  // Handle potentially slow router responses
  
  Serial.println("Done");
  
}  // End of setupComm()


// This function updates an IoT channel with a value
void updateChannelFeed (int value)
{
    Serial.print("Updating IoT feed ... ");

    // Connect to IoT server
    esp8266.println(String("AT+CIPSTART=0,\"TCP\",\"") + String(IOT_WEBSITE) + String("\",") + String(IOT_PORT));
    delay(WIFI_CMD_LONG_DELAY);
    String url = String("GET /update?api_key=") + String(IOT_KEY) + String("&field1=") + value; 

    // Send data to IoT server
    esp8266.println("AT+CIPSEND=0," + String(url.length()+2));
    delay(WIFI_CMD_LONG_DELAY);
    long bytes_written = esp8266.println(url);

    // Close connection
    esp8266.println("AT+CIPCLOSE=0");
    Serial.println("Done");
    
}  // End of updateChannelFeed()
