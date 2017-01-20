/* Julia Connelly and Kerim Celik, 01/20/2017 */

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include "000pixel.h"
#include "070vector.c"
#include "090matrix.c"
#include "040texture.c"
#include "090renderer.c"

#define renVERTNUMBOUND 50
#define renVARYDIMBOUND 16

#define renATTRX 0
#define renATTRY 1
#define renATTRS 2
#define renATTRT 3
#define renVARYX 0
#define renVARYY 1
#define renVARYS 2
#define renVARYT 3
#define renVARYR 4
#define renVARYG 5
#define renVARYB 6
#define renUNIFR 0
#define renUNIFX 1
#define renUNIFY 2
#define renUNIFM 3
#define renUNIFTHETA 0
#define renUNIFTRANSX 1
#define renUNIFTRANSY 2
#define renTEXR 0
#define renTEXG 1
#define renTEXB 2

/* Sets rgb, based on the other parameters, which are unaltered. vary is an 
interpolated varying vector. */
void colorPixel(renRenderer *ren, double unif[], texTexture *tex[], 
        double vary[], double rgb[]) {
    texSample(tex[0], vary[renVARYS], vary[renVARYT]);
    rgb[0] = tex[0]->sample[renTEXR];
    rgb[1] = tex[0]->sample[renTEXG];
    rgb[2] = tex[0]->sample[renTEXB];
}

/* Writes the vary vector, based on the other parameters. */
void transformVertex(renRenderer *ren, double unif[], double attr[], 
        double vary[]) {
    double vary3[3] = {attr[renVARYX], attr[renVARYY], 1};
    mat331Multiply((double(*)[3])(&unif[renUNIFM]), vary3, vary3);
    vary[renVARYX] = vary3[renVARYX]; vary[renVARYY] = vary3[renVARYY];
    vary[renVARYS] = attr[renATTRS]; vary[renVARYT] = attr[renATTRT];
}

/* If unifParent is NULL, then sets the uniform matrix to the 
rotation-translation M described by the other uniforms. If unifParent is not 
NULL, but instead contains a rotation-translation P, then sets the uniform 
matrix to the matrix product P * M. */
void updateUniform(renRenderer *ren, double unif[], double unifParent[]) {
    if (unifParent == NULL) {
        /* The nine uniforms for storing the matrix start at index 
        renUNIFM. So &unif[renUNIFM] is an array containing those 
        nine numbers. We use '(double(*)[3])' to cast it to a 3x3 matrix. */
        mat33Isometry(unif[renUNIFTHETA], unif[renUNIFTRANSX], 
            unif[renUNIFTRANSY], (double(*)[3])(&unif[renUNIFM]));
    }else {
        double m[3][3];
        mat33Isometry(unif[renUNIFTHETA], unif[renUNIFTRANSX], 
            unif[renUNIFTRANSY], m);
        mat333Multiply((double(*)[3])(&unifParent[renUNIFM]), m, 
            (double(*)[3])(&unif[renUNIFM]));
    }
}

#include "090triangle.c"
#include "090mesh.c"
#include "090scene.c"

texTexture *t00[1], *t01[1], *t02[1], *t03[1];
texTexture t0, t1, t2, t3;
renRenderer r;
meshMesh m0, m1, m2, m3;

void initMesh(void) {
    meshInitializeEllipse(&m0, 0, 0, 300, 150, 30);
    meshInitializeRectangle(&m1, 0, 100, 0, 100);
    meshInitializeEllipse(&m2, 0, 0, 50, 50, 30);
    meshInitializeRectangle(&m3, 0, 150, 0, 100);
}

void initRenderer(void) {
    r.unifDim = 12; r.texNum = 4; r.attrDim = 4; r.varyDim = 4;
    r.colorPixel = colorPixel;
    r.transformVertex = transformVertex;
    r.updateUniform = updateUniform;
}

int initTextures(void) {
    if(texInitializeFile(&t0, "cat.jpg") != 0){
        return 1;
    }
    if(texInitializeFile(&t1, "test.jpg") != 0){
        return 1;
    }
    if(texInitializeFile(&t2, "test2.png") != 0){
        return 1;
    }
    if(texInitializeFile(&t3, "crown.jpg") != 0){
        return 1;
    }
    t00[0] = &t0; t01[0] = &t1; t02[0] = &t2; t03[0] = &t3;
    texSetFiltering(&t0, 0); 
    texSetFiltering(&t1, 0); 
    texSetFiltering(&t2, 0); 
    texSetFiltering(&t3, 0);
    return 0;
}

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        
        /* Initialize all components needed to create the scene */
        double unif0[12] = {0, 250, 250,
                          1, 0, 0,
                          0, 1, 0,
                          0, 0, 1};
        double unif1[12] = {3.14, 170, -50,
                          1, 0, 0,
                          0, 1, 0,
                          0, 0, 1};
        double unif2[12] = {0, -170, 50,
                          1, 0, 0,
                          0, 1, 0,
                          0, 0, 1};
        double unif3[12] = {-0.4, 0, 90,
                          1, 0, 0,
                          0, 1, 0,
                          0, 0, 1};
        initMesh();
        initRenderer();
        if(initTextures() == 1) return 1;
        
        /* Initialize the scene */
        sceneNode top, mid1, mid2, bot;
        sceneInitialize(&bot, &r, unif3, t03, &m3, NULL, NULL);
        sceneInitialize(&mid2, &r, unif2, t02, &m2, &bot, NULL);
        sceneInitialize(&mid1, &r, unif1, t01, &m1, NULL, &mid2);
        sceneInitialize(&top, &r, unif0, t00, &m0, &mid1, NULL);
        
        /* Render the scene */
        sceneRender(&top, &r, NULL);
        
        /* start the event loop */
		pixRun();
        
        /* Clean up */
        texDestroy(&t0); texDestroy(&t1); texDestroy(&t2); texDestroy(&t3);
        meshDestroy(&m0); meshDestroy(&m1); meshDestroy(&m2); meshDestroy(&m3);
        sceneDestroyRecursively(&top);
		return 0;
	}
}