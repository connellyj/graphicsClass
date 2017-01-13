/* Julia Connelly and Kerim Celik, 01/10/2017 */

#include "000pixel.h"
#include "stb_image.h"
#include "030vector.c"
#include "030matrix.c"
#include "040texture.c"
#include "040triangle.c"

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        
        /* initialize triangle coords, tex coords, and texTexture */
        double a[2] = {0, 250}; double x[2] = {250, 500};
        double b[2] = {125, 0}; double y[2] = {375, 250};
        double c[2] = {250, 250}; double z[2] = {500, 500};
        double rgb[3] = {1, 1, 1};
        double alph[2] = {0, 1};
        double beta[2] = {0.5, 0};
        double gamm[2] = {1, 1};
        texTexture t;
        if(texInitializeFile(&t, "test.jpg") != 0){
            return 1;
        }
        
        /* renders a triangle using quadratic texture filtering */
        texSetFiltering(&t,0);
        triRender(a, b, c, rgb, alph, beta, gamm, t);
        
        /* renders a triangle using nearest neighbor texture filtering */
        texSetFiltering(&t,1);
        triRender(x, y, z, rgb, alph, beta, gamm, t);
        
        /* start the event loop */
		pixRun();
        texDestroy(&t);
		return 0;
	}
}