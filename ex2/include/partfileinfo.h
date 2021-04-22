#ifndef PARTFILEINFO_H
#define PARTFILEINFO_H

typedef struct
{
    int fileId;
    int n;
    double *x;
    double *y;
    double *valPrevious;
    double *valCurrent;
} PARTFILEINFO;

#endif