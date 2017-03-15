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

/* Used to specify the type of joint */
#define pJOINT_BALL 0
#define pJOINT_HINGE 1
#define pJOINT_SLIDER 2
#define pJOINT_FIXED 3

typedef struct {
    dJointID joint;
    int shape;
} pJointObject;

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
    int maxJoints;
    int objIndex;
    int jointIndex;
    pPhysicsObject *objects;
    pJointObject *joints;
    dWorldID world;
    pCollisionSimulation collider;
    dVector3 gravity;
} pPhysicsSimulation;

pPhysicsSimulation sim;
// jointFeedback needed if you want to tell which part of the joint contributed force (necessary?)
// dJointFeedback *feedback = new dJointFeedback;

/* Helper function to initialize the collision space. */
void pInitCollider() {
    if ((&sim) == NULL){
        printf("Must initialize the physics simulation before attaching the collider.\n");
        return;
    }
    sim.collider.space = dHashSpaceCreate(0);
    sim.collider.contactGroup = dJointGroupCreate(0);
    sim.collider.bouncy = 0.0;
    sim.collider.bounceVel = 0.0;
    sim.collider.mu = dInfinity;
    sim.collider.mode = dContactBounce;
}

/* Initializes the physics world, must be called before any other 
 * physics-related functions are called. Also limits the maximum number
 * of physics objects and joints that can exist in the world at one time.
 */
void pInitPhysics(int maxObjects, int maxJoints){
    dInitODE();
    pInitCollider();
    sim.maxObjects = maxObjects;
    sim.maxJoints = maxJoints;
    sim.objIndex = 0;
    sim.jointIndex = 0;
    sim.world = dWorldCreate();
    sim.gravity[0] = 0.0; sim.gravity[1] = 0.0; sim.gravity[2] = 0.0;
    dWorldSetGravity(sim.world, sim.gravity[0], sim.gravity[1], sim.gravity[2]);
    sim.objects = (pPhysicsObject *)malloc(maxObjects * sizeof(pPhysicsObject));
    sim.joints = (pJointObject *)malloc(maxObjects * sizeof(pJointObject));
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
    if ((&((sim.objects)[objLocation])) == NULL){
        printf("Specified object could not be found.\n");
        return;
    }
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
    if ((&((sim.objects)[objLocation])) == NULL){
        printf("Specified object could not be found.\n");
        return;
    }
    dBodyID body = (sim.objects)[objLocation].body;
    dGeomID geom = (sim.objects)[objLocation].geom; 
    dMatrix3 rot;
    pMat33ToPhysics(mat, rot);
    if(body != NULL) dBodySetRotation(body, rot);
    if(geom != NULL) dGeomSetRotation(geom, rot);
}

void pSetForce(int objLocation, double x, double y, double z){
    if ((&((sim.objects)[objLocation])) == NULL){
        printf("Specified object could not be found.\n");
        return;
    }
    if((sim.objects)[objLocation].body == NULL) {
        printf("Specified object does not have a physics body.\n");
        return;
    }
    dReal pX = (dReal) x; dReal pY = (dReal) y; dReal pZ = (dReal) z;
    dBodySetForce((sim.objects)[objLocation].body, pX, pY, pZ);
}

void pSetTorque(int objLocation, double x, double y, double z){
    if ((&((sim.objects)[objLocation])) == NULL){
        printf("Specified object could not be found.\n");
        return;
    }
    if((sim.objects)[objLocation].body == NULL) {
        printf("Specified object does not have a physics body.\n");
        return;
    }
    dReal pX = (dReal) x; dReal pY = (dReal) y; dReal pZ = (dReal) z;
    dBodySetTorque((sim.objects)[objLocation].body, pX, pY, pZ);
}

/* Sets the location of the ball joint for the linked objects to be attached to. */
void pSetBallJointAnchor(int jointLocation, double x, double y, double z){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_BALL){
        printf("Specified joint is not a ball.\n");
        return;
    }
    dReal pX = (dReal) x; dReal pY = (dReal) y; dReal pZ = (dReal) z;
    dJointSetBallAnchor((sim.joints)[jointLocation].joint, pX, pY, pZ);
}

