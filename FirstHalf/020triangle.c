/* Julia Connelly, 01/07/2017, CS311 */
    
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "000pixel.h"

int alphabetical = 0;

/* Returns the y value associated with the given x value on the
    line created by the two given points */
double findYOnLine(int x, double p1x, double p1y, double p2x, double p2y) {
    if(p2x - p1x != 0 ) return p1y + (p2y - p1y) / (p2x - p1x) * (x - p1x);
    else return p1y;
}

/* Fills the given column with the given color */
void fillColumn(int x, double minY, double maxY, double r, double g, double b) {
    int y;
    for(y = ceil(minY); y <= floor(maxY); ++y) {
        pixSetRGB(x, y, r, g, b);
    }
}

/* Renders a triangle */
void triRender(double a0, double a1, double b0, double b1, double c0, 
        double c1, double r, double g, double b) {
    
    /* Orders the points so the point with the lowest x value is first,
        and the rest follow in counterclockwise order. */
    double points[6];
    if(c0 <= a0 && c0 <= b0) {
        points[0] = c0;
        points[1] = c1;
        points[2] = a0;
        points[3] = a1;
        points[4] = b0;
        points[5] = b1;
        if(a0 <= b0) alphabetical = 1;
        else alphabetical = 0;
    }else if(b0 <= a0 && b0 <= c0) {
        points[0] = b0;
        points[1] = b1;
        points[2] = c0;
        points[3] = c1;
        points[4] = a0;
        points[5] = a1;
        if(c0 <= a0) alphabetical = 1;
        else alphabetical = 0;
    }else if(a0 <= b0 && a0 <= c0) {
        points[0] = a0;
        points[1] = a1;
        points[2] = b0;
        points[3] = b1;
        points[4] = c0;
        points[5] = c1;
        if(b0 <= c0) alphabetical = 1;
        else alphabetical = 0;
    }

    /* Fills in the correct pixels */
    int x;
    int maxY;
    int minY;
    /* If the second point is the point with the greatest x value... */
    if(alphabetical == 0) {
        /* First half of the triangle */
        for(x = ceil(points[0]); x <= floor(points[4]); ++x) {
            minY = findYOnLine(x, points[0], points[1], points[2], points[3]);
            maxY = findYOnLine(x, points[0], points[1], points[4], points[5]);
            fillColumn(x, minY, maxY, r, g, b);
        }
        /* Second half of the triangle */
        for(x = floor(points[4]) + 1; x <= floor(points[2]); ++x) {
            minY = findYOnLine(x, points[0], points[1], points[2], points[3]);
            maxY = findYOnLine(x, points[2], points[3], points[4], points[5]);
            fillColumn(x, minY, maxY, r, g, b);
        }
    /* If the second point is the point with the middle x value... */
    }else {
        /* First half of the triangle */
        for(x = ceil(points[0]); x <= floor(points[2]); ++x) {
            minY = findYOnLine(x, points[0], points[1], points[2], points[3]);
            maxY = findYOnLine(x, points[0], points[1], points[4], points[5]);
            fillColumn(x, minY, maxY, r, g, b);
        }
        /* Second half of the triangle */
        for(x = floor(points[2]) + 1; x <= floor(points[4]); ++x) {
            minY = findYOnLine(x, points[2], points[3], points[4], points[5]);
            maxY = findYOnLine(x, points[0], points[1], points[4], points[5]);
            fillColumn(x, minY, maxY, r, g, b);
        }
    }
}