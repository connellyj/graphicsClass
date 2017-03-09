/* Edited by Julia Connelly and Kerim Celik, 02/24/2017 */


/* On macOS, compile with...
    clang 590mainShadowing.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation
*/

/* START PHYSICS CHUNK */
#include <ode/ode.h>

typedef struct {
  dBodyID body;
  dGeomID geom;
} PhysicsObject;

static dWorldID world;
PhysicsObject ball;
const dReal radius = 1.0;
const dReal mass = 1.0;

static dSpaceID space;
static dGeomID trunk, leaves, ice;
static dJointGroupID contactgroup;
/* END PHYSICS CHUNK */

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

#include "500shader.c"
#include "530vector.c"
#include "580mesh.c"
#include "590matrix.c"
#include "520camera.c"
#include "540texture.c"
#include "580scene.c"
#include "560light.c"
#include "590shadow.c"

camCamera cam;
texTexture texH, texV, texW, texT, texL, texSwagLord;
meshGLMesh meshH, meshV, meshW, meshT, meshL, meshBall;
meshMesh meshMeshH, meshMeshV;
sceneNode nodeH, nodeV, nodeW, nodeT, nodeL, nodeBall;
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

/* START PHYSICS CHUNK */
static void nearCallback(void *data, dGeomID o1, dGeomID o2) {
    const int N = 10;
    dContact contact[N];

    int n =  dCollide(o1,o2,N,&contact[0].geom,sizeof(dContact));

    for (int i = 0; i < n; i++) {
        contact[i].surface.mode = dContactBounce;
        contact[i].surface.mu   = dInfinity;
        contact[i].surface.bounce     = 0.2; // (0.0~1.0) restitution parameter
        contact[i].surface.bounce_vel = 0.0; // minimum incoming velocity for bounce
        dJointID c = dJointCreateContact(world,contactgroup,&contact[i]);
        dJointAttach (c,dGeomGetBody(contact[i].geom.g1),dGeomGetBody(contact[i].geom.g2));
    }
}

