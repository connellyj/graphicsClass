/*** Methods filled in by Kerim Celik and Julia Connelly 01/10/2017 ***/

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

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. */
void mat333Multiply(double m[3][3], double n[3][3], double mTimesN[3][3])

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. */
void mat331Multiply(double m[3][3], double v[3], double mTimesV[3])

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous 
coordinates. More precisely, the transformation first rotates through the angle 
theta (in radians, counterclockwise), and then translates by the vector (x, y). 
*/
void mat33Isometry(double theta, double x, double y, double isom[3][3])