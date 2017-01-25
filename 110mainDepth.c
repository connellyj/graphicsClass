/* Julia Connelly and Kerim Celik, 01/20/2017 */

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include "000pixel.h"
#include "100vector.c"
#include "100matrix.c"
#include "040texture.c"
#include "110depth.c"
#include "110renderer.c"

#define screenWIDTH 512
#define screenHEIGHT 512

#define renVERTNUMBOUND 50
#define renVARYDIMBOUND 16

#define renATTRX 0
#define renATTRY 1
#define renATTRZ 2
#define renATTRS 3
#define renATTRT 4
#define renATTRN 5
#define renATTRO 6
#define renATTRP 7

#define renVARYX 0
#define renVARYY 1
#define renVARYZ 2
#define renVARYS 3
#define renVARYT 4

#define renUNIFR 0
#define renUNIFG 1
#define renUNIFB 2
#define renUNIFTHETA 3
#define renUNIFPHI 4
#define renUNIFALPHA 5
#define renUNIFTRANSX 6
#define renUNIFTRANSY 7
#define renUNIFTRANSZ 8
#define renUNIFM 9

#define renTEXR 0
#define renTEXG 1
#define renTEXB 2
#define renTEXZ 3

double animRot = 0;

/* Sets rgb, based on the other parameters, which are unaltered. vary is an 
interpolated varying vector. */
void colorPixel(renRenderer *ren, double unif[], texTexture *tex[], 
        double vary[], double rgbz[]) {
    texSample(tex[0], vary[renVARYS], vary[renVARYT]);
    rgbz[0] = tex[0]->sample[renTEXR];
    rgbz[1] = tex[0]->sample[renTEXG];
    rgbz[2] = tex[0]->sample[renTEXB];
    rgbz[3] = vary[renVARYZ];
}

/* Writes the vary vector, based on the other parameters. */
void transformVertex(renRenderer *ren, double unif[], double attr[], 
        double vary[]) {
    double vary4[4] = {attr[renATTRX], attr[renATTRY], attr[renATTRZ], 1};
    mat441Multiply((double(*)[4])(&unif[renUNIFM]), vary4, vary4);
    vary[renVARYX] = vary4[renVARYX]; 
    vary[renVARYY] = vary4[renVARYY];
    vary[renVARYZ] = vary4[renVARYZ];
    vary[renVARYS] = attr[renATTRS]; vary[renVARYT] = attr[renATTRT];
    //animRot is global, make it local as a fix 
    unif[renUNIFALPHA] += animRot;
}

/* If unifParent is NULL, then sets the uniform matrix to the 
rotation-translation M described by the other uniforms. If unifParent is not 
NULL, but instead contains a rotation-translation P, then sets the uniform 
matrix to the matrix product P * M. */
void updateUniform(renRenderer *ren, double unif[], double unifParent[]) {
    double axis[3];
    vec3Spherical(1.0, unif[renUNIFPHI], unif[renUNIFTHETA], axis);
    vecUnit(3, axis, axis);
    double rot[3][3];
    mat33AngleAxisRotation(unif[renUNIFALPHA], axis, rot);
    double trans[3] = {unif[renUNIFTRANSX], unif[renUNIFTRANSY], unif[renUNIFTRANSZ]};
    if (unifParent == NULL) {
        /* &unif[renUNIFM] is an array containing nine numbers */
        mat44Isometry(rot, trans, (double(*)[4])(&unif[renUNIFM]));
    }
    else {
        double m[4][4];
        mat44Isometry(rot, trans, m);
        mat444Multiply((double(*)[4])(&unifParent[renUNIFM]), m, 
            (double(*)[4])(&unif[renUNIFM]));
    }
}

#include "110triangle.c"
#include "100mesh.c"
#include "090scene.c"

texTexture *t00[1], *t01[1];
texTexture t0, t1;
renRenderer r;
meshMesh m0, m1;
sceneNode top, mid1;
depthBuffer d;
int animate = 0;

void initMesh(void) {
    meshInitializeSphere(&m0, 50, 40, 20);
    meshInitializeBox(&m1, 0, 100, 0, 100, 0, 100);
}

void initRenderer(void) {
    r.unifDim = 3 + 3 + 3 + 16; r.texNum = 2; r.attrDim = 8; r.varyDim = 5;
    r.colorPixel = colorPixel;
    r.transformVertex = transformVertex;
    r.updateUniform = updateUniform;
    depthInitialize(&d, screenWIDTH, screenHEIGHT);
    r.depth = &d;
}

int initTextures(void) {
    if(texInitializeFile(&t0, "cat.jpg") != 0){
        return 1;
    }
    if(texInitializeFile(&t1, "test.jpg") != 0){
        return 1;
    }
    t00[0] = &t0; t01[0] = &t1;
    texSetFiltering(&t0, 0); texSetFiltering(&t1, 0);
    return 0;
}

void handleMouseDown(int button, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	animate = 1;
}

/* Similarly, the following callbacks handle some mouse interactions. */
void handleMouseUp(int button, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	animate = 0;
}

void draw() {
    pixClearRGB(0.0, 0.0, 0.0);
    depthClearZs(r.depth, -1000);
    sceneRender(&top, &r, NULL);
}

/* This callback is called once per animation frame. As parameters it receives 
the time for the current frame and the time for the previous frame. Both times 
are measured in seconds since some distant past time. */ 
void handleTimeStep(double oldTime, double newTime) {
    if(animate == 1) {
        animRot = 0.001 * (newTime - oldTime);
        draw();
    }
}

int main(void) {
    /* initialize the window */
    if (pixInitialize(screenWIDTH, screenHEIGHT, "Pixel Graphics") != 0)
		return 1;
	else {        
        /* Initialize all components needed to create the scene */
        double unif0[25] = {1, 1, 1,
                            3.14 / 2, 3.14 / 2, 0,
                            250, 250, 0,
                            1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1};
        double unif1[25] = {1, 1, 1,
                            0, 0, 0,
                            0, 0, 0,
                            1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1};
        initMesh();
        initRenderer();
        if(initTextures() == 1) return 1;
        
        /* Initialize the scene */
        sceneInitialize(&mid1, &r, unif1, t01, &m1, NULL, NULL);
        sceneInitialize(&top, &r, unif0, t00, &m0, &mid1, NULL);
        
        /* Draw the scene */
        draw();
        
        pixSetMouseDownHandler(handleMouseDown);
        pixSetTimeStepHandler(handleTimeStep);
        pixSetMouseUpHandler(handleMouseUp);
        
        /* start the event loop */
		pixRun();
        
        /* Clean up */
        texDestroy(&t0); texDestroy(&t1);
        meshDestroy(&m0); meshDestroy(&m1);
        sceneDestroyRecursively(&top);
        depthDestroy(&d);
		return 0;
	}
}