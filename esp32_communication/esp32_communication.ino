#include <Adafruit_SSD1306.h>
#include <Adafruit_Keypad.h>

#include <esp_now.h>

#define KEYPAD_PID1824
#define MAXCHARS 256

typedef struct Msg {
  char[256] chars;
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

int timesPressed = 0;
char toPush;

enum inputMode { CHAR, DIGIT, RECV };
inputMode currentMode = RECV;

void setup()
{
  Serial.begin(115200);
  msg.len = 0;
  toPush = '\0';
}

void loop()
{
  keypad.tick();

  Serial.println(keypad.available());
  if (keypad.available() != NULL)
    readInput(keypad.read());
}

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
}

void readDigit(keypadEvent e)
{
  int key = e.bit.KEY;

  if (checkSymbols(key))
    return;
  
  if (key == 11)
    key = 0;
}

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
}

void clearBuffer()
{
  keypad.clear();
  timesPressed = 0;
}

void pushToDisplay()
{
  msg.chars[msg.len++] = toPush;
  toPush = '\0';

  // Update OLED
}

void deleteFromDisplay() 
{
  if (msg.len <= 0)
    return;
  
  msg.len--;

  // Update OLED
}