#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "sharedregion.h"
#include "worker.h"

int main(int argc, char *argv[])
{
    pthread_t *threads;
    clock_t t0, t1;
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
    t0 = clock();

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
    t1 = clock();

    //printProcessingTime
    printf("Processing Time: %f\n", (double)(t1 - t0) / CLOCKS_PER_SEC);

    // clean space
    free(threads);

    return 0;
}