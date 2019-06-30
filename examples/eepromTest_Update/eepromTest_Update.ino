// Test extEEPROM library - update() method.
// Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
// adapted for SoftwareWire by Scott Smith 29-06-2019

#include <extEEPROM.h>

// add SoftwareWire library and declare software I2C pins ...
#include <SoftwareWire.h>
uint8_t sdaPin = 36; // edit to suit the project
uint8_t sclPin = 37; //
SoftwareWire myWire(sdaPin, sclPin);  //


extEEPROM ep(kbits_256, 1, 64); // device size, number of devices, page size

void test_update() {
  Serial.println("\n(0) Test Started");

  constexpr auto addr  = 0;
  constexpr int length = (BUFFER_LENGTH * 2) + 13;

  byte writeBuf[length];

  for (auto i = 0; i < length; i++) {
    writeBuf[i] = i;
  }

  auto rv = ep.update(addr, writeBuf, length);
  if (rv) {
    Serial.print("(1) Error updating: ");
    Serial.println(rv);
  } else {
    Serial.println("(1)  update step complete ");
  }

  byte readBuf[length];
  for (auto i = 0; i < length; i++) {
    readBuf[i] = ~writeBuf[i];
  }

  rv = ep.read(addr, readBuf, length);
  if (rv) {
    Serial.print("(2) Error reading: ");
    Serial.println(rv);
  } else {
    Serial.println("(2)  read step complete ");
  }


  rv = memcmp(writeBuf, readBuf, length);
  if (rv) {
    Serial.print("(3) Error comparing: ");
    Serial.println(rv);
  } else {
    Serial.println("(3)  compare step complete ");
  }


  for (auto i = 0; i < length; i++) {
    if (i % 2 || i % 3)
      writeBuf[i] = i;
  }

  rv = ep.update(addr, writeBuf, length);
  if (rv) {
    Serial.print("(4) Error updating: ");
    Serial.println(rv);
  } else {
    Serial.println("(4)  update step complete ");
  }


  rv = ep.read(addr, readBuf, length);
  if (rv) {
    Serial.print("(5) Error reading: ");
    Serial.println(rv);
  } else {
    Serial.println("(5)  read step complete ");
  }


  rv = memcmp(writeBuf, readBuf, length);
  if (rv) {
    Serial.print("(6) Error comparing: ");
    Serial.println(rv);
  } else {
    Serial.println("(6)  compare step complete ");
  }


  Serial.println("(7) Test OK");
}

void setup(void) {
  Serial.begin(115200);

// change begin() call to call SoftwareWire
  const auto eepStatus = ep.begin(ep.twiClock100kHz, &myWire);
//  const auto eepStatus = ep.begin(ep.twiClock100kHz);
  if (eepStatus) {
    Serial.print("extEEPROM.begin() failed, status = ");
    Serial.println(eepStatus);
    while (1)
      ;
  }

  test_update();
}

void loop(void) {
}