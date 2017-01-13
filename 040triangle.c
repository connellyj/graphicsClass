/* Julia Connelly and Kerim Celik, 01/10/2017 */
    
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Returns the y value associated with the given x value on the
    line created by the two given points */
double findYOnLine(int x, double p1x, double p1y, double p2x, double p2y) {
    if(p2x - p1x != 0 ) return p1y + (p2y - p1y) / (p2x - p1x) * (x - p1x);
    else return p1y;
}

/* Fills the chi input for the given x[] point */
void getChi(double a[2], double x[2],  double chi[2], 
            double alpha[2], double beta[2], double gamma[2], double m[2][2]){
    double xMinusA[2];
    vecSubtract(2, x, a, xMinusA);
    double pq[2];
    mat221Multiply(m, xMinusA, pq);
    double betMinusAlp[2]; double gamMinusAlp[3];
    vecSubtract(2, beta, alpha, betMinusAlp);
    vecSubtract(2, gamma, alpha, gamMinusAlp);
    double p[2]; double q[2];
    vecScale(2, pq[0], betMinusAlp, p);
    vecScale(2, pq[1], gamMinusAlp, q);
    double pPlusQ[2];
    vecAdd(2, p, q, pPlusQ);
    vecAdd(2, alpha, pPlusQ, chi);
}

/* Fills the given column with the given color */
void fillColumn(int x, double minY, double maxY, double rgb[3], double a[2], 
                double alpha[2], double beta[2], double gamma[2], 
                double m[2][2], texTexture t) {
    int y;
    double chi[2];
    double xVec[2];
    xVec[0] = x;
    for(y = ceil(minY); y <= floor(maxY); ++y) {
        xVec[1] = y;
        getChi(a, xVec, chi, alpha, beta, gamma, m);
        texSample(&t, chi[0], chi[1]);
        pixSetRGB(x, y, t.sample[0] * rgb[0], t.sample[1] * rgb[1], t.sample[2] * rgb[2]);
    }
}

/* Orders the points so the point with the lowest x value is first,
        and the rest follow in counterclockwise order.
        Yes, this is gross, will maybe fix later. */
void permutePoints(double a[2], double b[2], double c[2], double points[3][2], 
                   double alpha[3], double beta[3], double gamma[3], double colors[3][2]) {
    if(c[0] <= a[0] && c[0] <= b[0]) {
        points[0][0] = c[0];
        points[0][1] = c[1];
        points[1][0] = a[0];
        points[1][1] = a[1];
        points[2][0] = b[0];
        points[2][1] = b[1];
        colors[0][0] = gamma[0];
        colors[0][1] = gamma[1];
        colors[1][0] = alpha[0];
        colors[1][1] = alpha[1];
        colors[2][0] = beta[0];
        colors[2][1] = beta[1];
    }else if(b[0] <= a[0] && b[0] <= c[0]) {
        points[0][0] = b[0];
        points[0][1] = b[1];
        points[1][0] = c[0];
        points[1][1] = c[1];
        points[2][0] = a[0];
        points[2][1] = a[1];
        colors[0][0] = beta[0];
        colors[0][1] = beta[1];
        colors[1][0] = gamma[0];
        colors[1][1] = gamma[1];
        colors[2][0] = alpha[0];
        colors[2][1] = alpha[1];
    }else if(a[0] <= b[0] && a[0] <= c[0]) {
        points[0][0] = a[0];
        points[0][1] = a[1];
        points[1][0] = b[0];
        points[1][1] = b[1];
        points[2][0] = c[0];
        points[2][1] = c[1];
        colors[0][0] = alpha[0];
        colors[0][1] = alpha[1];
        colors[1][0] = beta[0];
        colors[1][1] = beta[1];
        colors[2][0] = gamma[0];
        colors[2][1] = gamma[1];
    }
}

/* Calculates a matrix used in interpolation */
void calculateMatrix(double a[2], double b[2], double c[2], double m[2][2]) {
    double bMinusA[2]; double cMinusA[2];
    vecSubtract(2, b, a, bMinusA);
    vecSubtract(2, c, a, cMinusA);
    double matrix[2][2];
    mat22Columns(bMinusA, cMinusA, matrix);
    mat22Invert(matrix, m);
}

/* Renders a triangle */
void triRender(double a[2], double b[2], double c[2], double rgb[3],
              double alpha[2], double beta[2], double gamma[2], texTexture t) {
    double points[3][2]; 
    double colors[3][2];
    double m[2][2];
    permutePoints(a, b, c, points, alpha, beta, gamma, colors);
    calculateMatrix(points[0], points[1], points[2], m);
    
    /* Fills in the correct pixels */
    int x;
    int maxY;
    int minY;
    /* If the second point is the point with the greatest x value... */
    if(points[1][0] >= points[2][0]) {
        /* First half of the triangle */
        for(x = ceil(points[0][0]); x <= floor(points[2][0]); ++x) {
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, rgb, points[0], colors[0], colors[1], colors[2], m, t);
        }
        /* Second half of the triangle */
        for(x = floor(points[2][0]) + 1; x <= floor(points[1][0]); ++x) {
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[1][0], points[1][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, rgb, points[0], colors[0], colors[1], colors[2], m, t);
        }
    /* If the second point is the point with the middle x value... */
    }else {
        /* First half of the triangle */
        for(x = ceil(points[0][0]); x <= floor(points[1][0]); ++x) {
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, rgb, points[0], colors[0], colors[1], colors[2], m, t);
        }
        /* Second half of the triangle */
        for(x = floor(points[1][0]) + 1; x <= floor(points[2][0]); ++x) {
            minY = findYOnLine(x, points[1][0], points[1][1], points[2][0], points[2][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, rgb, points[0], colors[0], colors[1], colors[2], m, t);
        }
    }
}