//Test extEEPROM library.
//Writes the EEPROM full of 32-bit integers and reads them back to verify.
//Wire a button from digital pin 6 to ground, this is used as a start button
//so the sketch doesn't do unnecessary EEPROM writes every time it's reset.
//Jack Christensen 09Jul2014
//Paolo Paolucci 17Mar2016 (fix 28Jun2017)
// adapted for SoftwareWire by Scott Smith 29-06-2019 --
// use Serial character receive instead of pushbutton to start test


// add SoftwareWire library and declare software I2C pins ...
#include <SoftwareWire.h>
uint8_t sdaPin = 36; // edit to suit the project
uint8_t sclPin = 37; //
SoftwareWire myWire(sdaPin, sclPin);  //

#include <extEEPROM.h>    //https://github.com/PaoloP74/extEEPROM

//One 24LC256 EEPROMs on the bus
const uint32_t totalKBytes = 32;         //for read and write test functions
extEEPROM eep(kbits_256, 1, 64);         //device size, number of devices, page size

const uint8_t btnStart = 6;              //start button

void setup(void)
{
  Serial.begin(115200);
  //
  // change begin() to call SoftwareWire
  uint8_t eepStatus = eep.begin(eep.twiClock400kHz, &myWire);   //go fast!
  if (eepStatus) {
    Serial.print(F("extEEPROM.begin() failed, status = "));
    Serial.println(eepStatus);
    while (1);
  }

  Serial.println(F("\n\nSend a character to start..."));
  while (!Serial.available()) delay(10);    //wait for Serial character

  uint8_t chunkSize = 64;    //this can be changed, but must be a multiple of 4 since we're writing 32-bit integers
//  uint8_t chunkSize = 72;    //this can be changed, but must be a multiple of 4 since we're writing 32-bit integers
//  eeErase(chunkSize, 0, totalKBytes * 1024 - 1);
  eeWrite(chunkSize);
  eeRead(chunkSize);

  dump(0, 32);          //the first 32 bytes
  dump(0, 128);         //the first 128 bytes
  dump(0x6FE0, 64);     // a chunk out of the middle, using hex address
  dump(32704, 64);      // last 64 bytes of EEPROM
  dump(0x7FE0, 64);     // 64 bytes starting 32 bytes from the end of the EEPROM, doesn't wrap around
}

void loop(void)
{
}

void eeWrite(uint8_t chunk)
{
  chunk &= 0xFC;                //force chunk to be a multiple of 4 -- Why?
  uint8_t data[chunk];
  uint32_t val = 0;
  uint8_t result = 0;
  Serial.println(F("Writing, errors will trigger an error message line..."));
  uint32_t msStart = millis();

  for (uint32_t addr = 0; addr < totalKBytes * 1024; addr += chunk) {
    if ( (addr & 0xFFF) == 0 ) {
      Serial.print(F("0x"));
      if ( addr < 16 * 16 * 16 ) Serial.print(F("0"));
      if ( addr < 16 * 16 )      Serial.print(F("0"));
      if ( addr < 16 )           Serial.print(F("0"));
      Serial.print(addr, HEX);
      Serial.print(F(" -- "));
      Serial.println(addr);
    }
    //    Serial.print("0x\t");
    //    Serial.print(addr, HEX);
    //    Serial.print("\t\t");
    //    Serial.println(addr);
    for (uint8_t c = 0; c < chunk; c += 4) {
      data[c + 0] = val >> 24;
      data[c + 1] = val >> 16;
      data[c + 2] = val >> 8;
      data[c + 3] = val;
      ++val;
    }
    result = eep.write(addr, data, chunk);
  }
  uint32_t msLapse = millis() - msStart;
  Serial.print(F("\nWrite lapse: ")); Serial.print(msLapse); Serial.println(F(" ms"));
}

