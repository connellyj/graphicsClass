/* Julia Connelly and Kerim Celik, 01/26/2017 */

#define screenWIDTH 512
#define screenHEIGHT 512

#define renVERTNUMBOUND 1000
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
#define renVARYW 3
#define renVARYS 4
#define renVARYT 5

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
#define renUNIFC 25

#define renTEXR 0
#define renTEXG 1
#define renTEXB 2
#define renTEXZ 3

#define GLFW_KEY_W 87
#define GLFW_KEY_A 65
#define GLFW_KEY_S 83
#define GLFW_KEY_D 68
#define GLFW_KEY_RIGHT 262
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_UP 265

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include "000pixel.h"
#include "100vector.c"
#include "130matrix.c"
#include "040texture.c"
#include "110depth.c"
#include "130renderer.c"
#include "110triangle.c"
#include "140clipping.c"
#include "140mesh.c"
#include "090scene.c"


/******************** START SHADER ********************/

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
    double attrXYZ1[4] = {attr[renATTRX], attr[renATTRY], attr[renATTRZ], 1};
    double varyXYZ1[4];
    double varyClip[4];
    mat441Multiply((double(*)[4])(&unif[renUNIFM]), attrXYZ1, varyXYZ1);
    mat441Multiply((double(*)[4])(&unif[renUNIFC]), varyXYZ1, vary);
    vary[renVARYS] = attr[renATTRS]; vary[renVARYT] = attr[renATTRT];
}

/* If unifParent is NULL, then sets the uniform matrix to the 
rotation-translation M described by the other uniforms. If unifParent is not 
NULL, but instead contains a rotation-translation P, then sets the uniform 
matrix to the matrix product P * M. */
void updateUniform(renRenderer *ren, double unif[], double unifParent[]) {
    // Finds the 3x3 rotation matrix based on input from unif
    double axis[3];
    vec3Spherical(1.0, unif[renUNIFPHI], unif[renUNIFTHETA], axis);
    vecUnit(3, axis, axis);
    double rot[3][3];
    mat33AngleAxisRotation(unif[renUNIFALPHA], axis, rot);
    
    // The amount to be translated
    double trans[3] = {unif[renUNIFTRANSX], unif[renUNIFTRANSY], unif[renUNIFTRANSZ]};
    
    // Places the camera's viewing matrix into unif
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            unif[renUNIFC + j + (4 * i)] = ren->viewing[i][j];
        }
    }
    
    // Applies transformation
    if (unifParent == NULL) {
        mat44Isometry(rot, trans, (double(*)[4])(&unif[renUNIFM]));
    }
    else {
        double m[4][4];
        mat44Isometry(rot, trans, m);
        mat444Multiply((double(*)[4])(&unifParent[renUNIFM]), m, 
            (double(*)[4])(&unif[renUNIFM]));
    }
}

/******************** END SHADER ********************/

renRenderer r;
sceneNode top, mid;
double camRot = 0.0;
int rotate = 0;
int vertical = 0;
int horizontal = 0;
int animate = 0;
int projType = 0;

void draw() {
    pixClearRGB(0.0, 0.0, 0.0);
    depthClearZs(r.depth, -10000);
    renUpdateViewing(&r);
    sceneRender(&top, &r, NULL);
}

void handleMouseDown(int button, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
    if(shiftIsDown) animate = 1;
	else rotate = 1;
}

void handleMouseUp(int button, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	rotate = 0;
    animate = 0;
}

void handleKeyDown(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
    if(key == GLFW_KEY_UP) {
        vertical = 1;
    }
    if(key == GLFW_KEY_DOWN) {
        vertical = -1;
    }
    if(key == GLFW_KEY_RIGHT) {
        horizontal = 1;
    }
    if(key == GLFW_KEY_LEFT) {
        horizontal = -1;
    }
    if(key == GLFW_KEY_W) {
        projType = 1 - projType;
        renSetFrustum(&r, projType, M_PI / 6.0, 10.0, 10.0);
        draw();
    }
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
    if(key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) {
        vertical = 0;
    }
    if(key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT) {
        horizontal = 0;
    }
}

