/* On macOS, compile with...
    clang 500texturing.c -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <GLFW/glfw3.h>

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

#define triNum 8
#define vertNum 6
#define attrDim 8
GLdouble alpha = 0.0;
GLuint buffers[2];

void initializeMesh(void) {
	/* Append two texture coordinates to each vertex from 500openGL20b.c. 
	Notice that attrDim, above, is now 8. */
	GLdouble attributes[vertNum * attrDim] = {
		1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.5, 
		-1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.5, 0.5, 
		0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.25, 0.5, 
		0.0, -1.0, 0.0, 0.0, 1.0, 1.0, 0.75, 0.5, 
		0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 
		0.0, 0.0, -1.0, 1.0, 0.0, 1.0, 0.0, 0.0};
	/* The rest of this function is the same as in 500openGL20b.c. */
	GLuint triangles[triNum * 3] = {
		0, 2, 4, 
		2, 1, 4, 
		1, 3, 4, 
		3, 0, 4, 
		2, 0, 5, 
		1, 2, 5, 
		3, 1, 5, 
		0, 3, 5};
	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertNum * attrDim * sizeof(GLdouble),
		(GLvoid *)attributes, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triNum * 3 * sizeof(GLuint),
		(GLvoid *)triangles, GL_STATIC_DRAW);
}

/* This texture stuff is new. */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_FAILURE_USERMSG

GLuint texture;
GLint texCoordsLoc, textureLoc;

/* Loads the given image file into an OpenGL texture. The width and height of the image must be powers of 2. Returns 0 on success, non-zero on failure. On 
success, the user must later deallocate the texture using a call like 
glDeleteTextures(1, texture). */
int initializeTexture(GLuint *texture, char *path) {
	/* Use STB Image to load the texture data from the file. */
	int width, height, texelDim;
	unsigned char *rawData;
	rawData = stbi_load(path, &width, &height, &texelDim, 0);
	if (rawData == NULL) {
		fprintf(stderr, "initializeTexture: failed to load %s\n", path);
		fprintf(stderr, "with STB Image reason: %s.\n", stbi_failure_reason());
		return 1;
	}
	/* Right now we support only 3-channel images. */
	if (texelDim != 3) {
		fprintf(stderr, "initializeTexture: %d != 3 channels.\n", texelDim);
		return 2;
	}
	/* Load the data into OpenGL. The calls to glTexParameteri demonstrate how 
	you might set border and filtering behavior. */
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, 
		GL_UNSIGNED_BYTE, rawData);
	stbi_image_free(rawData);
	if (glGetError() != GL_NO_ERROR) {
		fprintf(stderr, "initializeTexture: OpenGL error.\n");
		glDeleteTextures(1, texture);
		return 3;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return 0;
}

#include "500shader.c"

GLuint program;
GLint positionLoc, colorLoc;
GLint viewingLoc, modelingLoc;

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
		positionLoc = glGetAttribLocation(program, "position");
		colorLoc = glGetAttribLocation(program, "color");
		viewingLoc = glGetUniformLocation(program, "viewing");
		modelingLoc = glGetUniformLocation(program, "modeling");
		/* Don't forget to record the new locations. */
		texCoordsLoc = glGetAttribLocation(program, "texCoords");
		textureLoc = glGetUniformLocation(program, "texture");
	}
	return (program == 0);
}

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

#include "100vector.c"
#include "130matrix.c"

void mat44OpenGL(double m[4][4], GLfloat openGL[4][4]) {
	for (int i = 0; i < 4; i += 1)
		for (int j = 0; j < 4; j += 1)
			openGL[i][j] = m[j][i];
}

void render(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	GLfloat modeling[4][4];
	double trans[3] = {0.0, 0.0, 0.0}, axis[3] = {1.0, 1.0, 1.0};
	double rot[3][3], model[4][4];
	vecUnit(3, axis, axis);
	alpha += 0.01;
	mat33AngleAxisRotation(alpha, axis, rot);
	mat44Isometry(rot, trans, model);
	mat44OpenGL(model, modeling);
	glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, (GLfloat *)modeling);
	GLfloat viewing[4][4];
	double camInv[4][4], proj[4][4], projCamInv[4][4];
	trans[2] = 4.0;
	mat33AngleAxisRotation(0.0, axis, rot);
	mat44InverseIsometry(rot, trans, camInv);
	mat44Orthographic(-2.0, 2.0, -2.0, 2.0, -6.0, -2.0, proj);
	mat444Multiply(proj, camInv, projCamInv);
	mat44OpenGL(projCamInv, viewing);
	glUniformMatrix4fv(viewingLoc, 1, GL_FALSE, (GLfloat *)viewing);
	/* Connect the texture to the shader program. By sending the number 0 into 
	textureLoc, we are telling the shaders to use texture unit 0, which is the 
	same one we've been configuring this whole time. (See the main function, 
	below.) */
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(textureLoc, 0);
	/* The mesh is now not just positions and colors, but also texture 
	coordinates. So pass them into OpenGL. */
	glEnableVertexAttribArray(positionLoc);
	glEnableVertexAttribArray(colorLoc);
	glEnableVertexAttribArray(texCoordsLoc);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(positionLoc, 3, GL_DOUBLE, GL_FALSE, 
		attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
	glVertexAttribPointer(colorLoc, 3, GL_DOUBLE, GL_FALSE, 
		attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
	glVertexAttribPointer(texCoordsLoc, 2, GL_DOUBLE, GL_FALSE, 
		attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glDrawElements(GL_TRIANGLES, triNum * 3, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	glDisableVertexAttribArray(positionLoc);
	glDisableVertexAttribArray(colorLoc);
	/* Unbind the texture, because cleaning up after yourself is a good idea. */
	glBindTexture(GL_TEXTURE_2D, 0);
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
    /* A 'texture unit' is a piece of machinery inside the GPU that performs 
    texture mapping. A GPU might have many texture units, allowing you to map 
    many textures onto your meshes in complicated ways. The glActiveTexture 
    function selects which texture unit is affected by subsequent OpenGL calls. 
    In this tutorial, we use only texture unit 0, so we activate it here, once 
    and for all. */
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    if (initializeTexture(&texture, "cat.jpg") != 0) {
    	glfwDestroyWindow(window);
        glfwTerminate();
        return 3;
    }
    initializeMesh();
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
    glDeleteTextures(1, &texture);
    glDeleteProgram(program);
	glDeleteBuffers(2, buffers);
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

