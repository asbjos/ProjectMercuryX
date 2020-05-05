#pragma once
// ==============================================================
//				Header file for Mercury Redstone.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2019
// 
// Based on Project Mercury addon by "estar", with permission.
// This code is my own work.
// 
// Thank you to Rob Conley and the rest of the original Project
// Mercury team. Especially thank you for releasing the source.
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================


// ==============================================================
// Some vessel parameters
// ==============================================================

//FILEHANDLE jetVaneLogFile;

const VECTOR3 OFS_ADAPTRING1 = { -0.61, 0.35, 9.33 };
const VECTOR3 OFS_ADAPTRING2 = { 0.0, -0.82, 9.33 };
const VECTOR3 OFS_ADAPTRING3 = { 0.61, 0.35, 9.33 };
const VECTOR3 OFS_ADAPTCOVER1 = { 0.0, 0.91, 9.36 };
const VECTOR3 OFS_ADAPTCOVER2 = { -0.74, -0.43, 9.36 };
const VECTOR3 OFS_ADAPTCOVER3 = { 0.74, -0.43, 9.36 };

const double MERCURY_LENGTH_CAPSULE = 2.3042;
const double MERCURY_LENGTH_ABORT = 5.1604;
const double MERCURY_LENGTH_ANTHOUSE = 0.687;
const double MERCURY_LENGTH_SHIELD = 0.29;
const double MERCURY_LENGTH_LANDBAG = 0.97;
const double MERCURY_LENGTH_RETRO = 0.50;
const double STAGE1_LENGTH = 18.2;
const VECTOR3 MERCURY_OFS_CAPSULE = { 0.0, 0.0, (STAGE1_LENGTH + MERCURY_LENGTH_CAPSULE) / 2.0 };
const VECTOR3 REDSTONE_OFFSET = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2.0 - STAGE1_LENGTH / 2.0 }; // Also known as 0, 0, 0
const VECTOR3 REDSTONE_EXHAUST_POS = { 0.0, 0.0, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0 };
const VECTOR3 REDSTONE_EXHAUST_DRI = { 0.0, 0.0, 1.0 };
const VECTOR3 ABORT_OFFSET = { 0.0, -0.04, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2.0 + MERCURY_LENGTH_ABORT / 2.0 };
const VECTOR3 MERCURY_CAPSULE_OFFSET = _V(0.0, 0.0, 0.0);
//const VECTOR3 MERCURY_CAPSULE_OFFSET = _V(0.0, 0.0, -0.60); // sets an offset to the capsule to have CM at roll thrusters

const VECTOR3 MERCURY_OFS_SHIELD = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2 - MERCURY_LENGTH_SHIELD / 2 + MERCURY_LENGTH_SHIELD - .05 };
const VECTOR3 MERCURY_OFS_ANTHOUSE = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2 + MERCURY_LENGTH_ANTHOUSE / 2 };
const VECTOR3 MERCURY_OFS_ABORT = { 0.0, -0.04, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2 + MERCURY_LENGTH_ABORT / 2 };
const VECTOR3 MERCURY_OFS_MAINCHUTE = { 0.0, 0.0, MERCURY_LENGTH_CAPSULE / 2 - .05 };
const VECTOR3 MERCURY_OFS_DROGUE = { -0.12, 0.08, -.2 + MERCURY_LENGTH_ANTHOUSE / 2 };
const VECTOR3 MERCURY_OFS_DROGUECOVER = { -0.12, 0.08, -0.07 + MERCURY_OFS_ANTHOUSE.z + MERCURY_LENGTH_ANTHOUSE / 2 };
const VECTOR3 MERCURY_OFS_LANDBAG = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2 + MERCURY_LENGTH_SHIELD - .05 - MERCURY_LENGTH_LANDBAG / 2 };
const VECTOR3 MERCURY_OFS_RETRO = { 0.0, 0.025, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2 - MERCURY_LENGTH_RETRO / 2 + .15 };
const VECTOR3 MERCURY_OFS_BEACON = { 0.0, 0.425, MERCURY_OFS_RETRO.z };
const VECTOR3 MERCURY_OFS_RETROCOVER1 = { -0.196, MERCURY_OFS_RETRO.y + 0.091, MERCURY_OFS_RETRO.z - .2 };
const VECTOR3 MERCURY_OFS_RETROCOVER2 = { -0.005, MERCURY_OFS_RETRO.y - 0.23, MERCURY_OFS_RETRO.z - .2 };
const VECTOR3 MERCURY_OFS_RETROCOVER3 = { 0.175, MERCURY_OFS_RETRO.y + 0.091, MERCURY_OFS_RETRO.z - .2 };
const VECTOR3 MERCURY_OFS_RETROSTRAP1 = { -0.03, MERCURY_OFS_RETRO.y + 0.46, MERCURY_OFS_RETRO.z + .093 };
const VECTOR3 MERCURY_OFS_RETROSTRAP2 = { -0.417, MERCURY_OFS_RETRO.y - 0.287, MERCURY_OFS_RETRO.z + .093 };
const VECTOR3 MERCURY_OFS_RETROSTRAP3 = { 0.425, MERCURY_OFS_RETRO.y - 0.27, MERCURY_OFS_RETRO.z + .093 };
const VECTOR3 MERCURY_OFS_EXPLOSIVEBOLT = { 0.0, 0.0, MERCURY_OFS_RETRO.z - .18 };

