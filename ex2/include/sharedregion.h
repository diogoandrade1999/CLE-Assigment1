#ifndef SHARED_REGION_H
#define SHARED_REGION_H

#include <pthread.h>
#include "partfileinfo.h"

typedef struct
{
    pthread_mutex_t lock;
    int nFileNames;
    char **fileNames;
    PARTFILEINFO *fileInfos;
    int fileIdProcessed;
    int nReaded;
} SHAREDREGION;

void storeFileNames(int nFileNames, char *fileNames[]);
void printProcessingResults();
int processConvPoint(int threadId, int *fileId, int *n, double **x, double **y, int *point);
void savePartialResult(int threadId, int fileId, int point, double val);

#endif