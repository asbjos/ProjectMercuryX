#pragma once
// ==============================================================
//				Header file for Mercury Atlas.
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

//FILEHANDLE pitchDataLogFile;
//FILEHANDLE PIDdebug;

// ==============================================================
// Some vessel parameters
// ==============================================================

typedef struct texture {
	char file[50];
	int w;
	int h;
} ATEX; // Asbjos Texture

const double MERCURY_LENGTH_CAPSULE = 2.3042;
const double MERCURY_LENGTH_ABORT = 5.1604;
const double MERCURY_LENGTH_ANTHOUSE = 0.687;
const double MERCURY_LENGTH_SHIELD = 0.29;
const double MERCURY_LENGTH_LANDBAG = 0.97;
const double MERCURY_LENGTH_RETRO = 0.50;
const double ATLAS_CORE_LENGTH = 20.90; // 19.83796; 
const double ATLAS_BOOSTER_LENGTH = 3.86;
const VECTOR3 ATLAS_CORE_OFFSET = { 0.0, 0.0, 0.0 }; // MERCURY_OFS_CAPSULE.z - 11.6 = 0.0
const VECTOR3 ATLAS_BOOSTER_OFFSET = { 0.0, 0.0, ATLAS_CORE_OFFSET.z - 8.6 }; // MERCURY_OFS_CAPSULE.z - 20.2 
const VECTOR3 CORE_EXHAUST_POS = { 0.0, 0.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0 };
const VECTOR3 BOOSTER_EXHAUST_POS = { 0.0, 1.38, ATLAS_BOOSTER_OFFSET.z - ATLAS_BOOSTER_LENGTH / 2.0};
const VECTOR3 VERNIER_EXHAUST_POS = { 1.90, 0.0, -6.822 + ATLAS_CORE_OFFSET.z };
const VECTOR3 TURBINE_EXHAUST_POS = { 0.60114, -0.1868404, -1.689692 + ATLAS_BOOSTER_OFFSET.z};
const VECTOR3 CORE_EXHAUST_DIR = { 0.0, 0.0, 1.0 };
const VECTOR3 BOOSTER_EXHAUST_DIR = { 0.0, 0.0, 1.0 };
const VECTOR3 VERNIER_EXHAUST_DIR = { -0.134382, 0.0, 0.99093 };
const VECTOR3 TURBINE_EXHAUST_DIR = { -sin(20 * RAD), 0.0, cos(20 * RAD) }; // pick angle to fit
//const VECTOR3 MERCURY_CAPSULE_OFFSET = _V(0.0, 0.0, -0.60); // sets an offset to the capsule to have CM at roll thrusters

const VECTOR3 MERCURY_OFS_MAINCHUTE = { 0.0, 0.0, MERCURY_LENGTH_CAPSULE / 2 - .05 };
const VECTOR3 MERCURY_OFS_DROGUE = { -0.12, 0.08, -.2 + MERCURY_LENGTH_ANTHOUSE / 2 };

