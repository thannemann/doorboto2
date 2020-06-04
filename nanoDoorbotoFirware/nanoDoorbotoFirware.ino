// nanoDoorbotoFirmware.ino ~ Copyright 2017  Manchester Makerspace ~ License MIT
// This sketch sends card ids to the raspberry pi over the serial port when scanned, and switches relay output based on the response from the pi.
// Member is provided with status via LCD and Blinking LED's

//load libraries
#include <LiquidCrystal_I2C.h> // https://github.com/johnrickman/LiquidCrystal_I2C
// good tutorial for implementation of the i2c LCD https://www.makerguides.com/character-i2c-lcd-arduino-tutorial/
#include <SPI.h>      // local arduino library
#include <MFRC522.h>  // https://github.com/miguelbalboa/rfid uses SPI, firmware works with Version 1.4.6 

//Define variables 

#define OPEN_TIME 3000
#define RELAY     4 //digital 4
#define RED_LED   2 //digital 2
#define GREEN_LED 3 //igital 3

// the following pins are based on the git hub documentation for the rfid 
 #define SS_PIN    10
 #define RST_PIN   9

#define INTERFACE Serial // quickly switch between Serial and Serial1 for testing purposes
#define BUFFER_SIZE 32    // buffer for serial recieve and get UID

//Initialise the LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,16,2); // Change to (0x27, 20, 4) for 20x4 LCD.

//Setup the cardreader pins
MFRC522 cardReader = MFRC522(SS_PIN, RST_PIN);

int beforeRelay = A6;
int afterRelay = A7;
float vout = 0.0;
float vin = 0.0;
float before = 0.0;
float during = 0.0;
float after = 0.0;
float R1 = 100000.0; // resistance of R1=100K to the positive terminal
float R2 = 10000.0; // resistance of R2 =10K to the ground terminal
int value = 0;


const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void setup(){
  lcd.init(); //initilize the LCD
  lcd.backlight(); //Switch on the backlight
  lcd.setCursor(0, 0); // Set the cursor on the first column and first row. Numbers start counting at 0.
  lcd.print("Booting Doorboto");//Print at cursor Location
  
  INTERFACE.begin(9600);      // open communication
  while(!INTERFACE){;}
  SPI.begin();                // Init SPI bus to communicate with card reader
  cardReader.PCD_Init();      // Init MFRC522 / start up card reader
  
  //define Pin Modes
  pinMode(RED_LED, OUTPUT);   // use LED
  pinMode(GREEN_LED, OUTPUT); // use LED
  pinMode(RELAY, OUTPUT);     // make relay pin an output

  //blink LED's during bootup
  blink(RED_LED, 15, 100);
  digitalWrite(RED_LED, LOW);              // make sure red is off
  blink(GREEN_LED, 15, 100);
  digitalWrite(GREEN_LED, LOW);            // make sure green is off
  
  lcd.clear();  //Clears the text from the display
  lcd.noBacklight(); //Switch off the backlight
  }

void loop(){
         getCardId();
  while(INTERFACE.available()){INTERFACE.read();} // make sure serial buffer is clear
}


