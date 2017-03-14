/* Julia Connelly, Kerim Celik, 01/10/2017 */

#include "030triangle.c"
#include "000pixel.h"

int main(void) {
    /* initialize the window */
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
        /* reset the screen to black */
		pixClearRGB(0.0, 0.0, 0.0);
        
        /* draw the same triangle three times with the points in dif order */
        double a[2] = {50, 400};
        double b[2] = {150, 350};
        double c[2] = {100, 400};
        double rgb[3] = {1, 0, 0};
        triRender(a, b, c, rgb);
        pixSetRGB(50, 400, 1, 1, 1);
        pixSetRGB(150, 350, 1, 1, 1);
        pixSetRGB(100, 400, 1, 1, 1);
//        triRender(300, 350, 250, 450, 200, 400, 1, 1, 0);
//        triRender(400, 450, 350, 400, 450, 350, 0, 1, 0);
//        
//        /* draw the same triangle three times with the points in dif order */
//        triRender(50, 250, 100, 200, 150, 300, 0, 1, 1);
//        triRender(300, 300, 200, 250, 250, 200, 0, 0, 1);
//        triRender(400, 200, 450, 300, 350, 250, 1, 0, 1);
//        
//        /* draw a triangle with a horizontal side */
//        triRender(50, 150, 100, 50, 150, 150, 1, 1, 1);
//        
//        /* draw a triangle with a vertical side */
//        triRender(200, 150, 200, 50, 300, 100, 1, 1, 1);
//        
//        /* draw a line */
//        triRender(100.5, 100.5, 200.5, 200.5, 300.5, 300.5, 1, 1, 1);
        
        /* start the event loop */
		pixRun();
		return 0;
	}
}