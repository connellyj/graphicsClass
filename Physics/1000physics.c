/* By Julia Connelly and Kerim Celik 03/15/2017 
 * CS 311 - Computer Graphics
 * A Physics API that sits on top of the ODE. Designed with our
 * hardware 3D graphics engine in mind.
 *
 * Requires ODE installation (Open Dynamics Engine)
 * Some code modelled after tutorials from http://demura.net/english
 */
    
#include <ode/ode.h>

/* Used to specify whether or not a physics object is built with
 * a body and/or a geometry 
 */
#define pGEOM_ONLY 0
#define pBODY_ONLY 1
#define pGEOM_AND_BODY 2

/* Used to specify which shape a physics object is */
#define pSPHERE 0
#define pBOX 1
#define pCAPSULE 2
#define pCYLINDER 3

/* Defines a physics object as its body, geometry, mass, and shape */
typedef struct {
    dBodyID body;
    dGeomID geom;
    int shape;
    dMass massObj;
} pPhysicsObject;

/* Defines properties and spaces involved in physics collisions */
typedef struct {
    dSpaceID space;
    dJointGroupID contactGroup;
    dReal bouncy; // between 0.0 and 1.0
    dReal bounceVel; // minimum incoming velocity required for an object to bounce
    dReal mu;
    int mode;
} pCollisionSimulation;

/* Defines the physics simulation as the world, the collision properties, 
 * the physical properties, and the objects that exist in that world.
 */
typedef struct {
    int maxObjects;
    int objIndex;
    pPhysicsObject *objects;
    dWorldID world;
    pCollisionSimulation collider;
    dVector3 gravity;
} pPhysicsSimulation;

pPhysicsSimulation sim;

/* Helper function to initialize the collision space. */
void pInitCollider() {
    sim.collider.space = dHashSpaceCreate(0);
    sim.collider.contactGroup = dJointGroupCreate(0);
    sim.collider.bouncy = 0.0;
    sim.collider.bounceVel = 0.0;
    sim.collider.mu = dInfinity;
    sim.collider.mode = dContactBounce;
}

/* Initializes the physics world, must be called before any other 
 * physics-related functions are called. Also limits the maximum number
 * of physics objects that can exist in the world at one time.
 */
void pInitPhysics(int maxObjects){
    dInitODE();
    pInitCollider();
    sim.maxObjects = maxObjects;
    sim.objIndex = 0;
    sim.world = dWorldCreate();
    sim.gravity[0] = 0.0; sim.gravity[1] = 0.0; sim.gravity[2] = 0.0;
    dWorldSetGravity(sim.world, sim.gravity[0], sim.gravity[1], sim.gravity[2]);
    sim.objects = (pPhysicsObject *)malloc(maxObjects * sizeof(pPhysicsObject));
}

/*** MATRIX AND VECTOR CONVERTERS ***/

/* Converts matrices/vectors to and from dReal format used by ODE */
void pMat33ToPhysics(double m[3][3], dMatrix3 p) {
    int offset = 0;
    for(int i = 0; i < 3; i++) {
        p[offset] = (dReal) m[i][0];
        p[1 + offset] = (dReal) m[i][1];
        p[2 + offset] = (dReal) m[i][2];
        offset += 4;
    }
}

void pMat33FromPhysics(double m[3][3], const dReal *p) {
    int offset = 0;
    for(int i = 0; i < 3; i++) {
        m[i][0] = (double) p[offset];
        m[i][1] = (double) p[1 + offset];
        m[i][2] = (double) p[2 + offset];
        offset += 4;
    }
}

void pVec3FromPhysics(double v[3], const dReal *p) {
    for(int i = 0; i < 3; i++) {
        v[i] = (double) p[i];
    }
}

/*** END MATRIX AND VECTOR CONVERTERS ***/

/*** SETTERS ***/

void pSetBounciness(double bounciness) {
    if(bounciness <= 1.0 && bounciness >= 0.0) (sim.collider).bouncy = (dReal)bounciness;
    else printf("Bounciness must be between 0.0 and 1.0");
}

void pSetBounceVelocity(double velocity){
    (sim.collider).bounceVel = (dReal)velocity;
}

void pSetFriction(double mu){
    (sim.collider).mu = (dReal)mu;
}

void pSetGravity(double gravX, double gravY, double gravZ){
    sim.gravity[0] = (dReal)gravX;
    sim.gravity[1] = (dReal)gravY;
    sim.gravity[2] = (dReal)gravZ;
    dWorldSetGravity(sim.world, sim.gravity[0], sim.gravity[1], sim.gravity[2]);
}

