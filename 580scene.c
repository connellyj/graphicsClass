/* A few methods by Julia Connelly and Kerim Celik, 02/24/2017 */


/*** Creation and destruction ***/

/* Feel free to read from this struct's members, but don't write to them except 
through the accessor functions. */
typedef struct sceneNode sceneNode;
struct sceneNode {
	GLdouble rotation[3][3];
	GLdouble translation[3];
	GLuint unifDim;
	GLdouble *unif;
	meshGLMesh *meshGL;
	sceneNode *firstChild, *nextSibling;
    texTexture **tex;
    GLuint texNum;
};

/* Initializes a sceneNode struct. The translation and rotation are initialized to trivial values. The user must remember to call sceneDestroy or 
sceneDestroyRecursively when finished. Returns 0 if no error occurred. */
int sceneInitialize(sceneNode *node, GLuint unifDim, GLuint texNum, 
        meshGLMesh *mesh, sceneNode *firstChild, sceneNode *nextSibling) {
	node->unif = (GLdouble *)malloc(unifDim * sizeof(GLdouble) + 
        texNum * sizeof(texTexture *));
	if (node->unif == NULL)
		return 1;
    node->tex = (texTexture **)&(node->unif[unifDim]);
	mat33Identity(node->rotation);
	vecSet(3, node->translation, 0.0, 0.0, 0.0);
    node->texNum = texNum;
	node->unifDim = unifDim;
	node->meshGL = mesh;
	node->firstChild = firstChild;
	node->nextSibling = nextSibling;
	return 0;
}

/* Deallocates the resources backing this scene node. Does not destroy the 
resources backing the mesh, etc. */
void sceneDestroy(sceneNode *node) {
	if (node->unif != NULL)
		free(node->unif);
	node->unif = NULL;
}



/*** Accessors ***/

/* Copies the texNum-dimensional vector from tex into the node. */
void sceneSetTexture(sceneNode *node, texTexture **tex) {
    node->tex = tex;
}

/* Sets one uniform in the node, based on its index in the unif array. */
void sceneSetOneTexture(sceneNode *node, int index, texTexture *tex) {
	node->tex[index] = tex;
}

/* Copies the unifDim-dimensional vector from unif into the node. */
void sceneSetUniform(sceneNode *node, double unif[]) {
	vecCopy(node->unifDim, unif, node->unif);
}

/* Sets one uniform in the node, based on its index in the unif array. */
void sceneSetOneUniform(sceneNode *node, int index, double unif) {
	node->unif[index] = unif;
}

/* Calls sceneDestroy recursively on the node's descendants and younger 
siblings, and then on the node itself. */
void sceneDestroyRecursively(sceneNode *node) {
	if (node->firstChild != NULL)
		sceneDestroyRecursively(node->firstChild);
	if (node->nextSibling != NULL)
		sceneDestroyRecursively(node->nextSibling);
	sceneDestroy(node);
}

/* Sets the node's rotation. */
void sceneSetRotation(sceneNode *node, GLdouble rot[3][3]) {
	vecCopy(9, (GLdouble *)rot, (GLdouble *)(node->rotation));
}

/* Sets the node's rotation. */
void sceneSetRotationArray(sceneNode *node, GLdouble rot[9]) {
	vecCopy(9, rot, (GLdouble *)(node->rotation));
}

/* Sets the node's translation. */
void sceneSetTranslation(sceneNode *node, GLdouble transl[3]) {
	vecCopy(3, transl, node->translation);
}

/* Sets the scene's mesh. */
void sceneSetMesh(sceneNode *node, meshGLMesh *mesh) {
	node->meshGL = mesh;
}

/* Sets the node's first child. */
void sceneSetFirstChild(sceneNode *node, sceneNode *child) {
	node->firstChild = child;
}

/* Sets the node's next sibling. */
void sceneSetNextSibling(sceneNode *node, sceneNode *sibling) {
	node->nextSibling = sibling;
}

