/* On macOS, compile with...
    clang++ mainTexturingPhysics.c -lglfw -framework OpenGL -l ode
*/

/* START PHYSICS CHUNK */
#include <ode/ode.h>

typedef struct {
  dBodyID body;
  dGeomID geom;
} MyObject;

static dWorldID world;
MyObject ball;
const dReal   radius = 1.0;
const dReal   mass   = 1.0;
dReal gravity = 0.0;

static dSpaceID space;
static dGeomID  ground;
static dJointGroupID contactgroup;
static int flag = 0;
/* END PHYSICS CHUNK */

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
#include "scenePhysics.c"

GLdouble alpha = 0.0;
GLuint program;
GLint attrLocs[3];
GLint viewingLoc, modelingLoc;
GLint unifLocs[1];
GLint textureLocs[2];
camCamera cam;
/* Allocate three meshes and three scene graph nodes. */
meshGLMesh rootMesh, collideTestMesh;
sceneNode rootNode, collideTestNode;
texTexture tiger, pattern;

/* START PHYSICS CHUNK */
static void nearCallback(void *data, dGeomID o1, dGeomID o2) {
  const int N = 10;
  dContact contact[N];

  int isGround = ((ground == o1) || (ground == o2));

  int n =  dCollide(o1,o2,N,&contact[0].geom,sizeof(dContact));

  if (isGround)  {
    if (n >= 1) flag = 1;
    else        flag = 0;
    for (int i = 0; i < n; i++) {
      contact[i].surface.mode = dContactBounce;
      contact[i].surface.mu   = dInfinity;
      contact[i].surface.bounce     = 0.9; // (0.0~1.0) restitution parameter
      contact[i].surface.bounce_vel = 0.0; // minimum incoming velocity for bounce
      dJointID c = dJointCreateContact(world,contactgroup,&contact[i]);
      dJointAttach (c,dGeomGetBody(contact[i].geom.g1),dGeomGetBody(contact[i].geom.g2));
    }
  }
}

static void simLoop ()
{
    const dReal *pos,*R;
    
    dSpaceCollide(space,0,&nearCallback);
    
    dWorldStep(world,0.05);
    
    dJointGroupEmpty(contactgroup);
    
    pos = dBodyGetPosition(ball.body);
    R   = dBodyGetRotation(ball.body);
    
    GLdouble trans[3];
    GLdouble rot[9];
    for(int i = 0; i < 3; i++) {
        trans[i] = (GLdouble)pos[i];
        rot[i] = (GLdouble)R[i];
    }
    
    sceneSetTranslation(&rootNode, trans);
    sceneSetRotationArray(&rootNode, rot);
}
/* END PHYSICS CHUNK */

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
        else if (key == GLFW_KEY_Z){
            gravity -= 0.05;
            dWorldSetGravity(world,0,gravity,0);
        }else if (key == GLFW_KEY_A){
            gravity += 0.05;
            dWorldSetGravity(world,0,gravity,0);
        }
	}
}

/* Returns 0 on success, non-zero on failure. Warning: If initialization fails 
midway through, then does not properly deallocate all resources. But that's 
okay, because the program terminates almost immediately after this function 
returns. */
int initializeScene(void) {
	/* Initialize meshes. */
	meshMesh mesh;
	if (meshInitializeSphere(&mesh, 1.0, 20, 40) != 0)
		return 1;
	meshGLInitialize(&rootMesh, &mesh);
	meshDestroy(&mesh);
	if (meshInitializeBox(&mesh, -2.5, 2.5, -0.05, 0.05, -2.5, 2.5) != 0)
		return 1;
	meshGLInitialize(&collideTestMesh, &mesh);
	meshDestroy(&mesh);
	/* Initialize scene graph nodes. */
	if (sceneInitialize(&rootNode, 2, 2, &rootMesh, NULL, &collideTestNode) != 0)
		return 2;
    if (sceneInitialize(&collideTestNode, 2, 2, &collideTestMesh, NULL, NULL) != 0)
		return 2;
	/* Customize the uniforms. */
	GLdouble unif[2] = {1.0, 1.0};
	sceneSetUniform(&rootNode, unif);
    sceneSetUniform(&collideTestNode, unif);
    GLdouble trans[3] = {0, -2, 0};
    sceneSetTranslation(&collideTestNode, trans);
	return 0;
}

