# ProjectMercuryX
Updated Project Mercury for Orbiter 2016
Made by Asbjørn ('asbjos' on Orbiter-Forum)

The file and folder layout may seem like a mess, but this is how it works:
- Common backbone capsule functions (thrusters, attitude, retrosequence, chute and landing bag animations, etc.) in MercuryCapsule.h.
- More dynamic features for one specific configuration (orbital landing calculator, HUD display, scenario reading, etc.) is in rocket files.
- Redstone configuration in ProjectMercuryRedstone.cpp and MercuryRedstone.h
- Atlas configuration in MercuryAtlas\MercuryAtlas.cpp and \MercuryAtlas.h
- All smaller unmanned components have their own single .cpp file, but the code is similar with eachother and main Redstone and Atlas files. 
- Excecutable for calculating launch heading and inclination for base targeting in LaunchAzimuthProgram\LaunchAzimuthTool.cpp.