//static const DWORD tchdwnLaunchNum = 4;
const VECTOR3 TOUCHDOWN_LAUNCH0 = _V(0.0, -1.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_LAUNCH1 = _V(-sqrt(0.5), sqrt(0.5), ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_LAUNCH2 = _V(sqrt(0.5), sqrt(0.5), ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_LAUNCH3 = _V(0.0, 0.0, 10.45 + 1.16 + 1.16 + 2.6 + MERCURY_LENGTH_ABORT / 2.0); // 10.45 + 1.16 + 1.16 + 2.6 is MERCURY_OFS_ABORT.z
//static TOUCHDOWNVTX tchdwnLaunch[tchdwnLaunchNum] = {
//	// pos, stiff, damping, mu, mu long
//	{TOUCHDOWN_LAUNCH0, 1e7, 1e5, 10},
//	{TOUCHDOWN_LAUNCH1, 1e7, 1e5, 10},
//	{TOUCHDOWN_LAUNCH2, 1e7, 1e5, 10},
//	{TOUCHDOWN_LAUNCH3, 1e7, 1e5, 10},
//};

//static const DWORD tchdwnTowSepNum = 4;
const VECTOR3 TOUCHDOWN_TOWSEP0 = _V(0.0, -1.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_TOWSEP1 = _V(-0.7, 0.7, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_TOWSEP2 = _V(0.7, 0.7, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_TOWSEP3 = _V(0.0, 0.0, 10.45 + 1.16 + MERCURY_LENGTH_CAPSULE / 2.0); // 10.45 + 1.16 is MERCURY_OFS_CAPSULE.z
//static TOUCHDOWNVTX tchdwnTowSep[tchdwnTowSepNum] = {
//	// pos, stiff, damping, mu, mu long
//	{TOUCHDOWN_TOWSEP0, 1e7, 1e5, 10},
//	{TOUCHDOWN_TOWSEP1, 1e7, 1e5, 10},
//	{TOUCHDOWN_TOWSEP2, 1e7, 1e5, 10},
//	{TOUCHDOWN_TOWSEP3, 1e7, 1e5, 10},
//};

//static const DWORD tchdwnFlightNum = 4;
const double depression = 0.3;
const double stiffness = abs(-1224.24 * G / (3 * depression)); // abs for sanity check, as I have a tendency to forget signs
const double damping = 0.3 * 2 * sqrt(1224.24 * stiffness);
const VECTOR3 TOUCHDOWN_FLIGHT0 = _V(0.0, -3.5, -MERCURY_LENGTH_CAPSULE / 2.0 + depression);
const VECTOR3 TOUCHDOWN_FLIGHT1 = _V(-2.5, 2.5, -MERCURY_LENGTH_CAPSULE / 2.0 + depression);
const VECTOR3 TOUCHDOWN_FLIGHT2 = _V(2.5, 2.5, -MERCURY_LENGTH_CAPSULE / 2.0 + depression);
const VECTOR3 TOUCHDOWN_FLIGHT3 = _V(0.0, 0.0, MERCURY_LENGTH_CAPSULE / 2.0);
//static TOUCHDOWNVTX tchdwnFlight[tchdwnFlightNum] = {
//	// pos, stiff, damping, mu, mu long
//	{TOUCHDOWN_FLIGHT0, stiffness, damping, 1e1},
//	{TOUCHDOWN_FLIGHT1, stiffness, damping, 1e1},
//	{TOUCHDOWN_FLIGHT2, stiffness, damping, 1e1},
//	{TOUCHDOWN_FLIGHT3, stiffness, damping, 1e1},
//};

//static const DWORD tchdwnAbortNum = 4;
const VECTOR3 TOUCHDOWN_ABORT0 = _V(0.0, -1.0, -MERCURY_LENGTH_CAPSULE / 2.0);
const VECTOR3 TOUCHDOWN_ABORT1 = _V(-0.7, 0.7, -MERCURY_LENGTH_CAPSULE / 2.0);
const VECTOR3 TOUCHDOWN_ABORT2 = _V(0.7, 0.7, -MERCURY_LENGTH_CAPSULE / 2.0);
const VECTOR3 TOUCHDOWN_ABORT3 = _V(0.0, 0.0, MERCURY_LENGTH_ABORT / 2.0);
//static TOUCHDOWNVTX tchdwnAbort[tchdwnAbortNum] = {
//	// pos, stiff, damping, mu, mu long
//	{TOUCHDOWN_ABORT0, 1e7, 1e5, 10},
//	{TOUCHDOWN_ABORT1, 1e7, 1e5, 10},
//	{TOUCHDOWN_ABORT2, 1e7, 1e5, 10},
//	{TOUCHDOWN_ABORT3, 1e7, 1e5, 10},
//};

// Contrail conditions. Calibrated to Sigma 7 launch video, from T+65 to T+77 s
const double contrailBegin = 0.35; // Air density for contrail to begin
const double contrailEnd = 0.1; // Air density for contrail to end
// Secondary contrail. From T+103 to T+141 s
const double contrailBegin2 = 6e-3;
const double contrailEnd2 = 1e-5;

const double CORE_MASS = 113050.0; // 28440, but now shaving off to get correct total weight (66 000 lbs = ) - 28089 kg
const double CORE_DRY_MASS = 2633.0; // From MA6_memos, sustainer & adapter dry weight is 5805 lb = 2633.1 kg. Value from old project was 2347 kg
const double CORE_FUEL_MASS = CORE_MASS - CORE_DRY_MASS;
const double CORE_ISP_SL = 215.0 * G; // Sea Level (215), astronautix
const double CORE_ISP_VAC = 309.0 * G;
const double CORE_THRUST = 363218.0; // astronautix. Or 253.5 kN (19670022198 page 27)

const double BOOSTER_MASS = 3050.0;
const double BOOSTER_DRY_MASS = 3050.0; // Total booster dry weight 8 195 lbs
//const double BOOSTER_FUEL_MASS = CORE_MASS - CORE_DRY_MASS; // uses core fuel
const double BOOSTER_ISP_SL = 248.0 * G; // Sea Level (248), astronautix
const double BOOSTER_ISP_VAC = 282.0 * G;
const double BOOSTER_THRUST = 1517422.0 / 2.0; // Two thrusters, totaling 1.5 MN

const double VERNIER_THRUST_VAC = 2975.0; // https://www.scss.tcd.ie/Stephen.Farrell/ipn/background/Braeunig/specs/atlas.htm Or 4448 N (19630012071 page 97)
const double  VERNIER_ISP_SL = 172.0 * G;
const double  VERNIER_ISP_VAC = 231.0 * G;
const VECTOR3 VERNIER1_OFFSET = _V(1.829, 0.023, -6.614) + ATLAS_CORE_OFFSET;
const VECTOR3 VERNIER2_OFFSET = _V(-1.841, -0.053, -6.614) + ATLAS_CORE_OFFSET;

const double ABORT_MASS_FUEL = 131.77; // For Little Joe it was 290.5 lb (19670022198 page 27)
const double ABORT_THRUST = 231308; // or 1267, or 231.3 kN, 52000 lb force
const double ABORT_ISP = 1.1 * ABORT_THRUST / ABORT_MASS_FUEL; // Calculated to give burn time 0.78 sec. But total impulse is 56795 lb s (p. 27 in same doc). Which gives burn time 1.1 sec
const double ABORT_MASS = 460.4; // or 580 kg. For Little Joe it was 1015 lbs. Total mass (wet)

const double RETRO_MASS = 237.0; // 237 kg, according to astronautix and http://www.braeunig.us/space/specs/mercury.htm
const double POSIGRADE_THRUST = 1779; // MA-6 results pdf, page 14, 400 lb force (1779 N). Total dV is 28 ft/s. But that is from exerting force on booster, which cannot be simulated. Without it should be something like 15 ft/s
const double POSIGRADE_ISP = 1.0 * POSIGRADE_THRUST / 3.6; // Seems like Isp should be similar to retro. Is now 494 s. 3.6 is posigrade_mass_fuel
const double RETRO_MASS_FUEL = 20.0; // Astronautix, total 60 kg
const double RETRO_MASS_EMPTY = RETRO_MASS - RETRO_MASS_FUEL * 3 - 3.6 * 3; // should be 177 kg. 3.6 is posigrade_mass_fuel
const double RETRO_THRUST = 5590.0; // MA-6 results pdf, page 14, 1000 lb force (4448 N). Total dV is 440 ft/s (134 m/s), but at an angle of 34 deg. Each retro produces 0.4 G acc (tot. 1.2 G) 
const double RETRO_ISP = 11.5 * RETRO_THRUST / RETRO_MASS_FUEL; // 10 sec. burn time. Isp should be approx 230 s (astronautix) or 233 s (old proj.mercury addon). Now is 226.7 s. Some sources cite 12 sec. MA-9 postlaunch report part 1 has 11.5 second firing time

const double CAPSULE_MASS = 1224.24; // Was 1224.24 kg. According to Astronautix and 19670022650 it is closer to 1100 kg, plus person inside, which becomes something like 1224 kg anyway
const double MERCURY_MASS = RETRO_MASS + CAPSULE_MASS;
const double LANDING_MASS = CAPSULE_MASS - 104.0; // Mass of drogue and antenna is 104 kg according to old sourcecode of mercury_antenna
const VECTOR3 MERCURY_ROT_DRAG = _V(0.05, 0.05, 0.0015); // From Apollo CM from AAPO (0.07, 0.07, 0.003). From old Mercury (0.7, 0.7, 0.1). Must allow 10 deg roll during reentry
const VECTOR3 MERCURY_ROT_DRAG_CHUTE = _V(10.0, 10.0, 7.0); // Also from Apollo CM, now with chutes, from AAPO
const VECTOR3 MERCURY_ROT_DRAG_DROGUE = _V(1.0, 1.0, 1.0); // guesstimate based on two other values

const double MERCURY_FUEL_MASS_AUTO = 14.51; // 32 pounds
const double MERCURY_FUEL_MASS_MAN = 10.61; // 23.4 pounds (MercuryFamiliarizationManual page 158, although 20150018552 page 9 lists 23 1/2)
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

// Periscope aparture sizes from 19740075935_1974075935 page 72
const double PERISCOPE_NARROW = 19.0 * RAD / 2.0;
const double PERISCOPE_WIDE = 175.0 * RAD / 2.0; // Actually 175 deg, but Orbiter only supports up to 160.0 deg, so this will be truncated to 160 deg FOV by Orbiter
const double PERISCOPE_ANGLE = (90.0 - 14.5) * RAD; // 19630012071 page 291
const VECTOR3 CAMERA_DIRECTION = _V(0, -sin(PERISCOPE_ANGLE), cos(PERISCOPE_ANGLE));
const VECTOR3 CAMERA_OFFSET = _V(0.0, -0.6, -0.2);

const int numRocketCamModes = 2;
const VECTOR3 ROCKET_CAMERA_DIRECTION[numRocketCamModes] = { _V(0.0, 0.0, -1.0), _V(0.0, 0.0, 1.0) };
const VECTOR3 ROCKET_CAMERA_OFFSET[numRocketCamModes] = { _V(1.5, 1.5, -ATLAS_CORE_LENGTH / 2.0 + 7.5), _V(0.0,2.0, ATLAS_CORE_LENGTH / 2.0 - 4.0) };

// Pitch data from 19730073391 page 107
const double MET[13] =		   { 0.00, 15.0, 27.0, 39.0, 64.0, 79.0, 89.0, 105.0, 120.0, 131.34, 136.34, 151.34 , 1e10 }; // last entry is "infinity"
const double aimPitchover[13] = { 0.0, 0.98, 0.76, 0.64, 0.68, 0.60, 0.45, 0.240, 0.160, 0.1600, 2.0000, 0.0000, 0.000 };

// Preprogrammed retrosequence times. Are from following sources, with authority in descending order (preferring first source if same landing zone in two sources):
//	- MA-6 communications
//	- MA-8 communications
//	- MA-9 communications
//	- MA6_FlightOps.pdf, page 93, list of all retrosequence times from 1B to 7-1.
const int STORED_RETROSEQUENCE_TIMES = 46;
const int retroTimes[STORED_RETROSEQUENCE_TIMES] = {		17 * 60 + 50,	32 * 60 + 12,	50 * 60 + 24,	1 * 3600 + 15 * 60 + 42,	1 * 3600 + 28 * 60 + 50,	1 * 3600 + 36 * 60 + 38,	1 * 3600 + 50 * 60,		2 * 3600 + 5 * 60 + 59,	2 * 3600 + 38 * 60 + 31,	2 * 3600 + 48 * 60 + 59,	3 * 3600 + 39,	3 * 3600 + 11 * 60 + 26,	3 * 3600 + 22 * 60 + 32,	3 * 3600 + 40 * 60 + 18,	4 * 3600 + 12 * 60 + 32,	4 * 3600 + 22 * 60 + 12,	4 * 3600 + 32 * 60 + 37,	4 * 3600 + 43 * 60 + 53,	4 * 3600 + 54 * 60 + 40,	5 * 3600 + 31 * 60 + 29,	5 * 3600 + 44 * 60 + 5,	5 * 3600 + 55 * 60 + 14,	6 * 3600 + 3 * 60 + 48,	6 * 3600 + 28 * 60 + 12,	7 * 3600 + 3 * 60 + 52,	7 * 3600 + 18 * 60 + 10,	7 * 3600 + 28 * 60 + 30,	7 * 3600 + 36 * 60 + 9,	8 * 3600 + 11 * 60 + 38,	8 * 3600 + 37 * 60 + 23,	8 * 3600 + 51 * 60 + 28,	9 * 3600 + 24,	9 * 3600 + 11 * 60 + 56,	9 * 3600 + 40 * 60 + 22,	10 * 3600 + 14 * 60 + 13,	10 * 3600 + 23 * 60 + 37,	11 * 3600 + 56 * 60 + 24,	13 * 3600 + 19 * 60 + 20,	23 * 3600 + 31 * 60 + 3,	26 * 3600 + 14 * 60 + 48,	26 * 3600 + 34 * 60 + 48,	26 * 3600 + 58 * 60 + 50,	27 * 3600 + 43 * 60 + 48,	28 * 3600 + 31 * 60 + 24,	30 * 3600 + 53 * 60 + 1,	33 * 3600 + 59 * 60 + 24 };
const char retroNames[][256] =							{	"1Bravo",		"1Charlie",		"1Delta",		"1Echo",					"Foxtrot",					"2Alpha",					"2Bravo",				"2Charlie",				"2Delta",					"2Echo",					"Golf",			"3Alpha",					"3Bravo",					"3Charlie",					"3Delta",					"3Echo",					"Hotel",				"4Alpha",						"4Bravo",					"4Delta",					"4-2",					"4Echo",					"5Alpha",				"5Bravo",					"5Delta",				"5-1",						"5Echo",					"5Foxtrot",				"6Bravo",					"6Delta",					"6-1",						"6Echo",		"7Alpha",					"7Bravo",					"7Delta",					"7-1",						"8-1",						"9-1",						"16-1",						"17Bravo",					"18-1",						"18Alpha",					"18-2",						"19Bravo",					"20-1",						"22-1" };

const int NUMBER_SUPPORTED_CONFIG_CAPSULES = 10;

class ProjectMercury : public VESSELVER {
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
	void clbkVisualCreated(VISHANDLE vis, int refcount);
	//void clbkVisualDestroyed(VISHANDLE vis, int refcount);
	void clbkFocusChanged(bool getfocus, OBJHANDLE hNewVessel, OBJHANDLE hOldVessel);
	bool clbkLoadGenericCockpit(void);
	bool clbkPanelMouseEvent(int id, int event, int mx, int my, void* context);
	void clbkRenderHUD(int mode, const HUDPAINTSPEC* hps, SURFHANDLE hDefaultTex);
	//bool clbkLoadVC(int id);
	void clbkLoadStateEx(FILEHANDLE scn, void* status);
	void clbkSaveState(FILEHANDLE scn);

	void VersionDependentTouchdown(VECTOR3 touch1, VECTOR3 touch2, VECTOR3 touch3, VECTOR3 touch4, double stiff, double damp, double mu);
	void VersionDependentPanelClick(int id, const RECT& pos, int texidx, int draw_event, int mouse_event, PANELHANDLE hPanel, const RECT& texpos, int bkmode);
	void VersionDependentPadHUD(oapi::Sketchpad* skp, double simt, int* yIndexUpdate, char* cbuf, VESSEL* v);
	/*double normangle(double angle);
	void oapiWriteLogV(const char* format, ...);
	double GetGroundspeed(void);
	double GetAnimation(UINT anim);
	void GetGroundspeedVector(int frame, VECTOR3& v);
	double length2(VECTOR3 vec);
	void GetAirspeedVector(int frame, VECTOR3& v);*/

	bool SetTargetBaseIdx(char *rstr, bool launch);
	bool SetNumberOfOrbits(char* rstr);

	void AtlasAutopilot(double simt, double simdt);
	void AimEulerAngle(double pitch, double yaw);
	double PitchProgramAim(double met);
	double OrbitalFrameSlipAngle(VECTOR3 pos, VECTOR3 vel);
	double OrbitalFrameSlipAngle2(VECTOR3 pos, VECTOR3 vel);
	double AtlasPitchControl(double cutoffAlt, double cutoffVel);
	double AtlasTargetCutOffAzimuth(double simt, double ri, double longI, double latI, bool realData);
	double EccentricAnomaly(double ecc, double TrA);
	double TimeFromPerigee(double period, double ecc, double TrA);
	double MnA2TrA(double MnA, double Ecc);
	double TrA2MnA(double TrA, double Ecc);
	VECTOR3 Ecl2Equ(VECTOR3 Ecl);
	VECTOR3 Equ2Ecl(VECTOR3 Equ);
	void GetEquPosInTime(double t, double SMa, double Ecc, double Inc, double Per, double LPe, double LAN, double M, double longAtNow, double* longitude, double* latitude);
	bool GetLandingPointIfRetroInXSeconds(double t, ELEMENTS el, ORBITPARAM prm, double longAtNow, double* longitude, double* latitude);
	void AtlasEngineDir(void);
	void DefineVernierAnimations(void);
	void CreateAirfoilsAtlas(void);

	void DisableAutopilot(bool turnOff);
	void AuxDampingAuto(bool highThrust);
	void RetroAttitudeAuto(double simt, double simdt, bool highTorque);
	bool SetPitchAuto(double targetPitch, bool highTorque);
	bool SetYawAuto(bool highTorque);
	bool SetRollAuto(bool highTorque);
	void ReentryAttitudeAuto(double simt, double simdt);
	void GRollAuto(double simt, double simdt);
	void InitiateRetroSequence(void);
	void FireRetroRocket(int rckNum);

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
	//void SwitchAttitudeMode(void);
	//void SwitchPropellantSource(void);
	MATRIX3 RotationMatrix(VECTOR3 angles, bool xyz = false);
	void DumpFuelRCS(void);
	VECTOR3 FlipX(VECTOR3 vIn);
	VECTOR3 FlipY(VECTOR3 vIn);
	VECTOR3 SwapXY(VECTOR3 vIn);
	void myStrncpy(char* writeTo, const char* readFrom, int len);

	//double OrbitArea(double angle, double ecc);

	void SeparateTower(bool noAbortSep);
	void SeparateAtlasBooster(bool noAbortSep);
	void SeparateAtlasCore();
	void SeparateRingsAndAdapters(double offZ);
	void SeparateConceptAdapter(void);
	void SeparateConceptCovers(void);

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

	bool InRadioContact(OBJHANDLE planet);

	// Periscope altitude indicators
	void GetPixelDeviationForAltitude(double inputAltitude, double *deg0Pix, double *deg5Pix);
	void SetPeriscopeAltitude(double inputAltitude);

	void SetCameraSceneVisibility(WORD mode);

	void CreatePanelSwitchClick(int ID_L, int ID_R, int x, int y, PANELHANDLE hPanel);
	void CreatePanelTHandleClick(int ID, int x, int y, PANELHANDLE hPanel);
	void FitPanelToScreen(int w, int h);
	void AnimateDials(void);
	void RotateArmGroup(int groupNum, float x0, float y0, float length, float width, float angleR, float pointiness, float negLength = 0.0f, bool includeLatency = true);
	float ValueToAngle(float value, float minValue, float maxValue, float minAngle, float maxAngle);
	void RotateGlobe(float angularResolution, float viewAngularRadius, float longitude0, float latitude0, float rotationAngle = 0.0f);
	void ChangePanelNumber(int group, int num);
	void ChangeIndicatorStatus(void);
	void SetIndicatorStatus(int indicatorNr, int status);
	void SetIndicatorButtonStatus(int buttonNr, int status);
	void SetPhysicalSwitchStatus(int switchNr, int status);
	void SetTHandleState(int groupIdx, bool pushed, int handleNum);

	void GetPanelRetroTimes(double met, int* rH, int* rM, int* rS, int* dH, int* dM, int* dS);

	// Random number generation. Used for failure simulation
	double GenerateRandom01(void);
	double GenerateRandomNorm(double a1 = 2.0, double a2 = 0.5);
	double GenerateRandomAngleNorm(double a1 = 2.0, double a2 = 0.5);
	double NormAngleDeg(double ang);
	void GetNoisyAngularVel(VECTOR3& avel, double stdDev = 0.25 * RAD);
	void DisableAttitudeThruster(int num);

	// Functions that are common in both Redstone and Atlas, and which are called in the default Orbiter callback functions
	void MercuryGenericConstructor(void);
	void WriteFlightParameters(void);
	void ReadConfigSettings(FILEHANDLE cfg);
	void CreateCapsuleFuelTanks(void);
	void AddDefaultMeshes(void);
	void CapsuleGenericPostCreation(void);
	void DeleteRogueVessels(void);
	void CapsuleAttitudeControl(double simt, double simdt);
	void FlyByWireControlSingleDirection(double thrustLevel, THRUSTER_HANDLE high, THRUSTER_HANDLE low, bool tHandlePushed);
	void MercuryCapsuleGenericTimestep(double simt, double simdt, double latit, double longit, double getAlt);
	void WriteHUDAutoFlightReentry(oapi::Sketchpad* skp, double simt, int *yIndexUpdate, char *cbuf);
	void WriteHUDIndicators(oapi::Sketchpad* skp, double simt, int* yIndexUpdate, char* cbuf);
	void LoadCapsule(const char *cbuf);
	void ReadCapsuleTextureReplacement(const char* cbuf);
	bool ReadTextureString(const char* cbuf, const int len, char* texturePath, int* textureWidth, int* textureHeight);
	void LoadCapsuleTextureReplacement(void);
	void LoadTextureFile(ATEX tex, const char* type, MESHHANDLE mesh, DWORD meshTexIdx);
	// Rocket specific similar functions
	bool ReadRocketTextureReplacement(const char* flag, const char* cbuf, int len);
	void LoadRocketTextureReplacement(void);

	// Config settings
	double heightOverGround;
	double integratedSpeedLimit = 2130.0;
	double ampFactor = 0.10;
	double ampAdder = 0.05;
	double rudderLift = 1.7;
	double rudderDelay = 0.5;
	double timeStepLimit = 0.1;

	// HUD constants
	DWORD ScreenWidth, ScreenHeight, ScreenColour;
	int TextX0, TextY0, LineSpacing;
	int secondColumnHUDx = 28; // The x-pos of the second collumn on the HUD

private:

	// Previously were const's, but as we now have manouver unit, they are changed. Thus have to be moved to private
	VECTOR3 MERCURY_OFS_CAPSULE = { 0.0, 0.0, (MERCURY_LENGTH_CAPSULE) / 2.0 + ATLAS_CORE_LENGTH / 2.0 };
	VECTOR3 ATLAS_ADAPTER_OFFSET = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - 1.55 };
	VECTOR3 ABORT_OFFSET = { 0.0, -0.04, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2.0 + MERCURY_LENGTH_ABORT / 2.0 };
	VECTOR3 MERCURY_OFS_SHIELD = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2 - MERCURY_LENGTH_SHIELD / 2 + MERCURY_LENGTH_SHIELD - .05 };
	VECTOR3 MERCURY_OFS_ANTHOUSE = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2 + MERCURY_LENGTH_ANTHOUSE / 2 };
	VECTOR3 MERCURY_OFS_ABORT = { 0.0, -0.04, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2 + MERCURY_LENGTH_ABORT / 2 };
	VECTOR3 MERCURY_OFS_DROGUECOVER = { -0.12, 0.08, -0.08 + MERCURY_OFS_ANTHOUSE.z + MERCURY_LENGTH_ANTHOUSE / 2 };
	VECTOR3 MERCURY_OFS_LANDBAG = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2 + MERCURY_LENGTH_SHIELD - .05 - MERCURY_LENGTH_LANDBAG / 2 };
	VECTOR3 MERCURY_OFS_RETRO = { 0.0, 0.025, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2 - MERCURY_LENGTH_RETRO / 2 + .15 };
	VECTOR3 MERCURY_OFS_RETROCOVER1 = { -0.196, MERCURY_OFS_RETRO.y + 0.091, MERCURY_OFS_RETRO.z - .2 };
	VECTOR3 MERCURY_OFS_RETROCOVER2 = { -0.005, MERCURY_OFS_RETRO.y - 0.23, MERCURY_OFS_RETRO.z - .2 };
	VECTOR3 MERCURY_OFS_RETROCOVER3 = { 0.175, MERCURY_OFS_RETRO.y + 0.091, MERCURY_OFS_RETRO.z - .2 };
	VECTOR3 MERCURY_OFS_RETROSTRAP1 = { -0.03, MERCURY_OFS_RETRO.y + 0.46, MERCURY_OFS_RETRO.z + .093 };
	VECTOR3 MERCURY_OFS_RETROSTRAP2 = { -0.417, MERCURY_OFS_RETRO.y - 0.287, MERCURY_OFS_RETRO.z + .093 };
	VECTOR3 MERCURY_OFS_RETROSTRAP3 = { 0.425, MERCURY_OFS_RETRO.y - 0.27, MERCURY_OFS_RETRO.z + .093 };
	VECTOR3 MERCURY_OFS_EXPLOSIVEBOLT = { 0.0, 0.0, MERCURY_OFS_RETRO.z - .18 };
	VECTOR3 OFS_ADAPTRING1 = { -0.61, 0.35,  -0.93 + MERCURY_OFS_CAPSULE.z };
	VECTOR3 OFS_ADAPTRING2 = { 0.0, -0.82,  -0.93 + MERCURY_OFS_CAPSULE.z };
	VECTOR3 OFS_ADAPTRING3 = { 0.61, 0.35,  -0.93 + MERCURY_OFS_CAPSULE.z };
	VECTOR3 OFS_ADAPTCOVER1 = { 0.0, 0.91, -0.9 + MERCURY_OFS_CAPSULE.z };
	VECTOR3 OFS_ADAPTCOVER2 = { -0.74, -0.43,  -0.9 + MERCURY_OFS_CAPSULE.z };
	VECTOR3 OFS_ADAPTCOVER3 = { 0.74, -0.43,  -0.9 + MERCURY_OFS_CAPSULE.z };
	double POSIGRADE_MASS_FUEL = 3.6; // Total 24 lb set (3.6 kg * 3), according to https://www.wired.com/2014/09/one-man-space-station-1960/. But edited to result dV 28 ft/s

	static void vlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	static void vliftEscape(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftEscape(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	static void vliftAtlas(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftAtlas(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	THRUSTER_HANDLE th_main, th_booster[2], th_vernier[2], thCluster[5], th_rcsDummyBooster[6], escape_engine, thruster_retro[3], thruster_posigrade[3], thruster_man_py[4],
		thruster_man_roll[2], thruster_auto_py[4], thruster_auto_py_1lb[4], thruster_auto_roll[2], thruster_auto_roll_1lb[2],
		//pitchup, pitchdown, yawleft, yawright, bankleft, bankright,
		controllerPitchup, controllerPitchdown, controllerYawleft, controllerYawright, controllerBankleft, controllerBankright; // the dummy thrusters for capsule RCS
	PROPELLANT_HANDLE atlas_propellant, retro_propellant[3], posigrade_propellant[3], fuel_auto, fuel_manual, escape_tank, dummyControllerFuel;
	//THRUSTER_HANDLE th_main, th_rcs[14], th_group[4];
	PARTICLESTREAMSPEC contrail_main, contrail_second;
	PSTREAM_HANDLE contrail, contrail2, rcsStream[18], turbineExhaustContrail, boosterExhaustContrail[2], iceVenting[3];
	UINT exMain, exBooster[2], exVernier[2];
	bool contrailActive = true;
	bool contrail2Active = true;
	//double engineLevel02 = 0.0;
	double turbineContrailLevel = 0.0;
	//double iceVentLevel = 0.0;
	bool suborbitalMission = false; // change to false for Mercury Atlas!
	bool boilerplateMission = false;
	bool limitApogee = false;
	double targetInclination = 32.55;
	double holdDownTime = 3.0;
	double becoTime = 128.6;
	double previousSustainerLevel = 0.0;
	bool inclinationTarget = false;
	double PIDintegral = 0.0, PIDpreviousError = 0.0;
	double fireflyLevel = 0.0;
	double fireflyBangTime = 0.0;

	MESHHANDLE atlas,
		atlasIce, atlasIce2, atlasIce3,
		atlasBooster,
		atlasAdapter,
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
		periscopeMesh,
		periscopeFilterRed, periscopeFilterYellow, periscopeFilterGray,
		circularFrameMesh,
		cockpitPanelMesh,
		vcFrame; // mesh handles

	//DEVMESHHANDLE cockpitPanelDevMesh;

	UINT Atlas; // rocket mesh
	UINT AtlasBooster;
	UINT AtlasAdapter;
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
	UINT PeriscopeFilter;

	UINT VcFrame;

	// MGROUP_SCALE* DrogueDeploy;
	MGROUP_SCALE* MainChuteDeploy;
	MGROUP_SCALE* LandingBagDeploy;
	UINT DrogueDeployAnim, MainChuteDeployAnim, LandingBagDeployAnim, Vernier1AnimX, Vernier1AnimY, Vernier2AnimX, Vernier2AnimY, PeriscopeDeployAnim, DestabiliserDeployAnim;
	ANIMATIONCOMPONENT_HANDLE DrogueDeployAnim1, MainChuteDeployAnim1, LandingBagDeployAnim1, PeriscopeDeployAnim1;
	int heatShieldGroup = 29;

	CTRLSURFHANDLE Verniers[3];

	ATTACHMENTHANDLE padAttach;

	// ReplacementTextures
	int numTextures = 0;
	char textureString[50][100] = { NULL }; // now supports up to 50 appended textures. Should be more than plenty enough
	ATEX texPorthole, texBrailmap, texMBrail1, texMBrail2, texRidges, texVerrail1, texMetal, texVerrail2, texOrange, texPythr, texRollthst, texScanner, texScope, texSnorkel, texTrailmap, texTopboxes, texUsa, // all these present in Freedom 7 mesh
		texArtwork,
		texCrack, texGlass, texFoilwindow, texFoil, texWindowfr,
		texWfrfront, texWindow, texFlag,
		texBoilerplateCapsule,
		texMetalant, texAntridge, texScrew, texDialec,
		texMetalret, texDialecret, texBw,
		texAtlas3, texAtlas4, texAtlas5, texAtlas3ice, texAtlas3ice2, texAtlas3ice3, texAdapterridge, texMetaladapt, texAtlas5boost, texAtlas3boost;
	int configTextureUserNum = -1; // if between 0 and NUMBER_SUPPORTED_CONFIG_CAPSULES - 1, it is defined
	char configTextureUserName[NUMBER_SUPPORTED_CONFIG_CAPSULES][20]; // up to NUMBER_SUPPORTED_CONFIG_CAPSULES user defined capsules, supports up to length 20
	int configTextureUserBasis[NUMBER_SUPPORTED_CONFIG_CAPSULES]; // The original frame to build upon
	bool configTextureUserEnable = false; // by default, don't load textures defined in config. Only if actual capsule is called
	bool scenarioTextureUserEnable = false;


	// Actions
	bool separateTowerAction = false;
	bool separateBoosterAction = false;
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
	double currentYawAim = 0.0;
	double currentRollAim = 0.0;
	bool spaceLaunch = false;
	bool launchFromLC14 = false;
	bool boosterSeparated = false;
	bool coreSeparated = false;
	bool engageRetro = false;
	double retroStartTime;
	bool autoPilot = false;
	double launchTime = 0.0;
	double boosterShutdownTime = 0.0;
	double previousFrameLatitude;
	bool posigradeDampingActivated = false;
	double posigradeDampingTime = 0.0;
	bool turnAroundFinished = false;
	//bool attitudeHold14deg = false;
	double integratedSpeed = 0.0;
	double integratedPitch = 90.0;
	double integratedYaw = 0.0;
	double integratedRoll = 0.0;
	double rollLimit = 6.4;
	bool CGshifted;
	double escapeLevel = 0.0;
	//bool inFlightAbort = false;
	//bool abortSepSequencePerformed = false;
	double abortTime = 0.0;
	double towerJetTime = 0.0;
	//double capsuleSepTime = 0.0;
	bool retroCoverSeparated[3] = { false, false, false };
	//bool attitudeControlManual = true;
	//bool attitudeFuelAuto = true;
	double RETRO_THRUST_LEVEL[3];
	//bool abortDamping = true;
	bool rcsExists = false;
	bool engageFuelDump = false;
	bool rollProgram = false;
	bool pitchProgram = false;
	//bool retroAttitude = false;
	double eulerPitch = 0.0;
	double eulerYaw = 0.0;

	int showInfoOnHud = 0;	/* 0 = Both key commands and flight data
							   1 = Only flight data
							   2 = Nothing (only stock HUD)*/
	char contactBase[50];
	bool leftMFDwasOn = false;
	bool rightMFDwasOn = false;
	bool MercuryNetwork = true;
	double joystickThresholdLow = 0.33; // the RCS joystick threshold activating the low torque thrusters in Fly-By-Wire
	double joystickThresholdHigh = 0.75; // the RCS joystick threshold activating the high torque thrusters in Fly-By-Wire
	double ASCSstdDev = 0.25; // in deg/s
	double RSCSstdDev = 1.0; // in deg/s
	double RSCSresolutionD = 3.0; // in deg
	double RSCSmax = 10.0; // in deg/s

	// Left cockpit lights
	bool towerJettisoned = false;


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
	double periscopeProgress = 0.0;
	double periscopeAltitude = 160.0;
	bool rocketCam = false;
	int rocketCamMode = 0;
	bool panelView = false;
	int armGroups[50] = { NULL }; // Support up to 50 for now. Real number more like 15
	int totalArmGroups = 0;
	int panelMeshGroupSide[100] = { 0 }; // 0 empty, 1 left, 2 right, -1 ignore
	//float addScreenWidthValue = 0.0;
	int globeGroup = NULL;
	int globeVertices = NULL;
	float previousDialAngle[200] = { 0.0f }; // must be longer than total mesh group number
	float dialAngularSpeed = float(180.0 * RAD); // Degrees per second
	int abortIndicatorGroup = NULL;
	int previousIndicatorStatus[13] = { 0 };
	bool retroWarnLight = false;
	double indicatorButtonPressTime[20]; // right now support 20 buttons, but this limit is arbitrary. Increase if needed
	int indicatorButtonPressState[20] = { -1 };
	int indicatorButtonFirstGroup = 13; // right now it's 13, but doesn't matter what we set to, as it will be correctly assigned every frame
	//int physicalSwitchState[5] = { -2 }; // -1 left, 0 centre, 1 right. -2 is reset
	int physicalSwitchFirstGroup = 17; // right now it's 17, but doesn't matter what we set to, as it will be correctly assigned every frame
	double animateDialsPreviousSimt = 0.0; // even if we have scenario with simt < 0.0 (scnEditor), we will only have a smooth transition once every time jump backwards.

	// -1 left, 0 centre, 1 right. -2 is reset
	int switchAutoRetroJet = -1; // ARM (-1), OFF (1)
	int switchRetroDelay = -1; // NORM (-1), INST (1)
	int switchRetroAttitude = -1; // AUTO (-1), MAN (1)
	int switchASCSMode = -1; // NORM (-1), AUX DAMP (0), FLY BY WIRE (1)
	int switchControlMode = -1; // AUTO (-1), RATE COMD (1)

	// MA6_FlightOps.pdf calls for control fuel handles to be pushed IN during launch (page 9).
	// This means (see MR3_FlightOps.pdf page 3) that ASCS is completely on (roll, yaw, pitch), and double authority (manual handle [MAN CONT FUEL]).
	bool tHandleManualPushed = true;
	bool tHandleRollPushed = true;
	bool tHandleYawPushed = true;
	bool tHandlePitchPushed = true;


	//SURFHANDLE panelDynamicTexture;
	//SURFHANDLE panelTexture = NULL;


	double vesselAcceleration;
	double longitudinalAcc;
	double maxVesselAcceleration = -1e9; // random initialiser value (neg inft.)
	double minVesselAcceleration = 1e9; // rand. init. val. (plus inft.)
	int orbitCount = 0;
	int currentLandingTargetIdx = 0;
	bool passedBase = false;
	double oldAngle;
	bool launchTargetPosition = true;
	double lowGLat, lowGLong, lowGHeading;
	int manualInputRetroTime = NULL;

	double entryAng, entryAngleToBase;
	VECTOR3 entryLoc, entryVel;

	// Defaults to MA-6 data
	int missionOrbitNumber = 3;
	double missionApogee = 267.43;
	double missionPerigee = 161.05;
	double missionLandLat = 21.33; // deg
	double missionLandLong = 291.33; // deg, from Greenwich eastward
	bool noMissionLandLat = false;
	bool landingComputing = true;

	bool missileMission = false;
	double missileCutoffVelocity = 7282.0;
	double missileCutoffAngle = -1.50;
	double missileCutoffAltitude = 140.04;

	double historyMaxAltitude = 0.0;
	double historyPerigee = 1e10;
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
	double historyLaunchHeading = 105.0;
	double historyWeightlessTime = 0.0;
	OBJHANDLE historyReference;

	enum capsulever { FREEDOM7, LIBERTYBELL7, FRIENDSHIP7, AURORA7, SIGMA7, FAITH7, FREEDOM7II, CAPSULEBIGJOE, CAPSULELITTLEJOE, CAPSULEBD } CapsuleVersion;
	enum vesselstate { LAUNCH, LAUNCHCORE, TOWERSEP, LAUNCHCORETOWERSEP, FLIGHT, REENTRY, REENTRYNODROGUE, REENTRYMAIN, ABORT, ABORTNORETRO } VesselStatus, PreviousVesselStatus;
	enum chutestate { CLOSED, DEPLOYED, OPENING, REEFED, UNREEFING, OPENED } DrogueStatus, MainChuteStatus, ReserveChuteStatus;
	//enum mainchutestate { M_CLOSED, M_DEPLOYED, M_OPENING, M_REEFED, M_UNREEFING, M_OPENED } MainChuteStatus, ReserveChuteStatus;
	enum landingbagstate { L_CLOSED, L_DEPLOYED, L_OPENING, L_OPENED } LandingBagStatus;
	enum periscopestate { P_CLOSED, P_DEPLOYED, P_OPENING, P_CLOSING } PeriscopeStatus, DestabiliserStatus;
	//enum rcsstate { MANUAL, AUTOHIGH, AUTOLOW } RcsStatus;
	enum autopilotstate { AUTOLAUNCH, POSIGRADEDAMP, TURNAROUND, ORBITATTITUDE, RETROATTITUDE, REENTRYATTITUDE, LOWG } AutopilotStatus; // TURNAROUND, PITCHHOLD, REENTRYATT, LOWG } AutopilotStatus;
	enum icemesh { ICE0, ICE1, ICE2, ICE3 } AtlasIceStatus;
	enum filtertype { CLEAR, RED, YELLOW, GRAY } CurrentFilter;

	// Failure definitions
	enum failure { NONE, LOWGACTIVATE, LOWGDEACTIVE, ATTSTUCKON, ATTSTUCKOFF, ATTMODEOFF, RETROSTUCKOFF, RETROSTUCKON, RETRONOSEP, MAINCHUTETORN, NOLANDBAG, RETROCALCOFF, BOOSTERDEVIATIONSPEED, BOOSTERPROBLEM, ATTITUDEOFFSET, LASTENTRY} FailureMode = NONE; // LASTENTRY is used to dynamically get length of enum. Always have it last in the array
	bool difficultyHard = false; // Used for most extreme cases. If false and a hard case is chosen, no error occurs.
	double timeOfError = 1e10; // initialise to "infinity". This says when an error first occurs / breaks out
	double pitchOffset = 0.0, yawOffset = 0.0, rollOffset = 0.0; // radians
	double speedError = 0.0; // m/s
	double chuteDestroyFactor = 0.0; // 1 is completely mess, 0 is no problem
	int retroErrorNum = 10; // the retro to be failed. 10 is "infinity", must be either 0, 1 or 2
	int attitudeThrusterErrorNum = 100; // "infinity". Must be between 0 and 17

	int stuffCreated = 0;
	OBJHANDLE createdVessel[25]; // number is close to 20, but don't bother counting exactly

	bool capsuleOnly = false; // Is possibly overloaded at SetClassCaps. If set to true spawns a capsule in FLIGHT stage
	bool capsuleTowerRetroOnly = false; // Is possibly overloaded at SetClassCaps. If set to true, spawns a capsule in ABORT stage, with LES and retropack
	bool capsuleTowerOnly = false; // Is possibly overloaded at SetClassCaps. If set to true, spawns a capsule in ABORTNORETRO stage, with LES but no retropack

	bool conceptManouverUnit = false;
	bool conceptManouverUnitAttached = true;
	bool conceptCoverAttached = true;
	bool separateConceptAdapterAction = false;
	bool separateConceptCoverAction = false;
	MESHHANDLE conceptRetrogradeThrusters;
	MESHHANDLE conceptCover1, conceptCover2;
	UINT ConceptRetrogradeThrusters;
	UINT ConceptCover1, ConceptCover2;
	double CONCEPT_POSIGRADE_EMPTY_MASS = 167.4;
	double CONCEPT_POSIGRADE_FUEL_MASS = 240.9;
	double CONCEPT_POSIGRADE_THRUST = 1112.0; // per thruster. Two forward, two aft
	double CONCEPT_POSIGRADE_ISP = 232.0 * G; // vacuum, from document page 105. JP4-H2O2. Hypothetical maximum is 319 s (astronautix). Should have dV of 800ft/s = 244 m/s after circularisation at 150 nm. Tot dV should be roughly 310 m/s
	VECTOR3 CONCEPT_RETROGRADE_MESH_OFFSET = MERCURY_OFS_CAPSULE + _V(0, 0, -1.5);
	VECTOR3 CONCEPT_RETROGRADE_THRUSTER_UP = CONCEPT_RETROGRADE_MESH_OFFSET + _V(-0.214, 0.472, 2.418);
	VECTOR3 CONCEPT_RETROGRADE_THRUSTER_DOWN = CONCEPT_RETROGRADE_MESH_OFFSET + _V(0.214, -0.472, 2.418);
	VECTOR3 CONCEPT_RETROGRADE_THRUSTER_UP_DIR = _V(0.214 * 0.957172, -0.472 * 0.957172, -0.868293); // should produce a vector length 1, passing through z axis with same angle to z as the one above
	VECTOR3 CONCEPT_RETROGRADE_THRUSTER_DOWN_DIR = FlipX(FlipY(CONCEPT_RETROGRADE_THRUSTER_UP_DIR));
	VECTOR3 CONCEPT_RETROGRADE_COVER1_OFFSET = CONCEPT_RETROGRADE_THRUSTER_UP + CONCEPT_RETROGRADE_THRUSTER_UP_DIR * 0.01;
	VECTOR3 CONCEPT_RETROGRADE_COVER2_OFFSET = CONCEPT_RETROGRADE_THRUSTER_DOWN + CONCEPT_RETROGRADE_THRUSTER_DOWN_DIR * 0.01;
	VECTOR3 CONCEPT_POSIGRADE_THRUSTER_LEFT = ATLAS_ADAPTER_OFFSET + _V(0.12, 0.0, 0.2 - 0.1);
	VECTOR3 CONCEPT_POSIGRADE_THRUSTER_RIGHT = ATLAS_ADAPTER_OFFSET + _V(-0.12, 0.0, 0.2 - 0.1);
	VECTOR3 CONCEPT_POSIGRADE_THRUSTER_LEFT_DIR = _V(0, 0, 1);
	VECTOR3 CONCEPT_POSIGRADE_THRUSTER_RIGHT_DIR = _V(0, 0, 1);
	double CONCEPT_EXTRA_LENGTH = 0.5; // 0.238
	double conceptThrusterLevel[4]; // for local light sources

	const VECTOR3 CONCEPT_LINEAR_UP = ATLAS_ADAPTER_OFFSET + _V(0.0, -0.952, 0.20825);
	const VECTOR3 CONCEPT_LINEAR_DOWN = FlipY(CONCEPT_LINEAR_UP);
	const VECTOR3 CONCEPT_LINEAR_RIGHT = ATLAS_ADAPTER_OFFSET + _V(0.952, 0.0, 0.20825);
	const VECTOR3 CONCEPT_LINEAR_LEFT = FlipX(CONCEPT_LINEAR_RIGHT);
	const double CONCEPT_LINEAR_THRUST = 120.0; // Guess. Should at least be as much as rot att (106.8 N)

	THRUSTER_HANDLE conceptPosigrade[2], conceptRetrograde[2], conceptLinear[4];
	PROPELLANT_HANDLE conceptPropellant;
};
