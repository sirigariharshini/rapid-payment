/***************************************************
  Modified for ESP32-S by Bubbly & GPT-5
  Original: Adafruit Fingerprint Sensor Example
  Library: Adafruit_Fingerprint

  Connections:
  ---------------------------
  Sensor  |  ESP32-S
  ---------------------------
  VCC     |  3.3V
  GND     |  GND
  RX (Y)  |  TX (GPIO17)
  TX (G)  |  RX (GPIO16)
****************************************************/

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

// Use Hardware Serial 2 (UART2)
HardwareSerial mySerial(2);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment (ESP32-S)");

  // Initialize UART2 for fingerprint sensor
  mySerial.begin(57600, SERIAL_8N1, 16, 17); 
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("✅ Found fingerprint sensor!");
  } else {
    Serial.println("❌ Fingerprint sensor not detected. Check wiring!");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters..."));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  Serial.println("\nReady to enroll fingerprints!");
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  Serial.println("Enter the ID # (1 to 127) to save this finger as:");
  id = readnumber();
  if (id == 0) return;

  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!getFingerprintEnroll());
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK: Serial.println("✅ Image taken"); break;
      case FINGERPRINT_NOFINGER: Serial.print("."); break;
      case FINGERPRINT_PACKETRECIEVEERR: Serial.println("❌ Communication error"); break;
      case FINGERPRINT_IMAGEFAIL: Serial.println("❌ Imaging error"); break;
      default: Serial.println("❌ Unknown error"); break;
    }
    delay(100);
  }

  // Convert image to characteristics and store in buffer 1
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK: Serial.println("✅ Image converted"); break;
    case FINGERPRINT_IMAGEMESS: Serial.println("❌ Image too messy"); return p;
    case FINGERPRINT_PACKETRECIEVEERR: Serial.println("❌ Communication error"); return p;
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("❌ Could not find fingerprint features"); return p;
    default: Serial.println("❌ Unknown error"); return p;
  }

  Serial.println("Remove finger...");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("Place same finger again...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK: Serial.println("✅ Image taken"); break;
      case FINGERPRINT_NOFINGER: Serial.print("."); break;
      case FINGERPRINT_PACKETRECIEVEERR: Serial.println("❌ Communication error"); break;
      case FINGERPRINT_IMAGEFAIL: Serial.println("❌ Imaging error"); break;
      default: Serial.println("❌ Unknown error"); break;
    }
    delay(100);
  }

  // Convert image to characteristics and store in buffer 2
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK: Serial.println("✅ Image converted"); break;
    case FINGERPRINT_IMAGEMESS: Serial.println("❌ Image too messy"); return p;
    case FINGERPRINT_PACKETRECIEVEERR: Serial.println("❌ Communication error"); return p;
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("❌ Could not find fingerprint features"); return p;
    default: Serial.println("❌ Unknown error"); return p;
  }

  // Create and store model
  Serial.print("Creating model for #"); Serial.println(id);
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Fingerprints matched!");
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("❌ Fingerprints did not match");
    return p;
  } else {
    Serial.println("❌ Unknown error creating model");
    return p;
  }

  Serial.print("Storing fingerprint with ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("✅ Fingerprint stored successfully!");
  } else {
    Serial.println("❌ Error storing fingerprint");
    return p;
  }

  return true;
}