void handleTimeStep(double oldTime, double newTime) {
    if(rotate == 1) {
        double axisY[3] = {0, 0, 1};
        double rot[3][3];
        camRot += 0.5 * (newTime - oldTime);
        mat33AngleAxisRotation(camRot, axisY, rot);
        mat33Scale(rot, r.cameraRotation, 1);
        draw();
    }
    if(vertical == 1) {
        r.cameraTranslation[1] += 20 * (newTime - oldTime);
        draw();
    }
    if(vertical == -1) {
        r.cameraTranslation[1] -= 20 * (newTime - oldTime);
        draw();
    }
    if(horizontal == 1) {
        r.cameraTranslation[0] += 20 * (newTime - oldTime);
        draw();
    }
    if(horizontal == -1) {
        r.cameraTranslation[0] -= 20 * (newTime - oldTime);
        draw();
    }
    if(animate == 1) {
        top.unif[renUNIFALPHA] += 0.5 * (newTime - oldTime);
        draw();
    }
}

void setEventHandlers() {
    pixSetMouseDownHandler(handleMouseDown);
    pixSetTimeStepHandler(handleTimeStep);
    pixSetMouseUpHandler(handleMouseUp);
    pixSetKeyUpHandler(handleKeyUp);
    pixSetKeyDownHandler(handleKeyDown);
}

int main(void) {
    // CHECK Z VALUES IN DEPTH BUFFER AND WHERE WE SET THEM
    // WEIRD STITCHING PROBLEM
    
    /* initialize the window */
    if (pixInitialize(screenWIDTH, screenHEIGHT, "Pixel Graphics") != 0)
		return 1;
	else {        
        /* allocate space */
        texTexture *texTigerArray[1], *texHuskiesArray[1];
        texTexture texTiger, texHuskies;
        meshMesh sphere, cube;
        depthBuffer d;
        
        /* Initialize unifs */
        double unifTop[41] = {1, 1, 1,
                            M_PI / 2, M_PI / 2, 0,
                            0, 0, -20,
                            1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1,
                            1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1};
        double unifMid[41] = {1, 1, 1,
                            0, 0, 0,
                            0, 0, -5,
                            1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1,
                            1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1};
        
        /* Initialize meshes */
        meshInitializeSphere(&sphere, 3, 40, 20);
        meshInitializeBox(&cube, 0, 2, 0, 2, 0, 2);
        
        /* Initialize renderer */
        r.unifDim = 3 + 3 + 3 + 16 + 16; r.texNum = 2; r.attrDim = 8; r.varyDim = 6;
        r.colorPixel = colorPixel;
        r.transformVertex = transformVertex;
        r.updateUniform = updateUniform;
        depthInitialize(&d, screenWIDTH, screenHEIGHT);
        r.depth = &d;
        for (int i = 0; i < 3; i++){
            r.cameraRotation[i][i] = 1;
        }
        int neg = -1;
        for(int i = 0; i < 6; i++) {
            r.projection[i] = neg * 250;
            neg *= -1;
        }
        
        /* Initialize textures */
        if(texInitializeFile(&texTiger, "cat.jpg") != 0){
            return 1;
        }
        if(texInitializeFile(&texHuskies, "test.jpg") != 0){
            return 1;
        }
        texTigerArray[0] = &texTiger; texHuskiesArray[0] = &texHuskies;
        texSetFiltering(&texTiger, 0); texSetFiltering(&texHuskies, 0);
        
        /* Initialize the scene */
        sceneInitialize(&mid, &r, unifMid, texHuskiesArray, &cube, NULL, NULL);
        sceneInitialize(&top, &r, unifTop, texTigerArray, &sphere, &mid, NULL);
        
        /* Draw the scene */
        renSetFrustum(&r, projType, M_PI / 6.0, 10.0, 10.0);
        draw();
        
        /* start the event loop */
        setEventHandlers();
		pixRun();
        
        /* Clean up */
        texDestroy(&texTiger); texDestroy(&texHuskies);
        meshDestroy(&sphere); meshDestroy(&cube);
        sceneDestroyRecursively(&top);
        depthDestroy(&d);
		return 0;
	}
}