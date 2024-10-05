#include <Adafruit_SSD1306.h>
#include <Adafruit_Keypad.h>

#include <esp_now.h>
#include <Wire.h>

#define KEYPAD_PID1824
#define MAXCHARS 256
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

typedef struct Msg {
  char chars[MAXCHARS];
  int len;
} Msg;

Msg msg;

char alphabet[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                  'J', 'K', 'L', 'M', 'N', 'O', 'P', 'R', 'S', 'T', 
                  'U', 'V', 'W', 'X', 'Y', 'Q', 'Z'};

byte characterTable[] = { 1, 2, 3,
                          4, 5, 6,
                          7, 8, 9,
                          10, 11, 12 };

//               row1   2     3     4
byte rowPin[] = {NULL, NULL, NULL, NULL};
//               col1   2     3
byte colPin[] = {NULL, NULL, NULL};

Adafruit_Keypad keypad(characterTable, rowPin, colPin, 4, 3);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int timesPressed = 0;
char toPush = '\0';

enum inputMode { CHAR, DIGIT, RECV };
inputMode currentMode = RECV;

void setup()
{
  // Runs once
  Serial.begin(115200);
  msg.len = 0;
}

void loop()
{
  // Looped code
  keypad.tick();

  Serial.println(keypad.available());
  if (keypad.available() != NULL)
    readInput(keypad.read());

  display.println("HELLO WORLD");
}

// Get input from read key press
// Based on the current read mode
void readInput(keypadEvent e)
{
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
  
  if(checkSymbols(key))
    return;

  timesPressed++;

  int mod = timesPressed % 3;
  if (mod == 0)
    mod = 3;

  int indx = ((key - 2) * 3) + mod;
  toPush = alphabet[indx];
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
  
  if (key == 11)
    key = 0;

  // String intAsString = ""+key;
  // toPush = intAsString.charAt(0);

  toPush = 30 + key;
}

// Checks if the button press was the * or #, then perform that action
bool checkSymbols(int key)
{
  if (key == 10)
  {
    space();
    return true;
  }
    
  if (key == 12)
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
  msg.chars[msg.len++] = toPush;
  toPush = '\0';

  // Update OLED
}

// Delete the last char from the queue and screen
void deleteFromDisplay() 
{
  if (msg.len <= 0)
    return;
  
  msg.len--;

  // Update OLED
}