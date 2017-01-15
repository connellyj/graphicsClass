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
#define renATTRS 2
#define renATTRT 3
#define renATTRR 4
#define renATTRG 5
#define renATTRB 6
#define renUNIFR 0
#define renUNIFG 1
#define renUNIFB 2
#define renTEXR 0
#define renTEXG 1
#define renTEXB 2

/* Sets rgb, based on the other parameters, which are unaltered. attr is an 
interpolated attribute vector. */
void colorPixel(renRenderer *ren, double unif[], texTexture *tex[], 
        double attr[], double rgb[]) {
    texSample(tex[0], attr[renATTRS], attr[renATTRT]);
    rgb[0] = tex[0]->sample[renTEXR] * unif[renUNIFR] * attr[renATTRR];
    rgb[1] = tex[0]->sample[renTEXG] * unif[renUNIFG] * attr[renATTRG];
    rgb[2] = tex[0]->sample[renTEXB] * unif[renUNIFB] * attr[renATTRB];
}

#include "050triangle.c"

texTexture *t[1];
texTexture t0;
renRenderer r;
int filter = 0;

int draw(int f) {
    /* initialize triangle coords, tex coords, and texTexture */
    double a[7] = {0, 250, .3, .7, 1, 1, 1}; double x[7] = {250, 500, .3, .7, 1, 1, 1};
    double b[7] = {125, 0, 0.5, .3, 1, 1, 1}; double y[7] = {375, 250, 0.5, .3, 1, 1, 1};
    double c[7] = {250, 250, .7, .7, 1, 1, 1}; double z[7] = {500, 500, .7, .7, 1, 1, 1};
    double unif[3] = {1, 1, 1};
    r.unifDim = 3; r.texNum = 1; r.attrDim = 4;
    if(texInitializeFile(&t0, "test2.png") != 0){
        return 1;
    }
    
    /* renders a triangle using quadratic texture filtering */
    texSetFiltering(&t0, f);
    triRender(unif, &r, a, b, c, t);

    /* renders a triangle using nearest neighbor texture filtering */
    texSetFiltering(&t0, 1 - f);
    triRender(unif, &r, x, y, z, t);
    return 0;
}

/* Called when a key comes up, used to swap texture filtures */
void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
    if(key == 257) {
        filter = 1 - filter;
        draw(filter);
    }
}

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        
        t[0] = &t0;
        
        /* draw stuff */
        draw(filter);
        
        /* so the enter key can switch texture filters */
        pixSetKeyUpHandler(handleKeyUp);
        
        /* start the event loop */
		pixRun();
        texDestroy(&t0);
		return 0;
	}
}