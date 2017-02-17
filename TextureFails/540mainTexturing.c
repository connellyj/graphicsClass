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

GLdouble alpha = 0.0;
GLuint program;
GLint viewingLoc, modelingLoc;
GLint attrLocs[3], textureLocs[2], unifLocs[1];
camCamera cam;
sceneNode root, child;
meshGLMesh capsule, box;
texTexture texA, texB, texC;

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    camSetWidthHeight(&cam, width, height);
}

void handleKey(GLFWwindow *window, int key, int scancode, int action,
		int mods) {
	int shiftIsDown = mods & GLFW_MOD_SHIFT;
	int controlIsDown = mods & GLFW_MOD_CONTROL;
	int altOptionIsDown = mods & GLFW_MOD_ALT;
	int superCommandIsDown = mods & GLFW_MOD_SUPER;
	if (action == GLFW_PRESS && key == GLFW_KEY_L) {
		camSwitchProjectionType(&cam);
	} else if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_O)
			camAddTheta(&cam, -0.1);
		else if (key == GLFW_KEY_P)
			camAddTheta(&cam, 0.1);
		else if (key == GLFW_KEY_I)
			camAddPhi(&cam, -0.1);
		else if (key == GLFW_KEY_K)
			camAddPhi(&cam, 0.1);
		else if (key == GLFW_KEY_U)
			camAddDistance(&cam, -0.1);
		else if (key == GLFW_KEY_J)
			camAddDistance(&cam, 0.1);
	}
}

int initializeScene(void) {
    /* Initialize meshes. */
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
	if (sceneInitialize(&child, 2, 2, &capsule, NULL, NULL) != 0)
		return 4;
	if (sceneInitialize(&root, 2, 2, &box, &child, NULL) != 0)
		return 5;
	/* Customize the uniforms. */
	GLdouble trans[3] = {1.0, 0.0, 0.0};
	sceneSetTranslation(&child, trans);
	GLdouble unif[2] = {1.0, 1.0};
	sceneSetUniform(&child, unif);
	sceneSetUniform(&root, unif);
	return 0;
}

void destroyScene(void) {
	meshGLDestroy(&box);
	meshGLDestroy(&capsule);
	sceneDestroyRecursively(&root);
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	GLchar vertexCode[] = "\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		attribute vec3 position;\
		attribute vec3 normal;\
		attribute vec2 texCoords;\
		uniform vec3 spice;\
		varying vec4 rgba;\
	    varying vec2 st;\
		void main() {\
			gl_Position = viewing * modeling * vec4(position, 1.0);\
			rgba = vec4(spice, 1.0) + vec4(normal, 1.0);\
			st = texCoords;\
		}";

	GLchar fragmentCode[] = "\
        uniform sampler2D texture;\
        uniform sampler2D textureB;\
		varying vec4 rgba;\
	    varying vec2 st;\
		void main() {\
			vec4 first, second;\
			first = texture2D(texture,st);\
			second = texture2D(textureB,st);\
			gl_FragColor = rgba * first * second;\
		}";
	program = makeProgram(vertexCode, fragmentCode);
	if (program != 0) {
		glUseProgram(program);
		attrLocs[0] = glGetAttribLocation(program, "position");
		attrLocs[1] = glGetAttribLocation(program, "texCoords");
		attrLocs[2] = glGetAttribLocation(program, "normal");
		viewingLoc = glGetUniformLocation(program, "viewing");
		modelingLoc = glGetUniformLocation(program, "modeling");
		unifLocs[0] = glGetUniformLocation(program, "spice");
		textureLocs[0] = glGetUniformLocation(program, "texture");
		textureLocs[1] = glGetUniformLocation(program, "textureB");
	}
	return (program == 0);
}

int initializeTex() {
    /* A 'texture unit' is a piece of machinery inside the GPU that performs 
    texture mapping. A GPU might have many texture units, allowing you to map 
    many textures onto your meshes in complicated ways. The glActiveTexture 
    function selects which texture unit is affected by subsequent OpenGL calls. 
    In this tutorial, we use only texture unit 0, so we activate it here, once 
    and for all. */

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
	/* This part is the same as in 520mainCamera.c. */
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
    window = glfwCreateWindow(512, 512, "Textures", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwSetKeyCallback(window, handleKey);
    glfwMakeContextCurrent(window);
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n", 
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);
    glDepthRange(1.0, 0.0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    
    if (initializeScene() != 0){
        return 3;
    }
    
    /*texTexture tiger, huskies, crown, pattern;
    if (texInitializeFile(&tiger, "cat.jpg", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT) != 0) {
    	glfwDestroyWindow(window);
        glfwTerminate();
        return 3;
    }
    if (texInitializeFile(&huskies, "test.jpg", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT) != 0) {
    	glfwDestroyWindow(window);
        glfwTerminate();
        return 3;
    }
    if (texInitializeFile(&pattern, "test2.png", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT) != 0) {
    	glfwDestroyWindow(window);
        glfwTerminate();
        return 3;
    }
    if (texInitializeFile(&crown, "crown.jpg", GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT) != 0) {
    	glfwDestroyWindow(window);
        glfwTerminate();
        return 3;
    }
    texTexture *rootTex[2];
    texTexture *childTex[2];
    rootTex[0] = &tiger;
    rootTex[1] = &huskies;
    childTex[0] = &pattern;
    childTex[1] = &crown;
    sceneSetOneTexture(&root, 0, &tiger);
    sceneSetOneTexture(&root, 1, &pattern);
    sceneSetOneTexture(&child, 0, &huskies);
    sceneSetOneTexture(&child, 1, &crown);*/
    
    
    if (initializeShaderProgram() != 0) {
    	glfwDestroyWindow(window);
        glfwTerminate();
        return 4;
    }
    
    if (initializeTex() != 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 5;
    }
    GLdouble target[3] = {0.0, 0.0, 0.0};
	camSetControls(&cam, camPERSPECTIVE, M_PI / 6.0, 10.0, 512.0, 512.0, 10.0, 
		M_PI / 4.0, M_PI / 4.0, target);
    while (glfwWindowShouldClose(window) == 0) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // how destroy multiple?
    //glDeleteTextures(1, &texture);
    glDeleteProgram(program);
    destroyScene();
	glfwDestroyWindow(window);
    glfwTerminate();
    // TEX DESTROY
    // destory both types of meshes
    // scene destroy
    return 0;
}

