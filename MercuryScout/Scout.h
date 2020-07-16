#pragma once
// ==============================================================
//				Source file for Mercury Scout.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2020
// 
// Made for Mercury Scout mesh by Ricardo Nunes ('4th rock')
// 
// Thank you to Ricardo Nunes for meshes, data, etc.
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================

FILEHANDLE pitchDataLogFile;

// ==============================================================
// Some vessel parameters
// ==============================================================

// Dimensions
const double STAGE1_LENGTH = 9.120;
const double STAGE2_LENGTH = 6.040;
const double STAGE3_LENGTH = 3.380;
const double STAGE4_LENGTH = 1.830;
const double FAIRING_LENGTH = 3.000;

const double STAGE1_DIAMETER = 1.010;
const double STAGE2_DIAMETER = 0.790;
const double STAGE3_DIAMETER = 0.780;
const double STAGE4_DIAMETER = 0.460;
const double FAIRING_DIAMETER = 1.000;

// Positions
const VECTOR3 STAGE1_OFFSET = _V(0.0, 0.0, 0.0); // Translated z=-5.319541
const VECTOR3 STAGE2_OFFSET = STAGE1_OFFSET + _V(0.0, 0.0, 9.412 + 3.66705 - 5.319541); // Translated z=-3.66705
const VECTOR3 STAGE3_OFFSET = STAGE1_OFFSET + _V(0.0, 0.0, 15.712 + 1.5266 - 5.319541); // Translated z=-1.5266
const VECTOR3 STAGE4_OFFSET = STAGE1_OFFSET + _V(0.0, 0.0, 18.750 + 0.9872 - 5.319541); // Translated z=-0.9872. Originally 18.770, but there was a gap between stages 3 and 4
const VECTOR3 FAIRING_OFFSET = STAGE1_OFFSET + _V(-0.178, 0.0, 19.837 - 5.319541); // Fix this later! Debug. Translated to zero-zero

const VECTOR3 STAGE1_ENGINE_POS = _V(0.0, 0.0, -STAGE1_LENGTH / 2.0 - 1.0); // Fix this later! Debug  STAGE1_OFFSET + 
const VECTOR3 STAGE2_ENGINE_POS = _V(0.0, 0.0, -STAGE2_LENGTH / 2.0 - 0.5); // Fix this later! Debug  STAGE2_OFFSET + 
const VECTOR3 STAGE3_ENGINE_POS = _V(0.0, 0.0, -STAGE3_LENGTH / 2.0 - 0.0); // Fix this later! Debug  STAGE3_OFFSET + 
const VECTOR3 STAGE4_ENGINE_POS = _V(0.0, 0.0, -STAGE4_LENGTH / 2.0 - 0.0); // Fix this later! Debug  STAGE4_OFFSET + 

const VECTOR3 STAGE1_ENGINE_DIR = _V(0.0, 0.0, 1.0);
const VECTOR3 STAGE2_ENGINE_DIR = _V(0.0, 0.0, 1.0);
const VECTOR3 STAGE3_ENGINE_DIR = _V(0.0, 0.0, 1.0);
const VECTOR3 STAGE4_ENGINE_DIR = _V(0.0, 0.0, 1.0);

const double RCS2OFF = -0.1;
const VECTOR3 STAGE2_RCS_PU_POS = _V(0, -STAGE2_DIAMETER / 2.0, -STAGE2_LENGTH / 2.0 + RCS2OFF); // Guesses from 19620003288 page 124
const VECTOR3 STAGE2_RCS_PD_POS = _V(0, STAGE2_DIAMETER / 2.0, -STAGE2_LENGTH / 2.0 + RCS2OFF);
const VECTOR3 STAGE2_RCS_YL_POS = _V(-STAGE2_DIAMETER / 2.0, 0, -STAGE2_LENGTH / 2.0 + RCS2OFF);
const VECTOR3 STAGE2_RCS_YR_POS = _V(STAGE2_DIAMETER / 2.0, 0, -STAGE2_LENGTH / 2.0 + RCS2OFF);
const VECTOR3 STAGE2_RCS_RUL_POS = _V(0, STAGE2_DIAMETER / 2.0, -STAGE2_LENGTH / 2.0 + RCS2OFF); // Roll Upper Left, upper roll thruster, rolling left
const VECTOR3 STAGE2_RCS_RDL_POS = _V(0, -STAGE2_DIAMETER / 2.0, -STAGE2_LENGTH / 2.0 + RCS2OFF);
const VECTOR3 STAGE2_RCS_RUR_POS = _V(0, STAGE2_DIAMETER / 2.0, -STAGE2_LENGTH / 2.0 + RCS2OFF);
const VECTOR3 STAGE2_RCS_RDR_POS = _V(0, -STAGE2_DIAMETER / 2.0, -STAGE2_LENGTH / 2.0 + RCS2OFF);

