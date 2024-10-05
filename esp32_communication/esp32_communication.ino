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
}

void keypadSetup(byte[] table)
{
  keypad = new Adafruit_Keypad(characterTable, rowPin, colPin, 4, 3);
}

void readInput()
{
  

  switch (inputMode)
    case "CHAR":
      readChar();
      break;
    case "DIGIT":
      readDigit();
      break;
}

void readChar()
{

}

void readDigit()
{

}

void clearBuffer()
{
  // if commit character button
  // keypad.clear();

  // if backspace button
  // keypad.clear();

  // if cancel button
  // keypad.clear();
}