/* Sets the location of the hinge joint for the linked objects to rotate around. */
void pSetHingeJointAnchor(int jointLocation, double x, double y, double z){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_HINGE){
        printf("Specified joint is not a hinge.\n");
        return;
    }
    dReal pX = (dReal) x; dReal pY = (dReal) y; dReal pZ = (dReal) z;
    dJointSetHingeAnchor((sim.joints)[jointLocation].joint, pX, pY, pZ);
}

/* Sets the axis that the hinge joint rotates around. */
void pSetHingeJointAxis(int jointLocation, double x, double y, double z){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_HINGE){
        printf("Specified joint is not a hinge.\n");
        return;
    }
    dReal pX = (dReal) x; dReal pY = (dReal) y; dReal pZ = (dReal) z;
    dJointSetHingeAxis((sim.joints)[jointLocation].joint, pX, pY, pZ);
}

/* Sets the axis the two objects linked by the specified joint can slide along. */
void pSetSliderJointAxis(int jointLocation, double x, double y, double z){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_SLIDER){
        printf("Specified joint is not a slider.\n");
        return;
    }
    dReal pX = (dReal) x; dReal pY = (dReal) y; dReal pZ = (dReal) z;
    dJointSetSliderAxis((sim.joints)[jointLocation].joint, pX, pY, pZ);
}

/* Fixes the specified joint so no movement is possible around that joint.
 * According to the ODE manual, these joints are not typically used outside of debugging.
 */
void pSetFixedJoint(int jointLocation){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_FIXED){
        printf("Specified joint is not a fixed joint.\n");
        return;
    }
    dJointSetFixed((sim.joints)[jointLocation].joint);
}

/* Allows the user to specify certain characteristic of a hinge joint.
 * lowStop: joint's low stop angle, set to dInfinity to turn off.
 * hiStop: joint's high stop angle, set to dInfinity to turn off.
 * velocity: joint's desired motor velocity, set to zero if you don't want a motor.
 * maxForce:
 * We also set a few other parameters for all joints.
 */
void pSetHingeJointParams(int jointLocation, double lowStop, double hiStop, double velocity, double maxForce){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_HINGE){
        printf("Specified joint is not a hinge.\n");
        return;
    }
    dReal pLowStop, pHiStop, pVelocity, pMaxForce;
    dReal pBounce = 0.05; dReal pFudge = 0.9;
    pLowStop = (dReal) lowStop; pHiStop = (dReal) hiStop; 
    pVelocity = (dReal) velocity; pMaxForce = (dReal) maxForce;
    dJointSetHingeParam((sim.joints)[jointLocation].joint, dParamLoStop, pLowStop);
    dJointSetHingeParam((sim.joints)[jointLocation].joint, dParamHiStop, pHiStop);
    dJointSetHingeParam((sim.joints)[jointLocation].joint, dParamVel, pVelocity);
    dJointSetHingeParam((sim.joints)[jointLocation].joint, dParamFMax, pMaxForce);
    dJointSetHingeParam((sim.joints)[jointLocation].joint, dParamBounce, pBounce);
    dJointSetHingeParam((sim.joints)[jointLocation].joint, dParamFudgeFactor, pFudge);
}

/* Allows the user to specify certain characteristic of a slider joint.
 * lowStop: joint's low stop angle, set to dInfinity to turn off.
 * hiStop: joint's high stop angle, set to dInfinity to turn off.
 * velocity: joint's desired motor velocity, set to zero if you don't want a motor.
 * maxForce:
 * We also set a few other parameters for all joints.
 */
void pSetSliderJointParams(int jointLocation, double lowStop, double hiStop, double velocity, double maxForce){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_SLIDER){
        printf("Specified joint is not a slider.\n");
        return;
    }
    dReal pLowStop, pHiStop, pVelocity, pMaxForce;
    dReal pBounce = 0.05; dReal pFudge = 0.9;
    pLowStop = (dReal) lowStop; pHiStop = (dReal) hiStop; 
    pVelocity = (dReal) velocity; pMaxForce = (dReal) maxForce;
    dJointSetSliderParam((sim.joints)[jointLocation].joint, dParamLoStop, pLowStop);
    dJointSetSliderParam((sim.joints)[jointLocation].joint, dParamHiStop, pHiStop);
    dJointSetSliderParam((sim.joints)[jointLocation].joint, dParamVel, pVelocity);
    dJointSetSliderParam((sim.joints)[jointLocation].joint, dParamFMax, pMaxForce);
    dJointSetSliderParam((sim.joints)[jointLocation].joint, dParamBounce, pBounce);
    dJointSetSliderParam((sim.joints)[jointLocation].joint, dParamFudgeFactor, pFudge);
}

