/* Julia Connelly and Kerim Celik, 01/20/2017 */

typedef struct renRenderer renRenderer;
    
struct renRenderer {
    int unifDim;
    int texNum;
    int attrDim;
    int varyDim;
    depthBuffer *depth;
    void (*colorPixel)(renRenderer*, double[], texTexture*[], double[], double[]);
    void (*transformVertex)(renRenderer*, double[], double[], double[]);
    void (*updateUniform)(renRenderer*, double[], double[]);
};