void pSetCollisionMode(int mode){
    (sim.collider).mode = mode;
}

/* Places the CENTER (center of mass) of the object at x, y, z. */
void pSetPosition(int objLocation, double x, double y, double z) {
    dBodyID body = (sim.objects)[objLocation].body;
    dGeomID geom = (sim.objects)[objLocation].geom;
    if(body != NULL) {
        dBodySetPosition(body, (dReal) x, (dReal) y, (dReal) z);
    }
    if(geom != NULL) {
        dGeomSetPosition(geom, (dReal) x, (dReal) y, (dReal) z);
    }
}

void pSetRotation(int objLocation, double mat[3][3]) {
    dBodyID body = (sim.objects)[objLocation].body;
    dGeomID geom = (sim.objects)[objLocation].geom; 
    dMatrix3 rot;
    pMat33ToPhysics(mat, rot);
    if(body != NULL) dBodySetRotation(body, rot);
    if(geom != NULL) dGeomSetRotation(geom, rot);
}

/*** END SETTERS ***/

/*** GETTERS ***/

double pGetBounciness() {
    return (double)(sim.collider).bouncy;
}

double pGetBounceVelocity(){
    return (double)(sim.collider).bounceVel;
}

double pGetFriction(){
    return (double)(sim.collider).mu;
}

void pGetGravity(double grav[3]){
    pVec3FromPhysics(grav, sim.gravity);
}

double pGetCollisionMode(){
    return (double)(sim.collider).mode;
}

/* Gets the position of the CENTER (center of mass) of the object. */
void pGetPosition(int objLocation, double pos[3]) {
    if((sim.objects)[objLocation].body != NULL) {
        const dReal *p = dBodyGetPosition((sim.objects)[objLocation].body);
        pVec3FromPhysics(pos, p);
    }else printf("Physics does not apply forces to objects without bodies\n");
}

void pGetRotation(int objLocation, double rot[3][3]) {
    if((sim.objects)[objLocation].body != NULL) {
        const dReal *r = dBodyGetRotation((sim.objects)[objLocation].body);
        pMat33FromPhysics(rot, r);
    }else printf("Physics does not apply forces to objects without bodies\n");
}

/*** END GETTERS ***/

/* Creates a sphere in the physics world.
 * takes the object specifications, the mass, and the radius as arguments.
 * option must be one of: pGEOM_ONLY, pBODY_ONLY, pGEOM_AND_BODY.
 */