/* Disables the specified joint, causing it to stop affecting its related objects.
 * The joint retains all of its information like its anchor and axis, if it has any.
 */
void pDisableJoint(int jointLocation){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if (dJointIsEnabled((sim.joints)[jointLocation].joint) == 0){
        printf("Specified joint already disabled.\n");
        return;
    }
    dJointDisable((sim.joints)[jointLocation].joint);
}

/* Enables the specified joint, causing to to start affecting its related objects.
 * The joint affects its related objects based on the information in its properties.
 */
void pEnableJoint(int jointLocation){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if (dJointIsEnabled((sim.joints)[jointLocation].joint) == 1){
        printf("Specified joint already enabled.\n");
        return;
    }
    dJointEnable((sim.joints)[jointLocation].joint);
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
    if ((&((sim.objects)[objLocation])) == NULL){
        printf("Specified object could not be found.\n");
        return;
    }
    if((sim.objects)[objLocation].body != NULL) {
        const dReal *p = dBodyGetPosition((sim.objects)[objLocation].body);
        pVec3FromPhysics(pos, p);
    }else printf("Physics does not apply forces to objects without bodies.\n");
}

void pGetRotation(int objLocation, double rot[3][3]) {
    if ((&((sim.objects)[objLocation])) == NULL){
        printf("Specified object could not be found.\n");
        return;
    }
    if((sim.objects)[objLocation].body != NULL) {
        const dReal *r = dBodyGetRotation((sim.objects)[objLocation].body);
        pMat33FromPhysics(rot, r);
    }else printf("Physics does not apply forces to objects without bodies.\n");
}

/* Returns the torque of the object as an array of three doubles. */
void pGetObjTorque(int objLocation, double torque[3]){
    if ((&((sim.objects)[objLocation])) == NULL){
        printf("Specified object could not be found.\n");
        return;
    }
    if((sim.objects)[objLocation].body != NULL) {
        const dReal* get = dBodyGetTorque((sim.objects)[objLocation].body);
        pVec3FromPhysics(torque, get);
    }else printf("Cannot apply torque to an object without a physics body.\n");
}

/* Returns the force on the object as an array of three doubles. */
void pGetObjForce(int objLocation, double force[3]){
    if ((&((sim.objects)[objLocation])) == NULL){
        printf("Specified object could not be found.\n");
        return;
    }
    if((sim.objects)[objLocation].body != NULL) {
        const dReal* get = dBodyGetForce((sim.objects)[objLocation].body);
        pVec3FromPhysics(force, get);
    }else printf("Cannot apply force to an object without a physics body.\n");
}

int pGetJointShape(int jointLocation){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return -1;
    }
    return (((sim.joints)[jointLocation]).shape);
}

/* Returns 1 if the joint is enabled and 0 if it is disabled */
int pCheckJointEnabled(int jointLocation){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return -1;
    }
    return (dJointIsEnabled((sim.joints)[jointLocation].joint));
}

/* Given a hinge joint, this function places the parameters that the user can change into
 * a passed double array of size = 4. In the array, the specific parameters are arranged:
 * params[0] = hinge's low stop
 * params[1] = hinge's high stop
 * params[2] = hinge's velocity
 * params[3] = hinge's maximum force
 */
void pGetHingeParams(int jointLocation, double params[4]){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_HINGE){
        printf("Specified joint is not a hinge.\n");
        return;
    }
    double p1, p2, p3, p4;
    p1 = (double) dJointGetHingeParam((sim.joints)[jointLocation].joint, dParamLoStop);
    p2 = (double) dJointGetHingeParam((sim.joints)[jointLocation].joint, dParamHiStop);
    p3 = (double) dJointGetHingeParam((sim.joints)[jointLocation].joint, dParamVel);
    p4 = (double) dJointGetHingeParam((sim.joints)[jointLocation].joint, dParamFMax);
    params[0] = p1; params[1] = p2; params[2] = p3; params[3] = p4; 
}


