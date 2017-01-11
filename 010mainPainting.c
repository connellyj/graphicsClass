/* In this tutorial our compiler invocation becomes more complicated. On macOS, 
we compile with...
    clang 000linking.c 000pixel.o -lglfw -framework OpenGL
This invocation causes our compiled code to be 'linked' with three other pieces 
of software, that have been compiled elsewhere: OpenGL, GLFW, and pixel.o. 
OpenGL is a cross-platform, standardized library for interacting with graphics 
hardware. It comes pre-installed on macOS and Linux, and is easily installed on 
Windows. GLFW is a cross-platform, minimalist user interface toolkit, that we 
can use to build simple OpenGL applications. Often it is not pre-installed, but 
it is easy to install. 000pixel.o is a library that I have written, to provide 
a simple foundation for this graphics course. You must have 000pixel.o and 
000pixel.h in the same directory as 000linking.c, for the linking to work. */

/* Notice that we include 000pixel.h. The file name is enclosed in quotation 
marks rather than angle brackets, to tell the C compiler to search for the file 
in the working directory. Feel free to read 000pixel.h. It documents all of the 
public functionality of the library 000pixel.o. */
#include <stdio.h>
#include <math.h>
#include "000pixel.h"

double red = 0.0;
double green = 0.0;
double blue = 0.0;
int brushSize = 2;
short paint = 0;

void setColorRed() {
    red = 1.0;
    green = 0.0;
    blue = 0.0;
}

void setColorBlue() {
    red = 0.0;
    green = 0.0;
    blue = 1.0;
}

void setColorGreen() {
    red = 0.0;
    green = 1.0;
    blue = 0.0;
}

void setColorCyan() {
    red = 0.0;
    green = 1.0;
    blue = 1.0;
}

void setColorMagenta() {
    red = 1.0;
    green = 0.0;
    blue = 1.0;
}

void setColorYellow() {
    red = 1.0;
    green = 1.0;
    blue = 0.0;
}

void setColorWhite() {
    red = 1.0;
    green = 1.0;
    blue = 1.0;
}

void setColorBlack() {
    red = 0.0;
    green = 0.0;
    blue = 0.0;
}

void resetWindow() {
    pixClearRGB(0.0, 0.0, 0.0);
}

void paintPixels(int x, int y) {
    int i;
    int j;
    for(i = x - (floor(brushSize / 2)); i < x + (floor(brushSize / 2)); i = i + 1) {
        for(j = y - (floor(brushSize / 2)); j < y + (floor(brushSize / 2)); j = j + 1) {
            pixSetRGB(i, j, red, green, blue);
        }
    }
}    

/* This function is a user interface 'callback'. Once we register it with the 
user interface infrastructure, it is called whenever the user releases a 
keyboard key. For details, see the pixSetKeyUpHandler documentation. */
void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
    if(key == 265) brushSize = brushSize + 1;
    if(key == 264 && brushSize > 0) brushSize = brushSize - 1;
    if(key == 32) resetWindow();
	if(key == 82) setColorRed();
    if(key == 71) setColorGreen();
    if(key == 66) setColorBlue();
    if(key == 67) setColorCyan();
    if(key == 77) setColorMagenta();
    if(key == 89) setColorYellow();
    if(key == 87) setColorWhite();
    if(key == 75) setColorBlack();
}

void handleMouseDown(int button, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	paint = 1;
}

/* Similarly, the following callbacks handle some mouse interactions. */
void handleMouseUp(int button, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	paint = 0;
}

void handleMouseMove(double x, double y) {
	if(paint == 1) paintPixels(x, y);
}

void handleMouseScroll(double xOffset, double yOffset) {
}

/* This callback is called once per animation frame. As parameters it receives 
the time for the current frame and the time for the previous frame. Both times 
are measured in seconds since some distant past time. */ 
void handleTimeStep(double oldTime, double newTime) {
}

/* You can also set callbacks for key down, key repeat, and mouse down. See 
000pixel.h for details. */

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function 
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		/* Register the callbacks (defined above) with the user interface, so 
		that they are called as needed during pixRun (invoked below). */
		pixSetKeyUpHandler(handleKeyUp);
		pixSetMouseUpHandler(handleMouseUp);
        pixSetMouseDownHandler(handleMouseDown);
		pixSetMouseMoveHandler(handleMouseMove);
		pixSetMouseScrollHandler(handleMouseScroll);
		pixSetTimeStepHandler(handleTimeStep);
		/* Clear the window to black. */
		pixClearRGB(0.0, 0.0, 0.0);
		/* Run the event loop. The callbacks that were registered above are 
		invoked as needed. At the end, the resources supporting the window are 
		deallocated. */
		pixRun();
		return 0;
	}
}