int initializeTex() {
    char cat[] = "cat.jpg";
    char *img = cat;
    if(texInitializeFile(&tiger, img, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP) != 0) {
        return 1;
    }
    char test2[] = "test2.png";
    img = test2;
    if(texInitializeFile(&pattern, img, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP) != 0) {
        return 4;
    }
    sceneSetOneTexture(&rootNode, 0, &tiger);
    sceneSetOneTexture(&rootNode, 1, &pattern);
    sceneSetOneTexture(&collideTestNode, 0, &pattern);
    sceneSetOneTexture(&collideTestNode, 1, &pattern);
    return 0;
}

void destroyScene(void) {
    texDestroy(&tiger);
    texDestroy(&pattern);
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
		uniform vec2 spice;\
		varying vec4 rgba;\
        varying vec2 st;\
		void main() {\
			gl_Position = viewing * modeling * vec4(position, 1.0);\
			rgba = vec4(texCoords, spice) + vec4(normal, 1.0);\
            st = texCoords;\
		}";
	GLchar fragmentCode[] = "\
        uniform sampler2D textureA;\
        uniform sampler2D textureB;\
		varying vec4 rgba;\
        varying vec2 st;\
		void main() {\
			gl_FragColor = rgba * texture2D(textureA, st) * texture2D(textureB, st);\
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
        textureLocs[0] = glGetUniformLocation(program, "textureA");
        textureLocs[1] = glGetUniformLocation(program, "textureB");
	}
	return (program == 0);
}

void render(void) {
	/* This part is the same as in 520mainCamera.c. */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	camRender(&cam, viewingLoc);
	/* This animation code is different from that in 520mainCamera.c. */
	GLdouble rot[3][3], identity[4][4], axis[3] = {1.0, 1.0, 1.0};
	vecUnit(3, axis, axis);
	//alpha += 0.01;
	mat33AngleAxisRotation(alpha, axis, rot);
	sceneSetRotation(&rootNode, rot);
	sceneSetOneUniform(&rootNode, 0, 0.5 + 0.5 * sin(alpha * 7.0));
	/* This rendering code is different from that in 520mainCamera.c. */
	mat44Identity(identity);
	GLuint unifDims[1] = {2};
	GLuint attrDims[3] = {3, 2, 3};
	sceneRender(&rootNode, identity, modelingLoc, 1, unifDims, unifLocs, 3, 
		attrDims, attrLocs, textureLocs);
}


int main(void) {
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    GLFWwindow *window;
    window = glfwCreateWindow(512, 512, "Scene Graph", NULL, NULL);
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
    if(initializeTex() != 0)
        return 4;
    if (initializeShaderProgram() != 0)
    	return 4;
    GLdouble target[3] = {0.0, 0.0, 0.0};
	camSetControls(&cam, camPERSPECTIVE, M_PI / 6.0, 10.0, 512.0, 512.0, 10.0, 
		M_PI / 4.0, M_PI / 4.0, target);
    GLdouble trans[3] = {0, 0, 50};
    camSetTranslation(&cam, trans);
    GLdouble id[3][3];
    mat33Identity(id);
    camSetRotation(&cam, id);
    
    /* START PHYSICS CHUNK */
    dReal x0 = 0.0, y0 = 0.0, z0 = 1.0;
    dMass m1;

    dInitODE();
    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    contactgroup = dJointGroupCreate(0);
    dWorldSetGravity(world,0,gravity,0);

    ground = dCreateBox(space, 5, 0.1, 5);
    dGeomSetPosition(ground, 0, -2, 0);
    
    ball.body = dBodyCreate(world);
    dMassSetZero(&m1);
    dMassSetSphereTotal(&m1,mass,radius);
    dBodySetMass(ball.body,&m1);
    dBodySetPosition(ball.body, 0, 6, 0);
    dBodySetForce(ball.body, 0, -0.5, 0);
    
    ball.geom = dCreateSphere(space,radius);
    dGeomSetBody(ball.geom,ball.body);
    /* END PHYSICS CHUNK */
    
    while (glfwWindowShouldClose(window) == 0) {
        simLoop();
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteProgram(program);
    /* Don't forget to destroy the whole scene. */
    destroyScene();
	glfwDestroyWindow(window);
    glfwTerminate();
    
    /* START PHYSICS CHUNK */
    dWorldDestroy (world);
    dCloseODE();
    /* END PHYSICS CHUNK */
    return 0;
}

