#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Keypad.h>
#include <SPI.h>

#include <esp_now.h>
#include <Wire.h>

#include <encryption.h>

#define KEYPAD_PID1824

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MAXCHARS 168 // Num of chars that fit on oled screen
#define ROWS 4
#define COLS 3
#define STAR 42
#define POUND 35

#define OLED_MOSI   4
#define OLED_CLK   18
#define OLED_DC    17
#define OLED_CS    5
#define OLED_RESET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


typedef struct Msg {
  char chars[MAXCHARS];
  int len;
} Msg;

Msg msg;

char alphabet[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                  'J', 'K', 'L', 'M', 'N', 'O', 'P', 'R', 'S', 'T', 
                  'U', 'V', 'W', 'X', 'Y', 'Q', 'Z'};

char characterTable[ROWS][COLS] = {
                                  {'1', '2', '3'}, 
                                  {'4', '5', '6'}, 
                                  {'7', '8', '9'}, 
                                  {'*', '0', '#'}};


//               row1   2     3     4
byte rowPin[] = {33, 25, 26, 27};
//               col1   2     3
byte colPin[] = {21, 22, 32};

Adafruit_Keypad keypad( makeKeymap(characterTable), rowPin, colPin, ROWS, COLS);
// The keypad buttons result in the following e.bit.KEY outputs:
// 49 50 51
// 52 53 54
// 55 56 57
// 42 48 35

// This is because the table had to be converted to char (it just worked this way) 

// To get the 1-9 values, modulus by 12
// We do special cases for * and # anyway


uint8_t prevKey = 0;
int timesPressed = 0;
char toPush = '\0';

enum inputMode { CHAR, DIGIT, RECV };
inputMode currentMode = RECV;

void setup()
{
  // Runs once
  Serial.begin(115200);
  keypad.begin();


  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  //delay(2000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  msg.len = 0;

  // PLEASE CHANGE LATER, ONLY FOR TESTING
  currentMode = CHAR;
}

void loop()
{

  // Looped code
  keypad.tick();

  //Serial.println(keypad.available());
  if (keypad.available())
    readInput(keypad.read());
}

// Get input from read key press
// Based on the current read mode
void readInput(keypadEvent e)
{
  Serial.println(e.bit.KEY);
  // Button is considered triggered onPress and onRelease. This ignores release
  if (keypad.justReleased(e.bit.KEY)) return;
  
  switch (currentMode)
  {
    case CHAR:
      readChar(e);
      break;
    case DIGIT:
      readDigit(e);
      break;
  }
}

// Determines which char was pressed 
// Sets toPush to the char
void readChar(keypadEvent e)
{
  int key = e.bit.KEY;
  
  if(key == 49 || checkSymbols(key))
    return;

  // Resets timesPressed if different button is pressed
  if (key == prevKey)
    timesPressed++;
  else {
    timesPressed = 1;
    prevKey = key;
  }

  key %= 12;

  // Take into account that button 0 only has 2 chars not 3 (Q and Z)
  int mod, indx;
  if (key == 0) {
    mod = timesPressed % 2;
    if (mod == 0)
      mod = 2;
    indx = 24 + mod;
    toPush = alphabet[indx];
  } else {
    mod = timesPressed % 3;
    if (mod == 0)
      mod = 3;
    indx = ((key - 2) * 3) + mod;
    toPush = alphabet[indx];
  }

  Serial.print("ToPush = ");
  Serial.println(toPush);
  
}

// Determines which number was pressed
// Sets toPush to the digit
void readDigit(keypadEvent e)
{
  int key = e.bit.KEY;

  if (checkSymbols(key))
    return;
  
  if (key == 48)
    key = 0;

  // String intAsString = ""+key;
  // toPush = intAsString.charAt(0);

  toPush = 30 + key % 12;
}

// Checks if the button press was the * or #, then perform that action
bool checkSymbols(int key)
{
  if (key == STAR)
  {
    space();
    return true;
  }
    
  if (key == POUND)
  {
    confirm();
    return true;
  }

  return false;
}


void space()
{
  toPush = ' ';
  pushToDisplay();
}

void confirm()
{
  clearBuffer();
  pushToDisplay();
}

void clearBuffer()
{
  keypad.clear();
  timesPressed = 0;
}

// Write the toPush char to the queue and screen
void pushToDisplay()
{
  if (toPush == '\0')
    return; 

  msg.chars[msg.len++] = toPush;
  toPush = '\0';

  

  for (int i = 0; i < msg.len; i++) {
    Serial.print(msg.chars[i]);
  }
  Serial.println();

  //display.clearDisplay();
  display.print(msg.chars[msg.len-1]);
  display.display();
}

// Delete the last char from the queue and screen
void deleteFromDisplay() 
{
  if (msg.len <= 0)
    return;
  
  msg.len--;

  // Update OLED
}






