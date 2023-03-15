#include "mandelbrot.h"

void compute(TaskData *task, ResultData *result) {
    result->C_x = task->C_x;
    result->C_y = task->C_y;
    result->iMax = task->iMax;
    result->_ER2 = task->_ER2;
    result->res = GiveEscapeTime(task->C_x, task->C_y, task->iMax, task->_ER2);
}


int GiveEscapeTime(double C_x, double C_y, int iMax, double _ER2)
{
    int i;
    double Zx, Zy;
    double Zx2, Zy2; /* Zx2=Zx*Zx; Zy2=Zy*Zy */
    Zx=0.0; /* initial value of orbit = critical point
    Z= 0 */
    Zy=0.0;
    Zx2=Zx*Zx;
    Zy2=Zy*Zy;
    for (i=0;i<iMax && ((Zx2+Zy2)<_ER2);i++) { 
        Zy = 2*Zx*Zy + C_y;
        Zx = Zx2-Zy2 +C_x;
        Zx2 = Zx*Zx;
        Zy2 = Zy*Zy;
    };
    return i;
}