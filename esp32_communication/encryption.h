#include <Arduino.h>

void toLowerCase(char plain[], int ps);

int removeSpaces(char* plain, int ps);

void generateKeyTable(char key[], int ks, char keyT[5][5]);

void search(char keyT[5][5], char a, char b, int arr[]);

int mod5(int a);

int prepare(char str[], int ptrs);

char* encrypt(char str[], char keyT[5][5], int ps);

char* encryptByPlayfair(char str[], char key[]);