int pInitSphere(int option, double mass, double rad){
    if (sim.objIndex >= sim.maxObjects) {
        return -1;
    }
    if ((option != pBODY_ONLY) && (option != pGEOM_ONLY) && (option != pGEOM_AND_BODY)) {
        printf("Invalid physics object creation option selected.\n");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pSPHERE;
    if (option == pGEOM_ONLY) newObj.body = NULL;
    else if (option == pBODY_ONLY) newObj.geom = NULL;
    if ((option == pGEOM_ONLY) || (option == pGEOM_AND_BODY)) {
        newObj.geom = dCreateSphere((sim.collider).space, (dReal) rad);
    }
    if (((option == pBODY_ONLY) || (option == pGEOM_AND_BODY)) == 1) {
        newObj.body = dBodyCreate(sim.world);
        dMass m;
        dMassSetZero(&m);
        dMassSetSphereTotal(&m, (dReal) mass, (dReal) rad);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY) dGeomSetBody(newObj.geom, newObj.body);
    (sim.objects)[sim.objIndex] = newObj;
    sim.objIndex++;
    return (sim.objIndex) - 1;
}

/* Creates a box in the physics world.
 * takes the object specifications, the mass, and the side lengths as arguments.
 * option must be one of: pGEOM_ONLY, pBODY_ONLY, pGEOM_AND_BODY.
 */
int pInitBox(int option, double mass, double sideX, double sideY, double sideZ) {
    if (sim.objIndex >= sim.maxObjects) return -1;
    if ((option != pBODY_ONLY) && (option != pGEOM_ONLY) && (option != pGEOM_AND_BODY)) {
        printf("Invalid physics object creation option selected.\n");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pBOX;
    if(option == pGEOM_ONLY) newObj.body = NULL;
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if ((option == pGEOM_ONLY) || (option == pGEOM_AND_BODY)){
        newObj.geom = dCreateBox((sim.collider).space, (dReal) sideX, (dReal) sideY, (dReal) sideZ);
    }
    if ((option == pBODY_ONLY) || (option == pGEOM_AND_BODY)){
        newObj.body = dBodyCreate(sim.world);
        dMass m;
        dMassSetZero(&m);
        dMassSetBoxTotal(&m, (dReal) mass, (dReal) sideX, (dReal) sideY, (dReal) sideZ);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY) dGeomSetBody(newObj.geom, newObj.body);
    (sim.objects)[sim.objIndex] = newObj;
    sim.objIndex++;
    return (sim.objIndex) - 1;
}

/* Creates a capsule in the physics world.
 * takes the object specifications, the mass, the radius, and the length as arguments.
 * option must be one of: pGEOM_ONLY, pBODY_ONLY, pGEOM_AND_BODY.
 * Assumes that length is the total length of the capsule.
 */
int pInitCapsule(int option, double mass, double rad, double length) {
    length -= 2 * rad;
    if (length <= 0) length = 0;
    if (sim.objIndex >= sim.maxObjects) return -1;
    if ((option != pBODY_ONLY) && (option != pGEOM_ONLY) && (option != pGEOM_AND_BODY)) {
        printf("Invalid physics object creation option selected.\n");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pCAPSULE;
    if(option == pGEOM_ONLY) newObj.body = NULL;
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if ((option == pGEOM_ONLY) || (option == pGEOM_AND_BODY)) {
        newObj.geom = dCreateCapsule((sim.collider).space, (dReal) rad, (dReal) length);
    }
    if ((option == pBODY_ONLY) || (option == pGEOM_AND_BODY)) {
        newObj.body = dBodyCreate(sim.world);
        dMass m;
        dMassSetZero(&m);
        // assumes that the capsule's mass is oriented along it's y-axis
        dMassSetCapsuleTotal(&m, (dReal) mass, 2, (dReal) rad, (dReal) length);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY) dGeomSetBody(newObj.geom, newObj.body);
    (sim.objects)[sim.objIndex] = newObj;
    sim.objIndex++;
    return (sim.objIndex) - 1;
}

/* Creates a cylinder in the physics world.
 * takes the object specifications, the mass, the radius, and the length as arguments.
 * option must be one of: pGEOM_ONLY, pBODY_ONLY, pGEOM_AND_BODY.
 */
int pInitCylinder(int option, double mass, double rad, double length){
    if (sim.objIndex >= sim.maxObjects) return -1;
    if ((option != pBODY_ONLY) && (option != pGEOM_ONLY) && (option != pGEOM_AND_BODY)) {
        printf("Invalid physics object creation option selected.\n");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pCYLINDER;
    if(option == pGEOM_ONLY) newObj.body = NULL;
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if ((option == pGEOM_ONLY) || (option == pGEOM_AND_BODY)) {
        newObj.geom = dCreateCylinder((sim.collider).space, (dReal) rad, (dReal) length);
    }
    if ((option == pBODY_ONLY) || (option == pGEOM_AND_BODY)) {
        newObj.body = dBodyCreate(sim.world);
        dMass m;
        dMassSetZero(&m);
        // assumes that the cylinder's mass is oriented along it's y-axis
        dMassSetCylinderTotal(&m, (dReal) mass, 2, (dReal) rad, (dReal) length);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY) dGeomSetBody(newObj.geom, newObj.body);
    (sim.objects)[sim.objIndex] = newObj;
    sim.objIndex++;
    return (sim.objIndex) - 1;
}

/* Called by the physics engine whenever two geometries intersect (a collision). 
 * This method then determines how the objects react to their collision by creating
 * contact joints to represent each point of contact between them.
 */
void pNearCallback(void *data, dGeomID o1, dGeomID o2) {
    const int N = 10;
    dContact contact[N];
    int n =  dCollide(o1, o2, N, &contact[0].geom,sizeof(dContact));
    for (int i = 0; i < n; i++) {
        contact[i].surface.mode = sim.collider.mode;
        contact[i].surface.mu = sim.collider.mu;
        contact[i].surface.bounce = sim.collider.bouncy;
        contact[i].surface.bounce_vel = sim.collider.bounceVel;
        dJointID c = dJointCreateContact(sim.world, sim.collider.contactGroup, &contact[i]);
        dJointAttach (c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));
    }
}

/* Steps the physics simulation forward; must get called in the event loop. */
void pSimLoop() {
    double timeStep = 0.05;
    dSpaceCollide((sim.collider).space, 0, &pNearCallback);
    dWorldStep(sim.world, timeStep);
    dJointGroupEmpty((sim.collider).contactGroup);
}

/* Destroys all physics objects. */
void pPhysicsDestroy() {
    dWorldDestroy(sim.world);
    dJointGroupDestroy((sim.collider).contactGroup);
    free(sim.objects);
}