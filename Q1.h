// Christian Catalano, 1120832, Assignment 4, March 20th 2022
#ifndef Q1_HEADER
#define Q1_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

struct keyValStruct {
    char* key;
    int occurenceAmt;
    float p;
} typedef keyVal;

keyVal** readDataFromFile(char* inputfile, int* listSize);
void searchBST(char* search, keyVal** pList, int listSize, int** rootTable, float** freqTable);
void greedySearchBST(char* search, keyVal** pList, int listSize);

/* KeyVal functions */
keyVal** initializePList();
keyVal* createPair(char* key, int occurenceAmt);
void freeList(keyVal** head, int listSize);
void freeKV(keyVal* node);
void printList(keyVal** head, int listSize);
bool addToList(keyVal*** head, int* listSize, char* key);
void calculateP(keyVal** head, int listSize, int wordCount);
int indexOfLargestP(keyVal** pList, int start, int end);
int compareKeyVal (const void* a, const void* b);
void offsetList(keyVal*** pList, int listSize);

/* frequency and root table functions */
float** optimalBST(keyVal** pList, int listSize, int*** rootTable);
void freeTable(void** table, int length);
void printFTable(float** table, int length);
void printRTable(int** table, int length);

#endif /*Q1_HEADER */