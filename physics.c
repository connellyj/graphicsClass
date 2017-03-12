// by JC and KC on ...
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

/* initialization fxns */
void pInitCollider(pCollisionSimulation collSim){
    collSim.space = dHashSpaceCreate(0);
    collSim.contactGroup = dJointGroupCreate(0);
    collSim.bouncy = 0.0;
    collSim.bounceVel = 0.0;
    collSim.mu = dInfinity;
    collSim.mode = dContactBounce;
}

void pInitPhysics(pPhysicsSimulation *sim, int maxObjects){
    dInitODE();
    pCollisionSimulation collider;
    pInitCollider(collider);
    sim->collider = collider;
    sim->objIndex = 0;
    sim->world = dWorldCreate();
    sim->gravity[0] = 0.0; sim->gravity[1] = 0.0; sim->gravity[2] = 0.0;
    dWorldSetGravity(sim->world, sim->gravity[0], sim->gravity[1], sim->gravity[2]);
    sim->objects = (pPhysicsObject *)malloc(maxObjects * sizeof(pPhysicsObject));
}

/* basic setter fxns */
void setBounciness(pPhysicsSimulation *sim, double bounciness){
    (sim->collider).bouncy = bounciness;
}

void setBounceVelocity(pPhysicsSimulation *sim, double velocity){
    (sim->collider).bounceVel = velocity;
}

void setFriction(pPhysicsSimulation *sim, double mu){
    (sim->collider).mu = mu;
}

void setGravity(pPhysicsSimulation *sim, double gravX, double gravY, double gravZ){
    sim->gravity[0] = gravX;
    sim->gravity[1] = gravY;
    sim->gravity[2] = gravZ;
}

void setCollisionMode(pPhysicsSimulation *sim, int mode){
    (sim->collider).mode = mode;
}


// geom, body represent booleans allowing the user to ask for a geometry, a body, or both
int pInitSphere(pPhysicsSimulation *sim, int option, double mass, double rad){
    if (sim->objIndex >= sim->maxObjects) return -1;
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
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if (option == pGEOM_ONLY || option == pGEOM_AND_BODY){
        newObj.geom = dCreateSphere((sim->collider).space, (dReal) rad);
    }
    else if (option == pBODY_ONLY || option == pGEOM_AND_BODY){
        newObj.body = dBodyCreate(sim->world);
        dMass m;
        dMassSetZero(&m);
        dMassSetSphereTotal(&m, mass, rad);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY){
        dGeomSetBody(newObj.geom, newObj.body);
    }
    (sim->objects)[sim->objIndex] = newObj;
    sim->objIndex++;
    return 0;
}

int pInitBox(pPhysicsSimulation *sim, int option, double mass, double sideX, double sideY, double sideZ){
    if (sim->objIndex >= sim->maxObjects) return -1;
    if (option != pBODY_ONLY || option != pGEOM_ONLY || option != pGEOM_AND_BODY) {
        printf("Invalid physics object creation option selected.");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pBOX;
    if(option == pGEOM_ONLY){
        newObj.body = NULL;
    }
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if (option == pGEOM_ONLY || option == pGEOM_AND_BODY){
        newObj.geom = dCreateBox((sim->collider).space, (dReal) sideX, (dReal) sideY, (dReal) sideZ);
    }
    else if (option == pBODY_ONLY || option == pGEOM_AND_BODY){
        newObj.body = dBodyCreate(sim->world);
        dMass m;
        dMassSetZero(&m);
        dMassSetBoxTotal(&m, mass, sideX, sideY, sideZ);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY){
        dGeomSetBody(newObj.geom, newObj.body);
    }
    (sim->objects)[sim->objIndex] = newObj;
    sim->objIndex++;
    return 0;
}

int pInitCapsule(pPhysicsSimulation *sim, int option, double mass, double rad, double length){
    if (sim->objIndex >= sim->maxObjects) return -1;
    if (option != pBODY_ONLY || option != pGEOM_ONLY || option != pGEOM_AND_BODY) {
        printf("Invalid physics object creation option selected.");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pCAPSULE;
    if(option == pGEOM_ONLY){
        newObj.body = NULL;
    }
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if (option == pGEOM_ONLY || option == pGEOM_AND_BODY){
        newObj.geom = dCreateCapsule((sim->collider).space, (dReal) rad, (dReal) length);
    }
    else if (option == pBODY_ONLY || option == pGEOM_AND_BODY){
        newObj.body = dBodyCreate(sim->world);
        dMass m;
        dMassSetZero(&m);
        // assumes that the capsule's mass is oriented along it's y-axis (fix later?)
        dMassSetCapsuleTotal(&m, mass, 2, rad, length);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY){
        dGeomSetBody(newObj.geom, newObj.body);
    }
    (sim->objects)[sim->objIndex] = newObj;
    sim->objIndex++;
    return 0;
}

int pInitCylinder(pPhysicsSimulation *sim, int option, double mass, double rad, double length){
    if (sim->objIndex >= sim->maxObjects) return -1;
    if (option != pBODY_ONLY || option != pGEOM_ONLY || option != pGEOM_AND_BODY) {
        printf("Invalid physics object creation option selected.");
        return -1;
    }
    pPhysicsObject newObj;
    newObj.shape = pCYLINDER;
    if(option == pGEOM_ONLY){
        newObj.body = NULL;
    }
    else if(option == pBODY_ONLY) newObj.geom = NULL;
    if (option == pGEOM_ONLY || option == pGEOM_AND_BODY){
        newObj.geom = dCreateCylinder((sim->collider).space, (dReal) rad, (dReal) length);
    }
    else if (option == pBODY_ONLY || option == pGEOM_AND_BODY){
        newObj.body = dBodyCreate(sim->world);
        dMass m;
        dMassSetZero(&m);
        // assumes that the cylinder's mass is oriented along it's y-axis (fix later?)
        dMassSetCylinderTotal(&m, mass, 2, rad, length);
        dBodySetMass(newObj.body, &m);
        newObj.massObj = m;
    }
    if (option == pGEOM_AND_BODY){
        dGeomSetBody(newObj.geom, newObj.body);
    }
    (sim->objects)[sim->objIndex] = newObj;
    sim->objIndex++;
    return 0;
}

int main(void) {
    return 0;
}