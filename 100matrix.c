/*** Methods filled in by Kerim Celik and Julia Connelly 01/20/2017 ***/

#include <stdio.h>

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(double m[2][2]) {
	for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 2; j++) {
            printf("%f ", m[j][i]);
        }
        printf("\n");
    }
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the 
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0, 
then the matrix is invertible, and its inverse is placed into mInv. */
double mat22Invert(double m[2][2], double mInv[2][2]) {
	double det = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
    if(det != 0.0) {
        double iDet = 1 / det;
        mInv[0][0] = m[1][1] * iDet;
        mInv[1][1] = m[0][0] * iDet;
        mInv[0][1] = m[0][1] * iDet * -1;
        mInv[1][0] = m[1][0] * iDet * -1;
    }
    return det;
}

/* Multiplies a 2x2 matrix m by a 2-column v, storing the result in mTimesV. 
The output should not */
void mat221Multiply(double m[2][2], double v[2], double mTimesV[2]) {
	for(int i = 0; i < 2; i++) {
        mTimesV[i] = (m[0][i] * v[0]) + (m[1][i] * v[1]);
    }
}

/* Fills the matrix m from its two columns. */
void mat22Columns(double col0[2], double col1[2], double m[2][2]) {
	for(int i = 0; i < 2; i++) {
        m[0][i] = col0[i];
        m[1][i] = col1[i];
    }
}

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat33Print(double m[3][3]) {
	for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            printf("%f ", m[j][i]);
        }
        printf("\n");
    }
}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. */
void mat333Multiply(double m[3][3], double n[3][3], double mTimesN[3][3]) {
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            mTimesN[i][j] = (n[i][0] * m[0][j]) + (n[i][1] * m[1][j]) + (n[i][2] * m[2][j]);
        }
    }
}

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. */
void mat331Multiply(double m[3][3], double v[3], double mTimesV[3]) {
    for(int i = 0; i < 3; i++) {
        mTimesV[i] = (m[0][i] * v[0]) + (m[1][i] * v[1]) + (m[2][i] * v[2]);
    }
}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous 
coordinates. More precisely, the transformation first rotates through the angle 
theta (in radians, counterclockwise), and then translates by the vector (x, y). 
*/
void mat33Isometry(double theta, double x, double y, double isom[3][3]) {
    double m[3][3] = {{cos(theta), sin(theta), 0}, 
                      {-1 * sin(theta), cos(theta), 0}, 
                      {0, 0, 1}};
    double n[3][3] = {{1, 0, 0}, 
                      {0, 1, 0}, 
                      {x, y, 1}};
    mat333Multiply(n, m, isom);
}

/* Helper function that builds a 3x3 matrix by adding the values of 
 * two passed matrices
 */
void mat33Add(double a[3][3], double b[3][3], double added[3][3]){
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            added[i][j] = a[i][j] + b[i][j];
        }
    }
}

/* Helper function that builds a 3x3 matrix by reflecting the values of 
 * a passed matrix across the diagonal
 */
void mat33Transpose(double a[3][3], double trans[3][3]){
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            trans[i][j] = a[j][i];
        }
    }
}

/* Helper function that builds a 3x3 matrix by scaling the values of 
 * a passed matrix by the passed scalar
 */
void mat33Scale(double a[3][3], double scaled[3][3], double s){
    for (int i = 0; i < 3; i++){
        for (int j = 0; j < 3; j++){
            scaled[i][j] = a[i][j] * s;
        }
    }
}

/* Helper function that builds a 3x3 matrix by taking the values from 
 * three dim-3 vectors and using each of them as the values of a column
 */
void mat33Columns(double col0[3], double col1[3], double col2[3], double m[3][3]){
    for(int i = 0; i < 2; i++) {
        m[0][i] = col0[i];
        m[1][i] = col1[i];
        m[2][i] = col2[i];
    }
}


/* Given a length-1 3D vector axis and an angle theta (in radians), builds the 
rotation matrix for the rotation about that axis through that angle. Based on 
Rodrigues' rotation formula R = I + (sin theta) U + (1 - cos theta) U^2. */
void mat33AngleAxisRotation(double theta, double axis[3], double rot[3][3]){
    double u[3][3] = {{0, axis[2], -axis[1]},
                      {-axis[2], 0, axis[0]},
                      {axis[1], -axis[0], 0}};
    double u2[3][3];
    mat333Multiply(u, u, u2);
    mat33Scale(u, u, sin(theta));
    mat33Scale(u2, u2, 1 - cos(theta));
    double id[3][3] = {{1, 0, 0},
                {0, 1, 0},
                {0, 0, 1}};
    mat33Add(id, u, rot);
    mat33Add(u2, rot, rot);
}

/* Given two length-1 3D vectors u, v that are perpendicular to each other. 
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds 
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(double u[3], double v[3], double a[3], double b[3], 
        double rot[3][3]){
    double w[3], c[3];
    vec3Cross(u, v, w); vec3Cross(a, b, c);
    double uvw[3][3], abc[3][3], trans[3][3];
    mat33Columns(u, v, w, uvw); mat33Columns(a, b, c, abc);
    mat33Transpose(uvw, trans);
    mat333Multiply(abc, trans, rot);
}

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat44Print(double m[4][4]) {
	for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            printf("%f ", m[j][i]);
        }
        printf("\n");
    }
}

/* Multiplies m by n, placing the answer in mTimesN. */
void mat444Multiply(double m[4][4], double n[4][4], double mTimesN[4][4]) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            mTimesN[i][j] = (n[i][0] * m[0][j]) + (n[i][1] * m[1][j])
                + (n[i][2] * m[2][j]) + (n[i][3] * m[3][j]);
        }
    }
}

/* Multiplies m by v, placing the answer in mTimesV. */
void mat441Multiply(double m[4][4], double v[4], double mTimesV[4]) {
    for(int i = 0; i < 4; i++) {
        mTimesV[i] = (m[0][i] * v[0]) + (m[1][i] * v[1])
            + (m[2][i] * v[2]) + (m[3][i] * v[3]);
    }
}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix 
representing the rotation followed in time by the translation. */
void mat44Isometry(double rot[3][3], double trans[3], double isom[4][4]) {
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            isom[i][j] = rot[i][j];
        }
    }
    for(int i = 0; i < 3; i++){
        isom[3][i] = trans[i];
    }
    for(int i = 0; i < 3; i++){
        isom[i][3] = 0;
    }
    isom[3][3] = 1;
}

/* Given a rotation and translation, forms the 4x4 homogeneous matrix 
representing the inverse translation followed in time by the inverse rotation. 
That is, the isom produced by this function is the inverse to the isom 
produced by mat44Isometry on the same inputs. */
void mat44InverseIsometry(double rot[3][3], double trans[3], 
        double isom[4][4]){
    for(int i = 0; i < 3; i++){
        trans[i] *= -1;
    }
    double inv[3][3];
    mat33Transpose(rot, inv);
    mat44Isometry(inv, trans, isom);
}