/*
  
  Emic 2 Text-to-Speech Module: Basic Demonstration       
                                                         
  Author: Joe Grand [www.grandideastudio.com]             
  Contact: support@parallax.com                            
  
  Program Description:
  
  This program provides a simple demonstration of the Emic 2 Text-to-Speech
  Module. Please refer to the product manual for full details of system 
  functionality and capabilities.

  Revisions:
  
  1.0 (February 13, 2012): Initial release
  
*/

// include the SoftwareSerial library so we can use it to talk to the Emic 2 module
#include <SoftwareSerial.h>

#define rxPin 2    // Serial input (connects to Emic 2 SOUT)
#define txPin 3    // Serial output (connects to Emic 2 SIN)
#define ledPin 13  // Most Arduino boards have an on-board LED on this pin

// set up a new serial port
SoftwareSerial emicSerial =  SoftwareSerial(rxPin, txPin);


void setup()  // Set up code called once on start-up
{
  // define pin modes
  pinMode(ledPin, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  // set the data rate for the SoftwareSerial port
  emicSerial.begin(9600);

  digitalWrite(ledPin, LOW);  // turn LED off
  
  /*
    When the Emic 2 powers on, it takes about 3 seconds for it to successfully
    intialize. It then sends a ":" character to indicate it's ready to accept
    commands. If the Emic 2 is already initialized, a CR will also cause it
    to send a ":"
  */
  emicSerial.print('\n');             // Send a CR in case the system is already up
  while (emicSerial.read() != ':');   // When the Emic 2 has initialized and is ready, it will send a single ':' character, so wait here until we receive it
  delay(10);                          // Short delay
  emicSerial.flush();                 // Flush the receive buffer
}

void loop()  // Main code, to run repeatedly
{
  // Speak some text
  emicSerial.print('S');
  emicSerial.print("Hello. My name is the Emic 2 Text-to-Speech module. I would like to sing you a song.");  // Send the desired string to convert to speech
  emicSerial.print('\n');
  digitalWrite(ledPin, HIGH);         // Turn on LED while Emic is outputting audio
  while (emicSerial.read() != ':');   // Wait here until the Emic 2 responds with a ":" indicating it's ready to accept the next command
  digitalWrite(ledPin, LOW);
    
  delay(500);    // 1/2 second delay
    
  // Sing a song
  emicSerial.print("D1\n");
  digitalWrite(ledPin, HIGH);         // Turn on LED while Emic is outputting audio
  while (emicSerial.read() != ':');   // Wait here until the Emic 2 responds with a ":" indicating it's ready to accept the next command
  digitalWrite(ledPin, LOW);

  while(1)      // Demonstration complete!
  {
    delay(500);
    digitalWrite(ledPin, HIGH);
    delay(500);              
    digitalWrite(ledPin, LOW);
  }
}



