#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "worker.h"
#include "partfileinfo.h"
#include "sharedregion.h"
#include "convertchar.h"

void *workerJob(void *arg)
{
    int threadId;
    unsigned char buff[1024];
    PARTFILEINFO partialInfo;

    threadId = *(int *)arg;

    while (getDataChunk(threadId, buff, &partialInfo) != 1)
    {
        processDataChunk(buff, &partialInfo);
        savePartialResults(threadId, &partialInfo);
        memset(buff, 0, 1024);
    }

    return NULL;
}

void processDataChunk(unsigned char *buf, PARTFILEINFO *partialInfo)
{
    int posStartWord, wordSize, countConsonantsWord;
    unsigned char buff, lastBuff;

    // init counters
    partialInfo->countWords = 0;
    partialInfo->biggestWord = 0;
    for (int i = 0; i < 30; i++)
    {
        partialInfo->countWordsSize[i] = 0;
        for (int z = 0; z < 30; z++)
            partialInfo->countConsonants[i][z] = 0;
    }

    // find words
    posStartWord = -1;
    wordSize = 0;
    countConsonantsWord = 0;
    lastBuff = ' ';
    for (int i = 0, z = 0; i < partialInfo->textSize; i++, z++)
    {
        // remove special char
        buff = convertChar(buf, &z);

        // in word
        if (!isspace(buff) && ((buff == '_' && posStartWord != -1) || !ispunct(buff)))
        {
            //printf("%c", buff);
            // init of word
            if (isspace(lastBuff) || ispunct(lastBuff))
                posStartWord = i;

            // check if is a consonant
            if (isConsonant(buff))
                countConsonantsWord++;

            // increase size of word
            wordSize++;

            // end of word
            if (z >= partialInfo->textSize - 1)
            {
                //printf(" %d %d\n", wordSize, countConsonantsWord);
                // check if is the biggest word
                if (partialInfo->biggestWord < wordSize)
                    partialInfo->biggestWord = wordSize;

                partialInfo->countWords++;
                partialInfo->countWordsSize[wordSize]++;
                partialInfo->countConsonants[wordSize][countConsonantsWord]++;
            }
        }
        else
        {
            // end of word
            if (!isspace(lastBuff) && !ispunct(lastBuff))
            {
                //printf(" %d %d\n", wordSize, countConsonantsWord);
                // check if is the biggest word
                if (partialInfo->biggestWord < wordSize)
                    partialInfo->biggestWord = wordSize;

                partialInfo->countWords++;
                partialInfo->countWordsSize[wordSize]++;
                partialInfo->countConsonants[wordSize][countConsonantsWord]++;
                wordSize = 0;
                countConsonantsWord = 0;
                posStartWord = -1;
            }
        }
        lastBuff = buff;
        i = z;
    }
}