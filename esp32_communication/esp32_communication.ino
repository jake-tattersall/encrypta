#include <Adafruit_SSD1306.h>
#include <Adafruit_Keypad.h>

Adafruit_Keypad keypad;

byte characterTable[] = { 1, 2, 3,
                          4, 5, 6,
                          7, 8, 9,
                          10, 11, 12 }

//               row1   2     3     4
byte rowPin[] = {null, null, null, null};
//               col1   2     3
byte colPin[] = {null, null, null};

int timesPressed = 0;

enum inputMode { CHAR, DIGIT };

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  keypadSetup();
}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.println("Hello World!");
  tick()

  keypadEvent e = keypad.read()
  if (e != null)
    readInput(e);
}

void keypadSetup(byte[] table)
{
  keypad = new Adafruit_Keypad(characterTable, rowPin, colPin, 4, 3);
}

void readInput(keypadEvent e)
{
  switch (inputMode)
    case "CHAR":
      readChar();
      break;
    case "DIGIT":
      readDigit();
      break;
}

void readChar(keypadEvent e)
{
  int key = e.KEY;


}

void readDigit(keypadEvent e)
{

}

void clearBuffer()
{
  // if commit character button
  // keypad.clear();
  // timesPressed = 0;

  // if backspace button
  // keypad.clear();
  // timesPressed = 0;

  // if cancel button
  // keypad.clear();
  // timesPressed = 0;
}