// Demo für I2C-LED-Treiber mit TLC59116
// ELV-AG Leer
// letzte Änderung 21.4.2011
// I2C -Adresse = 0C hex (alle Jumper A0 bis A3 offen)
// Hinweis zur Slaveadresse:
// Da bei der "Wire"-Funktion entfällt das letzte Bit, das für Lesen oder Schreiben (R/W) steht.
// Die Adresse hat somit nur noch 7 Bit, und sieht am Beispiel von C0hex so aus : 1100000b (60hex)
// 8Bit : 11000000 = C0h
// 7Bit : 1100000  = 60h


#include <Wire.h>


// Setup wird zu Begin aufgerufen und initialisiert den TLC59166 mit Defaultwerten
// Die Werte können beliebig geändert werden 

void setup()
{
  Wire.begin();     //I2C-Start
  Wire.beginTransmission(B1100000); // TLC59116 Slave Adresse ->C0 hex
  Wire.send(0x80);  // autoincrement ab Register 0h

  Wire.send(0x00);  // Register 00 /  Mode1  
  Wire.send(0x00);  // Register 01 /  Mode2 

  Wire.send(0x00);  // Register 02 /  PWM LED 1    // Default alle PWM auf 0
  Wire.send(0x00);  // Register 03 /  PWM LED 2    
  Wire.send(0x00);  // Register 04 /  PWM LED 3
  Wire.send(0x00);  // Register 05 /  PWM LED 4
  Wire.send(0x00);  // Register 06 /  PWM LED 5
  Wire.send(0x00);  // Register 07 /  PWM LED 6
  Wire.send(0x00);  // Register 08 /  PWM LED 7
  Wire.send(0x00);  // Register 09 /  PWM LED 8
  Wire.send(0x00);  // Register 0A /  PWM LED 9
  Wire.send(0x00);  // Register 0B /  PWM LED 10
  Wire.send(0x00);  // Register 0C /  PWM LED 11
  Wire.send(0x00);  // Register 0D /  PWM LED 12
  Wire.send(0x00);  // Register 0E /  PWM LED 13
  Wire.send(0x00);  // Register 0F /  PWM LED 14
  Wire.send(0x00);  // Register 10 /  PWM LED 15
  Wire.send(0x00);  // Register 11 /  PWM LED 16  // Default alle PWM auf 0

  Wire.send(0xFF);  // Register 12 /  Group duty cycle control
  Wire.send(0x00);  // Register 13 /  Group frequency
  Wire.send(0xAA);  // Register 14 /  LED output state 0  // Default alle LEDs auf PWM
  Wire.send(0xAA);  // Register 15 /  LED output state 1  // Default alle LEDs auf PWM
  Wire.send(0xAA);  // Register 16 /  LED output state 2  // Default alle LEDs auf PWM
  Wire.send(0xAA);  // Register 17 /  LED output state 3  // Default alle LEDs auf PWM
  Wire.send(0x00);  // Register 18 /  I2C bus subaddress 1
  Wire.send(0x00);  // Register 19 /  I2C bus subaddress 2
  Wire.send(0x00);  // Register 1A /  I2C bus subaddress 3
  Wire.send(0x00);  // Register 1B /  All Call I2C bus address
  Wire.send(0xFF);  // Register 1C /  IREF configuration  
  Wire.endTransmission();  // I2C-Stop
}


// Diese Funktion setzt die Helligkeit für ein LED-Register 
// Voraussetzung ist, das im entsprechende Register 14 bis 17 die LED aktiviert ist
// Übergabeparameter: LED = Nummer der LED / PWM = Helligkeitswert 0 -255

void Set_LED_PWM(int LED, int PWM)
{
  Wire.begin();             //I2C-Start
  Wire.beginTransmission(B1100000); // TLC59116 Slave Adresse ->C0 hex
  Wire.send(0x01 + LED);    // Register LED-Nr
  Wire.send(PWM);
  Wire.endTransmission();   // I2C-Stop
}

// Diese Funktion setzt die Helligkeit für alle LED-Register gleichzeitig
// Voraussetzung ist, das im entsprechende Register 14 bis 17 die LED aktiviert ist
// Übergabeparameter: PWM = Helligkeitswert 0 -255

void Set_LED_ALL(int PWM)
{
  Wire.begin();            // I2C-Start
  Wire.beginTransmission(B1100000); // TLC59116 Slave Adresse ->C0 hex
  Wire.send(0x82);         // Startregister 02h 
  for (int i=1 ; i < 17; i++){  // 16Bytes (Register 02h bis 11h) schreiben
    Wire.send(PWM);
  }
  Wire.endTransmission();  // I2C-Stop
}

