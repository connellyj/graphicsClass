/* On macOS, compile with...
    clang 500texturing.c -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <GLFW/glfw3.h>

#include "500shader.c"
#include "540texture.c"
#include "530vector.c"
#include "510mesh.c"
#include "520matrix.c"
#include "520camera.c"
#include "540scene.c"

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLdouble alpha = 0.0;
GLint attrLocs[2], textureLocs[1], unifLocs[1];
GLint viewingLoc, modelingLoc;
GLuint program;
texTexture texA, texB, texC;
sceneNode root, child;
meshGLMesh capsule, box;
camCamera cam;

int initializeMesh(void) {
	meshMesh mesh;
	if (meshInitializeCapsule(&mesh, 0.5, 2.0, 16, 32) != 0)
		return 1;
	meshGLInitialize(&capsule, &mesh);
	meshDestroy(&mesh);
	if (meshInitializeBox(&mesh, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5) != 0)
		return 2;
	meshGLInitialize(&box, &mesh);
	meshDestroy(&mesh);

	/* Initialize scene graph nodes. */
	if (sceneInitialize(&child, 3, 1, &capsule, NULL, NULL) != 0)
		return 4;
	if (sceneInitialize(&root, 3, 1, &box, &child, NULL) != 0)
		return 5;
	/* Customize the uniforms. */
	GLdouble trans[3] = {1.0, 0.0, 0.0};
	sceneSetTranslation(&child, trans);
	GLdouble unif[3] = {1.0, 1.0, 1.0};
	sceneSetUniform(&child, unif);
	sceneSetUniform(&root, unif);
	return 0;
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	/* What do the shaders do with the texture coordinates? */
	GLchar vertexCode[] = "\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		attribute vec3 position;\
		attribute vec3 color;\
		attribute vec2 texCoords;\
		varying vec4 rgba;\
		varying vec2 st;\
		void main() {\
			gl_Position = viewing * modeling * vec4(position, 1.0);\
			rgba = vec4(color, 1.0);\
			st = texCoords;\
		}";
	GLchar fragmentCode[] = "\
		uniform sampler2D texture;\
		varying vec4 rgba;\
		varying vec2 st;\
		void main() {\
			gl_FragColor = rgba * texture2D(texture, st);\
		}";
	program = makeProgram(vertexCode, fragmentCode);
	if (program != 0) {
		glUseProgram(program);
		attrLocs[0] = glGetAttribLocation(program, "position");
		unifLocs[0] = glGetAttribLocation(program, "color");
		viewingLoc = glGetUniformLocation(program, "viewing");
		modelingLoc = glGetUniformLocation(program, "modeling");
		attrLocs[1] = glGetAttribLocation(program, "texCoords");
		textureLocs[0] = glGetUniformLocation(program, "texture");
	}
	return (program == 0);
}

int initializeTex() {
    if (texInitializeFile(&texA, "cat.jpg", GL_LINEAR, GL_LINEAR, 
            GL_REPEAT, GL_REPEAT) != 0)
        return 1;


    if (texInitializeFile(&texB, "test.jpg", GL_LINEAR, GL_LINEAR, 
            GL_REPEAT, GL_REPEAT) != 0)
        return 2;

    if (texInitializeFile(&texC, "crown.jpg", GL_LINEAR, GL_LINEAR, 
            GL_REPEAT, GL_REPEAT) != 0)
        return 3;


    sceneSetOneTexture(&root, 0, &texA);
    sceneSetOneTexture(&root, 1, &texB);

    sceneSetOneTexture(&child, 0, &texA);
    sceneSetOneTexture(&child, 1, &texC);
    return 0;
}

void render(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
    camRender(&cam, viewingLoc);
    /* This animation code is different from that in 520mainCamera.c. */
	GLdouble rot[3][3], identity[4][4], axis[3] = {1.0, 1.0, 1.0};
	vecUnit(3, axis, axis);
	alpha += 0.01;
	mat33AngleAxisRotation(alpha, axis, rot);
	sceneSetRotation(&root, rot);
	sceneSetOneUniform(&root, 0, 0.5 + 0.5 * sin(alpha * 7.0));
    /* This rendering code is different from that in 520mainCamera.c. */
	mat44Identity(identity);
	GLuint unifDims[1] = {3};
	GLuint attrDims[2] = {3, 2};
	sceneRender(&root, identity, modelingLoc, 1, unifDims, unifLocs, 2, 
		attrDims, attrLocs, textureLocs);
}

int main(void) {
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    GLFWwindow *window;
    window = glfwCreateWindow(768, 512, "Texture Mapping", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwMakeContextCurrent(window);
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n", 
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);
    glDepthRange(1.0, 0.0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    
    initializeMesh();
    if (initializeTex() != 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 5;
    }
    if (initializeShaderProgram() != 0) {
    	glfwDestroyWindow(window);
        glfwTerminate();
        return 4;
    }
    while (glfwWindowShouldClose(window) == 0) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteProgram(program);
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

