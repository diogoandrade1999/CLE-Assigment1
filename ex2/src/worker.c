#include "worker.h"
#include "sharedregion.h"

void *workerJob(void *arg)
{
    int threadId, fileId, n, point;
    double *x, *y, val;

    threadId = *(int *)arg;

    while (processConvPoint(threadId, &fileId, &n, &x, &y, &point) != 1)
    {
        val = computeValue(n, x, y, point);
        savePartialResult(threadId, fileId, point, val);
    }
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