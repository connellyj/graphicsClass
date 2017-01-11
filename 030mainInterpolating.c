/* Julia Connelly and Kerim Celik, 01/10/2017 */

#include "000pixel.h"
#include "030vector.c"
#include "030matrix.c"
#include "030triangle.c"

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        
        /* draw rgb triangle */
        double a[2] = {50, 50};
        double b[2] = {250, 50};
        double c[2] = {150, 200};
        double rgb[3] = {1, 1, 1};
        double alph[3] = {1, 0, 0};
        double beta[3] = {0, 1, 0};
        double gamm[3] = {0, 0, 1};
        triRender(a, b, c, rgb, alph, beta, gamm);
        
        /* draw cmy triangle */
        double d[2] = {50, 250};
        double e[2] = {250, 250};
        double f[2] = {150, 400};
        double col[3] = {1, 1, 1};
        double alpha[3] = {0, 1, 1};
        double betaa[3] = {1, 0, 1};
        double gamma[3] = {1, 1, 0};
        triRender(d, e, f, col, alpha, betaa, gamma);
        
        /* draw cmy triangle blue shifted */
        double g[2] = {250, 125};
        double h[2] = {450, 125};
        double i[2] = {350, 275};
        double color[3] = {0, 0, 1};
        double al[3] = {0, 1, 1};
        double be[3] = {1, 0, 1};
        double ga[3] = {1, 1, 0};
        triRender(g, h, i, color, al, be, ga);
        
        /* start the event loop */
		pixRun();
		return 0;
	}
}