#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

// time calculate libraries
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <I2CKeyPad.h>

// Fingerprint scanner Pins        / red - Vin / Black - GND
#define Finger_Rx 14 // D5 - connect yellow wire
#define Finger_Tx 12 // D6 - connect white wire
// connect blue wire to 3.3V out        / Green - Touch (Zero when touched)

// gloabl control variables
// enter the I2C address and the dimensions of your LCD here
LiquidCrystal_I2C lcd(0x3F, 16, 2); // lcd control
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); // fingerprint controll variable
I2CKeyPad keyPad(0x20);                                        // keypad control variable
uint8_t id;
uint8_t mode;

// Pin where the piezo buzzer is connected
const int BUZZER_PIN = D8;

// time operation
const long utcOffsetInSeconds = 19800;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// keypad
char keys[] = "123A456B789C*0#DNF"; // N = No Key, F = Fail
uint32_t lastKeyPressed = 0;

// function prototypes
bool detectFingerprintScanner();
void verifyScannerParameters();
String userIn(void);
uint8_t getFingerprintEnroll();
int accessControl(void);
void enrollFingerprint();
void deleteFingerprint(uint8_t id);
void deleteDatabase();
void display(String text, int cursor1, int cursor2);
void ringBuzzer(int frequency, long duration);
String getTime(void);

// Define the WiFi credentials hardcoded
#define WIFI_SSID "Galaxy M02s5656"
#define WIFI_PASSWORD "hiru2857756"

// time clients for ntp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup()
{
  Serial.begin(115200); // baud rate set to 115200
  delay(500);

  /* Set up for the LCD 16x2 display */
  lcd.init();
  lcd.clear();
  lcd.backlight(); // Make sure backlight is on

  Serial.println(">>>>>>>>>>>>>>>>System Boot Up>>>>>>>>>>>>>>>>>");
  lcd.setCursor(0, 1);
  lcd.print("System Booting");
  delay(1500);

  /*  Wifi Set up for the internet   */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcd.clear();
  Serial.print("Connecting to a Network");
  lcd.clear();
  lcd.autoscroll();
  lcd.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    // ringBuzzer(2600,100);
    lcd.setCursor(0, 1);
    lcd.print(".");
    Serial.print(".");

    delay(200);
  }

  Serial.println();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("--Connected---");
  Serial.println("Connected to Wifi");
  delay(1000);
  WiFi.setAutoConnect(true);   //  automatically connect to the last-connected network after a reboot or power-on
  WiFi.setAutoReconnect(true); // automatically reconnect to the network if the connection is lost

  /* Set up for 4x4 keypad */
  delay(1000);
  // Serial.println(__FILE__);

  Wire.begin();
  Wire.setClock(400000);
  if (keyPad.begin() == false)
  {
    Serial.println("\nERROR: cannot communicate to keypad.\nPlease reboot.\n");
    lcd.setCursor(0, 0);
    lcd.print("KeyPad Error!");
    delay(1000);
    ringBuzzer(2600, 2000);
    setup();
  }

  /* Set up for the Fingerprint sensor */
  while (!Serial)
    ;
  delay(200);
  Serial.println("..Welcome to ACCOL..");

  finger.begin(57600);
  if (!detectFingerprintScanner()) // could not detect any fingerprint scanner
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Error");
    delay(2000);
    Serial.println("Could not Found a Fingerprint Scanner");
    ringBuzzer(2600, 2500);
    Serial.println("System Reboot");

    setup();
  }
  delay(1000);
  verifyScannerParameters();
  // time client for ntp
  timeClient.begin();
}

void loop()
{
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

  /*   Basic Program   */
  Serial.println("Account Password:"); // acc password
  int accPassword = userIn().toInt();
  Serial.println("Device Password:");
  int devicePassword = userIn().toInt();

  mode = 0;
  while (1)
  {
    Serial.println("Select Mode :");
    delay(1000);
    Serial.println("A-Registration   B-Access Control");


  }

  
}

String getTime(void){
  timeClient.update();
  return timeClient.getFormattedTime();
}

int accessControlMode()
{

  return 0;
}

int registrationMOde(void)
{
  return 0;
}

// returns the verified person id if valid otherwise returns 0 
// in case of error returns -1 
int accessControl(void)
{
  int p = -1;
  while (p == FINGERPRINT_NOFINGER)
    ;

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
    Serial.printf("Welcome %i\n", finger.fingerID); // verified person
    return finger.fingerID;                         // return the fingerprint id of that person
    delay(5000);
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Access Denied");
    return 0;
  }
  else
  {
    Serial.println("Error! Try Again..");
    return -1;
  }
  
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
  id = userIn().toInt();
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

void display(String text, int cursor1, int cursor2)
{
  lcd.clear();                     // Clear the display
  lcd.setCursor(cursor1, cursor2); // Set the cursor
  lcd.print(text);                 // Print the text on the LCD
}

void ringBuzzer(int frequency, long duration)
{
  // Set the pin to output mode
  pinMode(BUZZER_PIN, OUTPUT);

  // Generate the tone
  tone(BUZZER_PIN, frequency, duration);

  // Wait for the tone to finish
  delay(duration);

  // Turn off the tone
  noTone(BUZZER_PIN);
}

String userIn(void)
{
  static uint8_t lastKey = 0;
  uint32_t now = millis();
  String input = "";
  uint8_t index = keyPad.getKey();
  while (index != 14) // exit after # key (enter key)
  {
    /* valid key press */
    lastKeyPressed = now;
    index = keyPad.getKey();
    if (index == 16) // no key pressed
      continue;

    if (index != lastKey && index != 14) // new key press other than #
    {
      lastKey = index;
      // next key press
      input += keys[index];
    }
    now = millis();
  }
  lastKey = 0;
  return input; // return the user input as a string
}
