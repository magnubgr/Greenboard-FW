#include <Arduino.h>
#include <BleKeyboard.h>
#include "driver/gpio.h"

void checkVolume();
void checkSwitchMatrix();
void scanSwitchMatrix();
void updateVolume();

BleKeyboard bleKeyboard;

#define rotA 4
#define rotB 5
#define statLed_R 0
#define statLed_G 3
#define capsLed 22
#define debounce_ms 20

uint8_t rotAState;
uint8_t rotALastState;

uint8_t lastRow = 0;
uint8_t lastCol = 0;

const uint8_t numRows = 10;
const uint8_t numCols = 8;
const uint8_t rows[] = {21, 17, 27, 26, 14, 1, 13, 23, 25, 12};
const uint8_t cols[] = {34, 35, 39, 16, 15, 2, 36, 19};


uint8_t keymap[numRows][numCols]  
{
  {KEY_ESC,       '1',  '2',        '3',  '4',            '5',           '6',  '7'},
  {KEY_TAB,       'q',  'w',        'e',  'r',            't',           'y',  'u'},
  {KEY_CAPS_LOCK, 'a',  's',        'd',  'f',            'g',           'h',  'j'},
  {KEY_LEFT_SHIFT,'<',  'z',        'x',  'c',            'v',           'b',  'n'},
  {KEY_LEFT_CTRL, KEY_LEFT_GUI,KEY_LEFT_ALT, ' ',  ' ',            ' ',           ' ',  ' '},
  {'8',           '9',  '0',        '-',  '\\',           KEY_BACKSPACE, ' ',  ' '},
  {'i',           'o',  'p',        '[',  ' ',            KEY_RETURN,    ' ',  ' '},
  {'k',           'l',  ';',        '\'',  ' ',           ' ',           ' ',  ' '},
  {'m',           ',',  '.',        '-',  KEY_RIGHT_SHIFT,' ',           ' ',  ' '},
  {' ',           ' ',KEY_RIGHT_ALT,KEY_RIGHT_GUI,  ' ',            KEY_RIGHT_CTRL,' ',  ' '}
};


short pressed[numRows][numCols]  = {HIGH};

//variables to keep track of the timing of recent interrupts
unsigned long last_button_time = 0; 

void IRAM_ATTR isr_switch_matrix() {
    scanSwitchMatrix();
}

void IRAM_ATTR isr_volume() {
    updateVolume();
}


void setup() {
  // Setting pin directions for Rotary Encoder
  pinMode(rotA,INPUT);
  pinMode(rotB,INPUT);
  // attachInterrupt(rotA, isr_volume, CHANGE);


  // Setting pin directions for the columns
  for (uint8_t colPin : cols){
    pinMode(colPin, INPUT_PULLUP);
    // attachInterrupt(colPin, isr_switch_matrix, FALLING);
  }

  // Setting pin directions for the rows and setting them high
  for (uint8_t rowPin : rows){
    pinMode(rowPin, OUTPUT);
    digitalWrite(rowPin, LOW);
  }

  pinMode(statLed_R, OUTPUT);
  pinMode(statLed_G, OUTPUT);
  pinMode(capsLed, OUTPUT);
  digitalWrite(statLed_R, HIGH);
  digitalWrite(statLed_G, HIGH);
  digitalWrite(capsLed, HIGH);

  // Serial.begin(115200);
  // pinMode(18,  7 INPUT);
  // int bv = analogRead(18);
  // Serial.println(bv);

  bleKeyboard.setName("Greenboard");
  bleKeyboard.setBatteryLevel(44);  

  bleKeyboard.begin();
}


void loop() {
  if(bleKeyboard.isConnected()) {

    checkSwitchMatrix();

    checkVolume();   
  }
}


