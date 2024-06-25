// Christian Catalano, 1120832, Assignment 4, March 20th 2022
#include "Q1.h"

int main(int argc, char* argv[]) {
    char* filename = "data_A4_Q1.txt";
    keyVal** probabilityList = NULL;
    int listSize = 0;
    int** rootTable = NULL;
    float** freqTable = NULL;
    char searchString[1000];

    // Error check selector for 1.1 and 1.2
    if(argc != 2) {
        printf("Usage: './q1 1' for Q1.1\n       './q1 2' for Q1.2\n");
        exit(1);
    }

    // Get the list of words from the file
    probabilityList = readDataFromFile(filename, &listSize);
    if(probabilityList == NULL) {
        fprintf(stderr, "Could not open filename: %s\n\n", filename);
        return 1;
    }

    // Prompt user for search string
    printf("Please enter a search string: \n");
    scanf("%s", searchString);

    if(argv[1][0] == '1') { // Dynamic Programming
        printf("\nOptimal BST using dynamic programming.\n");
        freqTable = optimalBST(probabilityList, listSize, &rootTable); // Generate main table and root table
        searchBST(searchString, probabilityList, listSize, rootTable, freqTable); // Use main and root tables to search through tree
    }
    else if(argv[1][0] == '2') { // Greedy
        printf("\nOptimal BST using greedy technique.\n");
        greedySearchBST(searchString, probabilityList, listSize);
    }
    else { // Incorrect flag, must be 1 or 2
        printf("Usage: './q1 1' for Q1.1\n       './q1 2' for Q1.2\n");
    }


    freeList(probabilityList, listSize);
    freeTable((void**)freqTable, listSize + 1);
    freeTable((void**)rootTable, listSize + 1);
    return 0;
}

// Takes in an input file and translates it into a list of keyval pairs.
// The keyval pair list represents each unique word and its associated number of occurences in the input file
keyVal** readDataFromFile(char* inputfile, int* listSize) {
    FILE *fp;
    int tempChar;
    char tempString[10000];
    char* outString = NULL;
    int fileLen;
    int len;

    fflush(stdin); // Empty out stdin just in case
    fp = fopen(inputfile, "r");

    // Error checking
    if(fp == NULL)
        return NULL;

    // Determine the amt of lines in the file
    fileLen = 0;
    int skipAmt = 0;
    bool bomFlag = true;
    for (tempChar = getc(fp); tempChar != EOF; tempChar = getc(fp)) {
        if((tempChar < ' ' || tempChar > '~') && bomFlag) // Used for skipping BOM in file
            skipAmt++;
        else
            bomFlag = false;

        if(tempChar == '\n')
            fileLen++;
    }
    fseek(fp, skipAmt, SEEK_SET); // skip past BOM is present and read input

    // Initialize outString
    len = 1;
    outString = (char*)calloc(len, sizeof(char));

    // Iterate through each line in the input file
    for(int i = 0; i < fileLen; i++) {
        fgets(tempString, 10000, fp); // Grab current line
        len += strlen(tempString); // Adjust length of dynamic string and reallocate
        char* newString = realloc(outString, len * sizeof(char));
        if (!newString) { // Error checking realloc
            fclose(fp);
            return NULL;
        }
        else 
            outString = newString;
        strcat(outString, tempString); // Concatenate string to end of dynamic string
    }
    fclose(fp);
    
    // Create a probability list
    keyVal** pList = initializePList();
    if(pList == NULL) {
        free(outString);
        return NULL;
    }

    // Iterate through each word in the input string, adding it as a node to the probability list
    fflush(stdin);
    char* cpyPtr = outString;
    char* token;
    int wordCount = 0;
    while ((token = strtok_r(cpyPtr, " \n", &cpyPtr))) {
        if(!addToList(&pList, listSize, token)) { // check if something went wrong when adding to list
            freeList(pList, *listSize);
            free(outString);
            return NULL;
        }
        wordCount ++;
    }

    calculateP(pList,*listSize, wordCount); // Update probability based off of total word count and occurences
    qsort((void*)pList, *listSize, sizeof(keyVal*), compareKeyVal); // Alphabetically sort the list
    offsetList(&pList, *listSize); // Offset the start index from 0 to 1
    
    free(outString);
    return pList;
}