// Funktion: Lauflicht1
// Läßt einen Leuchtpunkt LED hin und her wandern
// Geschwindigkeit wird durch den Parameter Speed bestimmt
// Übergabeparameter: Speed = 0 - 65536 Millisekunden

void Lauflicht1(int Speed)
{
  for (int i=1 ; i < 17; i++){  // Laufrichtung Links
    Set_LED_PWM(i,255);         // LED auf volle Helligkeit 255
    delay(Speed);               // Warten
    Set_LED_PWM(i,0);           // LED ausschalten
  }
  for (int i=15 ; i >0; i--){   // Laufrichtung Rechts
    Set_LED_PWM(i,255);         // LED auf volle Helligkeit 255
    delay(Speed);               // Warten
    Set_LED_PWM(i,0);           // LED ausschalten
  }
}

// Funktion: Lauflicht2
// Läßt einen Leuchtpunkt (LED) hin und her wandern
// Geschwindigkeit wird durch den Parameter Speed bestimmt
// Übergabeparameter: Speed = 0 - 65536 Millisekunden

void Lauflicht2 (int Speed)
{
  for (int i=1 ; i < 17; i++){  // Laufrichtung Links
    Set_LED_PWM(i,255);         // LED auf volle Helligkeit 255
    delay(Speed);               // Warten
    //Set_LED_PWM(i,0);         // LED ausschalten
  }
  Set_LED_PWM(16,0);          // LED Nr. 16 ausschalten
  for (int i=15 ; i >0; i--){   // Laufrichtung Rechts
    Set_LED_PWM(i,255);         // LED auf volle Helligkeit 255
    delay(Speed);               // Warten
    Set_LED_PWM(i,0);           // LED ausschalten
  }
}

// Funktion: Lauflicht3
// Läßt einen Leuchtpunkt (LED) hin und her wandern (mit Nachleuchten)
// Geschwindigkeit wird durch den Parameter Speed bestimmt
// Übergabeparameter: Speed = 0 - 65536 Millisekunden

void Lauflicht3 (int Speed)
{
  for (int i=1 ; i < 17; i++){  // Laufrichtung Links
    Set_LED_PWM(i,255);         // LED auf volle Helligkeit 255
    if ((i-1)>0) Set_LED_PWM(i-1,100);
    if ((i-2)>0) Set_LED_PWM(i-2,20);
    delay(Speed);               // Warten
    Set_LED_PWM(i,0);
    if ((i-1)>0) Set_LED_PWM(i-1,0); 
    if ((i-2)>0) Set_LED_PWM(i-2,0); 
  }
  for (int i=15 ; i > 1; i--){  // Laufrichtung Rechts
    Set_LED_PWM(i,255);         // LED auf volle Helligkeit 255
    if ((i+1)<17) Set_LED_PWM(i+1,50);
    if ((i+2)<17) Set_LED_PWM(i+2,10);
    delay(Speed);               // Warten
    Set_LED_PWM(i,0);
    if ((i+1)<17) Set_LED_PWM(i+1,0); 
    if ((i+2)<17) Set_LED_PWM(i+2,0); 
  }

}
void Dim_LED()
{ 
  for (int p=1 ; p <255; p++)   // Dimmrichtung heller
  {
    Set_LED_ALL(p);      
    delay(1);  // Warten
  } 
  for (int p=255 ; p >0; p--)   // Dimmrichtung dunkler
  {
    Set_LED_ALL(p);      
    delay(1);  // Warten
  } 
}

void loop()   // Endlosschleife / jedes Demo wird 3 mal ausgeführt
{
  Lauflicht1 (50);   // Geschwindigkeit 50
  Lauflicht1 (100);  // Geschwindigkeit 100
  Lauflicht1 (150);  // Geschwindigkeit 150
  Lauflicht2 (50);   // Geschwindigkeit 50
  Lauflicht2 (100);  // Geschwindigkeit 100
  Lauflicht2 (150);  // Geschwindigkeit 150
  Lauflicht3 (50);   // Geschwindigkeit 50
  Lauflicht3 (50);   // Geschwindigkeit 50
  Lauflicht3 (50);   // Geschwindigkeit 50
  Dim_LED();
  Dim_LED();
  Dim_LED();
}



