#include <ESP32Servo.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

// Pin Definitions
#define RED_LED_PIN   2
#define GREEN_LED_PIN 4
#define SERVO_PIN     13

// MFRC522 Hardware Setup
MFRC522DriverPinSimple ss_pin(5);
MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};

// Servo Instance
Servo doorServo;

// List of allowed UIDs (Must be in uppercase hex without spaces)
const String allowedUIDs[] = {
    "A1B2C3D4",
    "12345678",
    "90ABCDEF"
};
const int numAllowedUIDs = sizeof(allowedUIDs) / sizeof(allowedUIDs[0]);

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    // Initialize MFRC522 Reader
    mfrc522.PCD_Init();

    // Configure LED Pins
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);

    // Attach Servo Motor
    doorServo.attach(SERVO_PIN);
    doorServo.write(0); // Set initial locked/closed position (0 degrees)

    Serial.println("System initialized. Waiting for RFID cards...");
}

void loop()
{
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return;
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return;
    }

    // Build UID String in Uppercase Hex format
    String uidString = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        if (mfrc522.uid.uidByte[i] < 0x10)
        {
            uidString += "0";
        }
        uidString += String(mfrc522.uid.uidByte[i], HEX);
    }
    uidString.toUpperCase();

    // Print scanned UID
    Serial.print("Scanned Card UID: ");
    Serial.println(uidString);

    // Check if the scanned UID is allowed
    bool accessGranted = false;
    for (int i = 0; i < numAllowedUIDs; i++)
    {
        if (uidString == allowedUIDs[i])
        {
            accessGranted = true;
            break;
        }
    }

    // Handle Access Control
    if (accessGranted)
    {
        Serial.println("Access Granted!");
        digitalWrite(GREEN_LED_PIN, HIGH);
        doorServo.write(90); // Rotate servo to unlocked position

        delay(3000); // Keep open/active for 3 seconds

        doorServo.write(0); // Return servo to locked position
        digitalWrite(GREEN_LED_PIN, LOW);
    }
    else
    {
        Serial.println("Access Denied!");
        digitalWrite(RED_LED_PIN, HIGH);

        delay(3000); // Turn red LED on for 3 seconds

        digitalWrite(RED_LED_PIN, LOW);
    }

    // Halt card communication and reset Crypto state
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}