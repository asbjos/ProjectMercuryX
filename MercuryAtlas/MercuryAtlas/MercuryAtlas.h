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

FILEHANDLE pitchDataLogFile;

// ==============================================================
// Some vessel parameters
// ==============================================================

const double MERCURY_LENGTH_CAPSULE = 2.3042;
const double MERCURY_LENGTH_ABORT = 5.1604;
const double MERCURY_LENGTH_ANTHOUSE = 0.687;
const double MERCURY_LENGTH_SHIELD = 0.29;
const double MERCURY_LENGTH_LANDBAG = 0.97;
const double MERCURY_LENGTH_RETRO = 0.50;
const double ATLAS_CORE_LENGTH = 20.90; // 19.83796; 
const double ATLAS_BOOSTER_LENGTH = 3.86;
const VECTOR3 MERCURY_OFS_CAPSULE = { 0.0, 0.0, (MERCURY_LENGTH_CAPSULE) / 2.0 + ATLAS_CORE_LENGTH / 2.0};
const VECTOR3 ATLAS_CORE_OFFSET = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - 11.6 }; // Also known as 0, 0, 0
const VECTOR3 ATLAS_BOOSTER_OFFSET = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - 20.2 };
const VECTOR3 ATLAS_ADAPTER_OFFSET = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - 1.55 };
const VECTOR3 CORE_EXHAUST_POS = { 0.0, 0.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0 };
const VECTOR3 BOOSTER_EXHAUST_POS = { 1.38, 0.0, ATLAS_BOOSTER_OFFSET.z - ATLAS_BOOSTER_LENGTH / 2.0};
const VECTOR3 VERNIER_EXHAUST_POS = { 0.0, -1.90, -6.822 + ATLAS_CORE_OFFSET.z };
const VECTOR3 TURBINE_EXHAUST_POS = { -0.1868404, -0.60114, -1.689692 + ATLAS_BOOSTER_OFFSET.z};
const VECTOR3 CORE_EXHAUST_DIR = { 0.0, 0.0, 1.0 };
const VECTOR3 BOOSTER_EXHAUST_DIR = { 0.0, 0.0, 1.0 };
const VECTOR3 VERNIER_EXHAUST_DIR = { 0.0, 0.134382, 0.99093 };
const VECTOR3 TURBINE_EXHAUST_DIR = { 0.0, sin(20 * RAD), cos(20 * RAD) }; // pick angle to fit
const VECTOR3 ABORT_OFFSET = { 0.0, -0.04, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2.0 + MERCURY_LENGTH_ABORT / 2.0 };
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

const VECTOR3 OFS_ADAPTRING1 = { -0.61, 0.35,  -0.93 + MERCURY_OFS_CAPSULE.z };
const VECTOR3 OFS_ADAPTRING2 = { 0.0, -0.82,  -0.93 + MERCURY_OFS_CAPSULE.z};
const VECTOR3 OFS_ADAPTRING3 = { 0.61, 0.35,  -0.93 + MERCURY_OFS_CAPSULE.z};
const VECTOR3 OFS_ADAPTCOVER1 = { 0.0, 0.91, -0.9 + MERCURY_OFS_CAPSULE.z };
const VECTOR3 OFS_ADAPTCOVER2 = { -0.74, -0.43,  -0.9 + MERCURY_OFS_CAPSULE.z };
const VECTOR3 OFS_ADAPTCOVER3 = { 0.74, -0.43,  -0.9 + MERCURY_OFS_CAPSULE.z };


