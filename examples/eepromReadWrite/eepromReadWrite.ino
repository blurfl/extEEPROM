/*
  Copyright (c) 2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and / or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

// Scott Smith 28-06-2019 v3.4.3 add support for SoftwareWire library

#include "extEEPROM.h"

uint8_t i2cAddr = 0x50; // 0x57 in original example?
extEEPROM myEEPROM(kbits_256, 1, 64, i2cAddr);

// add SoftwareWire library and declare software I2C pins ...
#include <SoftwareWire.h>
uint8_t sdaPin = 36; // edit to suit the project
uint8_t sclPin = 37; //
SoftwareWire myWire(sdaPin, sclPin);  //

void setup(void)
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

// change begin() to call SoftwareWire
  byte i2cStat = myEEPROM.begin(myEEPROM.twiClock100kHz, &myWire);
//  byte i2cStat = myEEPROM.begin(myEEPROM.twiClock100kHz);

  if ( i2cStat != 0 ) {
    Serial.println(F("I2C Problem"));
  }

  Serial.println(F("EEPROM Memory commands:  read:(a)(l)(r) , write:(a)(d)(w), next read data (n)"));
  Serial.println(F("- Commands TO PRESS:"));
  Serial.println(F("\t a : memory address to read / write"));
  Serial.println(F("\t d : data to write"));
  Serial.println(F("\t l : data to write"));
  Serial.println(F("\t r : read command"));
  Serial.println(F("\t w : write command"));
}

unsigned long address = 0;
const unsigned int maxDataSize = 1024;  //0x8000;   // 32 k bytes (32768 = 0x8000) = 256 kbits

byte data[maxDataSize] = {'p', 'i', 'p', 'p', 'o'};
unsigned int dataSize = 5;

void eprom_read_write(bool write)
{
  byte i2cStat = 0;
  if (write) {
    i2cStat = myEEPROM.write(address, data, dataSize);
  } else {
    memset(data, 0, maxDataSize);
    i2cStat = myEEPROM.read(address, data, dataSize);
  }
  if ( i2cStat != 0 ) {
    //there was a problem
    Serial.print(F("I2C Problem: "));
    if ( i2cStat == EEPROM_ADDR_ERR) {
      Serial.println(F("Wrong address"));
    } else {
      Serial.print(F("I2C error: "));
      Serial.print(i2cStat);
      Serial.println(F(""));
    }
  }
}


void parse(char inChar)
{
  const char addr_len = 5;
  char       addr_char[addr_len] = "";
  const char data_len = 3;
  char       size_char[data_len] = "";
  unsigned char inc = 0, i = 0, j = 0;

  switch (inChar) {
    case 'a':
      Serial.print(F("Insert Address as 4 Hex chars (without '0x'):  "));

      while (i < 4) {
        while (Serial.available() <= 0)
          ;
        inc = Serial.read();

        if (inc == 'q')
          return;

        addr_char[i] = inc;
        ++i;
      }
      address = (unsigned long)strtol(addr_char, NULL, 16);
      Serial.println(address);
      break;

    case 'd':
      Serial.print(F("Insert Hex data sequence (without '0x'), return to enter: "));
      memset(data, 0, maxDataSize);
      while (true) {
        while (Serial.available() <= 0)
          ;
        inc = Serial.read();
        if (inc == 'q')
          return;
        if (inc == '\r' || inc == '\n')
          break;

        if (inc >= 'a' && inc <= 'f')
          data[j] += inc - 'a' + 10;
        else  if (inc >= 'A' && inc <= 'F')
          data[j] += inc - 'A' + 10;
        else if (inc >= '0' && inc <= '9')
          data[j] += inc - '0';
        else return;

        if (i % 2) {
          j++;
        } else {
          data[j] = data[j] << 4;
        }
        i++;
      }
      dataSize = j;
      Serial.println(dataSize);
      Serial.println(F(""));
      break;
    case 'l':
      Serial.print(F("Insert data len as 2 Hex chars (without '0x'): "));
      while (i < 2) {
        while (Serial.available() <= 0)
          ;
        inc = Serial.read();
        if (inc == 'q')
          return;

        size_char[i] = inc;
        ++i;
        if (inc == '\n') {
          return;
        }
      }

      dataSize = (unsigned int)strtol(size_char, NULL, 16);
      Serial.println(dataSize);
      break;


    case 'n':
      address += dataSize;
    /* FALLTHROUGH */
    case 'r':
      Serial.print(F("reading address: "));
      Serial.println(address, HEX);

      eprom_read_write(false);
      for (i = 0; i < dataSize ; ++i) {
        Serial.print(data[i], HEX);
        Serial.print(F(" "));
      }
      Serial.println();

      break;

    case 'w':
      Serial.print(F("writing at address: "));
      Serial.print(address, HEX);
      Serial.print(F(", len: "));
      Serial.println(address, dataSize);
      for (i = 0; i < dataSize ; ++i) {
        Serial.print(data[i], HEX);
        Serial.print(F(" "));
      }
      eprom_read_write(true);
      Serial.println();

      break;
    case 'T':
      Serial.println(F("Memory test: writing and verifying the whole memory"));
      break;

    default:
      break;
  }
}


void loop(void)
{
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    Serial.print(inChar);
    parse(inChar);
  }

  delay(10);
}

