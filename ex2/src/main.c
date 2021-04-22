#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "sharedregion.h"
#include "worker.h"

int main(int argc, char *argv[])
{
    pthread_t *threads;
    time_t t0, t1;
    int t, nThreads, nFileNames;

    // processCommandLine
    if (argc < 3)
    {
        printf("Execution:\n main <number_of_threads> <file_names_paths>");
        return 1;
    }

    nThreads = atoi(argv[1]);
    threads = malloc(nThreads * sizeof(pthread_t));

    nFileNames = argc - 2;
    char *fileNames[nFileNames];
    for (int i = 0; i < nFileNames; i++)
    {
        fileNames[i] = argv[i + 2];
    }

    //defineTimeOrigin
    time(&t0);

    storeFileNames(nFileNames, fileNames);

    //createWorkerThread
    for (t = 0; t < nThreads; t++)
    {
        pthread_create(&threads[t], NULL, workerJob, (void *)&threads[t]);
    }

    //waitForWorkerThreadToTerminate
    for (t = 0; t < nThreads; t++)
    {
        pthread_join(threads[t], NULL);
    }

    printProcessingResults();

    //getTime
    time(&t1);

    //printProcessingTime(
    printf("Processing Time: %ld\n", t1 - t0);

    free(threads);

    return 0;
}