const VECTOR3 STAGE2_RCS_PU_DIR = _V(0, 1, 0);
const VECTOR3 STAGE2_RCS_PD_DIR = _V(0, -1, 0);
const VECTOR3 STAGE2_RCS_YL_DIR = _V(1, 0, 0);
const VECTOR3 STAGE2_RCS_YR_DIR = _V(-1, 0, 0);
const VECTOR3 STAGE2_RCS_RUL_DIR = _V(-1, 0, 0);
const VECTOR3 STAGE2_RCS_RDL_DIR = _V(1, 0, 0);
const VECTOR3 STAGE2_RCS_RUR_DIR = _V(1, 0, 0);
const VECTOR3 STAGE2_RCS_RDR_DIR = _V(-1, 0, 0);

const double RCS3OFF = 0.65;
const VECTOR3 STAGE3_RCS_PU_POS = _V(0, STAGE3_DIAMETER / 2.0, -STAGE3_LENGTH / 2.0 + RCS3OFF); // Guesses from 19620003288 page 124
const VECTOR3 STAGE3_RCS_PD_POS = _V(0, -STAGE3_DIAMETER / 2.0, -STAGE3_LENGTH / 2.0 + RCS3OFF);
const VECTOR3 STAGE3_RCS_YL_POS = _V(-STAGE3_DIAMETER / 2.0, 0, -STAGE3_LENGTH / 2.0 + RCS3OFF);
const VECTOR3 STAGE3_RCS_YR_POS = _V(STAGE3_DIAMETER / 2.0, 0, -STAGE3_LENGTH / 2.0 + RCS3OFF);
const VECTOR3 STAGE3_RCS_RUL_POS = _V(0, STAGE3_DIAMETER / 2.0, -STAGE3_LENGTH / 2.0 + RCS3OFF); // Roll Upper Left, upper roll thruster, rolling left
const VECTOR3 STAGE3_RCS_RDL_POS = _V(0, -STAGE3_DIAMETER / 2.0, -STAGE3_LENGTH / 2.0 + RCS3OFF);
const VECTOR3 STAGE3_RCS_RUR_POS = _V(0, STAGE3_DIAMETER / 2.0, -STAGE3_LENGTH / 2.0 + RCS3OFF);
const VECTOR3 STAGE3_RCS_RDR_POS = _V(0, -STAGE3_DIAMETER / 2.0, -STAGE3_LENGTH / 2.0 + RCS3OFF);

const VECTOR3 STAGE3_RCS_PU_DIR = _V(0, 1, 0);
const VECTOR3 STAGE3_RCS_PD_DIR = _V(0, -1, 0);
const VECTOR3 STAGE3_RCS_YL_DIR = _V(1, 0, 0);
const VECTOR3 STAGE3_RCS_YR_DIR = _V(-1, 0, 0);
const VECTOR3 STAGE3_RCS_RUL_DIR = _V(-1, 0, 0);
const VECTOR3 STAGE3_RCS_RDL_DIR = _V(1, 0, 0);
const VECTOR3 STAGE3_RCS_RUR_DIR = _V(1, 0, 0);
const VECTOR3 STAGE3_RCS_RDR_DIR = _V(-1, 0, 0);

// Masses
const double STAGE1_DRY_MASS = 2057.0; // 19620003288 page 103, was 1900.0
const double STAGE2_DRY_MASS = 906.3; // was 695.0
const double STAGE3_DRY_MASS = 303.32; // was 294.0
const double STAGE4_DRY_MASS = 30.0 + 67.5; // was 30.0. 67.5 kg is sattelite
const double FAIRING_DRY_MASS = 67.81 / 2.0; // was 20.0

//const double STAGE1_FUEL_MASS = 8805.0;
//const double STAGE2_FUEL_MASS = 3729.0;
//const double STAGE3_FUEL_MASS = 1000.0;
//const double STAGE4_FUEL_MASS = 208.0;

