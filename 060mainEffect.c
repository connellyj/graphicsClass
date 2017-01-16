/* Julia Connelly and Kerim Celik, 01/15/2017 */

#define renATTRDIMBOUND 16

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "000pixel.h"
#include "030vector.c"
#include "030matrix.c"
#include "040texture.c"
#include "050renderer.c"

#define renATTRX 0
#define renATTRY 1
#define renATTRS0 2
#define renATTRT0 3
#define renATTRS1 4
#define renATTRT1 5
#define renATTRS2 6
#define renATTRT2 7
#define renTEXR 0
#define renTEXG 1
#define renTEXB 2

/* Sets rgb, based on the other parameters, which are unaltered. attr is an 
interpolated attribute vector. */
void colorPixel(renRenderer *ren, double unif[], texTexture *tex[], 
        double attr[], double rgb[]) {
    texSample(tex[1], attr[renATTRS1], attr[renATTRT1]);
    texSample(tex[2], attr[renATTRS2], attr[renATTRT2]);
    int texIndex = 0;
    if(tex[1]->sample[renTEXG] < 0.9){
        texIndex = 1;
    }
    else if(tex[2]->sample[renTEXG] < 0.9){
        texIndex = 2;
    }
    else {
        if (attr[renATTRX] < 256){
            attr[renATTRS0] += (2 * fabs((unif[0]) - attr[renATTRS0]));
        }
        texSample(tex[0], attr[renATTRS0], attr[renATTRT0]);
    }
    rgb[0] = tex[texIndex]->sample[renTEXR];
    rgb[1] = tex[texIndex]->sample[renTEXG];
    rgb[2] = tex[texIndex]->sample[renTEXB];
}

#include "050triangle.c"

texTexture *t[3];
texTexture t0; texTexture t1; texTexture t2;
renRenderer r;

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        t[0] = &t0;
        t[1] = &t1;
        t[2] = &t2;
        
        /* initialize triangle coords, tex coords, and texTexture */
        double a[8] = {-200, 500, -0.5, .85, -2.5, 1, -0.25, 0.87};
        double b[8] = {256, 0, .2, .05, -0.5, -1.8, 0.75, -1.6};
        double c[8] = {712, 500, .9, .85, 2.5, 1, 1.75, 0.87};
        double unif[1] = {.2};
        r.unifDim = 1; r.texNum = 3; r.attrDim = 8;
        if(texInitializeFile(&t0, "cat.jpg") != 0){
            return 1;
        }
        if(texInitializeFile(&t1, "hat.jpg") != 0){
            return 1;
        }
        if(texInitializeFile(&t2, "crown.jpg") != 0){
            return 1;
        }

        /* renders a triangle using quadratic texture filtering */
        texSetFiltering(&t0, 0); texSetFiltering(&t1, 0); texSetFiltering(&t2, 0);
        triRender(unif, &r, a, b, c, t);

        /* start the event loop */
		pixRun();
        texDestroy(&t0);
        texDestroy(&t1);
        texDestroy(&t2);
		return 0;
	}
}