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
GLint attrLocs[3];
GLint viewingLoc, modelingLoc;
GLint textureLoc[1];
GLint unifLocs[2];
camCamera cam;
/* Allocate three meshes and three scene graph nodes. */
meshGLMesh rootMesh, childMesh, siblingMesh;
sceneNode rootNode, childNode, siblingNode;
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

/* Returns 0 on success, non-zero on failure. Warning: If initialization fails 
midway through, then does not properly deallocate all resources. But that's 
okay, because the program terminates almost immediately after this function 
returns. */
int initializeScene(void) {
	/* Initialize meshes. */
	meshMesh mesh;
	if (meshInitializeCapsule(&mesh, 0.5, 2.0, 16, 32) != 0)
		return 1;
	meshGLInitialize(&rootMesh, &mesh);
	meshDestroy(&mesh);
	if (meshInitializeBox(&mesh, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5) != 0)
		return 2;
	meshGLInitialize(&childMesh, &mesh);
	meshDestroy(&mesh);
	if (meshInitializeSphere(&mesh, 0.5, 16, 32) != 0)
		return 3;
	meshGLInitialize(&siblingMesh, &mesh);
	meshDestroy(&mesh);

	/* Initialize scene graph nodes. */
	if (sceneInitialize(&siblingNode, 5, 1, &siblingMesh, NULL, NULL) != 0)
		return 4;
	if (sceneInitialize(&childNode, 5, 1, &childMesh, NULL, &siblingNode) != 0)
		return 5;
	if (sceneInitialize(&rootNode, 5, 1, &rootMesh, &childNode, NULL) != 0)
		return 6;
	/* Customize the uniforms. */
	GLdouble trans[3] = {1.0, 0.0, 0.0};
	sceneSetTranslation(&childNode, trans);
	vecSet(3, trans, 0.0, 1.0, 0.0);
	sceneSetTranslation(&siblingNode, trans);
	GLdouble unifRoot[5] = {1.0, 1.0, 1.0, 1.0, 1.0};
    GLdouble unifOther[5] = {1.0, 1.0, 0.3, 0.3, 0.3};
	sceneSetUniform(&siblingNode, unifOther);
	sceneSetUniform(&childNode, unifOther);
	sceneSetUniform(&rootNode, unifRoot);
	return 0;
}

void destroyScene(void) {
	meshGLDestroy(&siblingMesh);
	meshGLDestroy(&childMesh);
	meshGLDestroy(&rootMesh);
	sceneDestroyRecursively(&rootNode);
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	GLchar vertexCode[] = "\
    uniform mat4 viewing;\
    uniform mat4 modeling;\
    attribute vec3 position;\
    attribute vec2 texCoords;\
    attribute vec3 normal;\
    varying vec3 fragPos;\
    varying vec3 normalDir;\
    varying vec2 st;\
    void main() {\
        vec4 worldPos = modeling * vec4(position, 1.0);\
        gl_Position = viewing * worldPos;\
        fragPos = vec3(worldPos);\
        normalDir = vec3(modeling * vec4(normal, 0.0));\
        st = texCoords;\
    }";
    GLchar fragmentCode[] = "\
        uniform sampler2D texture0;\
        uniform vec3 specular;\
        uniform vec3 camPos;\
        uniform vec3 lightPos;\
        uniform vec3 lightCol;\
        uniform vec3 lightAtt;\
        varying vec3 fragPos;\
        varying vec3 normalDir;\
        varying vec2 st;\
        void main() {\
            vec3 surfCol = vec3(texture2D(texture0, st));\
            vec3 norDir = normalize(normalDir);\
            vec3 litDir = normalize(lightPos - fragPos);\
            vec3 camDir = normalize(camPos - fragPos);\
            vec3 refDir = 2.0 * dot(litDir, norDir) * norDir - litDir;\
            float d = distance(lightPos, fragPos);\
            float a = lightAtt[0] + lightAtt[1] * d + lightAtt[2] * d * d;\
            float diffInt = dot(norDir, litDir) / a;\
            float specInt = dot(refDir, camDir);\
            if (diffInt <= 0.0 || specInt <= 0.0)\
                specInt = 0.0;\
            float ambInt = 0.1;\
            if (diffInt <= ambInt)\
                diffInt = ambInt;\
            vec3 diffLight = diffInt * lightCol * surfCol;\
            float shininess = 64.0;\
            vec3 specLight = pow(specInt / a, shininess) * lightCol * specular;\
            gl_FragColor = vec4(diffLight + specLight, 1.0);\
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
		textureLoc[0] = glGetUniformLocation(program, "texture0");
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


    sceneSetOneTexture(&rootNode, 0, &texA);
    sceneSetOneTexture(&childNode, 0, &texB);
    sceneSetOneTexture(&siblingNode, 0, &texC);

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
	sceneSetRotation(&rootNode, rot);
	sceneSetOneUniform(&rootNode, 0, 0.5 + 0.5 * sin(alpha * 7.0));
	/* This rendering code is different from that in 520mainCamera.c. */
	mat44Identity(identity);
	GLuint unifDims[2] = {2, 3};
	GLuint attrDims[3] = {3, 2, 3};
	sceneRender(&rootNode, identity, modelingLoc, 2, unifDims, unifLocs, 3, 
		attrDims, attrLocs, textureLoc);
}

int main(void) {
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    GLFWwindow *window;
    window = glfwCreateWindow(512, 512, "Texture Mapping", NULL, NULL);
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
    /* Initialize a whole scene, rather than just one mesh. */
    if (initializeScene() != 0)
    	return 3;
    if (initializeShaderProgram() != 0)
    	return 4;

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
    glDeleteProgram(program);
    /* Don't forget to destroy the whole scene. */
    // glDeleteTextures(1, 0);??
    texDestroy(&texA);
    texDestroy(&texB);
    texDestroy(&texC);
    destroyScene();
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}