static const DWORD tchdwnLaunchNum = 4;
static TOUCHDOWNVTX tchdwnLaunch[tchdwnLaunchNum] = {
	// pos, stiff, damping, mu, mu long
	{_V(0.0, -1.0, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(-0.7, 0.7, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(0.7, 0.7, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(0.0, 0.0, MERCURY_OFS_ABORT.z + MERCURY_LENGTH_ABORT / 2.0), 1e7, 1e5, 10},
};

static const DWORD tchdwnTowSepNum = 4;
static TOUCHDOWNVTX tchdwnTowSep[tchdwnTowSepNum] = {
	// pos, stiff, damping, mu, mu long
	{_V(0.0, -1.0, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(-0.7, 0.7, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(0.7, 0.7, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(0.0, 0.0, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2.0), 1e7, 1e5, 10},
};

//static const DWORD tchdwnFlightNum = 4;
//static TOUCHDOWNVTX tchdwnFlight[tchdwnFlightNum] = {
//	// pos, stiff, damping, mu, mu long
//	{_V(0.0, -0.9461, -MERCURY_LENGTH_CAPSULE / 2.0 - MERCURY_LENGTH_LANDBAG), 1e4, 7e3, 10},
//	{_V(-0.669, 0.669, -MERCURY_LENGTH_CAPSULE / 2.0 - MERCURY_LENGTH_LANDBAG), 1e4, 7e3, 10},
//	{_V(0.669, 0.669, -MERCURY_LENGTH_CAPSULE / 2.0 - MERCURY_LENGTH_LANDBAG), 1e4, 7e3, 10},
//	{_V(0.0, 0.0, MERCURY_LENGTH_CAPSULE / 2.0), 1e4, 7e3, 10},
//};

//static const DWORD tchdwnFlightNum = 4;
//const double depression = -MERCURY_LENGTH_LANDBAG;
//const double stiffness = -1224.24 * G / (3 * depression);
//const double damping = 0.9 * 2 * sqrt(1224.24 * stiffness);
//static TOUCHDOWNVTX tchdwnFlight[tchdwnFlightNum] = {
//	// pos, stiff, damping, mu, mu long
//	{_V(0.0, -3.5, -MERCURY_LENGTH_CAPSULE / 2.0 - MERCURY_LENGTH_LANDBAG), stiffness, damping, 1e1},
//	{_V(-2.5, 2.5, -MERCURY_LENGTH_CAPSULE / 2.0 - MERCURY_LENGTH_LANDBAG), stiffness, damping, 1e1},
//	{_V(2.5, 2.5, -MERCURY_LENGTH_CAPSULE / 2.0 - MERCURY_LENGTH_LANDBAG), stiffness, damping, 1e1},
//	{_V(0.0, 0.0, MERCURY_LENGTH_CAPSULE / 2.0), stiffness, damping, 1e1},
//};


// This is the best one so far, but causes crash
static const DWORD tchdwnFlightNum = 4;
const double depression = 0.3;
const double stiffness = abs(-1224.24 * G / (3 * depression)); // abs for sanity check, as I have a tendency to forget signs
const double damping = 0.3 * 2 * sqrt(1224.24 * stiffness);
static TOUCHDOWNVTX tchdwnFlight[tchdwnFlightNum] = {
	// pos, stiff, damping, mu, mu long
	{_V(0.0, -3.5, -MERCURY_LENGTH_CAPSULE / 2.0 + depression), stiffness, damping, 1e1},
	{_V(-2.5, 2.5, -MERCURY_LENGTH_CAPSULE / 2.0 + depression), stiffness, damping, 1e1},
	{_V(2.5, 2.5, -MERCURY_LENGTH_CAPSULE / 2.0 + depression), stiffness, damping, 1e1},
	{_V(0.0, 0.0, MERCURY_LENGTH_CAPSULE / 2.0), stiffness, damping, 1e1},
};

static const DWORD tchdwnAbortNum = 4;
static TOUCHDOWNVTX tchdwnAbort[tchdwnAbortNum] = {
	// pos, stiff, damping, mu, mu long
	{_V(0.0, -1.0, -MERCURY_LENGTH_CAPSULE / 2.0), 1e7, 1e5, 10},
	{_V(-0.7, 0.7, -MERCURY_LENGTH_CAPSULE / 2.0), 1e7, 1e5, 10},
	{_V(0.7, 0.7, -MERCURY_LENGTH_CAPSULE / 2.0), 1e7, 1e5, 10},
	{_V(0.0, 0.0, MERCURY_LENGTH_ABORT / 2.0), 1e7, 1e5, 10},
};

// Contrail conditions. Calibrated to Liberty Bell 7 launch video, from T+80 to T+95 s
const double contrailBegin = 0.35; // Air density for contrail to begin
const double contrailEnd = 0.1; // Air density for contrail to end

const double STAGE1_MASS = 28440.0 - 607.5; // 28440, but now shaving off to get correct total weight (66 000 lbs = ) - 28089 kg
const double STAGE1_DRY_MASS = 3717; // Total booster dry weight 8 195 lbs
const double STAGE1_FUEL_MASS = STAGE1_MASS - STAGE1_DRY_MASS;
const double STAGE1_ISP_SL = 217.4 * G; // Value from MR-4
const double STAGE1_THRUST_SL = 79220.0 * 4.448; // Value from MR-4 (1 lb force = 4.448 N)
const double STAGE1_THRUST = 383e3; // For expected MR-BD data, vac thrust is 383e3 (from expected cutoff mass times expected cutoff acceleration). For actual MR-BD data, the value is 385e3 +- 12e3 N.
const double STAGE1_ISP_VAC = STAGE1_THRUST / STAGE1_THRUST_SL * STAGE1_ISP_SL;
//const double STAGE1_ISP_SL = 217.63*G; // Sea Level (235), 216 according to NTRS. MR-BD had 216.32 sec, which was 0.6 % below predicted
//const double STAGE1_THRUST = 383e3; // 414340, or 395892 (89 000 lbs). F = Isp * g0 * mdot. Flow rate for MR-BD was 364.19. 58.5 m/s^2 at cutoff of MR-BD. Predicted 57.0, which for predicted mass 14826 lb gives 383.3e3 N. This value is likely an underestimate
//const double STAGE1_ISP_VAC = STAGE1_THRUST / (78000.0 * 4.448) * STAGE1_ISP_SL; // This maybe has to be fixed. Don't know where to get a value, or if there is any calculation that can be done from other values.

const double ABORT_MASS_FUEL = 131.77; // For Little Joe it was 290.5 lb (19670022198 page 27)
const double ABORT_THRUST = 231308; // or 1267, or 231.3 kN, 52000 lb force
const double ABORT_ISP = 1.1 * ABORT_THRUST / ABORT_MASS_FUEL; // Calculated to give burn time 0.78 sec. But total impulse is 56795 lb s (p. 27 in same doc). Which gives burn time 1.1 sec
const double ABORT_MASS = 460.4; // or 580 kg. For Little Joe it was 1015 lbs. Total mass (wet)

const double RETRO_MASS = 237.0; // 237 kg, according to astronautix and http://www.braeunig.us/space/specs/mercury.htm
const double POSIGRADE_MASS_FUEL = 3.6; // Total 24 lb set (3.6 kg * 3), according to https://www.wired.com/2014/09/one-man-space-station-1960/. But edited to result dV 28 ft/s
const double POSIGRADE_THRUST = 1779; // MA-6 results pdf, page 14, 400 lb force (1779 N). Total dV is 28 ft/s. But that is from exerting force on booster, which cannot be simulated. Without it should be something like 15 ft/s
const double POSIGRADE_ISP = 1.0 * POSIGRADE_THRUST / POSIGRADE_MASS_FUEL; // Seems like Isp should be similar to retro. Is now 494 s
const double RETRO_MASS_FUEL = 20.0; // Astronautix, total 60 kg
const double RETRO_MASS_EMPTY = RETRO_MASS - RETRO_MASS_FUEL * 3 - POSIGRADE_MASS_FUEL * 3; // should be 177 kg
const double RETRO_THRUST = 4600; // MA-6 results pdf, page 14, 1000 lb force (4448 N). Total dV is 440 ft/s (134 m/s), but at an angle of 34 deg. Each retro produces 0.4 G acc (tot. 1.2 G) 
const double RETRO_ISP = 11.5 * RETRO_THRUST / RETRO_MASS_FUEL; // 10 sec. burn time. Isp should be approx 230 s (astronautix) or 233 s (old proj.mercury addon). Now is 226.7 s. Some sources cite 12 sec. MA-9 postlaunch report part 1 shows 11.5 s

const double CAPSULE_MASS = 1224.24; // Was 1224.24 kg. According to Astronautix and 19670022650 it is closer to 1100 kg, plus person inside, which becomes something like 1224 kg anyway
const double MERCURY_MASS = RETRO_MASS + CAPSULE_MASS;
const double LANDING_MASS = CAPSULE_MASS - 104.0; // Mass of drogue and antenna is 104 kg according to old sourcecode of mercury_antenna
const VECTOR3 MERCURY_ROT_DRAG = _V(0.05, 0.05, 0.0015); // From Apollo CM from AAPO (0.07, 0.07, 0.003). From old Mercury (0.7, 0.7, 0.1). Must allow 10 deg roll during reentry
const VECTOR3 MERCURY_ROT_DRAG_CHUTE = _V(10.0, 10.0, 7.0); // Also from Apollo CM, now with chutes, from AAPO
const VECTOR3 MERCURY_ROT_DRAG_DROGUE = _V(1.0, 1.0, 1.0); // guesstimate based on two other values

const double MERCURY_FUEL_MASS_AUTO = 14.51; // 32 pounds
const double MERCURY_FUEL_MASS_MAN = 11.3;
const double MERCURY_THRUST_ATT = 106.8; // 24 lb
const double MERCURY_THRUST_ROLL_ATT = 26.69; // 6 lb
const double MERCURY_ISP_ATT = 220 * G; // Also listed somewhere as 160 pound-seconds, but unsure how to convert
const double MERCURY_THRUST_ATT_LOW = 4.448; // 1 lb

// Inverse of time to operate
const double DROGUE_OPERATING_SPEED = 0.4; // was 0.4
const double MAIN_CHUTE_OPERATING_SPEED = 0.4; // was 0.4
const double LANDING_BAG_OPERATING_SPEED = 0.2;
const double PERISCOPE_OPERATING_SPEED = 0.2;
const double DESTABILISER_OPERATING_SPEED = 2.0; // spring loaded

// Periscope aparture sizes, from 19740075935_1974075935 page 72
const double PERISCOPE_NARROW = 19.0 * RAD / 2.0;
const double PERISCOPE_WIDE = 175.0 * RAD / 2.0; // Actually 175 deg, but Orbiter only supports up to 160.0 deg, so this will be truncated to 160 deg FOV by Orbiter

const double MET[19] = { 0.00, 24.0, 30.0, 35.0, 40.0, 45.0, 50.0, 55.0, 60.0, 68.0, 76.0, 84.0, 92.0, 100.0, 110.0, 116.0, 122.0, 132.0, 142.5 };
// 19670028606 says on page 33 that tilt program was from T+15 for MR-BD, MR-3 and MR-4. But Preliminary Evaluation of MR-BD documents says tilt program from T+24.3. So guess we must trust 24 sec?
// Pitch program at T+24 only for unmanned MR-flights. But MR-4 transcript says "Pitch is 77" at 1:11, which does not add up. Pitch data in 19670028606 page 53
//const double MET[19] =		{ 0.00, 15.0, 21.0, 26.0, 31.0, 36.0, 41.0, 46.0, 51.0, 59.0, 67.0, 75.0, 83.0, 91.00, 101.0, 107.0, 113.0, 123.0, 133.5 };
const double aimPitch[19] = { 90.0, 89.0, 88.0, 87.0, 86.0, 84.0, 82.0, 80.0, 76.0, 72.0, 68.0, 64.0, 60.0, 56.00, 54.00, 52.00, 50.00, 49.00, 49.00 };

class ProjectMercury : public VESSEL4 {
public:
	ProjectMercury(OBJHANDLE hVessel, int flightmodel);
	~ProjectMercury();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPostCreation();
	void clbkPreStep(double simt, double simdt, double mjd);
	void clbkPostStep(double simt, double simdt, double mjd);
	int clbkConsumeBufferedKey(DWORD key, bool down, char* kstate);
	bool clbkDrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp);
	bool clbkLoadPanel2D(int id, PANELHANDLE hPanel, DWORD viewW, DWORD viewH);
	bool clbkLoadGenericCockpit(void);
	void clbkLoadStateEx(FILEHANDLE scn, void* status);
	void clbkSaveState(FILEHANDLE scn);

	void RedstoneAutopilot(double simt, double simdt);
	void DisableAutopilot(bool turnOff);
	void DefineRudderAnimations(void);
	void DefineJetVaneAnimations(void);
	void CreateAirfoilsRedstone(void);

	void AuxDampingAuto(bool highThrust);
	void RetroAttitudeAuto(double simt, double simdt, bool retroAtt);
	bool SetPitchAuto(double targetPitch, bool highThrust);
	bool SetYawAuto(bool highThrust);
	bool SetRollAuto(bool highThrust);
	//void TurnAroundAuto(double simt, double simdt);
	void ReentryAttitudeAuto(double simt, double simdt);
	//void LowTorqueAttAuto(double simt, double simdt);
	//void PitchAttAuto(double simt, double simdt);
	void GRollAuto(double simt, double simdt);

	double EmptyMass(void);
	void TowerSeparation(void);
	void CheckAbortConditions(double simt, double simdt);

	void LaunchAbort(void);
	void CreateAbortThrusters(void);
	void InflightAbortSeparate(void);
	void OffPadAbortSeparate(void);
	void CapsuleSeparate(void);
	void CreatePosigradeRockets(void);
	void CreateRetroRockets(void);
	void PrepareReentry(void);
	void CreateRCS(void);
	void CreateAirfoils(void);
	void CreateAirfoilsEscape(void);
	void SwitchAttitudeMode(void);
	void SwitchPropellantSource(void);
	MATRIX3 RotationMatrix(VECTOR3 angles, bool xyz = FALSE);
	void DumpFuelRCS(void);
	VECTOR3 FlipX(VECTOR3 vIn);
	VECTOR3 FlipY(VECTOR3 vIn);

	double OrbitArea(double angle, double ecc);
	double OrbitalFrameSlipAngle(VECTOR3 pos, VECTOR3 vel);
	double OrbitalFrameSlipAngle2(VECTOR3 pos, VECTOR3 vel);

	void SeparateTower(bool noAbortSep);
	void SeparateRedstone(void);
	void SeparateRingsAndAdapters(VECTOR3 redstoneDirection);

	void SeparateRetroPack(bool deleteThrusters);
	void SeparateRetroCoverN(int i);
	void SeparateDrogueCover(void);
	void SeparateDrogue(void);
	void DeployDrogue(void);
	void DeployMainChute(void);
	void SeparateMainChute(void);
	void DeployLandingBag(void);

	void DefinePeriscopeAnimation(void);
	void DefineAntennaDestabiliser(void);
	void DefineDrogueAnimation(void);
	void DefineMainChuteAnimation(void);
	void DefineLandingBagAnimation(void);
	void AnimateAntennaDestabiliser(double simt, double simdt);
	void AnimatePeriscope(double simt, double simdt);
	void AnimateDrogueChute(double simt, double simdt);
	void AnimateMainChute(double simt, double simdt);
	void AnimateLandingBag(double simt, double simdt);

	// Random number generation. Used for failure simulation
	double GenerateRandom01(void);
	double GenerateRandomNorm(double a1 = 2.0, double a2 = 0.5);
	double GenerateRandomAngleNorm(double a1 = 2.0, double a2 = 0.5);
	double NormAngleDeg(double ang);
	void DisableAttitudeThruster(int num);

	// Functions that are common in both Redstone and Atlas, and which are called in the default Orbiter callback functions
	void MercuryGenericConstructor(void);
	void WriteFlightParameters(void);
	void ReadConfigSettings(FILEHANDLE cfg);
	void CreateCapsuleFuelTanks(void);
	void AddDefaultMeshes(void);
	void CapsuleGenericPostCreation(void);
	void DeleteRogueVessels(void);
	void CapsuleAutopilotControl(double simt, double simdt);
	void FlightReentryAbortControl(double simt, double simdt, double latit, double longit, double getAlt);
	void WriteHUDAutoFlightReentry(oapi::Sketchpad* skp, double simt, int* yIndexUpdate, char* cbuf);

	// Config settings
	double heightOverGround;
	double integratedSpeedLimit = 2130.0;
	double ampFactor = 0.10;
	double ampAdder = 0.05;
	double rudderLift = 1.7;
	double rudderDelay = 0.5;

	// HUD constants
	DWORD ScreenWidth, ScreenHeight, ScreenColour;
	int TextX0, TextY0, LineSpacing;
	int secondColumnHUDx = 28; // The x-pos of the second collumn on the HUD

private:
	static void vlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	static void vliftEscape(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftEscape(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	static void vliftRedstone(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftRedstone(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	THRUSTER_HANDLE th_main, th_rcsDummyBooster[6], escape_engine, thruster_retro[3], thruster_posigrade[3], thruster_man_py[4],
		thruster_man_roll[2], thruster_auto_py[4], thruster_auto_py_1lb[4], thruster_auto_roll[2], thruster_auto_roll_1lb[2],
		pitchup, pitchdown, yawleft, yawright, bankleft, bankright;
	PROPELLANT_HANDLE redstone_propellant, retro_propellant[3], posigrade_propellant[3], fuel_auto, fuel_manual, escape_tank;
	//THRUSTER_HANDLE th_main, th_rcs[14], th_group[4];
	PARTICLESTREAMSPEC contrail_main;
	PSTREAM_HANDLE contrail, rcsStream[18];
	bool contrailActive = true;
	bool suborbitalMission = true; // change to false for Mercury Atlas!

	MESHHANDLE redstone,
		capsule,
		adaptcover1,
		adaptcover2,
		adaptcover3,
		adaptring1,
		adaptring2,
		adaptring3,
		tower,
		antennahouse,
		droguecover,
		retrocover1,
		retrocover2,
		retrocover3,
		retrostrap1,
		retrostrap2,
		retrostrap3,
		explosivebolt,
		retro,
		drogue,
		mainChute,
		landingbag,
		periscopeMesh; // mesh handles

	UINT Redstone; // rocket mesh
	UINT Capsule; // capsule mesh
	UINT Adaptcover1; // etc
	UINT Adaptcover2;
	UINT Adaptcover3;
	UINT Adaptring1;
	UINT Adaptring2;
	UINT Adaptring3;
	UINT Tower;
	UINT Antennahouse;
	UINT Droguecover;
	UINT Retrocover1;
	UINT Retrocover2;
	UINT Retrocover3;
	UINT Retrostrap1;
	UINT Retrostrap2;
	UINT Retrostrap3;
	UINT Explosivebolt;
	UINT Retro;
	UINT Drogue;
	UINT Mainchute;
	UINT Landingbag;
	UINT PeriscopeMesh;

	// MGROUP_SCALE* DrogueDeploy;
	MGROUP_SCALE* MainChuteDeploy;
	MGROUP_SCALE* LandingBagDeploy;
	UINT DrogueDeployAnim, MainChuteDeployAnim, LandingBagDeployAnim, Rudder1Anim, Rudder2Anim, Rudder3Anim, Rudder4Anim, JetVaneAnim[4], PeriscopeDeployAnim, DestabiliserDeployAnim;
	ANIMATIONCOMPONENT_HANDLE DrogueDeployAnim1, MainChuteDeployAnim1, LandingBagDeployAnim1;
	int heatShieldGroup = 29;

	CTRLSURFHANDLE Rudders[4];

	ATTACHMENTHANDLE padAttach;

	// Actions
	bool separateTowerAction = false;
	bool separateCapsuleAction = false;
	bool separateRetroCoverAction[3] = { false, false, false };
	bool separateRetroPackAction = false;
	bool prepareReentryAction = false;
	bool separateDrogueCoverAction = false;
	bool separateDrogueAction = false;
	bool separateMainChuteAction = false;

	bool capsuleDefined = false;
	bool abort = false;
	bool abortConditionsMet = false;
	bool enableAbortConditions = true;
	double currentPitchAim = 90.0;
	bool spaceLaunch = false;
	bool engageRetro = false;
	double retroStartTime;
	bool autoPilot = false;
	double launchTime = 0.0;
	double boosterShutdownTime = 0.0;
	bool posigradeDampingActivated = false;
	double posigradeDampingTime = 0.0;
	bool turnAroundFinished = false;
	bool attitudeHold14deg = false;
	double integratedSpeed = 0.0;
	double integratedLongitudinalSpeed = 0.0;
	double integratedPitch = 90.0;
	double integratedYaw = 0.0;
	double integratedRoll = 0.0;
	bool CGshifted;
	double escapeLevel = 0.0;
	bool inFlightAbort = false;
	double abortTime = 0.0;
	double towerJetTime = 0.0;
	bool retroCoverSeparated[3] = { false, false, false };
	bool attitudeControlManual = true;
	bool attitudeFuelAuto = true;
	double RETRO_THRUST_LEVEL[3];
	bool abortDamping = true;
	bool rcsExists = false;
	bool engageFuelDump = false;
	bool retroAttitude = false;

	int showInfoOnHud = 0;	/* 0 = Both key commands and flight data
							   1 = Only flight data
							   2 = Nothing (only stock HUD)*/

	bool drogueCoverSeparated = false;
	bool drogueDeployed = false;
	bool drogueSeparated = false;
	bool mainChuteDeployed = false;
	bool mainChuteSeparated = false;
	double drogueProgress = 0.0;
	bool drogueMoving = false;
	bool drogueEndAnimation = false;
	double drogueDeployTime = 0.0;
	double drogueReefedTime;
	double mainChuteProgress = 0.0;
	double mainChuteProgressArea = 0.0;
	double mainChuteDeployTime = 0.0;
	bool mainChuteMoving = false;
	bool mainChuteEndAnimation = false;
	double mainChuteReefedTime;
	bool reefingWait = false;
	double reefingWaitStart = 0.0;
	bool reserveChuteDeployed = false;
	bool reserveChuteSeparated = false;
	bool landingBagDeployed = false;
	double landingBagProgress = 0.0;
	bool landingBagMoving = false;
	bool landingBagEndAnimation = false;
	double destabiliserProgress = 0.0;

	bool periscope = false;
	double oldFOV = 20.0 * RAD;
	bool narrowField = false;
	int oldHUDMode = HUD_SURFACE;
	double periscopeProgress = 0.0;

	double vesselAcceleration;
	int orbitCount = 0;
	int currentLandingTargetIdx = 0;
	bool passedBase = false;
	double oldAngle;
	bool launchTargetPosition = true;
	double lowGLat, lowGLong, lowGHeading;
	double entryAng, entryAngleToBase;
	VECTOR3 entryLoc, entryVel;
	// Defaults to MA-6 data
	int missionOrbitNumber = 3;
	double missionApogee = 267.43;
	double missionLandLat = 21.33; // deg
	double missionLandLong = 291.33; // deg, from Greenwich eastward
	bool noMissionLandLat = false;
	bool landingComputing = true;

	double historyMaxAltitude = 0.0;
	double historyPerigee = 1e10;
	double historyPeriod = 0.0;
	double historyBottomPrev = 1e10;
	double historyBottomPrevPrev = 1e10;
	double historyInclination = 0.0;
	double historyCutOffAlt = 0.0;
	double historyCutOffVel = 0.0;
	double historyCutOffAngl = 90.0;
	double historyCutOffLat = 0.0;
	double historyCutOffLong = 0.0;
	double historyMaxEarthSpeed = 0.0;
	double historyMaxSpaceSpeed = 0.0;
	double historyMaxLaunchAcc = 0.0;
	double historyMaxReentryAcc = 0.0;
	double historyLaunchLat;
	double historyLaunchLong;
	double historyLandLat;
	double historyLandLong;
	OBJHANDLE historyReference;
	double historyLaunchHeading = 105.0;
	double historyWeightlessTime = 0.0;

	enum capsule { FREEDOM7, LIBERTYBELL7, FRIENDSHIP7, AURORA7, SIGMA7, FAITH7, FREEDOM7II } CapsuleVersion;
	enum vesselstate { LAUNCH, TOWERSEP, FLIGHT, REENTRY, REENTRYNODROGUE, REENTRYMAIN, ABORT, ABORTNORETRO } VesselStatus, PreviousVesselStatus;
	//enum droguestate { D_CLOSED, D_DEPLOYED, D_OPENING, D_REEFED, D_UNREEFING, D_OPENED } DrogueStatus;
	enum chutestate { CLOSED, DEPLOYED, OPENING, REEFED, UNREEFING, OPENED } DrogueStatus, MainChuteStatus, ReserveChuteStatus;
	enum landingbagstate { L_CLOSED, L_DEPLOYED, L_OPENING, L_OPENED } LandingBagStatus;
	enum periscopestate { P_CLOSED, P_DEPLOYED, P_OPENING, P_CLOSING } PeriscopeStatus, DestabiliserStatus;
	enum rcsstate { MANUAL, AUTOHIGH, AUTOLOW } RcsStatus;
	enum autopilotstate { AUTOLAUNCH, POSIGRADEDAMP, TURNAROUND, PITCHHOLD, REENTRYATT, LOWG } AutopilotStatus;

	// Failure definitions
	enum failure { NONE, LOWGACTIVATE, LOWGDEACTIVE, ATTSTUCKON, ATTSTUCKOFF, ATTMODEOFF, RETROSTUCKOFF, RETROSTUCKON, RETRONOSEP, MAINCHUTETORN, NOLANDBAG, RETROCALCOFF, BOOSTERDEVIATIONSPEED, BOOSTERPROBLEM, ATTITUDEOFFSET, LASTENTRY } FailureMode = NONE; // LASTENTRY is used to dynamically get length of enum. Always have it last in the array
	bool difficultyHard = false; // Used for most extreme cases. If false and a hard case is chosen, no error occurs.
	double timeOfError = 1e10; // initialise to "infinity". This says when an error first occurs / breaks out
	double pitchOffset = 0.0, yawOffset = 0.0, rollOffset = 0.0; // radians
	double speedError = 0.0; // m/s
	double chuteDestroyFactor = 0.0; // 1 is completely mess, 0 is no problem
	int retroErrorNum = 10; // the retro to be failed. 10 is "infinity", must be either 0, 1 or 2
	int attitudeThrusterErrorNum = 100; // "infinity". Must be between 0 and 17
	bool logError = false;

	bool PMIcheck = false;
	double PMItime = 0.0;
	int PMIn = 0;

	int stuffCreated = 0;
	OBJHANDLE createdVessel[25]; // number is close to 20, but don't bother counting exactly
	bool createdAbove50km[25] = { false };
};
