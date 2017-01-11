/*** In general dimensions ***/
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

//int main(void){
//    double a[2] = {1,2}; double b[2]; double c[2];
//    vecCopy(2,a,b);
//    vecSubtract(2,a,b,c);
//    vecScale(2, 2.0, a, c);
//    printf("%f\n%f\n", c[0], c[1]);
//    printf("%f\n%f\n", b[0], b[1]);
//}