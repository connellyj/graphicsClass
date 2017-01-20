/* Julia Connelly and Kerim Celik, 01/20/2017 */

/* Returns the y value associated with the given x value on the
    line created by the two given points */
double findYOnLine(int x, double p1[2], double p2[2]) {
    if(p2[0] - p1[0] != 0 ) return p1[1] + (p2[1] - p1[1]) / (p2[0] - p1[0]) * (x - p1[0]);
    else return p2[1];
}

/* Interpolates to find the vary of x */
void getVary(double a[], double b[], double c[], double x[2],  double vary[], double m[2][2], renRenderer *ren){
    double aXY[2];
    aXY[0] = a[0]; aXY[1] = a[1];
    vecSubtract(2, x, aXY, aXY);
    double pq[2];
    mat221Multiply(m, aXY, pq);
    double bMinusA[(ren->varyDim)]; double cMinusA[(ren->varyDim)];
    vecSubtract((ren->varyDim), b, a, bMinusA);
    vecSubtract((ren->varyDim), c, a, cMinusA);
    vecScale((ren->varyDim), pq[0], bMinusA, bMinusA);
    vecScale((ren->varyDim), pq[1], cMinusA, cMinusA);
    vecAdd((ren->varyDim), bMinusA, cMinusA, cMinusA);
    vecAdd((ren->varyDim), a, cMinusA, vary);
}

/* Fills the given column with the appropriate color */
void fillColumn(int x, double minY, double maxY, double unif[], double a[], 
                double b[], double c[], double m[2][2], texTexture *t[], renRenderer *ren) {
    double vary[ren->varyDim];
    double rgb[3];
    double xVec[2];
    xVec[0] = x;
    for(int y = ceil(minY); y <= floor(maxY); ++y) {
        xVec[1] = y;
        getVary(a, b, c, xVec, vary, m, ren);
        ren->colorPixel(ren, unif, t, vary, rgb);
        pixSetRGB(x, y, rgb[0], rgb[1], rgb[2]);
    }
}

/* Orders the points so the point with the lowest x value is first,
        and the rest follow in counterclockwise order. */
void permutePoints(double *points[3]) {
    int minIndex = 0;
    for(int i = 1; i < 3; i++) {
        if(points[i][0] < points[minIndex][0]) minIndex = i;
    }
    if(minIndex > 0) {
        double *tmp[minIndex];
        for(int i = 0; i < minIndex; i++) {
            tmp[i] = points[i];
        }
        for(int i = minIndex; i < 3; i++) {
            points[i - minIndex] = points[i];
        }
        for(int i = 0; i < minIndex; i++) {
            points[i + (3 - minIndex)] = tmp[i];
        }
    }
}

/* Calculates a matrix used in interpolation */
void calculateMatrix(double a[2], double b[2], double c[2], double m[2][2]) {
    vecSubtract(2, b, a, b);
    vecSubtract(2, c, a, c);
    double matrix[2][2];
    mat22Columns(b, c, matrix);
    mat22Invert(matrix, m);
}

/* Renders a triangle */
void triRender(double unif[], renRenderer *ren, double a[], double b[], double c[], texTexture *tex[]) {
    /* Order the points to fit with the algorithm */
    double *points[3] = {a, b, c};
    permutePoints(points);
    
    /* Calculate a matrix used in interpolation */
    double tempA[2]; double tempB[2]; double tempC[2]; double m[2][2];
    tempA[0] = points[0][0]; tempA[1] = points[0][1]; 
    tempB[0] = points[1][0]; tempB[1] = points[1][1]; 
    tempC[0] = points[2][0]; tempC[1] = points[2][1];
    calculateMatrix(tempA, tempB, tempC, m);
    
    /* Fills in the correct pixels */
    int maxY;
    int minY;
    /* If the second point is the point with the greatest x value... */
    if(points[1][0] >= points[2][0]) {
        /* First half of the triangle */
        for(int x = ceil(points[0][0]); x <= floor(points[2][0]); ++x) {
            minY = findYOnLine(x, points[0], points[1]);
            maxY = findYOnLine(x, points[0], points[2]);
            fillColumn(x, minY, maxY, unif, points[0], points[1], points[2], m, tex, ren);
        }
        /* Second half of the triangle */
        for(int x = floor(points[2][0]) + 1; x <= floor(points[1][0]); ++x) {
            minY = findYOnLine(x, points[0], points[1]);
            maxY = findYOnLine(x, points[1], points[2]);
            fillColumn(x, minY, maxY, unif, points[0], points[1], points[2], m, tex, ren);
        }
    /* If the third point is the point with the greatest x value... */
    }else {
        /* First half of the triangle */
        for(int x = ceil(points[0][0]); x <= floor(points[1][0]); ++x) {
            minY = findYOnLine(x, points[0], points[1]);
            maxY = findYOnLine(x, points[0], points[2]);
            fillColumn(x, minY, maxY, unif, points[0], points[1], points[2], m, tex, ren);
        }
        /* Second half of the triangle */
        for(int x = floor(points[1][0]) + 1; x <= floor(points[2][0]); ++x) {
            minY = findYOnLine(x, points[1], points[2]);
            maxY = findYOnLine(x, points[0], points[2]);
            fillColumn(x, minY, maxY, unif, points[0], points[1], points[2], m, tex, ren);
        }
    }
}