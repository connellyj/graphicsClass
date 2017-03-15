/*** Written by Julia Connelly and Kerim Celik, 03/15/2017 ***/


/* On macOS, compile with...
    clang++ 1020mainJoints.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -l ode
*/

/* Demo using core API functions
 * to show relationship between two physics objects that 
 * are parent-child in the scene graph
 */

#include "1010physics.c"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <sys/time.h>

double getTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

#include "1000shader.c"
#include "1000vector.c"
#include "1000mesh.c"
#include "1000matrix.c"
#include "1000camera.c"
#include "1000texture.c"
#include "1020scene.c"
#include "1000light.c"
#include "1000shadow.c"

camCamera cam;
texTexture texW, texAB, texC1, texC3, texC4, texD, texE1, texE3, texE4, texRed;
meshGLMesh meshW, meshC1, meshC3, meshC4, meshE1, meshE3, meshE4, meshRed, meshRed1, meshRed2;
sceneNode nodeW, nodeC1, nodeC3, nodeC4, nodeE1, nodeE3, nodeE4, nodeRed, nodeRed1, nodeRed2;
/* We need just one shadow program, because all of our meshes have the same 
attribute structure. */
shadowProgram sdwProg;
/* We need one shadow map per shadow-casting light. */
lightLight light, lightStatic;
shadowMap sdwMap, sdwMapStatic;
/* The main shader program has extra hooks for shadowing. */
GLuint program;
GLint viewingLoc, modelingLoc;
GLint unifLocs[1], textureLocs[1];
GLint attrLocs[3];
GLint lightPosLoc, lightColLoc, lightAttLoc, lightDirLoc, lightCosLoc;
GLint staticPosLoc, staticColLoc, staticAttLoc, staticDirLoc, staticCosLoc;
GLint camPosLoc;
GLint viewingSdwLoc, textureSdwLoc;
GLint viewingSdwStaticLoc, textureSdwStaticLoc;
dReal sliderForce = -1.0;
dReal hingeDir = -1.0;

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
        if(key == GLFW_KEY_O) {
            hingeDir *= -1;
            pSetHingeJointParams(nodeE4.jointLoc, -2*M_PI, 2*M_PI, hingeDir, 15.0);
        }else if(key == GLFW_KEY_P) {
            sliderForce *= -1;
        }
		if (key == GLFW_KEY_A)
			camAddTheta(&cam, -0.1);
		else if (key == GLFW_KEY_D)
			camAddTheta(&cam, 0.1);
		else if (key == GLFW_KEY_W)
			camAddPhi(&cam, -0.1);
		else if (key == GLFW_KEY_S)
			camAddPhi(&cam, 0.1);
		else if (key == GLFW_KEY_Q)
			camAddDistance(&cam, -1);
		else if (key == GLFW_KEY_E)
			camAddDistance(&cam, 1);
		else if (key == GLFW_KEY_T) {
			GLdouble vec[3];
			vecCopy(3, light.translation, vec);
			vec[1] += 1.0;
			lightSetTranslation(&light, vec);
		} else if (key == GLFW_KEY_G) {
			GLdouble vec[3];
			vecCopy(3, light.translation, vec);
			vec[1] -= 1.0;
			lightSetTranslation(&light, vec);
		}
		else if (key == GLFW_KEY_H) {
			GLdouble vec[3];
			vecCopy(3, light.translation, vec);
			vec[0] += 1.0;
			lightSetTranslation(&light, vec);
		} else if (key == GLFW_KEY_F) {
			GLdouble vec[3];
			vecCopy(3, light.translation, vec);
			vec[0] -= 1.0;
			lightSetTranslation(&light, vec);
		}
	}
}

