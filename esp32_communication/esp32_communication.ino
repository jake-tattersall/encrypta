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

#define OLED_MOSI   16
#define OLED_CLK   18
#define OLED_DC    2
#define OLED_CS    15
#define OLED_RESET 4
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

byte rowPin[] = {13, 12, 14, 27};
byte colPin[] = {17, 22, 23};

Adafruit_Keypad keypad( makeKeymap(characterTable), rowPin, colPin, ROWS, COLS);
// The keypad buttons result in the following e.bit.KEY outputs:
// 49 50 51
// 52 53 54
// 55 56 57
// 42 48 35

// This is because the table had to be converted to char (it just worked this way) 
// To get the 1-9 values, modulus by 12
// We do special cases for * and # anyway


#define RED 5
#define GREEN 21
#define BLUE 25
#define YELLOW 33
#define YELLOW_LED 32
#define BLUE_LED 26


uint8_t prevKey = 0;
int timesPressed = 0;
char toPush = '\0';

enum inputMode { CHAR, DIGIT, RECV };
inputMode currentMode = RECV;

unsigned long timeElapsed = 0;
unsigned long lastTime = 0;

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

  pinMode(RED, INPUT);
  pinMode(GREEN, INPUT);
  pinMode(BLUE, INPUT);
  pinMode(YELLOW, INPUT);
  pinMode(YELLOW_LED, INPUT);
  pinMode(BLUE_LED, INPUT);

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

  // if blue button
  if (digitalRead(BLUE) == HIGH)
  {
    if (currentMode == CHAR)
      currentMode = DIGIT;
    else
      currentMode = CHAR;
  }

  if (digitalRead(YELLOW) == HIGH)
  {
    if (currentMode == RECV)
      currentMode = CHAR;
    else
      currentMode = RECV;
  }

  if (digitalRead(RED) == HIGH)
  {
    deleteFromDisplay();
  }

  if (digitalRead(GREEN) == HIGH)
  {
    // Send message
  }
}

void selectMode()
{
  switch (currentMode)
  {
    case CHAR:
      timeElapsed = 0;
      lastTime = milis();
      break;
    case RECV:
      // Clear board
      // Prepare for recv
      break;
  }
}

// Get input from read key press
// Based on the current read mode
void readInput(keypadEvent e)
{
  //Serial.println(e.bit.KEY);
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

  adjustCursorX(true, 10);
  display.print(' ');
  display.display();
  adjustCursorX(true, 10);
}

// true for decrement, false for increment
void adjustCursorX(bool mode, int offset)
{
  int temp = display.getCursorX();

  if (true)
    temp -= offset;
  else
    temp += offset;

  display.setCursor(temp, display.getCursorY());
}

void cursorPulse()
{
  lastTime = milis() - lastTime;
  timeElapsed += lastTime;

  if (timeElapsed > 500 && currentMode != RECV)
    drawCursorBlock();
}

void drawCursorBlock()
{
  display.fillRect(display.getCursorX(), display.getCursorY(), 10, 5);
}