const double STAGE1_FUEL_MASS = 8655.0; // 19620003288 page 93
const double STAGE2_FUEL_MASS = 3323.0;
const double STAGE3_FUEL_MASS = 948.9;
const double STAGE4_FUEL_MASS = 206.8;

const double STAGE2_RCS_FUEL_MASS = 87.54; // 19620003288 page 103. 54 lb consumption in 19620003288 page 69
const double STAGE3_RCS_FUEL_MASS = 9.072; // 19620003288 page 103. 5.5 lb consumption in 19620003288 page 69

const double STAGE1_TOTAL_MASS = STAGE1_DRY_MASS + STAGE1_FUEL_MASS;
const double STAGE2_TOTAL_MASS = STAGE2_DRY_MASS + STAGE2_FUEL_MASS;
const double STAGE3_TOTAL_MASS = STAGE3_DRY_MASS + STAGE3_FUEL_MASS;
const double STAGE4_TOTAL_MASS = STAGE4_DRY_MASS + STAGE4_FUEL_MASS;

// Engine parameters
//const double STAGE1_THRUST = 470934.0;
//const double STAGE2_THRUST = 258915.0;
//const double STAGE3_THRUST = 60497.0;
//const double STAGE4_THRUST = 12450.0;
const double STAGE2_RCS_THRUST = 600.0 * 4.448; // 19620003288 page 124
const double STAGE2_RCS_THRUST_ROLL = 24.0 * 4.448;
const double STAGE3_RCS_THRUST = 44.0 * 4.448;
const double STAGE3_RCS_THRUST_ROLL = 2.2 * 4.448;

//const double STAGE1_BURNTIME = 40.0; // Value from 4th rock. 19620003288 page 43 says 44.2
//const double STAGE2_BURNTIME = 37.0; // Value from 4th rock. 19620003288 page 43 says 40.0
//const double STAGE3_BURNTIME = 39.0; // Value from 4th rock. 19620003288 page 43 says 40.0
//const double STAGE4_BURNTIME = 38.0; // Value from 4th rock. 19620003288 page 43 says 38.8
//
//const double STAGE1_ISP = STAGE1_BURNTIME * STAGE1_THRUST / STAGE1_FUEL_MASS; // S95 document says 220 s, while this is 218 s
//const double STAGE2_ISP = STAGE2_BURNTIME * STAGE2_THRUST / STAGE2_FUEL_MASS; // S95 document says 273 s, while this is 262 s
//const double STAGE3_ISP = STAGE3_BURNTIME * STAGE3_THRUST / STAGE3_FUEL_MASS; // S95 document says 255 s, while this is 241 s
//const double STAGE4_ISP = STAGE4_BURNTIME * STAGE4_THRUST / STAGE4_FUEL_MASS; // S95 document says 256 s, while this is 232 s
const double STAGE2_RCS_ISP = 142.0 * G; // 19620003288 page 70
const double STAGE3_RCS_ISP = 142.0 * G;

const double STAGE1_BURNTIME = 36.06; // 19620003288 page 93
const double STAGE2_BURNTIME = 27.20;
const double STAGE3_BURNTIME = 36.80;
const double STAGE4_BURNTIME = 38.50;

const double STAGE1_THRUST = 4246000.0 * 4.448 / STAGE1_BURNTIME; // 19620003288 page 93
const double STAGE2_THRUST = 1945100.0 * 4.448 / STAGE2_BURNTIME;
const double STAGE3_THRUST = 533100.0 * 4.448 / STAGE3_BURNTIME;
const double STAGE4_THRUST = 116500.0 * 4.448 / STAGE4_BURNTIME;

const double STAGE1_ISP = STAGE1_BURNTIME * STAGE1_THRUST / STAGE1_FUEL_MASS; // S95 document says 220 s, while this is 218 s
const double STAGE2_ISP = STAGE2_BURNTIME * STAGE2_THRUST / STAGE2_FUEL_MASS; // S95 document says 273 s, while this is 262 s
const double STAGE3_ISP = STAGE3_BURNTIME * STAGE3_THRUST / STAGE3_FUEL_MASS; // S95 document says 255 s, while this is 241 s
const double STAGE4_ISP = STAGE4_BURNTIME * STAGE4_THRUST / STAGE4_FUEL_MASS; // S95 document says 256 s, while this is 232 s

