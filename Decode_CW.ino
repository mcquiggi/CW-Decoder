// Arduino Sketch for decoding Morse code on the pin 2 digital input pin.

//    A conversion of my old Assembler/Pascal CW decoder program circa 1983, modified to run on
//    the Arduino.  KMCQ April 2024.
//
//    Written essentially in C with some shortcuts taken via the Arduino Sketch convenience functions
//
//    Arduino: Version 1.0

//    Copyright 1983, 2024 by Kevin McQuiggin VE7ZD/KN7Q (mcquiggi@sfu.ca)
//    Released under the GNU Public License (GPL) 3.0 in 2024
//
//    This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as 
//    published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along with this program. If not, see \
//    <https://www.gnu.org/licenses/>. 



// Global variables:
const int CW_PIN = 2;         // CW input is on Digital Pin 2
int cw_state = 0;             // State of the pin: 0 is key up; 1 is key down
int time[16];                 // Vector that stores 16 most recent key down times in
                              // in increments of 10 milliseconds
int avg_speed, key_up_time,   // Duration variables (units of 10 ms)
    key_down_time;
char symbol[8];               // Accumulates the current CW symbol
char pattern[42][8];          // Lookup table: symbols to alphanumeric character
char alphabet[42];            // CW alphabet
int handled = 0;              // Flag to indicate that key up event has been handled
char letter[4];               // Symbol lookup result (string)


// Function to store the last element key-down time and recompute the average element timing:
int update_speed() {
  static int p=0;
  int j, n, sum;

  // Record last key down time:
  time[p]=key_down_time;
  p=(p+1)%16;
  
  // Recompute average time for a code element:
  sum=0;
  n=0;
  for (j=0; j < 16; j++) {
    if (time[j] != 0) {
      sum+=time[j];
      n++;
    }
  }
  return round(sum/n);
}


// Function to look up a dot-dash pattern and return the alphanumeric symbol it represents:
char lookup_symbol() {
  int j;

  for (j=0; j < strlen(alphabet); j++) {
    if (strcmp(pattern[j], symbol) == 0)
      return(alphabet[j]);
  }
  return 0;                                   // Return 0 if the symbol was invalid
}


// The setup function:
void setup() {

  // Initialize CW character patterns. 0 is a dit and 1 is a dah:
  strcpy(pattern[ 0], "01");     // A
  strcpy(pattern[ 1], "1000");   // B
  strcpy(pattern[ 2], "1010");   // C
  strcpy(pattern[ 3], "100");    // D
  strcpy(pattern[ 4], "0");      // E
  strcpy(pattern[ 5], "0010");   // F
  strcpy(pattern[ 6], "110");    // G
  strcpy(pattern[ 7], "0000");   // H
  strcpy(pattern[ 8], "00");     // I
  strcpy(pattern[ 9], "0111");   // J
  strcpy(pattern[10], "101");    // K
  strcpy(pattern[11], "0100");   // L
  strcpy(pattern[12], "11");     // M
  strcpy(pattern[13], "10");     // N
  strcpy(pattern[14], "111");    // O
  strcpy(pattern[15], "0110");   // P
  strcpy(pattern[16], "1101");   // Q
  strcpy(pattern[17], "010");    // R
  strcpy(pattern[18], "000");    // S
  strcpy(pattern[19], "1");      // T
  strcpy(pattern[20], "001");    // U
  strcpy(pattern[21], "0001");   // V
  strcpy(pattern[22], "011");    // W
  strcpy(pattern[23], "1001");   // X
  strcpy(pattern[24], "1011");   // Y
  strcpy(pattern[25], "1100");   // Z
  strcpy(pattern[26], "01111");  // 1
  strcpy(pattern[27], "00111");  // 2
  strcpy(pattern[28], "00011");  // 3
  strcpy(pattern[29], "00001");  // 4
  strcpy(pattern[30], "00000");  // 5
  strcpy(pattern[31], "10000");  // 6
  strcpy(pattern[32], "11000");  // 7
  strcpy(pattern[33], "11100");  // 8
  strcpy(pattern[34], "11110");  // 9
  strcpy(pattern[35], "11111");  // 0
  strcpy(pattern[36], "001100"); // ?
  strcpy(pattern[37], "110011"); // ,
  strcpy(pattern[38], "10001");  // -
  strcpy(pattern[39], "101101"); // (
  strcpy(pattern[40], "010101"); // .
  strcpy(pattern[41], "10010");  // /

  // Initialize known alphabet:
  strcpy(alphabet, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890?,-(./");

  // LED pin is an output:
  pinMode(LED_BUILTIN, OUTPUT);

  // CW pin is an input:
  pinMode(CW_PIN, INPUT);

  // Clear current symbol, time delay array, output letter and timers:
  strcpy(symbol, "");
  memset(time, 0, sizeof(time));
  memset(letter, 0, sizeof(letter));
  key_up_time=0;
  key_down_time=0;
  
  // Initialize serial port:
  Serial.begin(9600);
  Serial.println("CW Decoder for Arduino\nVersion 1.0\n\n");
}


// The processing loop:
void loop() {
  char c;

  // Get input state:
  cw_state = digitalRead(CW_PIN);
  
  // Switch on the CW state (key up, or key down) and do some timing:
  if (cw_state) {
    digitalWrite(LED_BUILTIN, HIGH);      // Note the input on Arduino's built-in LED
    
    // Time the key-down event:
    key_down_time=0;
    handled=0;
    while (digitalRead(CW_PIN)) {              
      key_down_time++;
      delay(10);
    }

    // Update the code speed average value:
    avg_speed=update_speed();

    // Interpret code element and append to symbol[]:
    if (key_down_time <= avg_speed)
      strcat(symbol, "0");
    else 
      strcat(symbol, "1");
  }

  else {

    // Key up time period:
    digitalWrite(LED_BUILTIN, LOW);       // Note the input on Arduino's built-in LED
    key_up_time=0;
    while (!digitalRead(CW_PIN) && !handled) {              
      key_up_time++;
      delay(10);
      if (key_up_time > avg_speed) {
        c=lookup_symbol();                // Look up the symbol and convert it to a character
        if (c != 0) {
          letter[0]=c;
          letter[1]=0;                    // Zero terminate the character
        }
        else 
          strcpy(letter, "@");            // Symbol not in the alphabet - indicate bad CW character!
        Serial.print(letter);
        strcpy(symbol, "");
        handled=1;
        break;
      }
    }
  }
}


