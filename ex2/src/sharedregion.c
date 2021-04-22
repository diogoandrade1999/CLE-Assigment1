#include <stdlib.h>
#include <pthread.h>
#include "sharedregion.h"
#include "partfileinfo.h"

SHAREDREGION sharedRegion;

void storeFileNames(int nFileNames, char *fileNames[])
{
    FILE *fp;

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
    sharedRegion.nReaded = 0;
    sharedRegion.fileInfos = (PARTFILEINFO *)malloc(nFileNames * sizeof(PARTFILEINFO));
    for (int i = 0; i < nFileNames; i++)
    {
        PARTFILEINFO partialInfo;
        partialInfo.fileId = i;
        partialInfo.valCurrent = 0;

        // open file
        fp = fopen(sharedRegion.fileNames[i], "rb");
        if (fp == NULL)
        {
            printf("ERROR: Failed to open the file!\n");
            exit(1);
        }

        fread(&partialInfo.n, sizeof(int), 1, fp);

        partialInfo.x = (double *)malloc(partialInfo.n * sizeof(double));
        partialInfo.y = (double *)malloc(partialInfo.n * sizeof(double));
        partialInfo.valPrevious = (double *)malloc(partialInfo.n * sizeof(double));
        partialInfo.valCurrent = (double *)malloc(partialInfo.n * sizeof(double));

        fread(partialInfo.x, sizeof(double), partialInfo.n, fp);
        fread(partialInfo.y, sizeof(double), partialInfo.n, fp);
        fread(partialInfo.valPrevious, sizeof(double), partialInfo.n, fp);

        // close file
        fclose(fp);

        sharedRegion.fileInfos[i] = partialInfo;
    }
}

int processConvPoint(int threadId, int *fileId, int *n, double *x, double *y, int *point)
{
    // access critical area
    pthread_mutex_lock(&sharedRegion.lock);

    // check if all work is done
    if (sharedRegion.fileIdProcessed == sharedRegion.nFileNames)
    {
        // release critical area
        pthread_mutex_unlock(&sharedRegion.lock);
        return 1;
    }

    // check is the first time on this file
    if (sharedRegion.nReaded == 0)
    {
        *fileId = sharedRegion.fileIdProcessed;
        *n = sharedRegion.fileInfos[sharedRegion.fileIdProcessed].n;
        if (*fileId == 0)
        {
            x = (double *)malloc(*n * sizeof(double));
            y = (double *)malloc(*n * sizeof(double));
        }
        else
        {
            x = (double *)realloc(x, *n * sizeof(double));
            y = (double *)realloc(y, *n * sizeof(double));
        }

        for (int i = 0; i < *n; i++)
        {
            x[i] = sharedRegion.fileInfos[sharedRegion.fileIdProcessed].x[i];
            y[i] = sharedRegion.fileInfos[sharedRegion.fileIdProcessed].y[i];
            //printf("%f %f\n", x[i], y[i]);
        }
    }
    //printf("%f %f\n", x[1023], y[1023]);

    *point = sharedRegion.nReaded;

    sharedRegion.nReaded++;
    if (*n < sharedRegion.nReaded)
    {
        sharedRegion.fileIdProcessed++;
        sharedRegion.nReaded = 0;
    }

    // release critical area
    pthread_mutex_unlock(&sharedRegion.lock);
    return 0;
}

void savePartialResult(int threadId, int fileId, int point, double val)
{
    // access critical area
    pthread_mutex_lock(&sharedRegion.lock);

    // save result
    sharedRegion.fileInfos[fileId].valCurrent[point] = val;

    // release critical area
    pthread_mutex_unlock(&sharedRegion.lock);
}

void printProcessingResults()
{
    int foundExpectedRes;

    // access critical area
    pthread_mutex_lock(&sharedRegion.lock);

    // print processing results
    for (int i = 0; i < sharedRegion.nFileNames; i++)
    {
        printf("File name: %s\n", sharedRegion.fileNames[i]);

        foundExpectedRes = 1;
        for (int k = 0; k < sharedRegion.fileInfos[i].n; k++)
            if (sharedRegion.fileInfos[i].valCurrent[k] != sharedRegion.fileInfos[i].valPrevious[k])
            {
                printf("\tResult: Expected result not found! [%f - %f]\n", sharedRegion.fileInfos[i].valCurrent[k], sharedRegion.fileInfos[i].valPrevious[k]);
                foundExpectedRes = 0;
                break;
            }
        if (foundExpectedRes == 1)
            printf("\tResult: Expected result founded!\n");
    }

    // release critical area
    pthread_mutex_unlock(&sharedRegion.lock);

    // clean space
    free(sharedRegion.fileInfos);

    // destroy lock
    pthread_mutex_destroy(&sharedRegion.lock);
}