const double BATTERY_TOTAL_CHARGE = 18.5 * 3600.0; // Seconds remaining, from Wikipedia

// Cameras
const int NUM_ROCKET_CAMERA = 2;
const VECTOR3 ROCKET_CAMERA_OFFSET[NUM_ROCKET_CAMERA] = { _V(0.0, 1.0, -3.0), _V(0.0, 1.0, 3.0) };
const VECTOR3 ROCKET_CAMERA_DIRECTION[NUM_ROCKET_CAMERA] = { _V(0,0,-1), _V(0,0,1) };

const int pitchRateEntries = 8;
//const double MET[pitchRateEntries] =			{ 0.00, 3.000, 10.00, 30.00, 80.800, 105.80, 148.8, 163.8 };
//const double pitchRateAim[pitchRateEntries] =	{ 0.00, -1.00, -0.37, -0.18, -0.075, -0.080, -1.00, 0.000 };
const double MET[pitchRateEntries] =			{ 0.00, 3.00, 10.00, 30.00, 80.80, 105.80, 148.8, 163.8 };
const double pitchRateAim[pitchRateEntries] =	{ 0.00, -2.0, -0.60, -0.40, -0.10, -0.000, -0.00, 0.000 }; // T+40 50 deg, T+70 40 deg

const int NUMBASES = 17;
char BASE_NAME_LIST[NUMBASES][15] = { "ATS", "BDA", "CAL", "Cape Canaveral", "CSQ", "CTN", "CYI", "GYM", "HAW", "IOS", "KNO", "MUC", "RKV", "RTK", "TEX", "WOM", "ZZB" };
const int baseContactLogLength = 10;

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
	void clbkRenderHUD(int mode, const HUDPAINTSPEC* hps, SURFHANDLE hDefaultTex);
	bool clbkLoadPanel2D(int id, PANELHANDLE hPanel, DWORD viewW, DWORD viewH);
	bool clbkLoadGenericCockpit(void);
	void clbkLoadStateEx(FILEHANDLE scn, void* status);
	void clbkSaveState(FILEHANDLE scn);

	void ScoutAutopilot(double met, double simt, double simdt);
	double PitchProgramAim(double met);
	double TargetEndBurnApogeePitch(double targetApogee, double dV);
	void Staging(int StageToSeparate);
	void SeparateStage(int stageNum);
	void SeparateFairing(void);

	void CreateStage2RCS(void);
	void CreateStage3RCS(void);

	void BaseContactManager(void);

	void VersionDependentTouchdown(VECTOR3 touch1, VECTOR3 touch2, VECTOR3 touch3, VECTOR3 touch4, double stiff, double damp, double mu);
	void VersionDependentPanelClick(int id, const RECT& pos, int texidx, int draw_event, int mouse_event, PANELHANDLE hPanel, const RECT& texpos, int bkmode);
	void VersionDependentPadHUD(oapi::Sketchpad* skp, double simt, int* yIndexUpdate, char* cbuf, VESSEL* v);
	double normangle(double angle);
	void oapiWriteLogV(const char* format, ...);
	double GetGroundspeed(void);
	double GetAnimation(UINT anim);
	void GetGroundspeedVector(int frame, VECTOR3& v);
	double length2(VECTOR3 vec);
	void GetAirspeedVector(int frame, VECTOR3& v);

	void CreateAirfoilsScout(void);

	double EmptyMass(void);
	void SetCameraSceneVisibility(WORD mode);

	VECTOR3 FlipX(VECTOR3 vIn);
	VECTOR3 FlipY(VECTOR3 vIn);
	VECTOR3 SwapXY(VECTOR3 vIn);
	double OrbitArea(double angle, double ecc);
	double OrbitalFrameSlipAngle(VECTOR3 pos, VECTOR3 vel);
	double OrbitalFrameSlipAngle2(VECTOR3 pos, VECTOR3 vel);
	void AimEulerAngle(double pitch, double yaw);
	// Random number generation. Used for failure simulation
	double GenerateRandom01(void);
	double GenerateRandomNorm(double a1 = 2.0, double a2 = 0.5);
	double GenerateRandomAngleNorm(double a1 = 2.0, double a2 = 0.5);
	double NormAngleDeg(double ang);
	// End random number
	bool InRadioContact(OBJHANDLE planet);
	void DeleteRogueVessels(void);
	double MnA2TrA(double MnA, double Ecc);
	double TrA2MnA(double TrA, double Ecc);
	double EccentricAnomaly(double ecc, double TrA);
	void myStrncpy(char* writeTo, const char* readFrom, int len);

	// Config settings
	double heightOverGround;
	double ampFactor = 0.10;
	double ampAdder = 0.05;
	double rudderLift = 1.7;
	double rudderDelay = 0.5;
	double timeStepLimit = 0.1;
	bool MercuryNetwork = true;
	double stage4SpinRate = 160.0; // RPM

	// HUD constants
	DWORD ScreenWidth, ScreenHeight, ScreenColour;
	int TextX0, TextY0, LineSpacing;
	int secondColumnHUDx = 28; // The x-pos of the second collumn on the HUD

