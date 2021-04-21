#ifndef PARTFILEINFO_H
#define PARTFILEINFO_H

typedef struct
{
    int fileId;
    int textSize;
    int biggestWord;
    int countWords;
    int countWordsSize[30];
    int countConsonants[30][30];
} PARTFILEINFO;

#endif