// Generates a frequency table and root table for a given list of keys and values
// Returns the frequency table, updates **rootTable as the root table. The size of the table is always given as listSize + 1, so it does not need to be a variable
// Algorithm provided in class, from textbook
float** optimalBST(keyVal** pList, int listSize, int*** rootTable){
    // Initialize tables
    float** freqTable = calloc(listSize + 2, sizeof(float*));
    *rootTable = calloc(listSize + 2, sizeof(int*));
    for(int i = 1; i <= listSize + 1; i++) {
        freqTable[i] = calloc(listSize + 1, sizeof(float));
        (*rootTable)[i] = calloc(listSize + 1, sizeof(int));

        if(i < listSize + 1) { // Not on the final row
            freqTable[i][i] = pList[i]->p;
            (*rootTable)[i][i] = i;
        }
    }

    int j;
    float minval;
    int kmin;
    float sum;
    for(int d = 1; d <= listSize - 1; d++) {
        for(int i = 1; i <= listSize - d; i++) {
            j = i + d;
            minval = __FLT_MAX__;
            for(int k = i; k <= j; k++) {
                if(freqTable[i][k - 1] + freqTable[k+1][j] < minval) {
                    minval = freqTable[i][k - 1] + freqTable[k+1][j];
                    kmin = k;
                }
            }
            (*rootTable)[i][j] = kmin;
            sum = pList[i]->p;
            for(int s = i + 1; s <= j; s++) {
                sum += pList[s]->p;
            }
            freqTable[i][j] = minval + sum;
        }
    }
    return freqTable;
}

// Searches for a given string using the list of key vals, the frequency table, and the root table.
// Prints info for each node it iterates through
// Dynamic programming BST
void searchBST(char* search, keyVal** pList, int listSize, int** rootTable, float** freqTable) {
    // Start with an interval of the entire list of words
    int i = 1;
    int j = listSize; 
    int k;

    while( i <= j ) {
        k = rootTable[i][j];
        printf("Compared with %s (%.3f), ", pList[k]->key, freqTable[i][j]);
        if(strcmp(search, pList[k]->key) < 0) { // Search term could be in left subtree, alphabetically less than this node
            printf("go left subtree.\n");
            j = k - 1;
        }
        else if (strcmp(search, pList[k]->key) > 0) { // Search term could be in right subtree, alphabetically more than this node
            printf("go right subtree.\n");
            i = k + 1;
        }
        else { // Search term is this node
            printf("found.\n");
            return;
        }
    }

    printf("Not found.\n");
}

// Searches for a given string using the list of words and their probabilities
// Prints info for each node it iterates through
// greedy BST
void greedySearchBST(char* search, keyVal** pList, int listSize) {
    // Start with an interval of the entire list of words
    int i = 1;
    int j = listSize;
    int k;

    while( i <= j ) {
        k = indexOfLargestP(pList, i, j); // Get the most probable word, left subtree is all words alphabeticall less than, right subtree greater than
        printf("Compared with %s (%.3f), ", pList[k]->key, pList[k]->p);
        if(strcmp(search, pList[k]->key) < 0) { // Search term could be in left subtree, alphabetically less than this node
            printf("go left subtree.\n");
            j = k - 1;
        }
        else if (strcmp(search, pList[k]->key) > 0) { // Search term could be in right subtree, alphabetically more than this node
            printf("go right subtree.\n");
            i = k + 1;
        }
        else { // Search term is this node
            printf("found.\n");
            return;
        }
    }

    printf("Not found.\n");
}

// Initializes a new keyval pair list
keyVal** initializePList() {
    keyVal** pList = (keyVal**)malloc(sizeof(keyVal*));
    *pList = NULL;
    return pList;
}

// Adds a new key to the list of keyval pairs. If already present in the list, then its associated node's $occurenceAmt is iterated by 1
bool addToList(keyVal*** list, int* listSize, char* key) {
    char* tempString;

    // Check if list exists
    if(*list == NULL)
        return false;

    // Check if first value in list exists
    if(**list == NULL) {
        **list = createPair(key, 1);
        *listSize = *listSize + 1;
        return true;
    }

    // See if word is already in list
    keyVal* current;
    for(int i = 0; i < *listSize; i++) {
        current = (*list)[i];
        if(current != NULL && strcmp(current->key, key) == 0) {
            current->occurenceAmt++;
            return true;
        }
    }

    // Append new node to list
    *listSize = *listSize + 1;
    *list = (keyVal**)realloc(*list, (*listSize) * sizeof(keyVal*));
    keyVal* newNode = createPair(key, 1);
    (*list)[*listSize - 1] = newNode;

    return true;
}

