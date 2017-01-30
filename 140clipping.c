/* Julia Connelly and Kerim Celik, 01/26/2017 */

void clipClipToScreen(renRenderer *ren, double x[4], double out[4]){
    double scale = 1 / x[3];
    //printf("%f , %f , %f , %f\n", x[0], x[1], x[2], x[3]);
    vecScale(4, scale, x, x);
    mat441Multiply(ren->viewport, x, out);
}

int clipIsClipped(double arr[]){
    if(arr[3] <= 0 || arr[2] > arr[3]){
        return 1;
    }
    return 0;
}

void clipFinishRender(double unif[], renRenderer *ren, double a[], double b[], double c[], texTexture *tex[]){
    double aXYZW[4] = {a[renVARYX], a[renVARYY], a[renVARYZ], a[renVARYW]};
    double bXYZW[4] = {b[renVARYX], b[renVARYY], b[renVARYZ], b[renVARYW]}; 
    double cXYZW[4] = {c[renVARYX], c[renVARYY], c[renVARYZ], c[renVARYW]};
    double outA[4]; double outB[4]; double outC[4];
    clipClipToScreen(ren, aXYZW, outA);
    clipClipToScreen(ren, bXYZW, outB);
    clipClipToScreen(ren, cXYZW, outC);
    double v0[ren->varyDim]; double v1[ren->varyDim]; double v2[ren->varyDim];
    for (int i = 0; i < 4; i++){
        v0[i] = outA[i];
        v1[i] = outB[i];
        v2[i] = outC[i];
    }
    for(int i = 4; i < ren->varyDim; i++) {
        v0[i] = a[i];
        v1[i] = b[i];
        v2[i] = c[i];
    }
    triRender(unif, ren, v0, v1, v2, tex);
}

// a is not clipped
void clipFindOneTriangleFromVertices(double unif[], renRenderer *ren, double a[], double b[], double c[], texTexture *tex[]) {
    double tAB = (b[3] - b[2]) / (b[3] - b[2] - a[3] + a[2]);
    double tAC = (c[3] - c[2]) / (c[3] - c[2] - a[3] + a[2]);
    double newB[ren->varyDim]; double newC[ren->varyDim];
    vecSubtract(ren->varyDim, b, a, newB);
    vecScale(ren->varyDim, tAB, newB, newB);
    vecAdd(ren->varyDim, a, newB, newB);
    vecSubtract(ren->varyDim, c, a, newC);
    vecScale(ren->varyDim, tAC, newC, newC);
    vecAdd(ren->varyDim, a, newC, newC);
    clipFinishRender(unif, ren, a, newB, newC, tex);
}

// c is clipped
void clipFindTwoTrianglesFromVertices(double unif[], renRenderer *ren, double a[], double b[], double c[], texTexture *tex[]) {
    double tAC = (c[3] - c[2]) / (c[3] - c[2] - a[3] + a[2]);
    double tBC = (c[3] - c[2]) / (c[3] - c[2] - b[3] + b[2]);
    double newC1[ren->varyDim]; double newC2[ren->varyDim];
    vecSubtract(ren->varyDim, c, a, newC1);
    vecScale(ren->varyDim, tAC, newC1, newC1);
    vecAdd(ren->varyDim, a, newC1, newC1);
    vecSubtract(ren->varyDim, c, b, newC2);
    vecScale(ren->varyDim, tBC, newC2, newC2);
    vecAdd(ren->varyDim, b, newC2, newC2);
    clipFinishRender(unif, ren, a, b, newC1, tex);
    clipFinishRender(unif, ren, b, newC2, newC1, tex);
}

void clipRender(double unif[], renRenderer *ren, double a[], double b[], double c[], texTexture *tex[]){
    int clippedA = clipIsClipped(a); 
    int clippedB = clipIsClipped(b); 
    int clippedC = clipIsClipped(c);
    if(clippedA == 0 && clippedB == 0 && clippedC == 0){
        clipFinishRender(unif, ren, a, b, c, tex);
    }
    else if(clippedA == 1 && clippedB == 0 && clippedC == 0){
        clipFindTwoTrianglesFromVertices(unif, ren, b, c, a, tex);
    }
    else if(clippedA == 0 && clippedB == 1 && clippedC == 0){
        clipFindTwoTrianglesFromVertices(unif, ren, c, a, b, tex);
    }
    else if(clippedA == 0 && clippedB == 0 && clippedC == 1){
        clipFindTwoTrianglesFromVertices(unif, ren, a, b, c, tex);
    }
    else if(clippedA == 0 && clippedB == 1 && clippedC == 1){
        clipFindOneTriangleFromVertices(unif, ren, a, b, c, tex);
    }
    else if(clippedA == 1 && clippedB == 0 && clippedC == 1){
        clipFindOneTriangleFromVertices(unif, ren, b, c, a, tex);
    }
    else if(clippedA == 1 && clippedB == 1 && clippedC == 0){
        clipFindOneTriangleFromVertices(unif, ren, c, a, b, tex);
    }
}