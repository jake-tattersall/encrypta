#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Keypad.h>

#include <esp_now.h>
//#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <string.h>
#include <encryption.h>

#define KEYPAD_PID1824

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MAXCHARS 168 // Num of chars that fit on oled screen
#define CHARWIDTH 6
#define ROWS 4
#define COLS 3
#define STAR 42
#define POUND 35

#define OLED_MOSI   16
#define OLED_CLK   18
#define OLED_DC    2
#define OLED_CS    15
#define OLED_RESET 4


esp_now_peer_info_t peerInfo;

uint8_t broadcastAddress[] = {0x88, 0x13, 0xbf, 0x63, 0x9a, 0xe0}; 
//uint8_t broadcastAddress[] = {0xac, 0x15, 0x18, 0xd5, 0x73, 0x5c};

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

int redLast = LOW;
int greenLast = LOW;
int blueLast = HIGH;
int yellowLast = HIGH;

uint8_t prevKey = 0;
int timesPressed = 0;
char toPush = '\0';

enum inputMode { CHAR, DIGIT, RECV };
inputMode currentMode = CHAR;

// unsigned long timeElapsed = 0;
unsigned long lastTime = 0;
bool cursor = false;

typedef struct struct_message {
  char msg[MAXCHARS];
} struct_message;

struct_message incoming_messages;

char *keyword = (char*) malloc(MAXCHARS * sizeof(char));
bool keyGiven = false;
/*Establishing ESP-NOW*/