/* Returns 0 on success, non-zero on failure. Warning: If initialization fails 
midway through, then does not properly deallocate all resources. But that's 
okay, because the program terminates almost immediately after this function 
returns. */
int initializeScene(void) {
    char cubes[] = "snowcliff.jpg";
	if (texInitializeFile(&texAB, cubes, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 1;
    char ground[] = "snowygrass.jpg";
	if (texInitializeFile(&texW, ground, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 2;
	char board[] = "board.jpg";
	if (texInitializeFile(&texC1, board, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 11;
    char iron[] = "iron.png";
    if (texInitializeFile(&texC3, iron, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 3;
    char trunk[] = "trunk.jpg";
    if (texInitializeFile(&texD, trunk, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 4;
    char plank[] = "grass.png";
    if (texInitializeFile(&texC4, plank, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 5;
    char chick[] = "chicken.jpg";
    if (texInitializeFile(&texE1, chick, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 6;
    char barber[] = "barber.jpg";
    if (texInitializeFile(&texE3, barber, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 8;
    char web[] = "leaves.jpg";
    if (texInitializeFile(&texE4, web, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 9;
    char red[] = "red.jpg";
    if (texInitializeFile(&texRed, red, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 91;
	GLuint attrDims[3] = {3, 2, 3};
	meshMesh mesh;
    if (meshInitializeBox(&mesh, -3.0, 3.0, -3.0, 3.0, -3.0, 3.0) != 0)
		return 10;
    meshGLInitialize(&meshC1, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshC1, 0, attrLocs);
	meshGLVAOInitialize(&meshC1, 1, sdwProg.attrLocs);
    meshGLInitialize(&meshC3, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshC3, 0, attrLocs);
	meshGLVAOInitialize(&meshC3, 1, sdwProg.attrLocs);
    meshGLInitialize(&meshC4, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshC4, 0, attrLocs);
	meshGLVAOInitialize(&meshC4, 1, sdwProg.attrLocs);
    meshGLInitialize(&meshE1, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshE1, 0, attrLocs);
	meshGLVAOInitialize(&meshE1, 1, sdwProg.attrLocs);
    meshGLInitialize(&meshE3, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshE3, 0, attrLocs);
	meshGLVAOInitialize(&meshE3, 1, sdwProg.attrLocs);
    meshGLInitialize(&meshE4, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshE4, 0, attrLocs);
	meshGLVAOInitialize(&meshE4, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
    if (meshInitializeSphere(&mesh, 1.0, 20, 40) != 0)
		return 10;
    meshGLInitialize(&meshRed, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshRed, 0, attrLocs);
	meshGLVAOInitialize(&meshRed, 1, sdwProg.attrLocs);
    if (meshInitializeCapsule(&mesh, 1.0, 12.0, 20, 40) != 0)
		return 10;
    meshGLInitialize(&meshRed1, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshRed1, 0, attrLocs);
	meshGLVAOInitialize(&meshRed1, 1, sdwProg.attrLocs);
    meshGLInitialize(&meshRed2, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshRed2, 0, attrLocs);
	meshGLVAOInitialize(&meshRed2, 1, sdwProg.attrLocs);
    meshDestroy(&mesh);
    if (meshInitializeBox(&mesh, -30.0, 30.0, -30.0, 30.0, -5.0, 5.0) != 0)
		return 10;
    meshGLInitialize(&meshW, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshW, 0, attrLocs);
	meshGLVAOInitialize(&meshW, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
    if (sceneInitialize(&nodeW, 3, 1, &meshW, NULL, &nodeC1) != 0)
		return 14;
    if (sceneInitialize(&nodeC1, 3, 1, &meshC1, NULL, &nodeC3) != 0)
		return 41;
	if (sceneInitialize(&nodeC3, 3, 1, &meshC3, NULL, &nodeC4) != 0)
		return 16;
	if (sceneInitialize(&nodeC4, 3, 1, &meshC4, NULL, &nodeE1) != 0)
		return 15;
	if (sceneInitialize(&nodeE1, 3, 1, &meshE1, NULL, &nodeE3) != 0)
		return 13;
    if (sceneInitialize(&nodeE3, 3, 1, &meshE3, NULL, &nodeE4) != 0)
		return 21;
    if (sceneInitialize(&nodeE4, 3, 1, &meshE4, NULL, &nodeRed) != 0)
		return 22;
    if (sceneInitialize(&nodeRed, 3, 1, &meshRed, NULL, &nodeRed1) != 0)
		return 22;
    if (sceneInitialize(&nodeRed1, 3, 1, &meshRed1, NULL, &nodeRed2) != 0)
		return 22;
    if (sceneInitialize(&nodeRed2, 3, 1, &meshRed2, NULL, NULL) != 0)
		return 22;
	GLdouble trans[3] = {30.0, 30.0, 0.0};
	sceneSetTranslation(&nodeW, trans);
	vecSet(3, trans, 15.0, 15.0, 12.0);
	sceneSetTranslation(&nodeC4, trans);
    vecSet(3, trans, 15.0, 30.0, 11.0);
	sceneSetTranslation(&nodeE4, trans);
    vecSet(3, trans, 20.0, 45.0, 12.0);
	sceneSetTranslation(&nodeC1, trans);
    vecSet(3, trans, 45.0, 15.0, 11.0);
	sceneSetTranslation(&nodeE3, trans);
    vecSet(3, trans, 40.0, 45.0, 11.0);
	sceneSetTranslation(&nodeE1, trans);
    vecSet(3, trans, 45.0, 15.0, 31.0);
	sceneSetTranslation(&nodeC3, trans);
    vecSet(3, trans, 45.0, 15.0, 21.0);
	sceneSetTranslation(&nodeRed, trans);
    vecSet(3, trans, 15.0, 22.5, 11.0);
	sceneSetTranslation(&nodeRed1, trans);
    vecSet(3, trans, 30.0, 45.0, 11.0);
	sceneSetTranslation(&nodeRed2, trans);
    GLdouble rotate[3][3]; GLdouble axis[3] = {0.0, 1.0, 0.0};
    mat33AngleAxisRotation(0.5 * M_PI, axis, rotate);
    sceneSetRotation(&nodeRed2, rotate);
    
	GLdouble unif[3] = {0.0, 0.0, 0.0};
    sceneSetUniform(&nodeC1, unif);
	sceneSetUniform(&nodeC3, unif);
	sceneSetUniform(&nodeC4, unif);
    sceneSetUniform(&nodeE1, unif);
	sceneSetUniform(&nodeE3, unif);
	sceneSetUniform(&nodeE4, unif);
    sceneSetUniform(&nodeRed, unif);
    sceneSetUniform(&nodeRed1, unif);
    sceneSetUniform(&nodeRed2, unif);
	vecSet(3, unif, 1.0, 1.0, 1.0);
	sceneSetUniform(&nodeW, unif);
	texTexture *tex;
	tex = &texC1;
	sceneSetOneTexture(&nodeC1, 0, tex);
	tex = &texW;
	sceneSetOneTexture(&nodeW, 0, tex);
	tex = &texC3;
	sceneSetOneTexture(&nodeC3, 0, tex);
	tex = &texC4;
	sceneSetOneTexture(&nodeC4, 0, tex);
    tex = &texE1;
    sceneSetOneTexture(&nodeE1, 0, tex);
    tex = &texE3;
	sceneSetOneTexture(&nodeE3, 0, tex);
	tex = &texE4;
	sceneSetOneTexture(&nodeE4, 0, tex);
    tex = &texRed;
	sceneSetOneTexture(&nodeRed, 0, tex);
    sceneSetOneTexture(&nodeRed1, 0, tex);
    sceneSetOneTexture(&nodeRed2, 0, tex);
	return 0;
}

void destroyScene(void) {
	texDestroy(&texC1);
	texDestroy(&texC3);
	texDestroy(&texC4);
	texDestroy(&texW);
    texDestroy(&texE1);
    texDestroy(&texE3);
    texDestroy(&texE4);
    texDestroy(&texRed);
	meshGLDestroy(&meshW);
	meshGLDestroy(&meshC1);
	meshGLDestroy(&meshC3);
	meshGLDestroy(&meshC4);
    meshGLDestroy(&meshE1);
    meshGLDestroy(&meshE3);
    meshGLDestroy(&meshE4);
    meshGLDestroy(&meshRed);
    meshGLDestroy(&meshRed1);
    meshGLDestroy(&meshRed2);
	sceneDestroyRecursively(&nodeW);
    pPhysicsDestroy();
}

/* Returns 0 on success, non-zero on failure. Warning: If initialization fails 
midway through, then does not properly deallocate all resources. But that's 
okay, because the program terminates almost immediately after this function 
returns. */
int initializeCameraLight(void) {
    GLdouble vec[3] = {30.0, 30.0, 5.0};
	camSetControls(&cam, camPERSPECTIVE, M_PI / 6.0, 10.0, 768.0, 768.0, 250.0, 
		M_PI / 4.0, M_PI / 4.0, vec);
	lightSetType(&light, lightSPOT);
	vecSet(3, vec, 45.0, 30.0, 20.0);
	lightShineFrom(&light, vec, M_PI * 3.0 / 4.0, M_PI * 3.0 / 4.0);
	vecSet(3, vec, 1.0, 1.0, 1.0);
	lightSetColor(&light, vec);
	vecSet(3, vec, 1.0, 0.0, 0.0);
	lightSetAttenuation(&light, vec);
	lightSetSpotAngle(&light, M_PI / 3.0);
    
    /* the other light */
    GLdouble vec1[3] = {45.0, 30.0, 20.0};
	lightSetType(&lightStatic, lightSPOT);
	lightShineFrom(&lightStatic, vec1, M_PI * 3.0 / 4.0, M_PI * 3.0 / 4.0);
	vecSet(3, vec1, 1.0, 1.0, 1.0);
	lightSetColor(&lightStatic, vec1);
	vecSet(3, vec1, 1.0, 0.0, 0.0);
	lightSetAttenuation(&lightStatic, vec1);
	lightSetSpotAngle(&lightStatic, M_PI / 3.0);
    
	/* Configure shadow mapping. */
	if (shadowProgramInitialize(&sdwProg, 3) != 0)
		return 1;
	if (shadowMapInitialize(&sdwMap, 1024, 1024) != 0)
		return 2;
    if (shadowMapInitialize(&sdwMapStatic, 1024, 1024) != 0)
		return 3;
	return 0;
}

/* Initializes the physics simulation and associates physics objects with graphics ones */
void initPhysics() {
    pInitPhysics(15, 10);
    pSetGravity(0.0, 0.0, -0.2);
    pSetBounciness(0.0);
    
    sceneSetPhysicsLoc(&nodeW, pInitBox(pGEOM_ONLY, 0.0, 60.0, 60.0, 10.0));
    pSetPosition(nodeW.physicsLoc, 30.0, 30.0, 0.0);
    sceneSetPhysicsLoc(&nodeC1, pInitBox(pGEOM_AND_BODY, 1.0, 6.0, 6.0, 6.0));
    pSetPosition(nodeC1.physicsLoc, nodeC1.translation[0], nodeC1.translation[1],
                 nodeC1.translation[2]);
    sceneSetPhysicsLoc(&nodeC3, pInitBox(pGEOM_AND_BODY, 1.0, 6.0, 6.0, 6.0));
    pSetPosition(nodeC3.physicsLoc, nodeC3.translation[0], nodeC3.translation[1],
                 nodeC3.translation[2]);
    sceneSetPhysicsLoc(&nodeC4, pInitBox(pGEOM_AND_BODY, 1.0, 6.0, 6.0, 6.0));
    pSetPosition(nodeC4.physicsLoc, nodeC4.translation[0], nodeC4.translation[1],
                 nodeC4.translation[2]);
    sceneSetPhysicsLoc(&nodeE1, pInitBox(pGEOM_AND_BODY, 100000000000.0, 6.0, 6.0, 6.0));
    pSetPosition(nodeE1.physicsLoc, nodeE1.translation[0], nodeE1.translation[1],
                 nodeE1.translation[2]);
    sceneSetPhysicsLoc(&nodeE3, pInitBox(pGEOM_AND_BODY, 100000000000.0, 6.0, 6.0, 6.0));
    pSetPosition(nodeE3.physicsLoc, nodeE3.translation[0], nodeE3.translation[1],
                 nodeE3.translation[2]);
    sceneSetPhysicsLoc(&nodeE4, pInitBox(pGEOM_AND_BODY, 100000000000.0, 6.0, 6.0, 6.0));
    pSetPosition(nodeE4.physicsLoc, nodeE4.translation[0], nodeE4.translation[1],
                 nodeE4.translation[2]);
    
    int jointLoc = pInitJoint(pJOINT_HINGE, nodeC4.physicsLoc, nodeE4.physicsLoc);
    sceneSetJointLoc(&nodeE4, jointLoc); sceneSetJointLoc(&nodeC4, jointLoc);
    jointLoc = pInitJoint(pJOINT_BALL, nodeC3.physicsLoc, nodeE3.physicsLoc);
    sceneSetJointLoc(&nodeE3, jointLoc); sceneSetJointLoc(&nodeC3, jointLoc);
    jointLoc = pInitJoint(pJOINT_SLIDER, nodeC1.physicsLoc, nodeE1.physicsLoc);
    sceneSetJointLoc(&nodeE1, jointLoc); sceneSetJointLoc(&nodeC1, jointLoc);
    
    pSetBallJointAnchor(nodeE3.jointLoc, 45.0, 15.0, 22.0);
    pSetHingeJointAnchor(nodeE4.jointLoc, 15.0, 22.5, 12.0);
    pSetHingeJointAxis(nodeE4.jointLoc, 0.0, 0.0, 1.0);
    pSetHingeJointParams(nodeE4.jointLoc, -2*M_PI, 2*M_PI, hingeDir, 15.0);
    pSetSliderJointAxis(nodeE1.jointLoc, 1.0, 0.0, 0.0);
    pSetSliderJointParams(nodeE1.jointLoc, -20.0, 10.0, 0.0, 0.0);
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
	GLchar vertexCode[] = "\
		#version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		uniform mat4 viewingSdw;\
        uniform mat4 viewingStaticSdw;\
		in vec3 position;\
		in vec2 texCoords;\
		in vec3 normal;\
		out vec3 fragPos;\
		out vec3 normalDir;\
		out vec2 st;\
		out vec4 fragSdw;\
        out vec4 fragSdwStatic;\
		void main(void) {\
			mat4 scaleBias = mat4(\
				0.5, 0.0, 0.0, 0.0, \
				0.0, 0.5, 0.0, 0.0, \
				0.0, 0.0, 0.5, 0.0, \
				0.5, 0.5, 0.5, 1.0);\
			vec4 worldPos = modeling * vec4(position, 1.0);\
			gl_Position = viewing * worldPos;\
			fragSdw = scaleBias * viewingSdw * worldPos;\
            fragSdwStatic = scaleBias * viewingStaticSdw * worldPos;\
			fragPos = vec3(worldPos);\
			normalDir = vec3(modeling * vec4(normal, 0.0));\
			st = texCoords;\
		}";
    GLchar fragmentCode[] = "\
		#version 140\n\
		uniform sampler2D texture0;\
		uniform vec3 specular;\
		uniform vec3 camPos;\
		uniform vec3 lightPos;\
		uniform vec3 lightCol;\
		uniform vec3 lightAtt;\
		uniform vec3 lightAim;\
		uniform float lightCos;\
        uniform vec3 staticPos;\
		uniform vec3 staticCol;\
		uniform vec3 staticAtt;\
		uniform vec3 staticAim;\
		uniform float staticCos;\
		uniform sampler2DShadow textureSdw;\
        uniform sampler2DShadow textureStaticSdw;\
		in vec3 fragPos;\
		in vec3 normalDir;\
		in vec2 st;\
		in vec4 fragSdw;\
        in vec4 fragSdwStatic;\
		out vec4 fragColor;\
		void main(void) {\
			vec3 diffuse = vec3(texture(texture0, st));\
            vec3 norDir = normalize(normalDir);\
			vec3 litDir = normalize(lightPos - fragPos);\
            vec3 camDir = normalize(camPos - fragPos);\
            vec3 refDir = 2.0 * dot(litDir, norDir) * norDir - litDir;\
			float d = distance(lightPos, fragPos);\
            float a = lightAtt[0] + lightAtt[1] * d + lightAtt[2] * d * d;\
            float diffInt = dot(norDir, litDir) / a;\
            float specInt = dot(refDir, camDir);\
            float ambInt = 0.6;\
			if (dot(lightAim, -litDir) < lightCos)\
				diffInt = 0.0;\
            if (diffInt <= 0.0 || specInt <= 0.0)\
                specInt = 0.0;\
            float shininess = 64.0;\
			float sdw = textureProj(textureSdw, fragSdw);\
			diffInt *= sdw;\
			specInt *= sdw;\
            if (diffInt <= ambInt)\
                diffInt = ambInt;\
			vec3 diffRefl = diffInt * lightCol * diffuse;\
			vec3 specRefl = pow(specInt / a, shininess) * lightCol * specular;\
            vec3 staticLitDir = normalize(staticPos - fragPos);\
            vec3 staticRefDir = 2.0 * dot(staticLitDir, norDir) * norDir - staticLitDir;\
            float dStatic = distance(staticPos, fragPos);\
            float aStatic = staticAtt[0] + staticAtt[1] * dStatic + staticAtt[2] * dStatic * dStatic;\
            float staticDiffInt = dot(norDir, staticLitDir) / aStatic;\
            float staticSpecInt = dot(staticRefDir, camDir);\
            if (dot(staticAim, -staticLitDir) < staticCos)\
				staticDiffInt = 0.0;\
            if (staticDiffInt <= 0.0 || staticSpecInt <= 0.0)\
                staticSpecInt = 0.0;\
            float staticSdw = textureProj(textureStaticSdw, fragSdwStatic);\
			staticDiffInt *= staticSdw;\
			staticSpecInt *= staticSdw;\
            if (staticDiffInt <= ambInt)\
                staticDiffInt = ambInt;\
			vec3 staticDiffRefl = staticDiffInt * staticCol * diffuse;\
			vec3 staticSpecRefl = pow(staticSpecInt / aStatic, shininess) * staticCol * specular;\
			fragColor = vec4(diffRefl + specRefl + staticDiffRefl + staticSpecRefl, 1.0);\
		}";
	program = makeProgram(vertexCode, fragmentCode);
	if (program != 0) {
		glUseProgram(program);
		attrLocs[0] = glGetAttribLocation(program, "position");
		attrLocs[1] = glGetAttribLocation(program, "texCoords");
		attrLocs[2] = glGetAttribLocation(program, "normal");
		viewingLoc = glGetUniformLocation(program, "viewing");
		modelingLoc = glGetUniformLocation(program, "modeling");
		unifLocs[0] = glGetUniformLocation(program, "specular");
		textureLocs[0] = glGetUniformLocation(program, "texture0");
		camPosLoc = glGetUniformLocation(program, "camPos");
		lightPosLoc = glGetUniformLocation(program, "lightPos");
		lightColLoc = glGetUniformLocation(program, "lightCol");
		lightAttLoc = glGetUniformLocation(program, "lightAtt");
		lightDirLoc = glGetUniformLocation(program, "lightAim");
		lightCosLoc = glGetUniformLocation(program, "lightCos");
        staticPosLoc = glGetUniformLocation(program, "staticPos");
		staticColLoc = glGetUniformLocation(program, "staticCol");
		staticAttLoc = glGetUniformLocation(program, "staticAtt");
		staticDirLoc = glGetUniformLocation(program, "staticAim");
		staticCosLoc = glGetUniformLocation(program, "staticCos");
		viewingSdwLoc = glGetUniformLocation(program, "viewingSdw");
		textureSdwLoc = glGetUniformLocation(program, "textureSdw");
        viewingSdwStaticLoc = glGetUniformLocation(program, "viewingStaticSdw");
		textureSdwStaticLoc = glGetUniformLocation(program, "textureStaticSdw");
	}
	return (program == 0);
}

/* Fetches the updated position and rotation of all physics objects */
void physicsRender() {
    double pos[3];
    double rot[3][3];
    GLdouble rotGL[3][3];
    GLdouble posGL[3];
    
    pSetForce(nodeC3.physicsLoc, 0.1, 0.1, 0.3);
    pSetForce(nodeC1.physicsLoc, sliderForce, 0.0, 0.2);
    pSetForce(nodeC4.physicsLoc, 0.0, 0.0, 0.2);
    
    pGetPosition(nodeC1.physicsLoc, pos);
    vec3ToOpenGL(posGL, pos);
    sceneSetTranslation(&nodeC1, posGL);
    pGetRotation(nodeC1.physicsLoc, rot);
    mat33ToOpenGL(rotGL, rot); 
    sceneSetRotation(&nodeC1, rotGL);
    
    pGetPosition(nodeC3.physicsLoc, pos);
    vec3ToOpenGL(posGL, pos);
    sceneSetTranslation(&nodeC3, posGL);
    pGetRotation(nodeC3.physicsLoc, rot);
    mat33ToOpenGL(rotGL, rot); 
    sceneSetRotation(&nodeC3, rotGL);
    
    pGetPosition(nodeC4.physicsLoc, pos);
    vec3ToOpenGL(posGL, pos);
    sceneSetTranslation(&nodeC4, posGL);
    pGetRotation(nodeC4.physicsLoc, rot);
    mat33ToOpenGL(rotGL, rot); 
    sceneSetRotation(&nodeC4, rotGL);
    
    pGetPosition(nodeE1.physicsLoc, pos);
    vec3ToOpenGL(posGL, pos);
    sceneSetTranslation(&nodeE1, posGL);
    pGetRotation(nodeE1.physicsLoc, rot);
    mat33ToOpenGL(rotGL, rot); 
    sceneSetRotation(&nodeE1, rotGL);
    
    pGetPosition(nodeE3.physicsLoc, pos);
    vec3ToOpenGL(posGL, pos);
    sceneSetTranslation(&nodeE3, posGL);
    pGetRotation(nodeE3.physicsLoc, rot);
    mat33ToOpenGL(rotGL, rot); 
    sceneSetRotation(&nodeE3, rotGL);
    
    pGetPosition(nodeE4.physicsLoc, pos);
    vec3ToOpenGL(posGL, pos);
    sceneSetTranslation(&nodeE4, posGL);
    pGetRotation(nodeE4.physicsLoc, rot);
    mat33ToOpenGL(rotGL, rot); 
    sceneSetRotation(&nodeE4, rotGL);
}

void render(void) {
    physicsRender();
    
	GLdouble identity[4][4];
	mat44Identity(identity);
	/* Save the viewport transformation. */
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	/* For each shadow-casting light, render its shadow map using minimal 
	uniforms and textures. */
	GLint sdwTextureLocs[2] = {-1, -1};
	shadowMapRender(&sdwMap, &sdwProg, &light, -100.0, -1.0);
    sceneRender(&nodeW, identity, sdwProg.modelingLoc, 0, NULL, NULL, 1, 
		sdwTextureLocs);
    shadowMapRender(&sdwMapStatic, &sdwProg, &lightStatic, -100.0, -1.0);
	sceneRender(&nodeW, identity, sdwProg.modelingLoc, 0, NULL, NULL, 1, 
		sdwTextureLocs);
	/* Finish preparing the shadow maps, restore the viewport, and begin to 
	render the scene. */
	shadowMapUnrender();
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	camRender(&cam, viewingLoc);
	GLfloat vec[3];
	vecOpenGL(3, cam.translation, vec);
	glUniform3fv(camPosLoc, 1, vec);
	/* For each light, we have to connect it to the shader program, as always. 
	For each shadow-casting light, we must also connect its shadow map. */
	lightRender(&light, lightPosLoc, lightColLoc, lightAttLoc, lightDirLoc, 
		lightCosLoc);
	shadowRender(&sdwMap, viewingSdwLoc, GL_TEXTURE7, 7, textureSdwLoc);
    lightRender(&lightStatic, staticPosLoc, staticColLoc, staticAttLoc, staticDirLoc, 
		staticCosLoc);
    shadowRender(&sdwMapStatic, viewingSdwStaticLoc, GL_TEXTURE8, 8, textureSdwStaticLoc);
	GLuint unifDims[1] = {3};
	sceneRender(&nodeW, identity, modelingLoc, 1, unifDims, unifLocs, 0, 
		textureLocs);
	/* For each shadow-casting light, turn it off when finished rendering. */
	shadowUnrender(GL_TEXTURE7);
    shadowUnrender(GL_TEXTURE8);
}

int main(void) {
	double oldTime;
	double newTime = getTime();
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0) {
    	fprintf(stderr, "main: glfwInit failed.\n");
        return 1;
    }
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window;
    window = glfwCreateWindow(768, 768, "Shadows", NULL, NULL);
    if (window == NULL) {
    	fprintf(stderr, "main: glfwCreateWindow failed.\n");
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwSetKeyCallback(window, handleKey);
    glfwMakeContextCurrent(window);
    if (gl3wInit() != 0) {
    	fprintf(stderr, "main: gl3wInit failed.\n");
    	glfwDestroyWindow(window);
    	glfwTerminate();
    	return 3;
    }
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n", 
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
	/* We no longer do glDepthRange(1.0, 0.0). Instead we have changed our 
	projection matrices. */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    if (initializeShaderProgram() != 0)
    	return 3;
    /* Initialize the shadow mapping before the meshes. Why? */
	if (initializeCameraLight() != 0)
		return 4;
    if (initializeScene() != 0)
    	return 5;
    initPhysics();
    
    while (glfwWindowShouldClose(window) == 0) {
    	oldTime = newTime;
    	newTime = getTime();
    	if (floor(newTime) - floor(oldTime) >= 1.0)
			fprintf(stderr, "main: %f frames/sec\n", 1.0 / (newTime - oldTime));
        /* step physics simulation forward */
        pSimLoop();
		render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    /* Deallocate more resources than ever. */
    shadowProgramDestroy(&sdwProg);
    shadowMapDestroy(&sdwMap);
    glDeleteProgram(program);
    destroyScene();
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}