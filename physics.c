// by JC and KC on ... NEED TO DESTROY PHYSICS OBJECTS and be able to delete them from the scene
#include <ode/ode.h>

#define pGEOM_ONLY 0
#define pBODY_ONLY 1
#define pGEOM_AND_BODY 2

#define pSPHERE 0
#define pBOX 1
#define pCAPSULE 2
#define pCYLINDER 3

typedef struct {
    dBodyID body;
    dGeomID geom;
    int shape;
    dMass massObj;
} pPhysicsObject;

typedef struct {
    dSpaceID space;
    dJointGroupID contactGroup;
    double bouncy; // between 0.0 and 1.0
    double bounceVel;
    double mu;
    int mode;
    // possibly add other collision-determining values
} pCollisionSimulation;

typedef struct {
    int maxObjects;
    int objIndex; // keeps track of the next open space in the object array
    pPhysicsObject *objects;
    dWorldID world; // maybe worry about supporting multiple worlds???
    pCollisionSimulation collider; // maybe worry about supporting multiple spaces???
    dVector3 gravity;
} pPhysicsSimulation;

pPhysicsSimulation sim;

/* initialization fxns */
void pInitCollider() {
    sim.collider.space = dHashSpaceCreate(0);
    sim.collider.contactGroup = dJointGroupCreate(0);
    sim.collider.bouncy = 0.0;
    sim.collider.bounceVel = 0.0;
    sim.collider.mu = dInfinity;
    sim.collider.mode = dContactBounce;
}

void pInitPhysics(int maxObjects){
    dInitODE();
    pInitCollider();
    sim.objIndex = 0;
    sim.world = dWorldCreate();
    sim.gravity[0] = 0.0; sim.gravity[1] = 0.0; sim.gravity[2] = 0.0;
    dWorldSetGravity(sim.world, sim.gravity[0], sim.gravity[1], sim.gravity[2]);
    sim.objects = (pPhysicsObject *)malloc(maxObjects * sizeof(pPhysicsObject));
}

/* basic setter fxns */
void setBounciness(double bounciness) {
    if(bounciness <= 1.0 && bounciness >= 0.0) (sim.collider).bouncy = bounciness;
    else printf("Bounciness must be between 0.0 and 1.0");
}

void setBounceVelocity(double velocity){
    (sim.collider).bounceVel = velocity;
}

void setFriction(double mu){
    (sim.collider).mu = mu;
}

void setGravity(double gravX, double gravY, double gravZ){
    sim.gravity[0] = gravX;
    sim.gravity[1] = gravY;
    sim.gravity[2] = gravZ;
}

void setCollisionMode(int mode){
    (sim.collider).mode = mode;
}


