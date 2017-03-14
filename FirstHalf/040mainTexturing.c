/* Julia Connelly and Kerim Celik, 01/12/2017 */

#include "000pixel.h"
#include "stb_image.h"
#include "030vector.c"
#include "030matrix.c"
#include "040texture.c"
#include "040triangle.c"

texTexture t;
int filter = 0;

int draw(texTexture t, int f) {
    /* initialize triangle coords, tex coords, and texTexture */
    double a[2] = {0, 250}; double x[2] = {250, 500};
    double b[2] = {125, 0}; double y[2] = {375, 250};
    double c[2] = {250, 250}; double z[2] = {500, 500};
    double rgb[3] = {1, 1, 1};
    double alph[2] = {.3, .7};
    double beta[2] = {0.5, .3};
    double gamm[2] = {.7, .7};
    if(texInitializeFile(&t, "test2.png") != 0){
        return 1;
    }

    /* renders a triangle using quadratic texture filtering */
    texSetFiltering(&t, f);
    triRender(a, b, c, rgb, alph, beta, gamm, t);

    /* renders a triangle using nearest neighbor texture filtering */
    texSetFiltering(&t, 1 - f);
    triRender(x, y, z, rgb, alph, beta, gamm, t);
    return 0;
}

/* Called when a key comes up, used to swap texture filtures */
void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
    if(key == 257) {
        filter = 1 - filter;
        draw(t, filter);
    }
}

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        
        /* draw stuff */
        draw(t, filter);
        
        /* so the enter key can switch texture filters */
        pixSetKeyUpHandler(handleKeyUp);
        
        /* start the event loop */
		pixRun();
        texDestroy(&t);
		return 0;
	}
}