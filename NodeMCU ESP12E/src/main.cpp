#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// set the LCD number of columns and rows
#define LCDColumns 16
#define LCDRow 2

// Fingerprint scanner Pins        / red - Vin / Black - GND
#define Finger_Rx 14 // D5 - connect yellow wire
#define Finger_Tx 12 // D6 - connect white wire
// connect blue wire to 3.3V out        / Green - Touch (Zero when touched)

// gloabl control variables
LiquidCrystal_I2C lcd(0x27, LCDColumns, LCDRow);
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);   // fingerprint controll variable
uint8_t id;
uint8_t operation;

// time operation
const long utcOffsetInSeconds = 19800;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// function prototypes
bool detectFingerprintScanner();
void verifyScannerParameters();
uint8_t readUserInput(void);
uint8_t getFingerprintEnroll();
void enrollFingerprint();
void deleteFingerprint(uint8_t id);
void deleteDatabase();

// Define the WiFi credentials
#define WIFI_SSID "Galaxy M02s5656"
#define WIFI_PASSWORD "hiru2857756"

// time clients for ntp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup()
{
  Serial.begin(115200); // baud rate set to 115200
  delay(100);

  Serial.println(">>>>>>>>>>>>>>>>System Boot Up>>>>>>>>>>>>>>>>>");

  /*  Wifi Set up for the internet   */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to a Network");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }
  Serial.println();
  Serial.println("Connected to Wifi");
  WiFi.setAutoConnect(true);   //  automatically connect to the last-connected network after a reboot or power-on
  WiFi.setAutoReconnect(true); // automatically reconnect to the network if the connection is lost

  /* Set up for the LCD 16x2 display */
  // Wire.begin();
  // Serial.println("\nI2C Scanner");

  /* Set up for the Fingerprint sensor */
  while (!Serial)
    ;
  delay(200);
  Serial.println("..Welcome to ACCOL..");
  finger.begin(57600);
  if (!detectFingerprintScanner()) // could not detect any fingerprint scanner
  {
    Serial.println("Could not Found a Fingerprint Scanner");
    Serial.println("System Reboot");

    setup();
  }

  verifyScannerParameters();
  // time client for ntp
  timeClient.begin();
}

void loop()
{
  int p = -1;
  /*   Network Connection  */
  // handle wifi connection errors and reconnect
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.reconnect();
    Serial.print("Reconnecting");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(250);
    }
    Serial.println();
  }

  /*    Fingerprint program   */
  Serial.println("Select an option: ");
  Serial.println("Press 1 to enroll a fingerprint");
  Serial.println("Press 2 to delete a fingerprint");
  Serial.println("Press 3 to access with fingerprint");
  Serial.println("Press 4 to delete the whole database");

  operation = readUserInput();
  if (operation == 1)
  {
    enrollFingerprint();
  }
  else if (operation == 2)
  {
    deleteFingerprint(id);
  }
  else if (operation == 3)
  {
    while (p == FINGERPRINT_NOFINGER);
    
    while (p != FINGERPRINT_OK)
    {
      delay(1000);
      p = finger.getImage();
      switch (p)
      {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
      }
    }
    p = finger.image2Tz();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
    default:
      Serial.println("Unknown error");
    }

    Serial.println("Remove finger");
    delay(1000);
    p = finger.fingerFastSearch();
    if (p == FINGERPRINT_OK)
    {
      Serial.printf("Welcome %i\n", finger.fingerID);
      delay(5000);
    }
    else
    {
      Serial.println(" Access Denied ");
    }
  }
  else if (operation == 4)
  {
    deleteDatabase();
  }
  else
     return;
  
  
   

}

bool detectFingerprintScanner()
{
  if (finger.verifyPassword())
  {
    return true;
  }
  else
  {
    return false;
  }
}

void verifyScannerParameters()
{
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
  Serial.println("");
  Serial.println("");
}

uint8_t readUserInput(void)
{
  uint8_t num = 0;
  while (num == 0)
  {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

uint8_t getFingerprintEnroll()
{
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK)
  {
    delay(1000);
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
    delay(1000);
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK)
  {
    delay(1000);
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

void deleteDatabase()
{
  Serial.println("Do you want to delete all fingerprint templates!");
  Serial.println("Press 'Y' key to continue");
  while (1)
  {
    if (Serial.available() && (Serial.read() == 'Y'))
    {
      break;
    }
  }
  finger.emptyDatabase();
  Serial.println("Now the database is empty.)");
  Serial.println("");
  Serial.println("");
}

void enrollFingerprint()
{
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readUserInput();
  if (id == 0)
  {
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);
  while (!getFingerprintEnroll())
    ;
}

void deleteFingerprint(uint8_t id)
{
  if (id == 0)
  {
    return;
  }
  Serial.print("Deleting ID #");
  Serial.println(id);
  uint8_t p = -1;
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Deleted!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not delete in that location");
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
  }
  else
  {
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
  }
  Serial.println("");
  Serial.println("");
  return;
}