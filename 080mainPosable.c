/* Julia Connelly and Kerim Celik, 01/15/2017 */

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include "000pixel.h"
#include "070vector.c"
#include "030matrix.c"
#include "040texture.c"
#include "080renderer.c"

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
    /* For now, just copy attr to varying. Baby steps. */
    double rotMat[2][2] = {{cos(unif[renUNIFR]), -1 * sin(unif[renUNIFR])}, 
                           {sin(unif[renUNIFR]), cos(unif[renUNIFR])}};
    mat221Multiply(rotMat, attr, vary);
    vary[renVARYX] += unif[renUNIFX];
    vary[renVARYY] += unif[renUNIFY];
    vary[renVARYS] = attr[renATTRS];
    vary[renVARYT] = attr[renATTRT];
}

#include "050triangle.c"
#include "080mesh.c"

texTexture *t[1];
texTexture t0;
renRenderer r;
meshMesh m;

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        t[0] = &t0;
        meshInitializeEllipse(&m, 0, 0, 300, 150, 30);
        
        /* initialize triangle coords, tex coords, and texTexture */
        double unif[3] = {3.14, 250, 250};
        r.unifDim = 3; r.texNum = 1; r.attrDim = 4; r.varyDim = 4;
        if(texInitializeFile(&t0, "cat.jpg") != 0){
            return 1;
        }

        /* renders a triangle using quadratic texture filtering */
        texSetFiltering(&t0, 0);
        meshRender(&m, &r, unif, t);

        /* start the event loop */
		pixRun();
        texDestroy(&t0);
        meshDestroy(&m);
		return 0;
	}
}