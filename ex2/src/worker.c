#include <stdlib.h>
#include <stdio.h>
#include "worker.h"

void *workerJob(void *arg)
{
    int threadId, fileId, n, point;
    double *x, *y, val;

    threadId = *(int *)arg;

    while (processConvPoint(threadId, &fileId, &n, x, y, &point) != 1)
    {
        //printf("%f %f\n", x[1023], y[1023]);
        //val = computeValue(n, x, y, point);
        savePartialResult(threadId, fileId, point, val);
    }

    // clean
    free(x);
    free(y);

    return NULL;
}

double computeValue(int n, double *x, double *y, int point)
{
    double val;
    val = 0;
    for (int k = 0; k < n; k++)
        val += (x[k] * y[(point + k) % n]);
    return val;
}