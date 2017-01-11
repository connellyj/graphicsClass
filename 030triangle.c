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
void getChi(double a[2], double b[2], double c[2], double x[2],  double chi[3],
           double alpha[3], double beta[3], double gamma[3]){
    /* b - a, c - a, and x - a for the first equation */
    double bMinusA[2]; double cMinusA[2]; double xMinusA[2];
    vecSubtract(2, b, a, bMinusA);
    vecSubtract(2, c, a, cMinusA);
    vecSubtract(2, x, a, xMinusA);
    
    /* inverted matrix for the first equation */
    double m[2][2]; double mInv[2][2];
    mat22Columns(bMinusA, cMinusA, m);
    mat22Invert(m, mInv);
    
    /* constants p and q used to interpolate colors */
    double pq[2];
    mat221Multiply(mInv, xMinusA, pq);
    
    /* interpolate the color for chi */
    double betMinusAlp[3]; double gamMinusAlp[3];
    vecSubtract(3, beta, alpha, betMinusAlp);
    vecSubtract(3, gamma, alpha, gamMinusAlp);
    double p[3]; double q[3];
    vecScale(3, pq[0], betMinusAlp, p);
    vecScale(3, pq[1], gamMinusAlp, q);
    double pPlusQ[3];
    vecAdd(3, p, q, pPlusQ);
    vecAdd(3, alpha, pPlusQ, chi);
}

/* Fills the given column with the given color */
void fillColumn(int x, double minY, double maxY, double rgb[3], 
                double a[2], double b[2], double c[2], 
                double alpha[3], double beta[3], double gamma[3]) {
    int y;
    double chi[3];
    double xVec[2];
    xVec[0] = x;
    for(y = ceil(minY); y <= floor(maxY); ++y) {
        xVec[1] = y;
        getChi(a, b, c, xVec, chi, alpha, beta, gamma);
        pixSetRGB(x, y, chi[0] * rgb[0], chi[1] * rgb[1], chi[2] * rgb[2]);
    }
}

/* Renders a triangle */
void triRender(double a[2], double b[2], double c[2], double rgb[3],
              double alpha[3], double beta[3], double gamma[3]) {
    int alphabetical = 0;
    
    /* Orders the points so the point with the lowest x value is first,
        and the rest follow in counterclockwise order. */
    double points[3][2];
    if(c[0] <= a[0] && c[0] <= b[0]) {
        points[0][0] = c[0];
        points[0][1] = c[1];
        points[1][0] = a[0];
        points[1][1] = a[1];
        points[2][0] = b[0];
        points[2][1] = c[1];
        if(a[0] <= b[0]) alphabetical = 1;
        else alphabetical = 0;
    }else if(b[0] <= a[0] && b[0] <= c[0]) {
        points[0][0] = b[0];
        points[0][1] = b[1];
        points[1][0] = c[0];
        points[1][1] = c[1];
        points[2][0] = a[0];
        points[2][1] = a[1];
        if(c[0] <= a[0]) alphabetical = 1;
        else alphabetical = 0;
    }else if(a[0] <= b[0] && a[0] <= c[0]) {
        points[0][0] = a[0];
        points[0][1] = a[1];
        points[1][0] = b[0];
        points[1][1] = b[1];
        points[2][0] = c[0];
        points[2][1] = c[1];
        if(b[0] <= c[0]) alphabetical = 1;
        else alphabetical = 0;
    }

    /* Fills in the correct pixels */
    int x;
    int maxY;
    int minY;
    /* If the second point is the point with the greatest x value... */
    if(alphabetical == 0) {
        /* First half of the triangle */
        for(x = ceil(points[0][0]); x <= floor(points[2][0]); ++x) {
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, rgb, a, b, c, alpha, beta, gamma);
        }
        /* Second half of the triangle */
        for(x = floor(points[2][0]) + 1; x <= floor(points[1][0]); ++x) {
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[1][0], points[1][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, rgb, a, b, c, alpha, beta, gamma);
        }
    /* If the second point is the point with the middle x value... */
    }else {
        /* First half of the triangle */
        for(x = ceil(points[0][0]); x <= floor(points[0][1]); ++x) {
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, rgb, a, b, c, alpha, beta, gamma);
        }
        /* Second half of the triangle */
        for(x = floor(points[1][0]) + 1; x <= floor(points[2][0]); ++x) {
            minY = findYOnLine(x, points[1][0], points[1][1], points[2][0], points[2][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, rgb, a, b, c, alpha, beta, gamma);
        }
    }
}