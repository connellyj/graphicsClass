/* Julia Connelly and Kerim Celik, 01/26/2017 */

/* Transforms the given vector from clip space to screen space */
void clipClipToScreen(renRenderer *ren, double x[4], double out[4]){
    double scale = 1 / x[3];
    vecScale(4, scale, x, x);
    mat441Multiply(ren->viewport, x, out);
}

/* Returns whether or not the given vertex should be clipped */
int clipIsClipped(double v[]){
    if(v[3] <= 0 || v[2] > v[3]){
        return 1;
    }
    return 0;
}

/* Converts the given vertices to screen space and renders the triangle */
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

/* Assumes a is not clipped, and creates a new triangle to render */
void clipFindOneTriangleFromVertices(double unif[], renRenderer *ren, double a[], double b[], double c[], texTexture *tex[]) {
    double tAB = (b[3] - b[2]) / (b[3] - b[2] - a[3] + a[2]);
    double tAC = (c[3] - c[2]) / (c[3] - c[2] - a[3] + a[2]);
    double newB[ren->varyDim]; double newC[ren->varyDim];
    vecSubtract(ren->varyDim, a, b, newB);
    vecScale(ren->varyDim, tAB, newB, newB);
    vecAdd(ren->varyDim, b, newB, newB);
    vecSubtract(ren->varyDim, a, c, newC);
    vecScale(ren->varyDim, tAC, newC, newC);
    vecAdd(ren->varyDim, c, newC, newC);
    clipFinishRender(unif, ren, a, newB, newC, tex);
}

/* Assumes c is clipped, and creates two new triangles to render */
void clipFindTwoTrianglesFromVertices(double unif[], renRenderer *ren, double a[], double b[], double c[], texTexture *tex[]) {
    double tAC = (c[3] - c[2]) / (c[3] - c[2] - a[3] + a[2]);
    double tBC = (c[3] - c[2]) / (c[3] - c[2] - b[3] + b[2]);
    double newC1[ren->varyDim]; double newC2[ren->varyDim];
    vecSubtract(ren->varyDim, a, c, newC1);
    vecScale(ren->varyDim, tAC, newC1, newC1);
    vecAdd(ren->varyDim, c, newC1, newC1);
    vecSubtract(ren->varyDim, b, c, newC2);
    vecScale(ren->varyDim, tBC, newC2, newC2);
    vecAdd(ren->varyDim, c, newC2, newC2);
    clipFinishRender(unif, ren, a, b, newC1, tex);
    clipFinishRender(unif, ren, b, newC2, newC1, tex);
}

/* Checks all clip cases and renders */
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