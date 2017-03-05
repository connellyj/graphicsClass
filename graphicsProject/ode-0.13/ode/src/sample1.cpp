// Sample1.cpp by Kosei Demura 2005-2011
// My web site is http://demura.net
// This program uses the Open Dynamics Engine (ODE) by Russell Smith.
// The ODE web site is http://ode.org/

#include <ode/ode.h>
#include "/../../drawstuff/drawstuff.h"

#ifdef dDOUBLE
#define dsDrawSphere dsDrawSphereD
#endif

static dWorldID world;
dBodyID ball;
const dReal   radius = 0.2;
const dReal   mass   = 1.0;

static void simLoop (int pause)
{
    const dReal *pos,*R;

    dWorldStep(world,0.05);

    dsSetColor(1.0,0.0,0.0);
    pos = dBodyGetPosition(ball);
    R   = dBodyGetRotation(ball);
    dsDrawSphere(pos,R,radius);
}

void start()
{
    static float xyz[3] = {0.0,-3.0,1.0};
    static float hpr[3] = {90.0,0.0,0.0};
    dsSetViewpoint (xyz,hpr);
}

int main (int argc, char **argv)
{
    dReal x0 = 0.0, y0 = 0.0, z0 = 1.0;
    dMass m1;

    dsFunctions fn;
    fn.version = DS_VERSION;
    fn.start   = &start;
    fn.step    = &simLoop;
    fn.command = NULL;
    fn.stop    = NULL;
    fn.path_to_textures = "../../drawstuff/textures";

    dInitODE();
    world = dWorldCreate();
    dWorldSetGravity(world,0,0,-0.001);

    ball = dBodyCreate(world);
    dMassSetZero(&m1);
    dMassSetSphereTotal(&m1,mass,radius);
    dBodySetMass(ball,&m1);
    dBodySetPosition(ball, x0, y0, z0);

    dsSimulationLoop (argc,argv,352,288,&fn);

    dWorldDestroy (world);
    dCloseODE();

    return 0;
}
