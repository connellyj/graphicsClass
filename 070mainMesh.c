/* Julia Connelly and Kerim Celik, 01/15/2017 */

#define renATTRDIMBOUND 16

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include "000pixel.h"
#include "070vector.c"
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
    rgb[0] = tex[0]->sample[renTEXR] * unif[renUNIFR];
    rgb[1] = tex[0]->sample[renTEXG] * unif[renUNIFG];
    rgb[2] = tex[0]->sample[renTEXB] * unif[renUNIFB];
}

#include "050triangle.c"
#include "070mesh.c"

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
        meshInitializeEllipse(&m, 250, 250, 300, 150, 30);
        
        /* initialize triangle coords, tex coords, and texTexture */
        double unif[3] = {1, 1, 1};
        r.unifDim = 3; r.texNum = 1; r.attrDim = 4;
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