void getCardId(){
  if(cardReader.PICC_IsNewCardPresent() && cardReader.PICC_ReadCardSerial()){

    INTERFACE.println(getRequestUID());      // send UID to relay
    char* response = blockingRecieve();      // wait for a response from server

    if(strcmp(response, "a") == 0){            // a is for acceptance
      lcd.backlight();                         //Switch on the backlight
      lcd.setCursor(0, 0);                     // Set the cursor on the first column and first row. Numbers start counting at 0.
      lcd.print("Door Unlocked");              //Print at cursor Location

         //measured the voltage before the relay, before relay operation
         value = analogRead(beforeRelay);
         vout = (value * 5.0) / 1024.0;
         before = vout / (R2/(R1+R2)); 

      digitalWrite(RELAY, HIGH);               // open relay, so member can come in

         //measured the voltage before the relay, during relay operation
         value = analogRead(beforeRelay); 
         vout = (value * 5.0) / 1024.0;
         during = vout / (R2/(R1+R2)); 

         //measured the voltage after the relay, during relay operation to confirm it opened
         value = analogRead(afterRelay);
         vout = (value * 5.0) / 1024.0;
         before = vout / (R2/(R1+R2));
          
// write code to send data out the serial port for logging 


      blink(GREEN_LED, 10, 50);                // blink green led to show success
      digitalWrite(GREEN_LED, HIGH);           // hold green led on

  } else if (strcmp(response, "d") == 0){  // d is for denial
      lcd.backlight();                         //Switch on the backlight
      lcd.setCursor(0, 0);                     // Set the cursor on the first column and first row. Numbers start counting at 0.
      lcd.print("Not Authorized");              //Print at cursor Location
      lcd.setCursor(0, 0);                     // Set the cursor on the first column and first row. Numbers start counting at 0.
      lcd.print("Contact Board");              //Print at cursor Location
      blink(RED_LED, 15, 200);               // indicate failure w/ red led blink
      digitalWrite(RED_LED, HIGH);
    }//else if
      delay(OPEN_TIME);                        // wait amount of time to keep the door unlocker or hold off a would be atacker
      digitalWrite(RELAY, LOW);                // stop sending current to relay
      digitalWrite(GREEN_LED, LOW);            // make sure green is off
      digitalWrite(RED_LED, LOW);              // make sure red is off
      lcd.clear();  //Clears the text from the display
      lcd.noBacklight(); //Switch off the backlight

  }
}


char* getRequestUID(){ // Getting the ASCII representation of card's hex UID.
  static char buffer[BUFFER_SIZE];

  byte index = 0;       // start a zero every time
  for(int i=0; i<cardReader.uid.size; ++i){
    buffer[index] = hexmap[(cardReader.uid.uidByte[i] & 0xF0) >> 4];
    index++;
    buffer[index] = hexmap[cardReader.uid.uidByte[i] & 0x0F];
    index++;
    cardReader.uid.uidByte[i] = 0;  // remove last (this) uid
  }
  cardReader.uid.size = 0;          // make sure this is read only once
  buffer[index] = '\0';             // terminate char array!
  return buffer;
}

void blink(byte led, int amount, int durration){
  static boolean toggle = false;

  toggle = ! toggle;                         // toggle LED state
  digitalWrite(led, toggle);                 // write LED state
  delay(durration);                          // block for a bit
  amount--;                                  // decrement blinks
  if(amount){blink(led, amount, durration);} // base case is no more blinks left
}

//======================== Serial Data Transfer (INTERFACE)
#define START_MARKER '<'
#define END_MARKER '>'

char* blockingRecieve(){
  char* response;                          // wait for a response from server
  while(!response){response = recieve();}  // block until response
  return response; // renturn pointer to recieve buffer when it is full
}

char* recieve(){
  static char buffer[BUFFER_SIZE];   // static buffer to point to
  static boolean inProgress = false; // note if read started
  static byte index = 0;             // note what is being read

  if(INTERFACE.available()) {         // is there anything to be read
    char readChar = INTERFACE.read(); // yes? read it
    if(inProgress){                   // did we read start maker?
      if(readChar == END_MARKER){     // did we read end marker
        buffer[index] = '\0';         // --terminate the string
        inProgress = false;           // --note finished
        index = 0;                    // --reset index
        return buffer;                // --return pointer to buffer
      } else {                        // given still in progress
        buffer[index] = readChar;     // concat this char
        index++;                      // increment index
        if(index >= BUFFER_SIZE){index = BUFFER_SIZE - 1;}  // prevent overflow by overwriting last char
      }
    } else if(readChar == START_MARKER){inProgress = true;} // indicate when to read when start marker is seen
  }
  return 0; // in the case the message has yet to be recieved
}