private:
	static void vliftScout(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftScout(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	THRUSTER_HANDLE th_main[4], th_rcs[18];
	PROPELLANT_HANDLE scout_propellant[4], rcs_propellant[4];

	PARTICLESTREAMSPEC exhaustMain[4], exhaustRCS;
	PSTREAM_HANDLE exhaustStream[4], rcsStream[18];

	MESHHANDLE stage[4], skirt[3], fairing[2],
		circularFrameMesh;
	UINT Stage[4], Skirt[3], Fairing[2];

	UINT RudderAnim[4], JetVaneAnim[4];
	CTRLSURFHANDLE Rudders[4];

	ATTACHMENTHANDLE padAttach;
	OBJHANDLE previousFrameAttachment = NULL;

	// Stage times
	double stageShutdownTime[4] = { NULL, NULL, NULL, NULL };
	double stageIgnitionTime[4] = { NULL, NULL, NULL, NULL };
	double stageSeparationTime[3] = { 65.8, 157.8, 177.8 }; // 19620003288 page 43. Should be equal ignition time of next stage
	double fairingSeparationTime = NULL;
	double launchTime = NULL;
	// Attitude
	double integratedPitch = PI05;
	double initPitch = PI05;
	double integratedYaw = 0.0;
	double integratedRoll = 0.0;
	double eulerPitch = 0.0;
	double eulerYaw = 0.0;
	double previousSimdt = 0.0;
	double pitchAim = PI05;
	// Camera
	double oldFOV = 40.0 * RAD;
	// Battery
	double batteryLevel = BATTERY_TOTAL_CHARGE;
	// Network bases, contact
	double baseContactTime[NUMBASES][20] = { NULL };
	double baseLossTime[NUMBASES][20] = { NULL };
	bool baseInContact[NUMBASES] = { false };
	int baseContactsNum[NUMBASES] = { 0 };
	VECTOR3 baseContactLog[baseContactLogLength] = { NULL };
	int baseContactLogIdx = 0;

	bool spaceLaunch = false;
	bool autopilot = false;
	bool CGshifted = false;
	bool rocketCam = false;
	bool leftMFDwasOn = false;
	bool rightMFDwasOn = false;
	bool separateFairingAction = false;
	bool staging1Action = false, staging2Action = false, staging3Action = false;
	bool pitchYawEliminated = false;
	bool sattelitePoweredOn = true;

	char contactBase[50];

	int numBases = 0;
	int showInfoOnHud = 0;	/* 0 = Both key commands and flight data
							   1 = Only flight data
							   2 = Nothing (only stock HUD)*/
	int rocketCamMode = 0;

	OBJHANDLE historyReference;
	double historyBottomPrevPrev, historyBottomPrev,
		historyMaxAltitude = 0.0, historyPerigee = 1e10, historyInclination = 0.0,
		historyMaxEarthSpeed = 0.0, historyMaxSpaceSpeed = 0.0;

	enum vesselState {STAGE1, STAGE2, STAGE3, STAGE4} VesselStatus, PreviousVesselStatus;
	enum fairingState { FAIRINGOFF , FAIRINGON } FairingStatus;

	enum failure { NONE, ATTSTUCKON, ATTSTUCKOFF, ATTMODEOFF, RETROSTUCKOFF, RETROCALCOFF, BOOSTERPROBLEM, ATTITUDEOFFSET, LASTENTRY } FailureMode;
	double timeOfError = 1e10;
	bool difficultyHard = false;

	int stuffCreated = 0;
	OBJHANDLE createdVessel[25]; // number is close to 20, but don't bother counting exactly
	bool createdAbove50km[25] = { false };
};