// geom, body represent booleans allowing the user to ask for a geometry, a body, or both
int pInitSphere(int option, double mass, double rad){
    if (sim.objIndex >= sim.maxObjects) return -1;
    if (option != pBODY_ONLY || option != pGEOM_ONLY || option != pGEOM_AND_BODY) {
        printf("Invalid physics object creation option selected.");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pSPHERE;
    if(option == pGEOM_ONLY){
        newObj.body = NULL;
        // check to see if mass is not garbage by checking if the object has a body
    }
    if(option == pBODY_ONLY) {
        newObj.geom = NULL;
    }
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if (option == pGEOM_ONLY || option == pGEOM_AND_BODY){
        newObj.geom = dCreateSphere((sim.collider).space, (dReal) rad);
    }
    else if (option == pBODY_ONLY || option == pGEOM_AND_BODY){
        newObj.body = dBodyCreate(sim.world);
        dMass m;
        dMassSetZero(&m);
        dMassSetSphereTotal(&m, (dReal) mass, (dReal) rad);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY){
        dGeomSetBody(newObj.geom, newObj.body);
    }
    (sim.objects)[sim.objIndex] = newObj;
    sim.objIndex++;
    return (sim.objIndex) - 1;
}

int pInitBox(int option, double mass, double sideX, double sideY, double sideZ){
    if (sim.objIndex >= sim.maxObjects) return -1;
    if (option != pBODY_ONLY || option != pGEOM_ONLY || option != pGEOM_AND_BODY) {
        printf("Invalid physics object creation option selected.");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pBOX;
    if(option == pGEOM_ONLY){
        newObj.body = NULL;
    }
    if(option == pBODY_ONLY) {
        newObj.geom = NULL;
    }
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if (option == pGEOM_ONLY || option == pGEOM_AND_BODY){
        newObj.geom = dCreateBox((sim.collider).space, (dReal) sideX, (dReal) sideY, (dReal) sideZ);
    }
    else if (option == pBODY_ONLY || option == pGEOM_AND_BODY){
        newObj.body = dBodyCreate(sim.world);
        dMass m;
        dMassSetZero(&m);
        dMassSetBoxTotal(&m, (dReal) mass, (dReal) sideX, (dReal) sideY, (dReal) sideZ);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY){
        dGeomSetBody(newObj.geom, newObj.body);
    }
    (sim.objects)[sim.objIndex] = newObj;
    sim.objIndex++;
    return (sim.objIndex) - 1;
}

int pInitCapsule(int option, double mass, double rad, double length){
    if (sim.objIndex >= sim.maxObjects) return -1;
    if (option != pBODY_ONLY || option != pGEOM_ONLY || option != pGEOM_AND_BODY) {
        printf("Invalid physics object creation option selected.");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pCAPSULE;
    if(option == pGEOM_ONLY){
        newObj.body = NULL;
    }
    if(option == pBODY_ONLY) {
        newObj.geom = NULL;
    }
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if (option == pGEOM_ONLY || option == pGEOM_AND_BODY){
        newObj.geom = dCreateCapsule((sim.collider).space, (dReal) rad, (dReal) length);
    }
    else if (option == pBODY_ONLY || option == pGEOM_AND_BODY){
        newObj.body = dBodyCreate(sim.world);
        dMass m;
        dMassSetZero(&m);
        // assumes that the capsule's mass is oriented along it's y-axis (fix later?)
        dMassSetCapsuleTotal(&m, (dReal) mass, 2, (dReal) rad, (dReal) length);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY){
        dGeomSetBody(newObj.geom, newObj.body);
    }
    (sim.objects)[sim.objIndex] = newObj;
    sim.objIndex++;
    return (sim.objIndex) - 1;
}

int pInitCylinder(int option, double mass, double rad, double length){
    if (sim.objIndex >= sim.maxObjects) return -1;
    if (option != pBODY_ONLY || option != pGEOM_ONLY || option != pGEOM_AND_BODY) {
        printf("Invalid physics object creation option selected.");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pCYLINDER;
    if(option == pGEOM_ONLY){
        newObj.body = NULL;
    }
    if(option == pBODY_ONLY) {
        newObj.geom = NULL;
    }
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if (option == pGEOM_ONLY || option == pGEOM_AND_BODY){
        newObj.geom = dCreateCylinder((sim.collider).space, (dReal) rad, (dReal) length);
    }
    else if (option == pBODY_ONLY || option == pGEOM_AND_BODY){
        newObj.body = dBodyCreate(sim.world);
        dMass m;
        dMassSetZero(&m);
        // assumes that the cylinder's mass is oriented along it's y-axis (fix later?)
        dMassSetCylinderTotal(&m, (dReal) mass, 2, (dReal) rad, (dReal) length);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY){
        dGeomSetBody(newObj.geom, newObj.body);
    }
    (sim.objects)[sim.objIndex] = newObj;
    sim.objIndex++;
    return (sim.objIndex) - 1;
}

void pMat33ToPhysics(float m[3][3], dMatrix3 p) {
    int offset = 0;
    for(int i = 0; i < 3; i++) {
        p[offset] = (dReal) m[i][0];
        p[1 + offset] = (dReal) m[i][1];
        p[2 + offset] = (dReal) m[i][2];
        offset += 4;
    }
}

void pMat33FromPhysics(float m[3][3], const dMatrix3 p) {
    int offset = 0;
    for(int i = 0; i < 9; i += 3) {
        m[i][0] = (float) p[i + offset];
        m[i][1] = (float) p[i + 1 + offset];
        m[i][2] = (float) p[i + 2 + offset];
        offset++;
    }
}

void pVec3FromPhysics(float v[3], const dVector3 p) {
    for(int i = 0; i < 3; i++) {
        v[i] = (float) p[i];
    }
}

void pSetPosition(int objLocation, double x, double y, double z) {
    dBodyID body = (sim.objects)[objLocation].body;
    dGeomID geom = (sim.objects)[objLocation].geom;
    if(body != NULL) dBodySetPosition(body, (dReal) x, (dReal) y, (dReal) z);
    if(geom != NULL) dGeomSetPosition(geom, (dReal) x, (dReal) y, (dReal) z);
}


void pGetPosition(int objLocation, float pos[3]) {
    if((sim.objects)[objLocation].body != NULL) {
        const dReal *p = dBodyGetPosition((sim.objects)[objLocation].body);
        pVec3FromPhysics(pos, p);
    }else printf("Physics does not apply forces to objects without bodies");
}

void pSetRotation(int objLocation, float mat[3][3]) {
    dBodyID body = (sim.objects)[objLocation].body;
    dGeomID geom = (sim.objects)[objLocation].geom; 
    dMatrix3 rot;
    pMat33ToPhysics(mat, rot);
    if(body != NULL) dBodySetRotation(body, rot);
    if(geom != NULL) dGeomSetRotation(geom, rot);
}

void pGetRotation(int objLocation, float rot[3][3]) {
    if((sim.objects)[objLocation].body != NULL) {
        const dReal *r = dBodyGetRotation((sim.objects)[objLocation].body);
        pMat33FromPhysics(rot, r);
    }else printf("Physics does not apply forces to objects without bodies");
}

void pNearCallback(void *data, dGeomID o1, dGeomID o2) {
    const int N = 10;
    dContact contact[N];
    int n =  dCollide(o1,o2,N,&contact[0].geom,sizeof(dContact));
    for (int i = 0; i < n; i++) {
        contact[i].surface.mode = sim.collider.mode;
        contact[i].surface.mu = sim.collider.mu;
        contact[i].surface.bounce = sim.collider.bouncy; // (0.0~1.0) restitution parameter
        contact[i].surface.bounce_vel = sim.collider.bounceVel; // minimum incoming velocity for bounce
        dJointID c = dJointCreateContact(sim.world, sim.collider.contactGroup, &contact[i]);
        dJointAttach (c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));
    }
}

void pSimLoop() {
    dSpaceCollide((sim.collider).space, 0, &pNearCallback);
    dWorldStep(sim.world, 0.05);
    dJointGroupEmpty((sim.collider).contactGroup);
}
