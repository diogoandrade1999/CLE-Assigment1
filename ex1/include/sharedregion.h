#ifndef SHARED_REGION_H
#define SHARED_REGION_H

#include <stdio.h>
#include <pthread.h>
#include "partfileinfo.h"

typedef struct
{
    pthread_mutex_t lock;
    int nFileNames;
    char **fileNames;
    PARTFILEINFO *fileInfos;
    int fileIdProcessed;
    FILE *fp;
    int sizeReaded;
} SHAREDREGION;

void storeFileNames(int nFileNames, char *fileNames[]);
void printProcessingResults();
int getDataChunk(int threadId, unsigned char *buf, PARTFILEINFO *partialInfo);
void savePartialResults(int threadId, PARTFILEINFO *partialInfo);

#endif