static const DWORD tchdwnLaunchNum = 4;
static TOUCHDOWNVTX tchdwnLaunch[tchdwnLaunchNum] = {
	// pos, stiff, damping, mu, mu long
	{_V(0.0, -1.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(-sqrt(0.5), sqrt(0.5), ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(sqrt(0.5), sqrt(0.5), ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(0.0, 0.0, MERCURY_OFS_ABORT.z + MERCURY_LENGTH_ABORT / 2.0), 1e7, 1e5, 10},
};

static const DWORD tchdwnTowSepNum = 4;
static TOUCHDOWNVTX tchdwnTowSep[tchdwnTowSepNum] = {
	// pos, stiff, damping, mu, mu long
	{_V(0.0, -1.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(-0.7, 0.7, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(0.7, 0.7, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0), 1e7, 1e5, 10},
	{_V(0.0, 0.0, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2.0), 1e7, 1e5, 10},
};

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
const VECTOR3 VERNIER1_OFFSET = _V(0.023, -1.829, -6.614) + ATLAS_CORE_OFFSET;
const VECTOR3 VERNIER2_OFFSET = _V(-0.053, 1.841, -6.614) + ATLAS_CORE_OFFSET;

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
const double RETRO_THRUST = 5590.0; // MA-6 results pdf, page 14, 1000 lb force (4448 N). Total dV is 440 ft/s (134 m/s), but at an angle of 34 deg. Each retro produces 0.4 G acc (tot. 1.2 G) 
const double RETRO_ISP = 11.5 * RETRO_THRUST / RETRO_MASS_FUEL; // 10 sec. burn time. Isp should be approx 230 s (astronautix) or 233 s (old proj.mercury addon). Now is 226.7 s. Some sources cite 12 sec. MA-9 postlaunch report part 1 has 11.5 second firing time

const double CAPSULE_MASS = 1224.24; // Was 1224.24 kg. According to Astronautix and 19670022650 it is closer to 1100 kg, plus person inside, which becomes something like 1224 kg anyway
const double MERCURY_MASS = RETRO_MASS + CAPSULE_MASS;
const double LANDING_MASS = CAPSULE_MASS - 104.0; // Mass of drogue and antenna is 104 kg according to old sourcecode of mercury_antenna
const VECTOR3 MERCURY_ROT_DRAG = _V(0.05, 0.05, 0.0015); // From Apollo CM from AAPO (0.07, 0.07, 0.003). From old Mercury (0.7, 0.7, 0.1). Must allow 10 deg roll during reentry
const VECTOR3 MERCURY_ROT_DRAG_CHUTE = _V(10.0, 10.0, 7.0); // Also from Apollo CM, now with chutes, from AAPO
const VECTOR3 MERCURY_ROT_DRAG_DROGUE = _V(1.0, 1.0, 1.0); // guesstimate based on two other values

const double MERCURY_FUEL_MASS_AUTO = 14.51; // 32 pounds
const double MERCURY_FUEL_MASS_MAN = 10.66; // 23.5 pounds (20150018552 page 9)
const double MERCURY_THRUST_ATT = 106.8; // 24 lb
const double MERCURY_THRUST_ROLL_ATT = 26.69; // 6 lb
const double MERCURY_ISP_ATT = 220 * G; // Also listed somewhere as 160 pound-seconds, but unsure how to convert
const double MERCURY_THRUST_ATT_LOW = 4.448; // 1 lb

// Inverse of time to operate
const double DROGUE_OPERATING_SPEED = 0.4; // was 0.4
const double MAIN_CHUTE_OPERATING_SPEED = 0.4; // was 0.4
const double LANDING_BAG_OPERATING_SPEED = 0.2;

// Pitch data from 19730073391 page 107
const double MET[12] =		   { 0.00, 15.0, 27.0, 39.0, 64.0, 79.0, 89.0, 105.0, 120.0, 131.34, 136.34, 151.34 };
const double aimPitchover[12] = { 0.0, 0.98, 0.76, 0.64, 0.72, 0.72, 0.50, 0.280, 0.240, 0.0000, 2.0000, 0.0000 };
//								1Bravo			1Charlie	1Delta		1Echo					 Foxtrot(end of 1st orbit)	2Alpha					2Bravo				2Charlie				2Delta						2Echo					Golf (end 2nd)	3Alpha					3Bravo					3Charlie				3Delta						3Echo					Hotel (Nominal deorbit)
const double retroTimes[31] = { 17 * 60 + 50, 32 * 60 + 12, 50 * 60 + 24, 1 * 3600 + 15 * 60 + 42, 1 * 3600 + 28 * 60 + 50, 1 * 3600 + 36 * 60 + 38, 1 * 3600 + 50 * 60, 2 * 3600 + 5 * 60 + 59, 2 * 3600 + 38 * 60 + 31, 2 * 3600 + 48 * 60 + 59, 3 * 3600 + 39, 3 * 3600 + 11 * 60 + 26, 3 * 3600 + 22 * 60 + 32, 3 * 3600 + 40 * 60 + 18, 4 * 3600 + 12 * 60 + 32, 4 * 3600 + 22 * 60 + 12, 4 * 3600 + 32 * 60 + 37, 5 * 3600 + 44 * 60 + 5,	7 * 3600 + 18 * 60 + 10, 8 * 3600 + 51 * 60 + 28, 10 * 3600 + 23 * 60 + 37, 11 * 3600 + 56 * 60 + 24, 13 * 3600 + 19 * 60 + 20, 23 * 3600 + 31 * 60 + 3, 26 * 3600 + 14 * 60 + 48, 26 * 3600 + 34 * 60 + 48, 26 * 3600 + 58 * 60 + 50, 27 * 3600 + 43 * 60 + 48, 28 * 3600 + 31 * 60 + 24, 30 * 3600 + 53 * 60 + 1, 33 * 3600 + 59 * 60 + 24 };
const char retroNames[][256] = { "1Bravo",	"1Charlie",		"1Delta",	"1Echo",				"Foxtrot",					"2Alpha",				"2Bravo",			"2Charlie",				"2Delta",					"2Echo",				"Golf",			"3Alpha",				"3Bravo",				"3Charlie",				"3Delta",					"3Echo",				"Hotel",				"4-2",					"5-1",					"6-1",					"7-1",						"8-1",					"9-1",						"16-1",					"17Bravo",					"18-1",					"18Alpha",					"18-2",					"19Bravo",					"20-1",					"22-1" };

//const double METp[47] = { 0.00 , 15.0 , 18.0 , 21.0 , 24.0 , 27.0 , 30.0 , 33.0 , 36.0 , 39.0 , 42.0 , 45.0 , 48.0 , 51.0 , 54.0 , 57.0 , 60.0 , 63.0 , 66.0 , 69.0 , 72.0 , 75.0 , 78.0 , 81.0 , 84.0 , 87.0 , 90.0 , 93.0 , 96.0 , 99.0 , 102.0 , 105.0 , 108.0 , 111.0 , 114.0 , 117.0 , 120.0 , 123.0 , 126.0 , 129.0 , 132.0 , 135.0 , 138.0 , 141.0 , 144.0 , 147.0, 150.0 };
//const double pitchP[47] = { 90.00 , 90.00 , 87.35 , 83.82 , 80.67 , 77.77 , 75.25 , 72.98 , 70.84 , 69.20 , 67.31 , 65.42 , 63.53 , 61.64 , 59.75 , 57.98 , 56.09 , 54.20 , 52.18 , 50.04 , 47.90 , 45.88 , 43.74 , 41.72 , 39.83 , 38.07 , 36.30 , 34.66 , 33.28 , 32.01 , 31.01 , 30.00 , 29.24 , 28.36 , 27.73 , 26.85 , 26.22 , 25.59 , 25.08 , 24.58 , 24.20 , 23.82 , 19.03 , 13.11 , 7.18 , 1.26 , -4.29 };

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
	void clbkLoadStateEx(FILEHANDLE scn, void* status);
	void clbkSaveState(FILEHANDLE scn);

	bool SetTargetBaseIdx(char *rstr);

	void AtlasAutopilot(double simt, double simdt);
	double OrbitalFrameSlipAngle(VECTOR3 pos, VECTOR3 vel);
	double AtlasPitchControl(void);
	double AtlasTargetCutOffAzimuth(double simt, double ri, double longI, double latI, bool realData);
	double EccentricAnomaly(double ecc, double TrA);
	double TimeFromPerigee(double period, double ecc, double TrA);
	double MnA2TrA(double MnA, double Ecc);
	double TrA2MnA(double TrA, double Ecc);
	VECTOR3 Ecl2Equ(VECTOR3 Ecl);
	VECTOR3 Equ2Ecl(VECTOR3 Equ);
	void GetEquPosInTime(double t, double SMa, double Ecc, double Inc, double Per, double LPe, double LAN, double M, double longAtNow, double* longitude, double* latitude);
	void GetLandingPointIfRetroInXSeconds(double t, ELEMENTS el, ORBITPARAM prm, double longAtNow, double* longitude, double* latitude);
	void AtlasEngineDir(void);
	void DefineVernierAnimations(void);
	void CreateAirfoilsAtlas(void);

	void DisableAutopilot(bool turnOff);
	void AuxDampingAuto(bool highThrust);
	void RetroAttitudeAuto(double simt, double simdt);
	bool SetPitchAuto(double targetPitch, bool highThrust);
	bool SetYawAuto(bool highThrust);
	bool SetRollAuto(bool highThrust);
	void ReentryAttitudeAuto(double simt, double simdt);
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

	void SeparateTower(bool noAbortSep);
	void SeparateAtlasBooster(bool noAbortSep);
	void SeparateAtlasCore();
	void SeparateRingsAndAdapters(void);

	void SeparateRetroPack(bool deleteThrusters);
	void SeparateRetroCoverN(int i);
	void SeparateDrogueCover(void);
	void SeparateDrogue(void);
	void DeployDrogue(void);
	void DeployMainChute(void);
	void SeparateMainChute(void);
	void DeployLandingBag(void);

	void DefineDrogueAnimation(void);
	void DefineMainChuteAnimation(void);
	void DefineLandingBagAnimation(void);
	void AnimateDrogueChute(double simt, double simdt);
	void AnimateMainChute(double simt, double simdt);
	void AnimateLandingBag(double simt, double simdt);

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
	void WriteHUDAutoFlightReentry(oapi::Sketchpad* skp, double simt, int *yIndexUpdate, char *cbuf);

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

	static void vliftAtlas(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftAtlas(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	THRUSTER_HANDLE th_main, th_booster[2], th_vernier[2], thCluster[5], th_rcsDummyBooster[6], escape_engine, thruster_retro[3], thruster_posigrade[3], thruster_man_py[4],
		thruster_man_roll[2], thruster_auto_py[4], thruster_auto_py_1lb[4], thruster_auto_roll[2], thruster_auto_roll_1lb[2],
		pitchup, pitchdown, yawleft, yawright, bankleft, bankright;
	PROPELLANT_HANDLE atlas_propellant, retro_propellant[3], posigrade_propellant[3], fuel_auto, fuel_manual, escape_tank;
	//THRUSTER_HANDLE th_main, th_rcs[14], th_group[4];
	PARTICLESTREAMSPEC contrail_main, contrail_second;
	PSTREAM_HANDLE contrail, contrail2, rcsStream[18], turbineExhaustContrail, boosterExhaustContrail[2];
	UINT exMain, exBooster[2], exVernier[2];
	bool contrailActive = true;
	bool contrail2Active = true;
	double engineLevel02 = 0.0;
	double turbineContrailLevel = 0.0;
	bool suborbitalMission = false; // change to false for Mercury Atlas!
	bool limitApogee = false;
	double targetApogee = 0.0;
	double targetPerigee = 161.05e3;
	double targetInclination = 32.55;
	bool inclinationTarget = false;

	MESHHANDLE atlas,
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
		landingbag; // mesh handles

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

	// MGROUP_SCALE* DrogueDeploy;
	MGROUP_SCALE* MainChuteDeploy;
	MGROUP_SCALE* LandingBagDeploy;
	UINT DrogueDeployAnim, MainChuteDeployAnim, LandingBagDeployAnim, Vernier1AnimX, Vernier1AnimY, Vernier2AnimX, Vernier2AnimY;
	ANIMATIONCOMPONENT_HANDLE DrogueDeployAnim1, MainChuteDeployAnim1, LandingBagDeployAnim1;
	int heatShieldGroup = 29;

	CTRLSURFHANDLE Verniers[3];

	ATTACHMENTHANDLE padAttach;

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
	bool engageRetro = false;
	double retroStartTime;
	bool autoPilot = false;
	double launchTime = 0.0;
	double boosterShutdownTime = 0.0;
	//double timeFromCutOffToMaxLat;
	double maxLatitudeTime;
	double recordedLAN = 0.0, recordedAPe = 0.0, recordedMA0 = 0.0, recordedLong = 0.0, recordedEpoch = 0.0;
	double previousOrbitSimt = 0.0;
	double previousFrameLatitude;
	bool posigradeDampingActivated = false;
	double posigradeDampingTime = 0.0;
	bool turnAroundFinished = false;
	double integratedSpeed = 0.0;
	double integratedPitch = 90.0;
	double integratedYaw = 0.0;
	double integratedRoll = 0.0;
	double rollLimit = 6.4;
	bool CGshifted;
	double escapeLevel = 0.0;
	bool inFlightAbort = false;
	bool abortSepSequencePerformed = false;
	double abortTime = 0.0;
	double towerJetTime = 0.0;
	bool retroCoverSeparated[3] = { false, false, false };
	bool attitudeControlManual = true;
	bool attitudeFuelAuto = true;
	double RETRO_THRUST_LEVEL[3];
	bool abortDamping = true;
	bool rcsExists = false;
	bool engageFuelDump = false;
	bool rollProgram = false;
	bool pitchProgram = false;

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

	double vesselAcceleration;
	int orbitCount = 0;
	int currentLandingTargetIdx = 0;
	bool passedBase = false;
	double oldAngle;
	bool launchTargetPosition = true;
	double lowGLat, lowGLong, lowGInc;

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
	double historyWeightlessTime = 0.0;
	OBJHANDLE historyReference;

	enum capsule { FREEDOM7, LIBERTYBELL7, FRIENDSHIP7, AURORA7, SIGMA7, FAITH7, FREEDOM7II } CapsuleVersion;
	enum vesselstate { LAUNCH, LAUNCHCORE, TOWERSEP, LAUNCHCORETOWERSEP, FLIGHT, REENTRY, REENTRYNODROGUE, REENTRYMAIN, ABORT, ABORTNORETRO } VesselStatus, PreviousVesselStatus;
	enum chutestate { CLOSED, DEPLOYED, OPENING, REEFED, UNREEFING, OPENED } DrogueStatus, MainChuteStatus, ReserveChuteStatus;
	//enum mainchutestate { M_CLOSED, M_DEPLOYED, M_OPENING, M_REEFED, M_UNREEFING, M_OPENED } MainChuteStatus, ReserveChuteStatus;
	enum landingbagstate { L_CLOSED, L_DEPLOYED, L_OPENING, L_OPENED } LandingBagStatus;
	enum rcsstate { MANUAL, AUTOHIGH, AUTOLOW } RcsStatus;
	enum autopilotstate { AUTOLAUNCH, POSIGRADEDAMP, TURNAROUND, PITCHHOLD, REENTRYATT, LOWG } AutopilotStatus;

	bool PMIcheck = false;
	double PMItime = 0.0;
	int PMIn = 0;

	int stuffCreated = 0;
	OBJHANDLE createdVessel[25]; // number is close to 20, but don't bother counting exactly
	bool createdAbove50km[25] = { false };
};
