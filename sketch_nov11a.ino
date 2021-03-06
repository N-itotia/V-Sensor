
#include <Filters.h>             
#include <LiquidCrystal_I2C.h>    //LCD iÂ²c library
#include <Wire.h>
#include <SoftwareSerial.h>
#include <String.h>

#define ACS_Pin A0              //ACS712 data pin
#define RELAYMODULE_PIN_SIGNAL  7

#define I2C_ADDR 0x27 //I2C adress
#define BACKLIGHT_PIN 3 // Declaring LCD Pins
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

SoftwareSerial sim800l(2, 3);

String Data_SMS; 
// cell numbers to which you want to send the security alert message

LiquidCrystal_I2C lcd(0x27, 16, 2); //Declaring the lcd


float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 40.0/testFrequency;     // how long to average the signal, for statistist

float intercept = - 0.14; // to be adjusted based on calibration testing
float slope = 0.0750; // to be adjusted based on calibration testing
                     

float Amps_TRMS; 
float ACS_Value;

unsigned long printPeriod = 1000; 
unsigned long previousMillis = 0;

int relay1 = 7;

void setup() {
  Serial.begin (9600);
  sim800l.begin(9600);
  Serial.println("IS II PROJECT");
  Serial.println("NIGEL NJAGI ITOTIA 102366");
  digitalWrite(2,HIGH);
  lcd.begin (16,2);
  lcd.backlight();
  lcd.print("IS II PROJECT");
  lcd.setCursor(0,1);
  lcd.print("Demo"); 
  pinMode(relay1, OUTPUT);
  digitalWrite(relay1, HIGH);
  delay(2000); 
  

}

void loop() {
  RunningStatistics inputStats;                 // create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );
   
  while( true ) {   
    ACS_Value = analogRead(ACS_Pin);  // read the analog in value:
    inputStats.input(ACS_Value);  // log to Stats function
        
    if((unsigned long)(millis() - previousMillis) >= printPeriod) { //every second we do the calculation
      previousMillis = millis();   // update time
      
      Amps_TRMS = intercept + slope * inputStats.sigma() ;  //Calibrate the values
      lcd.clear();               //clear the lcd and print in a certain position
      lcd.setCursor(0,0);
      lcd.print("Measurement");
      lcd.setCursor (0,1); // set to line 1, char 0  
      lcd.print("Current: ");
      lcd.setCursor (9,1); // go to start of 2nd line
      lcd.print(Amps_TRMS);
      lcd.setCursor (14,1); // go to start of 2nd line
      lcd.print("A");

      Serial.print("Without cal:");
      Serial.print(inputStats.sigma());
      Serial.print( "\t Amps: " ); 
      Serial.print( Amps_TRMS );
      Serial.print("\t To test: ");
      Serial.println(inputStats.sigma()*0.0744);
  
      if(Amps_TRMS > 0.15){
      digitalWrite(relay1, LOW);
      Serial.println("load exceeded");
      lcd.setCursor (0,0);
      lcd.print ("Load exceeded");
      delay(200);
      Send_DHT_Data();
      
      }
      
      Serialcom();     //If no SMS is being sent we constantly call this function, it permits the communication between you and the module via the serial monitor
      
      if (Amps_TRMS < 1){
      digitalWrite (relay1, HIGH);
      }   
    }     
  }
  
}

void Send_DHT_Data()
{
  Serial.println("Sending Data...");     //Displays on the serial monitor
  sim800l.print("AT+CMGF=1\r");          // Set the shield to SMS mode
  delay(100);
  sim800l.print("AT+CMGS=\"+254736367292\"\r");  //Your phone number
  delay(500);
  Data_SMS = "Load exceeded at point x";  
                                                                                      
  sim800l.print(Data_SMS);  //This string is sent as SMS
  delay(500);
  sim800l.print((char)26);//(required according to the datasheet)
  delay(500);
  sim800l.println();
  Serial.println("Data Sent.");
  delay(500);

}

void Serialcom()
{
  delay(500);
  while (Serial.available()) 
  {
    sim800l.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(sim800l.available()) 
  {
    Serial.write(sim800l.read());//Forward what Software Serial received to Serial Port
  }
}
 

/* About the slope and intercept
 * First you need to know that all the TRMS calucations are done by functions from the library, it's the "inputStats.sigma()" value
 * At first you can display that "inputStats.sigma()" as your TRMS value, then try to measure using it when the input is 0.00A
 * If the measured value is 0 keep the intercept as 0, otherwise add or substract to make that value equal to 0
 * In other words " remove the offset"
 * Then turn on the power to a known value, for example use a bulb or a led that ou know its power and you already know your voltage, so a little math you'll get the theoritical amps
 * you divide that theory value by the measured value and here you got the slope, now place them or modify them
 */
