/*** Methods filled in by Julia Connelly and Kerim Celik 01/20/2016 ***/

#include <stdio.h>

/* Copies the dim-dimensional vector v to the dim-dimensional vector copy. */
void vecCopy(int dim, double v[], double copy[]) {
	int i;
    for(i = 0; i < dim; ++i) {
        copy[i] = v[i];
    }
}

/* Adds the dim-dimensional vectors v and w. */
void vecAdd(int dim, double v[], double w[], double vPlusW[]) {
	int i;
    for(i = 0; i < dim; ++i) {
        vPlusW[i] = v[i] + w[i];
    }
}

/* Subtracts the dim-dimensional vectors v and w. */
void vecSubtract(int dim, double v[], double w[], double vMinusW[]) {
	int i;
    for(i = 0; i < dim; ++i) {
        vMinusW[i] = v[i] - w[i];
    }
}

/* Scales the dim-dimensional vector w by the number c. */
void vecScale(int dim, double c, double w[], double cTimesW[]) {
	int i;
    for(i = 0; i < dim; ++i) {
        cTimesW[i] = w[i] * c;
    }
}

/* Assumes that there are dim + 2 arguments, the last dim of which are doubles. 
Sets the dim-dimensional vector v to those doubles. */
void vecSet(int dim, double v[], ...) {
    va_list argumentPointer;
    va_start(argumentPointer, v);
    for (int i = 0; i < dim; i++) {
        v[i] = va_arg(argumentPointer, double);
    }
    va_end(argumentPointer);
}