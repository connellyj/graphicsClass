Project Goals:
- Create a physics API for our 3D graphics engine using the Open Dynamics Engine (ODE).
- Integrate our own API into our scene graph in a way that makes as much sense as possible.

Project Structure:
- NOTE: all files should have compile instructions at the top.
- 800demo.c: This is our very first functional program. The physics is hardcoded into main 
    because we didn't have the API created yet. The box is just a geometry and the sphere is 
    a geometry and a body. Use the 'A' and 'Z' keys to adjust gravity up and down.
- 900demo.c: The physics is still hardcoded, we're just using a more complicated scene.
    - 900pancake.c: A funny consequence of incorrectly converting from ODEs 3x4 matrices.
- 1000mainPhysics.c: This is the same scene as 900demo.c, but we're using our physics API (also
    the plane is actually the right size, so the sphere will roll off in the right place).
- 1010mainFamily.c: We integrated physics into the scene graph. This is an example showing what
    what happens when you have to physics bodies with one as a child of the other. This setup
    results in unrealistic behavior. Use the arrow keys to translate the sphere around in the scene,
    but also just watch it fall without moving it.
- 1010mainChild.c: This is an example of physics integration in the scene graph when a physics
    body is the child of just a geometry. This setup results in more realstic behavior. Use the
    arrow keys to translate the sphere around in the scene.