//read test data (32-bit integers) from eeprom, "chunk" bytes at a time
void eeRead(uint8_t chunk)
{
  chunk &= 0xFC;                //force chunk to be a multiple of 4
  uint8_t data[chunk];
  uint32_t val = 0, testVal;
  Serial.println(F("Reading, errors will trigger an error message line..."));
  uint32_t msStart = millis();

  for (uint32_t addr = 0; addr < totalKBytes * 1024; addr += chunk) {
    if ( (addr & 0xFFF) == 0 ) {
      Serial.print(F("0x"));
      if ( addr < 16 * 16 * 16 ) Serial.print(F("0"));
      if ( addr < 16 * 16 )      Serial.print(F("0"));
      if ( addr < 16 )           Serial.print(F("0"));
      Serial.print(addr, HEX);
      Serial.print(F(" -- "));
      Serial.println(addr);
    }
    eep.read(addr, data, chunk);
    for (uint8_t c = 0; c < chunk; c += 4) {
      testVal =  ((uint32_t)data[c + 0] << 24) + ((uint32_t)data[c + 1] << 16) + ((uint32_t)data[c + 2] << 8) + (uint32_t)data[c + 3];
      if (testVal != val)
      {
        Serial.print(F("Error @ addr "));
        Serial.print(addr + c);
        Serial.print(F(" Expected "));
        Serial.print(val);
        Serial.print(F(" Read "));
        Serial.print(testVal);
        Serial.print(F(" 0x"));
        Serial.println(testVal, HEX);
      }
      ++val;
    }
  }
  uint32_t msLapse = millis() - msStart;
  Serial.print(F("Last value: "));
  Serial.print(val);
  Serial.print(F(" Read lapse: ")); Serial.print(msLapse); Serial.println(F(" ms"));
}

//write 0xFF to eeprom, "chunk" bytes at a time
void eeErase(uint8_t chunk, uint32_t startAddr, uint32_t endAddr)
{
  chunk &= 0xFC;                //force chunk to be a multiple of 4
  uint8_t data[chunk];
  Serial.println(F("Erasing, errors will trigger an error message line..."));
  for (int i = 0; i < chunk; i++) data[i] = 0xFF;
  uint32_t msStart = millis();

  for (uint32_t a = startAddr; a <= endAddr; a += chunk) {
    //    if ( (a & 0xFFF) == 0 ) Serial.println(a);
    if ( (a & 0xFFF) == 0 ) {
      Serial.print(F("0x"));
      if ( a < 16 * 16 * 16 ) Serial.print(F("0"));
      if ( a < 16 * 16 )      Serial.print(F("0"));
      if ( a < 16 )           Serial.print(F("0"));
      Serial.print(a, HEX);
      Serial.print(F(" -- "));
      Serial.println(a);
    }
    eep.write(a, data, chunk);
  }
  uint32_t msLapse = millis() - msStart;
  Serial.print(F("Erase lapse: ")); Serial.print(msLapse); Serial.println(F(" ms"));
}

//dump eeprom contents, 16 bytes at a time.
//always dumps a multiple of 16 bytes. Does not wrap around at top of EEPROM
//
// EEPROM DUMP HEX start address  HEX chunk length -- decimal start address  decimal chunk length
// HEX address, HEX values, decimal values

// example output:

//EEPROM DUMP 0x0 0x20 -- 0 32
//0x0000 00 00 00 00 00 00 00 01  00 00 00 02 00 00 00 03       0   0   0   0   0   0   0   1    0   0   0   2   0   0   0   3
//0x0010 00 00 00 04 00 00 00 05  00 00 00 06 00 00 00 07       0   0   0   4   0   0   0   5    0   0   0   6   0   0   0   7

void dump(uint32_t startAddr, uint32_t nBytes)
{
  Serial.print(F("EEPROM DUMP 0x"));
  Serial.print(startAddr, HEX);
  Serial.print(F(" 0x"));
  Serial.print(nBytes, HEX);
  Serial.print(F(" -- "));
  Serial.print(startAddr);
  Serial.print(F(" "));
  Serial.println(nBytes);
  uint32_t nRows = (nBytes + 15) >> 4;

  uint8_t d[16];
  for (uint32_t r = 0; r < nRows; r++) {
    uint32_t a = startAddr + (16 * r);
    Serial.print(F("0x"));
    if ( a < 16 * 16 * 16 ) Serial.print(F("0"));
    if ( a < 16 * 16 ) Serial.print(F("0"));
    if ( a < 16 ) Serial.print(F("0"));
    Serial.print(a, HEX); Serial.print(F(" "));
    eep.read(a, d, 16);
    for ( int c = 0; c < 16; c++ ) {
      if ( d[c] < 16 ) Serial.print(F("0"));
      Serial.print(d[c], HEX);
      Serial.print( c == 7 ? "  " : " ");
    }
    Serial.print("    ");
    char buf[5];
    for ( int c = 0; c < 16; c++ ) {
      sprintf(buf, "%4d", d[c]);
      Serial.print(buf);
      Serial.print( c == 7 ? "  " : " ");
    }
    Serial.println(F(""));
  }
}