/*
 *  Copyright (C) 2018 - Handiko Gesang - www.github.com/handiko
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// include AD9851 library, from https://github.com/handiko/AD9851
#include <AD9851.h>

/*  define the I/O ports that being used
 *  
 *  | DDS pins | ESP12 pins|
 *  |----------|-----------|
 *  | RST      |  GPIO-13  |
 *  | DATA     |  GPIO-12  |
 *  | FQ       |  GPIO-14  |
 *  | CLK      |  GPIO-16  |
 *  
 */
#define RST   13
#define DATA  12
#define FQ    14
#define CLK   16

DDS dds;

// min_freq : the frequency which the Jammer starts transmitting on
// max_freq : the frequency which the Jammer stops transmitting on
unsigned long min_freq = 34800000UL;
unsigned long max_freq = 35200000UL;

void setup()
{
  Serial.begin(115200);
  
  Serial.println(" ");
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  
  Serial.println("Test - Started ! \n");

  // initialize the DDS
  dds = dds_init(RST, DATA, FQ, CLK);
  dds_reset(dds);
  writeFreq(dds, min_freq);
}

void loop()
{
  // create random hopping RF Jammer, by which the frequency
  // is bounded by the min_freq and max_freq values
  writeFreq(dds, random(min_freq, max_freq));
}
