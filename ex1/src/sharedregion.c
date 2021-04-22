#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "sharedregion.h"
#include "partfileinfo.h"
#include "convertchar.h"

SHAREDREGION sharedRegion;

void storeFileNames(int nFileNames, char *fileNames[])
{
    // init locker
    if (pthread_mutex_init(&sharedRegion.lock, NULL) != 0)
    {
        printf("ERROR: Failed to initiate lock!\n");
        exit(1);
    }

    // store files
    sharedRegion.nFileNames = nFileNames;
    sharedRegion.fileNames = fileNames;
    sharedRegion.fileIdProcessed = 0;
    sharedRegion.fp = NULL;
    sharedRegion.sizeReaded = 0;
    sharedRegion.fileInfos = (PARTFILEINFO *)malloc(nFileNames * sizeof(PARTFILEINFO));
    for (int i = 0; i < nFileNames; i++)
    {
        PARTFILEINFO partialInfo;
        partialInfo.fileId = i;
        partialInfo.countWords = 0;
        for (int i = 0; i < 30; i++)
        {
            partialInfo.countWordsSize[i] = 0;
            for (int z = 0; z < 30; z++)
                partialInfo.countConsonants[i][z] = 0;
        }
        partialInfo.biggestWord = 0;
        sharedRegion.fileInfos[i] = partialInfo;
    }
}

int getDataChunk(int threadId, unsigned char *buf, PARTFILEINFO *partialInfo)
{
    unsigned char buff;
    int endPosLastStr;

    // access critical area
    pthread_mutex_lock(&sharedRegion.lock);

    // check if all work is done
    if (sharedRegion.fileIdProcessed == sharedRegion.nFileNames)
    {
        // release critical area
        pthread_mutex_unlock(&sharedRegion.lock);
        return 1;
    }

    // open file if it is not open
    if (sharedRegion.fp == NULL)
    {
        sharedRegion.fp = fopen(sharedRegion.fileNames[sharedRegion.fileIdProcessed], "r");
        if (sharedRegion.fp == NULL)
        {
            // release critical area
            pthread_mutex_unlock(&sharedRegion.lock);
            printf("ERROR: Failed to open the file!\n");
            exit(1);
        }
    }

    // save partial data info file
    partialInfo->fileId = sharedRegion.fileIdProcessed;

    // read file
    endPosLastStr = 0;
    for (int i = 0; i < 1024; i++)
    {
        // reach end of file
        if (fscanf(sharedRegion.fp, "%c", &buff) == EOF)
        {
            // clean data from actual file
            fclose(sharedRegion.fp);
            sharedRegion.fileIdProcessed++;
            sharedRegion.fp = NULL;
            sharedRegion.sizeReaded = 0;
            break;
        }
        // find end position of last string
        else if (isSpace(buff) || isSeparation(buff) || isPunct(buff))
            endPosLastStr = i;

        *(buf + i) = buff;
    }

    // check if reach the end
    if (sharedRegion.fp != NULL)
    {
        // return to last string
        if (endPosLastStr != 1024)
            fseek(sharedRegion.fp, sharedRegion.sizeReaded + endPosLastStr, SEEK_SET);

        // save size of data readed
        sharedRegion.sizeReaded += endPosLastStr;
    }

    // save partial data info file
    partialInfo->textSize = endPosLastStr;

    // release critical area
    pthread_mutex_unlock(&sharedRegion.lock);
    return 0;
}

void savePartialResults(int threadId, PARTFILEINFO *partialInfo)
{
    // access critical area
    pthread_mutex_lock(&sharedRegion.lock);

    PARTFILEINFO *storedInfo = &sharedRegion.fileInfos[partialInfo->fileId];

    // store biggest word
    if (storedInfo->biggestWord < partialInfo->biggestWord)
        storedInfo->biggestWord = partialInfo->biggestWord;

    // store count of words
    storedInfo->countWords += partialInfo->countWords;

    for (int i = 0; i < 30; i++)
    {
        storedInfo->countWordsSize[i] += partialInfo->countWordsSize[i];

        for (int k = 0; k < 30; k++)
            storedInfo->countConsonants[i][k] += partialInfo->countConsonants[i][k];
    }

    // release critical area
    pthread_mutex_unlock(&sharedRegion.lock);
}

void printProcessingResults()
{
    // access critical area
    pthread_mutex_lock(&sharedRegion.lock);

    // print processing results
    for (int i = 0; i < sharedRegion.nFileNames; i++)
    {
        printf("File name: %s\n", sharedRegion.fileNames[i]);
        printf("\tTotal number of words: %d \n", sharedRegion.fileInfos[i].countWords);
        printf("\tSize of the biggest word: %d \n", sharedRegion.fileInfos[i].biggestWord);

        printf("\n\tNumber of words per size:\n");
        for (int j = 0; j < 3; j++)
        {
            if (j == 0)
                printf("\t\tsize:");
            else if (j == 1)
                printf("\n\t\tcount:");
            else
                printf("\n\t\t%c:", '%');
            for (int k = 1; k <= sharedRegion.fileInfos[i].biggestWord; k++)
                if (j == 0)
                    printf("\t%d ", k);
                else if (j == 1)
                    printf("\t%d ", sharedRegion.fileInfos[i].countWordsSize[k]);
                else
                    printf("\t%3.2f ", ((double)sharedRegion.fileInfos[i].countWordsSize[k] / (double)sharedRegion.fileInfos[i].countWords) * 100);
        }
        printf("\n\n");

        printf("\n\tFrequency of consonants per words per size:\n\t\tsize:");
        for (int j = 1; j <= sharedRegion.fileInfos[i].biggestWord; j++)
            printf("\t%d ", j);
        for (int j = 0; j <= sharedRegion.fileInfos[i].biggestWord; j++)
        {
            printf("\n\t");
            for (int k = 0; k <= sharedRegion.fileInfos[i].biggestWord; k++)
                if (k == 0)
                    printf("\t%d ", j);
                else if (k >= j)
                    if (sharedRegion.fileInfos[i].countConsonants[k][j] > 0 && sharedRegion.fileInfos[i].countWordsSize[k] > 0)
                        printf("\t%1.2f ", ((double)sharedRegion.fileInfos[i].countConsonants[k][j] / (double)sharedRegion.fileInfos[i].countWordsSize[k]));
                    else
                        printf("\t%d ", 0);
                else
                    printf("\t ");
        }
        printf("\n\n");
    }

    // release critical area
    pthread_mutex_unlock(&sharedRegion.lock);

    // clean space
    free(sharedRegion.fileInfos);

    // destroy lock
    pthread_mutex_destroy(&sharedRegion.lock);
}