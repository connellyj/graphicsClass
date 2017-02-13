/* On macOS, compile with...
    clang 500glfw.c -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <GLFW/glfw3.h>

/* This is a callback, which we register with GLFW below. It is automatically 
called whenever GLFW throws an error. */
void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

/* This is another callback, that is automatically called when the GLFW window 
is resized. */
void handleResize(GLFWwindow *window, int width, int height) {
    /* Tell OpenGL to update the viewport transformation. By the way, all 
    OpenGL function names begin with 'gl' (and not 'glfw'). */
    glViewport(0, 0, width, height);
}

/* This helper function uses OpenGL to render the window. */
void render(void) {
	/* Clear the OpenGL context to black, before we start rendering. By the 
	way, all OpenGL constant names begin with 'GL_'. */
	glClear(GL_COLOR_BUFFER_BIT);
	/* OpenGL's projection matrix corresponds to the matrix P in our software 
	graphics engine. Here we set an orthographic projection, in terms of left, 
	right, bottom, top, near, and far parameters.  Warning: glOrtho uses 
	different conventions for near and far, than we used in our engine. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	/* This is the simplest (and worst) way to draw a triangle in OpenGL. */
	glBegin(GL_TRIANGLES);
	glColor3f(0.0, 0.0, 1.0);
	glVertex2f(0.1, 0.1);
    glColor3f(0.0, 1.0, 0.2);
	glVertex2f(0.9, 0.2);
	glVertex2f(0.6, 0.9);
	glEnd();
}

int main(void) {
    /* Before calling any other GLFW functions, call glfwSetErrorCallback and 
    glfwInit. By the way, all GLFW function names begin with 'glfw'. */
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    /* Create a GLFW window and register some more callbacks. Read the GLFW 
    documentation to learn about keyboard, mouse, and other callbacks. */
    GLFWwindow *window;
    window = glfwCreateWindow(768, 512, "Learning GLFW", NULL, NULL);
    if (window == NULL) {
    	/* Creating the window failed. Clean up GLFW and abandon the program. */
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    /* A OpenGL 'context' is an instance of the OpenGL system. Each GLFW window 
    comes with an OpenGL context. The following call makes the window's OpenGL 
    context 'current' ---- meaning, the target for future OpenGL calls. As an 
    aside, ask OpenGL for its version. */
    glfwMakeContextCurrent(window);
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n", 
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
	/* Run the user interface loop, until the user closes the window. */
    while (glfwWindowShouldClose(window) == 0) {
    	/* Even though there is no animation or user interaction, other than 
    	resizing the window, we'll redraw the window on every frame, for 
    	simplicity. (000pixel.o redraws only when it's needed.) */
        render();
        /* GLFW windows are double-buffered. All of your OpenGL rendering goes 
        into the hidden 'back' buffer. Then you swap the buffers, to copy the 
        back buffer to the 'front' buffer, which the user sees. */
        glfwSwapBuffers(window);
        /* Process any pending user interface events, using the callbacks. */
        glfwPollEvents();
    }
	/* Always clean up GLFW like this, when you're finished. */
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