// Initializes a new keyVal pair with $key and $occurenceAmt. $p is set as -1 as the probability is calculated in calculateP()
// The new keyVal pair's key is copied from $key, so $key can be freed without destroying the copy inside of this pair.
keyVal* createPair(char* key, int occurenceAmt) {
    keyVal* newPair = malloc(sizeof(keyVal));
    if(newPair == NULL)
        return NULL;
    newPair->key = malloc((strlen(key) + 1) * sizeof(char));
    if(newPair->key == NULL)
        return NULL;
    strcpy(newPair->key, key);
    newPair->occurenceAmt = 1;
    newPair->p = -1;

    return newPair;
}

// Shifts all values in list one to the right, to account for index offset in algorithm
void offsetList(keyVal*** pList, int listSize) {
    *pList = (keyVal**)realloc(*pList, (listSize + 1) * sizeof(keyVal*));
    for(int i = listSize; i > 0; i--) {
        (*pList)[i] = (*pList)[i - 1];
    }
    (*pList)[0] = NULL;
}

// Calculates the probability of a key using its internal $occurenceAmt and the external $wordCount representing the word count of the input file
void calculateP(keyVal** head, int listSize, int wordCount) {
    if(head == NULL) 
        return;

    keyVal* current;
    for(int i = 0; i < listSize; i++) {
        current = head[i];
        if(current != NULL) {
            current->p = (float)(current->occurenceAmt) / (float)(wordCount);
        }
    }
}

// Compares the strings of two keyVal pairs together
// Used with qsort
int compareKeyVal (const void* a, const void* b) {
    keyVal* keyA = *( (keyVal**)(a) );
    keyVal* keyB = *( (keyVal**)(b) );

    return( strcmp(keyA->key, keyB->key) );
}

// Prints a set of keyVal pairs, for debugging
void printList(keyVal** head, int listSize) {
    if(head == NULL) 
        return;

    keyVal* current;
    for(int i = 0; i < listSize; i++) {
        current = head[i];
        if(current != NULL) {
            printf("[%s], occurences: %d, probability: %f\n", current->key, current->occurenceAmt, current->p);
        }
    }
}

// Prints a frequency table
void printFTable(float** table, int length) {
    if(table != NULL)
    {
        for(int i = 1; i <= length; i++) {
            for(int j = 0; j < length; j++) {
                printf("%.4f ", table[i][j]);
            }
            printf("\n");
        }
    }
}

// Prints a root table
void printRTable(int** table, int length) {
    if(table != NULL)
    {
        for(int i = 1; i <= length; i++) {
            for(int j = 0; j < length; j++) {
            
                printf("%d ", table[i][j]);
            }
            printf("\n");
        }
    }
}

// Finds the index of the largest probability in pList between start and end
int indexOfLargestP(keyVal** pList, int start, int end) {
    float max = -1;
    int index;

    for(int i = start; i <= end; i++) {
        if(max < pList[i]->p) {
            max = pList[i]->p;
            index = i;
        }
    }

    return index;
}

// Frees a list of keyval pairs and their associated members
void freeList(keyVal** head, int listSize) {
    if(head == NULL) 
        return;

    keyVal* current;
    for(int i = 0; i <= listSize; i++) {
        current = head[i];
        if(current != NULL)
            freeKV(current);
    }
    free(head);

}

// Frees a single keyval pair and its associated member
void freeKV(keyVal* node) {
    if(node != NULL) {
        if(node->key != NULL)
            free(node->key);
        free(node);
    }
}

// Frees a generic table (only float and int tables used in this program)
void freeTable(void** table, int length) {
    if(table != NULL)
    {
        for(int i = 0; i <= length; i++) {
            if(table[i] != NULL)
                free(table[i]);
        }
        free(table);
    }
}