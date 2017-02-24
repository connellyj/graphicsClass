/*** Methods filled in by Kerim Celik and Julia Connelly 02/17/2017 ***/

#include <stdio.h>

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(GLdouble m[2][2]) {
	for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 2; j++) {
            printf("%f ", m[i][j]);
        }
        printf("\n");
    }
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the 
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0, 
then the matrix is invertible, and its inverse is placed into mInv. */
GLdouble mat22Invert(GLdouble m[2][2], GLdouble mInv[2][2]) {
	GLdouble det = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
    if(det != 0.0) {
        GLdouble iDet = 1 / det;
        mInv[0][0] = m[1][1] * iDet;
        mInv[1][1] = m[0][0] * iDet;
        mInv[0][1] = m[0][1] * iDet * -1;
        mInv[1][0] = m[1][0] * iDet * -1;
    }
    return det;
}

/* Multiplies a 2x2 matrix m by a 2-column v, storing the result in mTimesV. 
The output should not */
void mat221Multiply(GLdouble m[2][2], GLdouble v[2], GLdouble mTimesV[2]) {
	for(int i = 0; i < 2; i++) {
        mTimesV[i] = (m[i][0] * v[0]) + (m[i][1] * v[1]);
    }
}

/* Fills the matrix m from its two columns. */
void mat22Columns(GLdouble col0[2], GLdouble col1[2], GLdouble m[2][2]) {
	for(int i = 0; i < 2; i++) {
        m[i][0] = col0[i];
        m[i][1] = col1[i];
    }
}

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat33Print(GLdouble m[3][3]) {
	for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            printf("%f ", m[i][j]);
        }
        printf("\n");
    }
}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. */
void mat333Multiply(GLdouble m[3][3], GLdouble n[3][3], GLdouble mTimesN[3][3]) {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            mTimesN[i][j] = (n[0][j] * m[i][0]) + (n[1][j] * m[i][1]) + (n[2][j] * m[i][2]);
        }
    }
}

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. */
void mat331Multiply(GLdouble m[3][3], GLdouble v[3], GLdouble mTimesV[3]) {
    for(int i = 0; i < 3; i++) {
        mTimesV[i] = (m[i][0] * v[0]) + (m[i][1] * v[1]) + (m[i][2] * v[2]);
    }
}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous 
coordinates. More precisely, the transformation first rotates through the angle 
theta (in radians, counterclockwise), and then translates by the vector (x, y). 
*/
void mat33Isometry(GLdouble theta, GLdouble x, GLdouble y, GLdouble isom[3][3]) {
    GLdouble m[3][3] = {{cos(theta), -1 * sin(theta), 0}, 
                      {sin(theta), cos(theta), 0}, 
                      {0, 0, 1}};
    GLdouble n[3][3] = {{1, 0, x}, 
                        {0, 1, y}, 
                        {0, 0, 1}};
    mat333Multiply(n, m, isom);
}

/* Helper function that builds a 3x3 matrix by adding the values of 
 * two passed matrices
 */
void mat33Add(GLdouble a[3][3], GLdouble b[3][3], GLdouble added[3][3]){
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            added[i][j] = a[i][j] + b[i][j];
        }
    }
}

/* Helper function that builds a 3x3 matrix by reflecting the values of 
 * a passed matrix across the diagonal
 */
void mat33Transpose(GLdouble a[3][3], GLdouble trans[3][3]){
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            trans[i][j] = a[j][i];
        }
    }
}

/* Helper function that builds a 3x3 matrix by scaling the values of 
 * a passed matrix by the passed scalar
 */
void mat33Scale(GLdouble a[3][3], GLdouble scaled[3][3], GLdouble s){
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            scaled[i][j] = a[i][j] * s;
        }
    }
}

/* Helper function that builds a 3x3 matrix by taking the values from 
 * three dim-3 vectors and using each of them as the values of a column
 */
void mat33Columns(GLdouble col0[3], GLdouble col1[3], GLdouble col2[3], GLdouble m[3][3]){
    for(int i = 0; i < 3; i++) {
        m[i][0] = col0[i];
        m[i][1] = col1[i];
        m[i][2] = col2[i];
    }
}


/* Given a length-1 3D vector axis and an angle theta (in radians), builds the 
rotation matrix for the rotation about that axis through that angle. Based on 
Rodrigues' rotation formula R = I + (sin theta) U + (1 - cos theta) U^2. */
void mat33AngleAxisRotation(GLdouble theta, GLdouble axis[3], GLdouble rot[3][3]){
    GLdouble u[3][3] = {{0, -axis[2], axis[1]},
                        {axis[2], 0, -axis[0]},
                        {-axis[1], axis[0], 0}};
    GLdouble u2[3][3];
    mat333Multiply(u, u, u2);
    mat33Scale(u, u, sin(theta));
    mat33Scale(u2, u2, 1 - cos(theta));
    GLdouble id[3][3] = {{1, 0, 0},
                         {0, 1, 0},
                         {0, 0, 1}};
    mat33Add(id, u, rot);
    mat33Add(u2, rot, rot);
}