/* Adds a sibling to the given node. The sibling shows up as the youngest of 
its siblings. */
void sceneAddSibling(sceneNode *node, sceneNode *sibling) {
	if (node->nextSibling == NULL)
		node->nextSibling = sibling;
	else
		sceneAddSibling(node->nextSibling, sibling);
}

/* Adds a child to the given node. The child shows up as the youngest of its 
siblings. */
void sceneAddChild(sceneNode *node, sceneNode *child) {
	if (node->firstChild == NULL)
		node->firstChild = child;
	else
		sceneAddSibling(node->firstChild, child);
}

/* Removes a sibling from the given node. Equality of nodes is assessed as 
equality of pointers. If the sibling is not present, then has no effect (fails 
silently). */
void sceneRemoveSibling(sceneNode *node, sceneNode *sibling) {
	if (node->nextSibling == NULL)
		return;
	else if (node->nextSibling == sibling)
		node->nextSibling = sibling->nextSibling;
	else
		sceneRemoveSibling(node->nextSibling, sibling);
}

/* Removes a child from the given node. Equality of nodes is assessed as 
equality of pointers. If the sibling is not present, then has no effect (fails 
silently). */
void sceneRemoveChild(sceneNode *node, sceneNode *child) {
	if (node->firstChild == NULL)
		return;
	else if (node->firstChild == child)
		node->firstChild = child->nextSibling;
	else
		sceneRemoveSibling(node->firstChild, child);
}



/*** OpenGL ***/

/* Renders the node, its younger siblings, and their descendants. parent is the 
modeling matrix at the parent of the node. If the node has no parent, then this 
matrix is the 4x4 identity matrix. Loads the modeling transformation into 
modelingLoc. The attribute information exists to be passed to meshGLRender. The 
uniform information is analogous, but sceneRender loads it, not meshGLRender. */
void sceneRender(sceneNode *node, GLdouble parent[4][4], GLint modelingLoc, 
		GLuint unifNum, GLuint unifDims[], GLint unifLocs[], 
		GLuint vaoIndex, GLint textureLocs[]) {
    if(node == NULL) return;
	/* Set the uniform modeling matrix. */
    GLfloat modeling[4][4];
    GLdouble parentTimesModel[4][4], model[4][4]; 
	mat44Isometry(node->rotation, node->translation, model);
    mat444Multiply(parent, model, parentTimesModel);
	mat44OpenGL(parentTimesModel, modeling);
	glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, (GLfloat *)modeling);
	/* Set the other uniforms. The casting from double to float is annoying. */
    int offset = 0;
    for(int i = 0; i < unifNum; i++) {
        GLfloat tmp[unifDims[i]];
        vecOpenGL(unifDims[i], &((node->unif)[offset]), tmp);
        if(unifDims[i] == 1){
            glUniform1fv(unifLocs[i], 1, tmp);
        }else if(unifDims[i] == 2) {
            glUniform2fv(unifLocs[i], 1, tmp);
        }else if(unifDims[i] == 3) {
            glUniform3fv(unifLocs[i], 1, tmp);
        }else if(unifDims[i] == 4) {
            glUniform4fv(unifLocs[i], 1, tmp);
        }
        offset += unifDims[i];
    }
	/* Render the mesh, the children, and the younger siblings. */
    GLenum texUnits[8] = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3,
                 GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7};
    for(int i = 0; i < node->texNum; i++) {
        texRender(node->tex[i], texUnits[i], i, textureLocs[i]);
    }
	meshGLRender(node->meshGL, vaoIndex);
    for(int i = 0; i < node->texNum; i++) {
        texUnrender(node->tex[i], texUnits[i]);
    }
    sceneRender(node->firstChild, parentTimesModel, modelingLoc,
                    unifNum, unifDims, unifLocs,
                    vaoIndex, textureLocs);
    sceneRender(node->nextSibling, parent, modelingLoc,
                    unifNum, unifDims, unifLocs,
                    vaoIndex, textureLocs);
}