void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status){
  Serial.print("Last Packet Sent Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len){
  Serial.println("Data Received");
  if (currentMode != RECV) return;
  memcpy(&incoming_messages, incomingData, sizeof(incoming_messages));
  Serial.print(len);
  int msg_len = strlen(incoming_messages.msg);

  // REMOVE LATER
  display.clearDisplay();
  display.setCursor(0, 0);
  for (int i = 0; i < msg_len; i++) {
    Serial.print(incoming_messages.msg[i]);
    display.print(incoming_messages.msg[i]);
  }
  display.setCursor(0, 8);

  char *temp = (char*) malloc(MAXCHARS * sizeof(char));
  strcpy(temp, decryptPlayfair(incoming_messages.msg, keyword));
  strcpy(incoming_messages.msg, temp);
  
  Serial.println(msg_len);
  //display.clearDisplay(); UNCOMMENT LATER
  //display.setCursor(0, 0); UNCOMMENT LATER
  for (int i = 0; i < msg_len; i++) {
    Serial.print(incoming_messages.msg[i]);
    display.print(incoming_messages.msg[i]);
  }
  display.display();
}




void setup()
{
  // Runs once
  Serial.begin(115200);
  keypad.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if(esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Can't add peer");
    return;
  }
  
  //Listen for when data is recieved
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  esp_now_register_send_cb(OnDataSent);

  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("Please Enter Key:");
  display.setCursor(0, 8);
  display.display();

  pinMode(RED, INPUT);
  pinMode(GREEN, INPUT);
  pinMode(BLUE, INPUT);
  pinMode(YELLOW, INPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(YELLOW_LED, HIGH);

  msg.len = 0;

}

void loop()
{
  cursorPulse();

  // Looped code
  keypad.tick();

  //Serial.println("hello");
  if (keypad.available())
    readInput(keypad.read());

  // if blue button
  int blueVal = digitalRead(BLUE);
  if (blueVal == HIGH && blueLast == LOW)
  {
    Serial.println("B");
    if (currentMode == CHAR) {
      currentMode = DIGIT;
      digitalWrite(BLUE_LED, LOW);
    }
    else {
      currentMode = CHAR;
      digitalWrite(BLUE_LED, HIGH);
    }    
  }
  blueLast = blueVal;

  int yellowVal = digitalRead(YELLOW);
  if (yellowVal == HIGH && yellowLast == LOW)
  {
    Serial.println("Y");
    if (currentMode == RECV) {
      currentMode = CHAR;
      digitalWrite(YELLOW_LED, HIGH);
      if (!keyGiven) {
        display.println("Please Enter Key:");
        display.setCursor(0, 8);
        display.display();
      }
    }
    else {
      currentMode = RECV;
      msg.len = 0;
      digitalWrite(YELLOW_LED, LOW);
    }
    display.clearDisplay();
  }
  yellowLast = yellowVal;

  int redVal = digitalRead(RED);
  if (redVal == HIGH && redLast == LOW)
  {

    Serial.println("red pressed");

    Serial.println("R");

    deleteFromDisplay();
  }
  redLast = redVal;

  int greenVal = digitalRead(GREEN);
  if (greenVal == HIGH && greenLast == LOW)
  {
    // Send message
    //Serial.println("green pressed");
    msg.chars[msg.len] = '\0';
    //char keyword[] = "test";
    
    //sendMessage(msg, keyword);

    Serial.println("G");
    if (keyGiven)
    {
      // If msg, send it
      if (msg.len > 0)
        sendMessage(msg, keyword);
    }
    else
    {
      strcpy(keyword, msg.chars);
      //keyword = msg.chars;
      //memset(msg.chars, 0, sizeof(msg.chars));
      keyGiven = true;
    }
    
    display.clearDisplay();
    display.setCursor(0, 0);
    msg.len = 0;
    delay(100);
  }
  greenLast = greenVal;

}

void selectMode()
{
  switch (currentMode)
  {
    case CHAR:
      //timeElapsed = 0;
      lastTime = millis();
      break;
    case RECV:
      // Clear board
      // Prepare for recv
      break;
  }
}

void readKeyword()
{

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

  drawCursorBlock();
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

  toPush = 48 + key % 12;

  drawCursorBlock();
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
  if (toPush == '\0' || msg.len == MAXCHARS)
    return; 

  if (!keyGiven) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Please Enter Key:");

    msg.chars[msg.len++] = toPush;
    toPush = '\0';

    display.setCursor(0, 8);
    for (int i = 0; i < msg.len; i++) {
      display.print(msg.chars[i]);
    }
    display.display();

  } else {
    msg.chars[msg.len++] = toPush;
    toPush = '\0';
    for (int i = 0; i < msg.len; i++) {
      Serial.print(msg.chars[i]);
    }
    Serial.println();

    // display.fillRect(display.getCursorX(), display.getCursorY(), 10, 10, SSD1306_INVERSE);

    // //display.clearDisplay();
    // display.print(msg.chars[msg.len-1]);
    // display.display();

    printMessage();
  }

  
}

// Delete the last char from the queue and screen
void deleteFromDisplay() 
{
  if (msg.len <= 0)
    return;
  
  msg.len--;

  // adjustCursorX(true, 10);
  // display.print(' ');
  // display.display();
  // adjustCursorX(true, 10);
  printMessage();
}


void printMessage() 
{
  display.clearDisplay();
  display.setCursor(0, 0);
  for (int i = 0; i < msg.len; i++) {
    display.print(msg.chars[i]);
  }
  display.display();
}

// true for decrement, false for increment
void adjustCursorX(bool mode, int offset)
{
  int temp = display.getCursorX();

  if (mode)
    temp -= offset;
  else
    temp += offset;

  display.setCursor(temp, display.getCursorY());
}

void cursorPulse()
{
  //lastTime = millis() - lastTime;
  //timeElapsed += lastTime;

  if (millis() > 500 + lastTime && currentMode != RECV)
  {
    drawCursorBlock();
    cursor = !cursor;
    lastTime = millis();
  }
    
}

void drawCursorBlock()
{

  if (cursor) {
    display.fillRect(display.getCursorX(), display.getCursorY(), CHARWIDTH+1, 8, SSD1306_INVERSE);
  } else {
    display.fillRect(display.getCursorX(), display.getCursorY(), CHARWIDTH+1, 8, SSD1306_WHITE);
  }
  peekToPush();
  if (toPush == '\0') 
    display.display();
}

void peekToPush() 
{
  if (toPush == '\0') return;

  if (cursor) {
    display.setTextColor(SSD1306_WHITE);
    display.print(toPush);
    adjustCursorX(true, CHARWIDTH);
  } else {
    display.setTextColor(SSD1306_INVERSE);
    display.print(toPush);
    display.setTextColor(SSD1306_WHITE);
    adjustCursorX(true, CHARWIDTH);
  }
  display.display();
}



void sendMessage(Msg msg, char keyword[]) {
  Serial.print("encrypting word: ");
  Serial.println(msg.chars);

  Serial.print("keyword: ");
  Serial.println(keyword);

  char *token = strtok(msg.chars, " ");
  char *encryptedText = (char*) malloc(MAXCHARS * sizeof(char));
  int flag = 0;
  //works
  while(token != NULL){
    
    if(flag == 0){
      //Serial.print("Current Word: ");
      char* currentWord = (char*)malloc(MAXCHARS);
      strcpy(currentWord, token);
      Serial.println(currentWord);

      char* encryptedWord = encryptByPlayfair(currentWord, keyword);
      //Serial.print("Encrypted Word: ");
      Serial.println(encryptedWord);
      strcat(encryptedText, encryptedWord);
      token = strtok(NULL, " ");
      free(currentWord);
      flag = 1; 
    }else{
      //Serial.print("Current Word: ");
      strcat(encryptedText, " ");
      
      char* currentWord = (char*)malloc(MAXCHARS);
      strcpy(currentWord, token);
      Serial.println(currentWord);

      char* encryptedWord = encryptByPlayfair(currentWord, keyword);
      strcat(encryptedText, encryptedWord);

      Serial.print("Encrypted Word: ");
      Serial.println(encryptedWord);

      Serial.print("Strlen");
      Serial.println(strlen(currentWord));

      token = strtok(NULL, " ");
      free(encryptedText);
      
    }
    
    //Serial.println(encryptedText);
    

  }
  encryptedText[strlen(encryptedText)] = '\0';
  
  Serial.print("Encrypted message: ");
  Serial.println(encryptedText);
  
  char* encryptToken = strtok(encryptedText, " ");
  char decryptedText[MAXCHARS] = "";
  flag = 0;
  
  while(encryptToken != NULL){
    
    if(flag == 0){
      Serial.print("Current Word: ");
      char* currentWord = (char*)malloc(MAXCHARS);
      strcpy(currentWord, encryptToken);
      Serial.println(currentWord);

      char* decryptedWord = decryptPlayfair(currentWord, keyword);
      Serial.print("Decrypted Word: ");
      Serial.println(decryptedWord);

      strcat(decryptedText, decryptedWord);
      encryptToken = strtok(NULL, " ");
      flag = 1; 
      
    }else{
      Serial.print("Current Word: ");
      strcat(decryptedText, " ");
      char* currentWord = (char*)malloc(MAXCHARS);
      strcpy(currentWord, encryptToken);

      Serial.println(currentWord);

      char* decryptedWord = decryptPlayfair(currentWord, keyword);
      strcat(decryptedText, decryptedWord);
      encryptToken = strtok(NULL, " ");
    
    }

  
    

  }
  
  Serial.print("Decrypted Text: ");
  decryptedText[strlen(decryptedText)] = '\0';
  Serial.println(decryptedText);
  
  struct_message to_send;
  strcpy(to_send.msg, encryptedText);

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*) &to_send, strlen(encryptedText));
  
}  
/*
void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);

  if(ret == ESP_OK){
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  }else{
    Serial.println("Failed to read MAC");
  }

}
*/










