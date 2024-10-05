#include "encryption.h"
#include <bits/stdc++.h>

#define SIZE 168

void toLowerCase(char plain[], int ps)
{
  int i; 
  for(i = 0; i < ps; i++){

    if(plain[i] > 64 && plain[i] < 91)
    {
      plain[i] += 32;
    }

  }
}

int removeSpaces(char* plain, int ps)
{
  int i, count = 0;
  for(i = 0; i < ps; i++){

    
    if(plain[i] != ' ')
    {
      plain[count++] = plain[i];
    }
    plain[count] = '\0';
    return count;

    

  }
}
// Generating a 5x5 square for Playfair cipher, ks is key length
void generateKeyTable(char key[], int ks, char keyT[5][5])
{
  int i, j, k, flag = 0;


  int dicty[26] = {0};
  for(i = 0; i < ks; i++)
  {

    if(key[i] != 'j') //the letter "j" is what we're ommitting 
    {
      //put 2 in
      dicty[key[i] - 97] = 2;
    }

  }
  dicty['j' - 97] = 1;
  
  i = 0;
  j = 0;
  //fill table with keyword
  for(k = 0; k < ks; k++)
  {

    if(dicty[key[k] - 97] == 2)
    {

      dicty[key[k] - 97] -= 1; //reduce to 2nd letter of letter pair
      keyT[i][j] = key[k];
      j++;

      //reach end of row
      if(j == 5)
      {
        i++;
        j = 0;
      }


    }
  
  }
  //fills the rest of the table with the rest of alphabet
  for(k = 0; k < 26; k++)
  {
    if(dicty[k] == 0)
    {
      keyT[i][j] = (char)(k + 97);
      j++;

      if(j == 5){
        i++;
        j = 0;
      }
    }
  }


}

void search(char keyT[5][5], char a, char b, int arr[])
{
  int i, j;

  //replacing the letter "i" with our ommitted letter
  if(a == 'j'){
    a = 'i';
  }
  else if(b == 'j'){
    b = 'i';
  }

  for(i = 0; i < 5; i++)
  {
    for(j = 0; j < 5; j++)
    {

      if(keyT[i][j] == a)
      {
        arr[0] = i;
        arr[1] = j;
      }
      else if(keyT[i][j] == b)
      {
        arr[2] = i;
        arr[3] = j;
      }

    }

  }

}

int mod5(int a) {return (a % 5);}

int prepare(char str[], int ptrs)
{

  if(ptrs % 2 != 0){
    str[ptrs++] = 'z'; //bogus letter, thinking about making it the first letter of the keyword
    str[ptrs] = '\0';
  }

  return ptrs;
}

char* encrypt(char str[], char keyT[5][5], int ps)
{
  int i, a[4];
  
  for(i = 0; i < ps; i += 2){
    search(keyT, str[i], str[i + 1], a);

    if(a[0] == a[2])
    {
      str[i] = keyT[a[0]][mod5(a[1] + 1)];
      str[i + 1] = keyT[a[0]][mod5(a[3] + 1)];
    }
    else if(a[1] == a[3])
    {
      str[i] = keyT[mod5(a[0] + 1)][a[1]];
      str[i + 1] = keyT[mod5(a[2] + 1)][a[1]];
    }
    else
    {
      str[i] = keyT[a[0]][a[3]];
      str[i + 1] = keyT[a[2]][a[1]];
    }

  }
  return str;
}

char* encryptByPlayfair(char str[], char key[])
{
    char ps, ks, keyT[5][5];
    ks = strlen(key);
    //ks = removeSpaces(key,ks);
    toLowerCase(key,ks);


    //Serial.println(str);
    Serial.println(key);



    ps = strlen(str);
    toLowerCase(str, ps);
    //ps = removeSpaces(str, ps);
    
    Serial.println(str);
    ps = prepare(str, ps);

    generateKeyTable(key, ks ,keyT);

    encrypt(str, keyT, ps);

  return str;

}
/*

  Decryption of Playfair

*/
void keyTableDecrypt(char key[], int ks, char keyT[5][5]){

  int i, j, k, flag = 0, *dicty;

  dicty = (int*)calloc(26, sizeof(int));

  for(i = 0; i < ks; i++){
    if(key[i] != 'j'){
      dicty[key[i] - 97] = 2;
    }
  }
  dicty['j' - 97] = 1;

  i = 0;
  j = 0;
  for(k = 0; k < ks; k++){
    if (dicty[key[k] - 97] == 2) {
        dicty[key[k] - 97] -= 1;
        keyT[i][j] = key[k];
        j++;
        if (j == 5) {
          i++;
          j = 0;
       }
    }
  }

  for (k = 0; k < 26; k++) {
    if (dicty[k] == 0) {
      keyT[i][j] = (char)(k + 97);
      j++;
      if (j == 5) {
        i++;
        j = 0;
      }
    }
  }
  /*
  for(int t = 0; t < 5; t++){
    for(int m = 0; m < 5; m++){
      Serial.print(keyT[t][m]);
    }
    Serial.println();
  }
  */
}

int overloadmod5(int a){
  if(a < 0){
    a += 5;
  }
  return (a % 5); 
}

char* decrypt(char str[], char keyT[5][5], int ps){
  int i, a[4];
  for(i = 0; i < ps; i += 2){
    search(keyT, str[i], str[i + 1], a);
    Serial.println(str);
    if(a[0] == a[2])
    {
      str[i] = keyT[a[0]][overloadmod5(a[1] - 1)];
      str[i + 1] = keyT[a[0]][overloadmod5(a[3] - 1)];
    }
    else if(a[1] == a[3])
    {
      str[i] = keyT[overloadmod5(a[0] - 1)][a[1]];
      str[i + 1] = keyT[overloadmod5(a[2] - 1)][a[1]];
    }
    else
    {
      str[i] = keyT[a[0]][a[3]];
      str[i + 1] = keyT[a[2]][a[1]];
    }

  }
  return str;
}

char* decryptPlayfair(char str[], char key[]){
  char ps, ks, keyT[5][5];
  //Serial.println(str);

  ks = strlen(key);
  toLowerCase(key, ks);

  ps = strlen(str);
  toLowerCase(str, ps);
  
  //Serial.println(str);

  keyTableDecrypt(key, ks, keyT);

  decrypt(str, keyT, ps);
  Serial.println(str);
  return str;

}
