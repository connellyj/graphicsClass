ODE 0.13 Release Notes
======================

This is the 0.13 release of the Open Dynamics Engine.

Among the new features:

* Stable, implicit gyroscopic forces (patch #185 by Joseph Cooper). Bodies with gyroscopic forces won't gain energy anymore.
* New joint: transmission joint (patch #182 by Dimitris Papavasiliou).
* Contacts now have rolling friction (patch #184 by Joseph Cooper).
* Removed the need for defining dSINGLE/dDOUBLE; this is stored now in the generated ode/precision.h header.
* New joint: Double Ball (AKA "distance joint").
* New joint: Double Hinge.
* Threaded execution support interface added (experimental).

Many bugs were fixed too:

* Joint feedback forces application fixed in QuickStep implementation
* Bug #89 by Luc: (dJointAddSliderForce() adds a zero force when the parent body is NULL)
* Bug #88 by Luc: heightfield data assigned to a wrong field in dGeomHeightfieldSetHeightfieldData
* Fixed cylinder AABB computation.
* Removed ALLOCA calls from dHashSpace; it should not depend on stack size limits anymore.
* Bugs in AMotor joint (patch #181)
* Bugs in PU joint (patch #186)
* Fixed issues reported in patches #151 and #22 (collisions with SAPSpace and QuadTreeSpace might not work because geometries list was misused in them).
* Fixed zero comparisons in OPCODE to use relative error instead of absolute epsilon value (found by Bill Sellers)
* Bug #80 by Georg Martius: better handling of capsule-box with deep penetrations.
* Fixed zero-mu issues: now either mu or mu2 can be set to zero. 

... and more!


After this release ODE is migrating from SourceForge + SVN to Bitbucket + Hg.
Check the wiki for the updated instructions on accessing the Hg repository.

