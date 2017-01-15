/* Julia Connelly and Kerim Celik, 01/12/2017 
NOTE TO GRADER: we fixed some bugs with intitial triangle rasterizing code
and refactored some stuff */

/* Returns the y value associated with the given x value on the
    line created by the two given points */
double findYOnLine(int x, double p1x, double p1y, double p2x, double p2y) {
    if(p2x - p1x != 0 ) return p1y + (p2y - p1y) / (p2x - p1x) * (x - p1x);
    else return p1y;
}

/* Interpolates to find the chi */
void getAttr(double a[], double b[], double c[], double x[2],  double attr[], double m[2][2], renRenderer *ren){
    double xMinusA[2];
    double tempA[2]; double tempAlp[(ren->attrDim) - 2]; double tempBet[(ren->attrDim) - 2]; double tempGam[(ren->attrDim) - 2];
    tempA[0] = a[0]; tempA[1] = a[1];
    for(int i = 0; i < (ren->attrDim) - 2; i++) {
        tempAlp[i] = a[i + 2];
        tempBet[i] = b[i + 2];
        tempGam[i] = c[i + 2];
    }
    vecSubtract(2, x, tempA, xMinusA);
    double pq[2];
    mat221Multiply(m, xMinusA, pq);
    double betMinusAlp[2]; double gamMinusAlp[3];
    vecSubtract(2, tempBet, tempAlp, betMinusAlp);
    vecSubtract(2, tempGam, tempAlp, gamMinusAlp);
    double p[2]; double q[2];
    vecScale(2, pq[0], betMinusAlp, p);
    vecScale(2, pq[1], gamMinusAlp, q);
    double pPlusQ[2];
    vecAdd(2, p, q, pPlusQ);
    vecAdd(2, tempAlp, pPlusQ, attr);
}

/* Fills the given column with the appropriate color */
void fillColumn(int x, double minY, double maxY, double unif[], double a[], 
                double b[], double c[], double m[2][2], texTexture *t[], renRenderer *ren) {
    double attr[ren->attrDim];
    double xVec[2];
    double rgb[3];
    xVec[0] = x;
    for(int y = ceil(minY); y <= floor(maxY); ++y) {
        xVec[1] = y;
        getAttr(a, b, c, xVec, attr, m, ren);
        colorPixel(ren, unif, t, attr, rgb);
//        for(int i = 0; i < 3; i++) {
//            printf("%f ", rgb[i]);
//        }
//        printf("\n");
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
    double bMinusA[2]; double cMinusA[2];
    vecSubtract(2, b, a, bMinusA);
    vecSubtract(2, c, a, cMinusA);
    double matrix[2][2];
    mat22Columns(bMinusA, cMinusA, matrix);
    mat22Invert(matrix, m);
}

/* Renders a triangle */
void triRender(double unif[], renRenderer *ren, double a[], double b[], double c[], texTexture *tex[]) {
    double *points[3] = {a, b, c};
    double m[2][2];
    double tempA[2]; double tempB[2]; double tempC[2];
    permutePoints(points);
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
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, unif, points[0], points[1], points[2], m, tex, ren);
        }
        /* Second half of the triangle */
        for(int x = floor(points[2][0]) + 1; x <= floor(points[1][0]); ++x) {
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[1][0], points[1][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, unif, points[0], points[1], points[2], m, tex, ren);
        }
    /* If the second point is the point with the middle x value... */
    }else {
        /* First half of the triangle */
        for(int x = ceil(points[0][0]); x <= floor(points[1][0]); ++x) {
            minY = findYOnLine(x, points[0][0], points[0][1], points[1][0], points[1][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, unif, points[0], points[1], points[2], m, tex, ren);
        }
        /* Second half of the triangle */
        for(int x = floor(points[1][0]) + 1; x <= floor(points[2][0]); ++x) {
            minY = findYOnLine(x, points[1][0], points[1][1], points[2][0], points[2][1]);
            maxY = findYOnLine(x, points[0][0], points[0][1], points[2][0], points[2][1]);
            fillColumn(x, minY, maxY, unif, points[0], points[1], points[2], m, tex, ren);
        }
    }
}