/* Given two length-1 3D vectors u, v that are perpendicular to each other. 
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds 
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(GLdouble u[3], GLdouble v[3], GLdouble a[3], GLdouble b[3], 
        GLdouble rot[3][3]) {
    GLdouble w[3], c[3];
    vec3Cross(u, v, w); vec3Cross(a, b, c);
    GLdouble uvw[3][3], abc[3][3], trans[3][3];
    mat33Columns(u, v, w, uvw); mat33Columns(a, b, c, abc);
    mat33Transpose(uvw, trans);
    mat333Multiply(abc, trans, rot);
}

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat44Print(GLdouble m[4][4]) {
	for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            printf("%f ", m[i][j]);
        }
        printf("\n");
    }
}

/* Multiplies m by n, placing the answer in mTimesN. */
void mat444Multiply(GLdouble m[4][4], GLdouble n[4][4], GLdouble mTimesN[4][4]) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            mTimesN[i][j] = (n[0][j] * m[i][0]) + (n[1][j] * m[i][1])
                + (n[2][j] * m[i][2]) + (n[3][j] * m[i][3]);
        }
    }
}

/* Multiplies m by v, placing the answer in mTimesV. */
void mat441Multiply(GLdouble m[4][4], GLdouble v[4], GLdouble mTimesV[4]) {
    for(int i = 0; i < 4; i++) {
        mTimesV[i] = (m[i][0] * v[0]) + (m[i][1] * v[1])
            + (m[i][2] * v[2]) + (m[i][3] * v[3]);
    }
}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix 
representing the rotation followed in time by the translation. */
void mat44Isometry(GLdouble rot[3][3], GLdouble trans[3], GLdouble isom[4][4]) {
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            isom[i][j] = rot[i][j];
        }
    }
    for(int i = 0; i < 3; i++){
        isom[i][3] = trans[i];
    }
    for(int i = 0; i < 3; i++){
        isom[3][i] = 0;
    }
    isom[3][3] = 1;
}

/* Given a rotation and translation, forms the 4x4 homogeneous matrix 
representing the inverse translation followed in time by the inverse rotation. 
That is, the isom produced by this function is the inverse to the isom 
produced by mat44Isometry on the same inputs. */
void mat44InverseIsometry(GLdouble rot[3][3], GLdouble trans[3], 
        GLdouble isom[4][4]){
    GLdouble tmp[3];
    vecScale(3, -1, trans, tmp);
    GLdouble inv[3][3];
    GLdouble tmp2[3];
    mat33Transpose(rot, inv);
    mat331Multiply(inv, tmp, tmp2);
    mat44Isometry(inv, tmp2, isom);
}

/* Builds a 4x4 matrix representing orthographic projection with a boxy viewing 
volume [left, right] x [bottom, top] x [far, near]. That is, on the near plane 
the box is the rectangle R = [left, right] x [bottom, top], and on the far 
plane the box is the same rectangle R. Keep in mind that 0 > near > far. Maps 
the viewing volume to [-1, 1] x [-1, 1] x [-1, 1]. */
void mat44Orthographic(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, 
        GLdouble far, GLdouble near, GLdouble proj[4][4]){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            proj[i][j] = 0;
        }
    }
    proj[0][0] = 2 / (right - left);
    proj[1][1] = 2 / (top - bottom);
    proj[2][2] = -2 / (near - far);
    proj[0][3] = (-right - left) / (right - left);
    proj[1][3] = (-top - bottom) / (top - bottom);
    proj[2][3] = -(-near - far) / (near - far);
    proj[3][3] = 1;
}

/* Builds a 4x4 matrix that maps a projected viewing volume 
[-1, 1] x [-1, 1] x [-1, 1] to screen [0, w - 1] x [0, h - 1] x [-1, 1]. */
void mat44Viewport(GLdouble width, GLdouble height, GLdouble view[4][4]){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            view[i][j] = 0;
        }
    }
    view[0][0] = (width - 1) / 2;
    view[1][1] = (height - 1) / 2;
    view[2][2] = 1;
    view[0][3] = (width - 1) / 2;
    view[1][3] = (height - 1) / 2;
    view[3][3] = 1;
}

/* Builds a 4x4 matrix representing perspective projection. The viewing frustum 
is contained between the near and far planes, with 0 > near > far. On the near 
plane, the frustum is the rectangle R = [left, right] x [bottom, top]. On the 
far plane, the frustum is the rectangle (far / near) * R. Maps the viewing 
volume to [-1, 1] x [-1, 1] x [-1, 1]. */
void mat44Perspective(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, 
        GLdouble far, GLdouble near, GLdouble proj[4][4]){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            proj[i][j] = 0;
        }
    }
    proj[0][0] = (-2 * near) / (right - left);
    proj[1][1] = (-2 * near) / (top - bottom);
    proj[2][2] = -(-near - far) / (near - far);
    proj[0][2] = (right + left) / (right - left);
    proj[1][2] = (top + bottom) / (top - bottom);
    proj[3][2] = -1;
    proj[2][3] = -(2 * near * far) / (near - far);
}

/* We want to pass matrices into OpenGL, but there are two obstacles. First, 
our matrix library uses GLdouble matrices, but OpenGL 2.x expects GLfloat 
matrices. Second, C matrices are implicitly stored one-row-after-another, while 
OpenGL expects matrices to be stored one-column-after-another. This function 
plows through both of those obstacles. */
void mat44OpenGL(GLdouble m[4][4], GLfloat openGL[4][4]) {
	for (int i = 0; i < 4; i += 1)
		for (int j = 0; j < 4; j += 1)
			openGL[i][j] = m[j][i];
}

/* builds the 3 by 3 identity matrix */
void mat33Identity(GLdouble m[3][3]){
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            if(i == j) m[i][j] = 1;
            else m[i][j] = 0;
        }
    }
}

/* builds the 4 by 4 identity matrix */
void mat44Identity(GLdouble m[4][4]){
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(i == j) m[i][j] = 1;
            else m[i][j] = 0;
        }
    }
}