/* Given a slider joint, this function places the parameters that the user can change into
 * a passed double array of size = 4. In the array, the specific parameters are arranged:
 * params[0] = hinge's low stop
 * params[1] = hinge's high stop
 * params[2] = hinge's velocity
 * params[3] = hinge's maximum force
 */
void pGetSliderParams(int jointLocation, double params[4]){
    if ((sim.joints)[jointLocation].joint == NULL){
        printf("Specified joint could not be found.\n");
        return;
    }
    if ((sim.joints)[jointLocation].shape != pJOINT_SLIDER){
        printf("Specified joint is not a slider.\n");
        return;
    }
    double p1, p2, p3, p4;
    p1 = (double) dJointGetSliderParam((sim.joints)[jointLocation].joint, dParamLoStop);
    p2 = (double) dJointGetSliderParam((sim.joints)[jointLocation].joint, dParamHiStop);
    p3 = (double) dJointGetSliderParam((sim.joints)[jointLocation].joint, dParamVel);
    p4 = (double) dJointGetSliderParam((sim.joints)[jointLocation].joint, dParamFMax);
    params[0] = p1; params[1] = p2; params[2] = p3; params[3] = p4; 
}

/*** END GETTERS ***/

/* Creates a joint in the physics world between two passed objects.
 * Takes the joint type specification, and the two objects to connect.
 * option must be one of: pJOINT_BALL, pJOINT_HINGE, pJOINT_SLIDER, pJOINT_FIXED
 * The properties of the joint are not yet initialized, use setter functions to do so.
 * Failing to use the setter functions to initialize joint properties
 * may lead to unrealistic behavior.
 */
int pInitJoint(int option, int objLoc1, int objLoc2){
    if ((&((sim.objects)[objLoc1])) == NULL){
        printf("First specified object could not be found.\n");
        return -1;
    }
    if ((sim.objects)[objLoc1].body == NULL){
        printf("Body of the first specified object could not be found.\n");
        return -1;
    }
    if ((&((sim.objects)[objLoc2])) == NULL){
        printf("Second specified object could not be found.\n");
        return -1;
    }
    if ((sim.objects)[objLoc1].body == NULL){
        printf("Body of the second specified object could not be found.\n");
        return -1;
    }
    if (sim.jointIndex >= sim.maxJoints) {
        printf("Physics joint maximum reached.\n");
        return -1;
    }
    pJointObject newJoint;
    if (option == pJOINT_BALL){
        newJoint.joint = dJointCreateBall(sim.world, 0);
        newJoint.shape = pJOINT_BALL;
    }
    else if (option == pJOINT_HINGE){
        newJoint.joint = dJointCreateHinge(sim.world, 0);
        newJoint.shape = pJOINT_HINGE;
    }
    else if (option == pJOINT_SLIDER){
        newJoint.joint = dJointCreateSlider(sim.world, 0);
        newJoint.shape = pJOINT_SLIDER;
    }
    else if (option == pJOINT_FIXED){
        newJoint.joint = dJointCreateFixed(sim.world, 0);
        newJoint.shape = pJOINT_FIXED;
        dJointSetFixed(newJoint.joint);
    }
    else{
        printf("Invalid joint object creation option selected.\n");
        return -1;
    }
    dJointAttach(newJoint.joint, (sim.objects)[objLoc1].body, (sim.objects)[objLoc2].body);
    (sim.joints)[sim.jointIndex] = newJoint;
    sim.jointIndex++;
    return (sim.jointIndex) - 1;
}

/* Creates a sphere in the physics world.
 * Takes the object specifications, the mass, and the radius as arguments.
 * option must be one of: pGEOM_ONLY, pBODY_ONLY, pGEOM_AND_BODY.
 */
int pInitSphere(int option, double mass, double rad){
    if (sim.objIndex >= sim.maxObjects) {
        printf("Physics object maximum reached.\n");
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
    free(sim.joints);
    free(sim.objects);
}