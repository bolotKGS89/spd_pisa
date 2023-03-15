#ifndef MANDELBROT_H
#define MANDELBROT_H

typedef struct {
    double C_x;
    double C_y;
    int iMax;
    double _ER2;
} TaskData;

typedef struct {
    double C_x;
    double C_y;
    int iMax;
    double _ER2;
    int res;
} ResultData;

void compute(TaskData *task, ResultData *result);

TaskData assignRandomData();

int GiveEscapeTime(double C_x, double C_y, int iMax, double _ER2);

#endif