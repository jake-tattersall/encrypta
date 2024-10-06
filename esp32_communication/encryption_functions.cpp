#include "encryption.h"
#include <bits/stdc++.h>

#define SIZE 168

char newStr[256];

char decStr[256];

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

// int removeSpaces(char* plain, int ps)
// {
//   int i, count = 0;
//   for(i = 0; i < ps; i++)
//   {
//     if(plain[i] != ' ')
//     {
//       plain[count++] = plain[i];
//     }
//     plain[count] = '\0';
//     return count;
//   }

//   return 0;
// }

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

void prepare(char str[], int ptrs)
{
  bool flag;

  int numPlaced = 0;

  char A, B;



  for (int i = 0; i < strlen(str); i = i + 2)
  {
    // printf("%d\n", i);
    flag = false;

    A = str[i];

    if (i + 1 >= strlen(str))
      B = 'x';
    else
      B = str[i + 1];

    if (A == B)
    {
        B = 'x';
        flag = true;
    }

    if (A == 'i')
        A = 'j';

    if (B == 'i')
        B = 'j';

    if (A == 'x' && B == 'x')
    {
        B = 'q';
        flag = true;
    }

    newStr[numPlaced++] = A;
    newStr[numPlaced++] = B;

    if (flag)
        i--;
  }

  
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
  char keyT[5][5];
  int ks, ps;

  ks = strlen(key);
  toLowerCase(key, ks);

  ps = strlen(str);
  toLowerCase(str, ps);

  // Serial.println(str);
  prepare(str, ps);


  generateKeyTable(key, ks, keyT);

  encrypt(newStr, keyT, strlen(newStr));


  return newStr;
}

/*
  Decryption of Playfair
*/

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
    // Serial.println(str);
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

char* decryptPlayfair(char str[], char key[])
{
  char ps, ks, keyT[5][5];

  ks = strlen(key);
  toLowerCase(key, ks);

  ps = strlen(str);
  toLowerCase(str, ps);

  generateKeyTable(key, strlen(key), keyT);

  
  

  decrypt(str, keyT, strlen(str));


  int numPlaced = 0;

  for (int i = 0; i < strlen(str); i++)
  {
    if (str[i] == 'x')
    {
      if (i + 1 == strlen(str))
        continue;

      if (str[i-1] == str[i+1])
      {
        decStr[numPlaced++] = str[i-1];
        i++;
        continue;
      }
    }

    decStr[numPlaced++] = str[i];
  }
  decStr[numPlaced++] = '\0';



  return decStr;
}