/* will need to change later to handle addition of new physics bodies to the scene/world */
static void simLoop () {
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
    }
    for(int i = 0; i < 9; i++) {
        rot[i] = (GLdouble)R[i];
    }
    
    sceneSetTranslation(&nodeBall, trans);
    sceneSetRotationArray(&nodeBall, rot);
    printf("%f, %f, %f\n%f, %f, %f\n%f, %f, %f\n", rot[0],rot[1],rot[2],rot[3],rot[4],rot[5],rot[6],rot[7],rot[8]);
    printf("%f, %f, %f\n", trans[0], trans[1], trans[2]);
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
    char grass[] = "snowygrass.jpg";
	if (texInitializeFile(&texH, grass, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 1;
    char cliff[] = "snowcliff.jpg";
    if (texInitializeFile(&texV, cliff, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 2;
    char ice[] = "ice.jpg";
    if (texInitializeFile(&texW, ice, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 3;
    char trunk[] = "trunk.jpg";
    if (texInitializeFile(&texT, trunk, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 4;
    char tree[] = "leaves.jpg";
    if (texInitializeFile(&texL, tree, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 5;
    char chick[] = "chicken.jpg";
    if (texInitializeFile(&texSwagLord, chick, GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 6;
	GLuint attrDims[3] = {3, 2, 3};
    double zs[12][12] = {
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 20.0}, 
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 20.0, 25.0}, 
		{5.0, 5.0, 10.0, 12.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 20.0, 25.0}, 
		{5.0, 5.0, 10.0, 10.0, 5.0, 5.0, 5.0, 5.0, 5.0, 20.0, 25.0, 27.0}, 
		{0.0, 0.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 20.0, 20.0, 25.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.0, 25.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.0, 20.0}, 
		{5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.0, 20.0, 20.0}, 
		{10.0, 10.0, 5.0, 5.0, 0.0, 0.0, 0.0, 5.0, 10.0, 15.0, 20.0, 25.0}};
	double ws[12][12] = {
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
	meshMesh mesh, meshLand;
	if (meshInitializeLandscape(&meshLand, 12, 12, 5.0, (double *)zs) != 0)
		return 6;
	if (meshInitializeDissectedLandscape(&meshMeshH, &meshLand, M_PI / 3.0, 1) != 0)
		return 7;
	/* There are now two VAOs per mesh. */
	meshGLInitialize(&meshH, &meshMeshH, 3, attrDims, 2);
	meshGLVAOInitialize(&meshH, 0, attrLocs);
	meshGLVAOInitialize(&meshH, 1, sdwProg.attrLocs);
	if (meshInitializeDissectedLandscape(&meshMeshV, &meshLand, M_PI / 3.0, 0) != 0)
		return 8;
	meshDestroy(&meshLand);
	double *vert, normal[2];
	for (int i = 0; i < meshMeshV.vertNum; i += 1) {
		vert = meshGetVertexPointer(&meshMeshV, i);
		normal[0] = -vert[6];
		normal[1] = vert[5];
		vert[3] = (vert[0] * normal[0] + vert[1] * normal[1]) / 20.0;
		vert[4] = vert[2] / 20.0;
	}
	meshGLInitialize(&meshV, &meshMeshV, 3, attrDims, 2);
	meshGLVAOInitialize(&meshV, 0, attrLocs);
	meshGLVAOInitialize(&meshV, 1, sdwProg.attrLocs);
	if (meshInitializeLandscape(&mesh, 12, 12, 5.0, (double *)ws) != 0)
		return 9;
	meshGLInitialize(&meshW, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshW, 0, attrLocs);
	meshGLVAOInitialize(&meshW, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeCapsule(&mesh, 1.0, 10.0, 1, 8) != 0)
		return 10;
	meshGLInitialize(&meshT, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshT, 0, attrLocs);
	meshGLVAOInitialize(&meshT, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeSphere(&mesh, 5.0, 8, 16) != 0)
		return 11;
	meshGLInitialize(&meshL, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshL, 0, attrLocs);
	meshGLVAOInitialize(&meshL, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
    if (meshInitializeSphere(&mesh, 3.0, 8, 16) != 0)
		return 21;
    meshGLInitialize(&meshBall, &mesh, 3, attrDims, 2);
    meshGLVAOInitialize(&meshBall, 0, attrLocs);
	meshGLVAOInitialize(&meshBall, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
    if (sceneInitialize(&nodeBall, 3, 1, &meshBall, NULL, NULL) != 0)
		return 51;
	if (sceneInitialize(&nodeW, 3, 1, &meshW, NULL, NULL) != 0)
		return 14;
	if (sceneInitialize(&nodeL, 3, 1, &meshL, NULL, NULL) != 0)
		return 16;
	if (sceneInitialize(&nodeT, 3, 1, &meshT, &nodeL, &nodeW) != 0)
		return 15;
	if (sceneInitialize(&nodeV, 3, 1, &meshV, NULL, &nodeT) != 0)
		return 13;
	if (sceneInitialize(&nodeH, 3, 1, &meshH, &nodeV, &nodeBall) != 0)
		return 12;
	GLdouble trans[3] = {40.0, 28.0, 5.0};
	sceneSetTranslation(&nodeT, trans);
	vecSet(3, trans, 0.0, 0.0, 7.0);
	sceneSetTranslation(&nodeL, trans);
    vecSet(3, trans, 38.0, 28.0, 45.0);
	sceneSetTranslation(&nodeBall, trans);
	GLdouble unif[3] = {0.0, 0.0, 0.0};
	sceneSetUniform(&nodeH, unif);
    sceneSetUniform(&nodeBall, unif);
	sceneSetUniform(&nodeV, unif);
	sceneSetUniform(&nodeT, unif);
	sceneSetUniform(&nodeL, unif);
	vecSet(3, unif, 1.0, 1.0, 1.0);
	sceneSetUniform(&nodeW, unif);
	texTexture *tex;
	tex = &texH;
	sceneSetOneTexture(&nodeH, 0, tex);
	tex = &texV;
	sceneSetOneTexture(&nodeV, 0, tex);
	tex = &texW;
	sceneSetOneTexture(&nodeW, 0, tex);
	tex = &texT;
	sceneSetOneTexture(&nodeT, 0, tex);
	tex = &texL;
	sceneSetOneTexture(&nodeL, 0, tex);
    tex = &texSwagLord;
    sceneSetOneTexture(&nodeBall, 0, tex);
	return 0;
}

void initializeTriMesh() {
//    dTriMeshDataID triMeshH = dGeomTriMeshDataCreate();
//    dTriMeshDataID triMeshV = dGeomTriMeshDataCreate();
//    GLdouble vertXYZH[meshMeshH.vertNum * 3];
//    for(int i = 0; i < meshMeshH.vertNum * 3; i += meshMeshH.attrDim) {
//        for(int j = 0; j < 3; j++) {
//            vertXYZH[(i / meshMeshH.attrDim) + j] = meshMeshH.vert[i + j];
//        }
//    }
//    dGeomTriMeshDataBuild(triMeshH, meshMeshH.vert, meshMeshH.attrDim, meshMeshH.vertNum, meshMeshH.tri, meshMeshH.triNum, 3, NULL);
}

void destroyScene(void) {
	texDestroy(&texH);
	texDestroy(&texV);
	texDestroy(&texW);
	texDestroy(&texT);
	texDestroy(&texL);
	meshGLDestroy(&meshH);
	meshGLDestroy(&meshV);
	meshGLDestroy(&meshW);
	meshGLDestroy(&meshT);
	meshGLDestroy(&meshL);
    meshDestroy(&meshMeshH);
    meshDestroy(&meshMeshV);
	sceneDestroyRecursively(&nodeH);
}

/* Returns 0 on success, non-zero on failure. Warning: If initialization fails 
midway through, then does not properly deallocate all resources. But that's 
okay, because the program terminates almost immediately after this function 
returns. */
int initializeCameraLight(void) {
    GLdouble vec[3] = {30.0, 30.0, 5.0};
	camSetControls(&cam, camPERSPECTIVE, M_PI / 6.0, 10.0, 768.0, 768.0, 100.0, 
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
            float ambInt = 0.2;\
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

void render(void) {
	GLdouble identity[4][4];
	mat44Identity(identity);
	/* Save the viewport transformation. */
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	/* For each shadow-casting light, render its shadow map using minimal 
	uniforms and textures. */
	GLint sdwTextureLocs[2] = {-1 };
	shadowMapRender(&sdwMap, &sdwProg, &light, -100.0, -1.0);
    sceneRender(&nodeH, identity, sdwProg.modelingLoc, 0, NULL, NULL, 1, 
		sdwTextureLocs);
    shadowMapRender(&sdwMapStatic, &sdwProg, &lightStatic, -100.0, -1.0);
	sceneRender(&nodeH, identity, sdwProg.modelingLoc, 0, NULL, NULL, 1, 
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
	sceneRender(&nodeH, identity, modelingLoc, 1, unifDims, unifLocs, 0, 
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
    
     /* START PHYSICS CHUNK */
    dReal x0 = 0.0, y0 = 0.0, z0 = 1.0;
    dMass m1;

    dInitODE();
    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    contactgroup = dJointGroupCreate(0);
    dWorldSetGravity(world, 0.0, 0.0, -0.5);

    ice = dCreateBox(space, 72.0, 72.0, 5.0);
    dGeomSetPosition(ice, 1.5 * nodeW.translation[0], 1.5 * nodeW.translation[1], nodeW.translation[2]);
    trunk = dCreateCapsule(space, 1.0, 10.0);
    dGeomSetPosition(trunk, nodeT.translation[0], nodeT.translation[1], nodeT.translation[2]);
    leaves = dCreateSphere(space, 5.0);
    dGeomSetPosition(leaves, 40.0, 28.0, 13.0);
    
    ball.body = dBodyCreate(world);
    dMassSetZero(&m1);
    dMassSetSphereTotal(&m1,mass,radius);
    dBodySetMass(ball.body,&m1);
    dBodySetPosition(ball.body, nodeBall.translation[0], nodeBall.translation[1], nodeBall.translation[2]);
    dMatrix3 mat;
    dRSetIdentity(mat);
    dBodySetRotation(ball.body, mat);
    
    ball.geom = dCreateSphere(space,radius);
    dGeomSetBody(ball.geom,ball.body);
    /* END PHYSICS CHUNK */
    
    while (glfwWindowShouldClose(window) == 0) {
    	oldTime = newTime;
    	newTime = getTime();
    	if (floor(newTime) - floor(oldTime) >= 1.0)
			fprintf(stderr, "main: %f frames/sec\n", 1.0 / (newTime - oldTime));
		simLoop();
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