void scanSwitchMatrix(){
  uint8_t irows;
  uint8_t icols;
  uint8_t reading;
  uint8_t curr_pressed;

  // Setting all rows to HIGH again to start scanning
  for (uint8_t rowPin : rows){
    digitalWrite(rowPin, HIGH);
  }

  for (irows = 0; irows<numRows; irows++){
    digitalWrite(rows[irows], LOW);
    for (icols = 0; icols<numCols; icols++){
      reading = digitalRead(cols[icols]);
      curr_pressed = pressed[irows][icols];

      
      if( millis() - last_button_time > debounce_ms){
        

        if (reading == LOW && curr_pressed == HIGH){
          if(icols == 6){
            if(irows == 5){
              bleKeyboard.press(KEY_MEDIA_MUTE);
            } else if (irows == 7){
              bleKeyboard.press(KEY_MEDIA_PREVIOUS_TRACK);
            } else if (irows == 8){
              bleKeyboard.press(KEY_MEDIA_NEXT_TRACK);
            } else if (irows == 9){
              bleKeyboard.press(KEY_MEDIA_PLAY_PAUSE);
            }
          } else{
            bleKeyboard.press(keymap[irows][icols]);
          }
          pressed[irows][icols] = LOW;
        }
        else if (reading == HIGH && curr_pressed==LOW){
          if(icols == 6){
            if(irows == 5){
              bleKeyboard.release(KEY_MEDIA_MUTE);
            } else if (irows == 7){            
              bleKeyboard.release(KEY_MEDIA_PREVIOUS_TRACK);
            } else if (irows == 8){
              bleKeyboard.release(KEY_MEDIA_NEXT_TRACK);
            } else if (irows == 9){
              bleKeyboard.release(KEY_MEDIA_PLAY_PAUSE);
            }
          } else{
            bleKeyboard.release(keymap[irows][icols]);
          }
          // bleKeyboard.release(keymap[irows][icols]);
          pressed[irows][icols] = HIGH;
          last_button_time = millis();
        }
      }
    }
    digitalWrite(rows[irows], HIGH);
  }
}

void updateVolume(){
  rotAState = digitalRead(rotA);
  if(rotAState != rotALastState){
    if(digitalRead(rotB) != rotAState){
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    } else {
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    }
  }
  rotALastState = rotAState;  
}


void checkSwitchMatrix(){
  uint8_t irows;
  uint8_t icols;
  uint8_t reading;
  uint8_t curr_pressed;
  for (irows = 0; irows<numRows; irows++){
    digitalWrite(rows[irows], LOW);
    for (icols = 0; icols<numCols; icols++){
      reading = digitalRead(cols[icols]);
      curr_pressed = pressed[irows][icols];
      
      if( millis() - last_button_time > debounce_ms){

        if (reading == LOW && curr_pressed == HIGH){
          if(icols == 6){
            if(irows == 5){
              bleKeyboard.press(KEY_MEDIA_MUTE);
            } else if (irows == 7){
              bleKeyboard.press(KEY_MEDIA_PREVIOUS_TRACK);
            } else if (irows == 8){
              bleKeyboard.press(KEY_MEDIA_NEXT_TRACK);
            } else if (irows == 9){
              bleKeyboard.press(KEY_MEDIA_PLAY_PAUSE);
            }
          } else{
            bleKeyboard.press(keymap[irows][icols]);
          }
          pressed[irows][icols] = LOW;
        }
        else if (reading == HIGH && curr_pressed==LOW){
          if(icols == 6){
            if(irows == 5){
              bleKeyboard.release(KEY_MEDIA_MUTE);
            } else if (irows == 7){            
              bleKeyboard.release(KEY_MEDIA_PREVIOUS_TRACK);
            } else if (irows == 8){
              bleKeyboard.release(KEY_MEDIA_NEXT_TRACK);
            } else if (irows == 9){
              bleKeyboard.release(KEY_MEDIA_PLAY_PAUSE);
            }
          } else{
            bleKeyboard.release(keymap[irows][icols]);
          }
          // bleKeyboard.release(keymap[irows][icols]);
          pressed[irows][icols] = HIGH;
          last_button_time = millis();
        }
        // if (reading == LOW && curr_pressed == HIGH){
        //   bleKeyboard.press(keymap[irows][icols]);
        //   pressed[irows][icols] = LOW;
        //   last_button_time = millis();
        // }
        // else if (reading == HIGH && curr_pressed==LOW){
        //   bleKeyboard.release(keymap[irows][icols]);
        //   pressed[irows][icols] = HIGH;
        //   last_button_time = millis();
        // }
      }

    }
    digitalWrite(rows[irows], HIGH);
  }
}



void checkVolume(){
  rotAState = digitalRead(rotA);
  if(rotAState != rotALastState){
    if(digitalRead(rotB) != rotAState){
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    } else {
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    }
  }
  rotALastState = rotAState;  
}
