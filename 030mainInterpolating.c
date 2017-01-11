#include "030triangle.c"

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        
        /* draw the same triangle three times with the points in dif order */
        double a[2] = {50, 50};
        double b[2] = {450, 50};
        double c[2] = {250, 450};
        double rgb[3] = {1, 1, 1};
        double alph[3] = {1, 0, 0};
        double beta[3] = {0, 1, 0};
        double gamm[3] = {0, 0, 1};
        triRender(a, b, c, rgb, alph, beta, gamm);
        
        /* start the event loop */
		pixRun();
		return 0;
	}
}