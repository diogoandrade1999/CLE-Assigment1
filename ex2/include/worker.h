#ifndef WORKER_H
#define WORKER_H

void *workerJob(void *arg);
double computeValue(int n, double *x, double *y, int point);

#endif