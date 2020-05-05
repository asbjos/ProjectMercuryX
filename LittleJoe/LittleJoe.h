// ==============================================================
//				Header file for Mercury Little Joe.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2020
// 
// Based on Little Joe addon by "4th rock"
// This code is my own work.
// 
// Thank you to Ricardo Nunes.
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================

#pragma once

typedef struct texture {
	char file[50];
	int w;
	int h;
} ATEX; // Asbjos Texture

const double OFFSET_ADAPTERS = -5.5;
const MATRIX3 ROTMATRIX = _M(cos(45.0 * RAD), -sin(45.0 * RAD), 0.0,
							sin(45.0 * RAD), cos(45.0 * RAD), 0.0,
							0.0, 0.0, 1.0);

const VECTOR3 OFS_ADAPTRING1 = { -0.61, 0.35, 9.33 + OFFSET_ADAPTERS };
const VECTOR3 OFS_ADAPTRING2 = { 0.0, -0.82, 9.33 + OFFSET_ADAPTERS};
const VECTOR3 OFS_ADAPTRING3 = { 0.61, 0.35, 9.33 + OFFSET_ADAPTERS};
const VECTOR3 OFS_ADAPTCOVER1 = { 0.0, 0.91, 9.36 + OFFSET_ADAPTERS};
const VECTOR3 OFS_ADAPTCOVER2 = { -0.74, -0.43, 9.36 + OFFSET_ADAPTERS };
const VECTOR3 OFS_ADAPTCOVER3 = { 0.74, -0.43, 9.36 + OFFSET_ADAPTERS };

const double MERCURY_LENGTH_CAPSULE = 2.3042;
const double MERCURY_LENGTH_ABORT = 5.1604;
const double MERCURY_LENGTH_ANTHOUSE = 0.687;
const double MERCURY_LENGTH_SHIELD = 0.29;
const double MERCURY_LENGTH_LANDBAG = 0.97;
const double MERCURY_LENGTH_RETRO = 0.50;
const double LITTLEJOE_LENGTH = 7.20;
const VECTOR3 MERCURY_OFS_CAPSULE = { 0.0, 0.0, (LITTLEJOE_LENGTH + MERCURY_LENGTH_CAPSULE) / 2.0};
const VECTOR3 LITTLEJOE_OFFSET = { 0.0, 0.0, 0.0}; // Also known as 0, 0, 0
const VECTOR3 LITTLEJOE_CASTOR_EXHAUST_POS = { 0.450, 0.450, LITTLEJOE_OFFSET.z - 2.15};
const VECTOR3 LITTLEJOE_RECRUIT_EXHAUST_POS = { 0.880, 0.0, LITTLEJOE_OFFSET.z - 2.55 };
const double CASTOR_ANGLE = 11.0 * RAD;
const double RECRUIT_ANGLE = 12.0 * RAD;
const VECTOR3 LITTLEJOE_CASTOR_EXHAUST_DIR = { -sqrt(0.5) * sin(CASTOR_ANGLE), -sqrt(0.5) * sin(CASTOR_ANGLE), cos(CASTOR_ANGLE) };
const VECTOR3 LITTLEJOE_RECRUIT_EXHAUST_DIR = { -sin(RECRUIT_ANGLE), 0.0, cos(CASTOR_ANGLE) };
const VECTOR3 ABORT_OFFSET = { 0.0, -0.04, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2.0 + MERCURY_LENGTH_ABORT / 2.0 };
const VECTOR3 MERCURY_CAPSULE_OFFSET = _V(0.0, 0.0, 0.0);
//const VECTOR3 MERCURY_CAPSULE_OFFSET = _V(0.0, 0.0, -0.60); // sets an offset to the capsule to have CM at roll thrusters

const VECTOR3 MERCURY_OFS_SHIELD = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2 - MERCURY_LENGTH_SHIELD / 2 + MERCURY_LENGTH_SHIELD - .05 };
const VECTOR3 MERCURY_OFS_ANTHOUSE = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2 + MERCURY_LENGTH_ANTHOUSE / 2 };
const VECTOR3 MERCURY_OFS_ABORT = { 0.0, -0.04, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2 + MERCURY_LENGTH_ABORT / 2 };
const VECTOR3 MERCURY_OFS_MAINCHUTE = { 0.0, 0.0, MERCURY_LENGTH_CAPSULE / 2 - .05 };
const VECTOR3 MERCURY_OFS_DROGUE = { -0.12, 0.08, -.2 + MERCURY_LENGTH_ANTHOUSE / 2 };
const VECTOR3 MERCURY_OFS_DROGUECOVER = { -0.12, 0.08, -0.08 + MERCURY_OFS_ANTHOUSE.z + MERCURY_LENGTH_ANTHOUSE / 2 };
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
const VECTOR3 TOUCHDOWN_LAUNCH0 = _V(0.0, -1.0, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_LAUNCH1 = _V(-0.7, 0.7, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_LAUNCH2 = _V(0.7, 0.7, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_LAUNCH3 = _V(0.0, 0.0, MERCURY_OFS_ABORT.z + MERCURY_LENGTH_ABORT / 2.0);
//static TOUCHDOWNVTX tchdwnLaunch[tchdwnLaunchNum] = {
//	// pos, stiff, damping, mu, mu long
//	{TOUCHDOWN_LAUNCH0, 1e7, 1e5, 10},
//	{TOUCHDOWN_LAUNCH1, 1e7, 1e5, 10},
//	{TOUCHDOWN_LAUNCH2, 1e7, 1e5, 10},
//	{TOUCHDOWN_LAUNCH3, 1e7, 1e5, 10},
//};


static const DWORD tchdwnTowSepNum = 4;
const VECTOR3 TOUCHDOWN_TOWSEP0 = _V(0.0, -1.0, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_TOWSEP1 = _V(-0.7, 0.7, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_TOWSEP2 = _V(0.7, 0.7, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0);
const VECTOR3 TOUCHDOWN_TOWSEP3 = _V(0.0, 0.0, MERCURY_OFS_CAPSULE.z + MERCURY_LENGTH_CAPSULE / 2.0);
//static TOUCHDOWNVTX tchdwnTowSep[tchdwnTowSepNum] = {
//	// pos, stiff, damping, mu, mu long
//	{TOUCHDOWN_TOWSEP0, 1e7, 1e5, 10},
//	{TOUCHDOWN_TOWSEP1, 1e7, 1e5, 10},
//	{TOUCHDOWN_TOWSEP2, 1e7, 1e5, 10},
//	{TOUCHDOWN_TOWSEP3, 1e7, 1e5, 10},
//};


static const DWORD tchdwnFlightNum = 4;
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

static const DWORD tchdwnAbortNum = 4;
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

// Contrail conditions. Calibrated to Liberty Bell 7 launch video, from T+80 to T+95 s
const double contrailBegin = 0.35; // Air density for contrail to begin
const double contrailEnd = 0.1; // Air density for contrail to end

// Values from 19670022649
const double SINGLECASTOR_MASS = 3985.0;
const double SINGLECASTOR_DRY_MASS = 604.6;
const double SINGLECASTOR_FUEL_MASS = SINGLECASTOR_MASS - SINGLECASTOR_DRY_MASS;
const double SINGLERECRUIT_MASS = 165.1;
const double SINGLERECRUIT_DRY_MASS = 45.4;
const double SINGLERECRUIT_FUEL_MASS = SINGLERECRUIT_MASS - SINGLERECRUIT_DRY_MASS;
const double LITTLEJOE_FRAME_MASS = 1100.0; // only rigid outer body
const double LITTLEJOE_TOT_MASS = 17700.0; // entire Little Joe booster
const double LITTLEJOE_TOT_DRY_MASS = LITTLEJOE_FRAME_MASS + SINGLERECRUIT_DRY_MASS * 4.0 + SINGLECASTOR_DRY_MASS * 4.0; // Total booster dry weight, i.e. empty mass

// Values from Wikipedia and 19670022650
const double SINGLECASTOR_THRUST_SL = 237.3e3; // picked 19670022650 value here (page 32)
const double SINGLECASTOR_ISP_SL = 27.0 * SINGLECASTOR_THRUST_SL / SINGLECASTOR_FUEL_MASS; // picked 19670022650 value here (page 32)
const double SINGLECASTOR_THRUST_VAC = 259e3; // picked wikipedia value here
const double SINGLECASTOR_ISP_VAC = 27.0 * SINGLECASTOR_THRUST_VAC / SINGLECASTOR_FUEL_MASS; // picked wikipedia value here

const double SINGLERECRUIT_THRUST_SL = 161.5e3; // picked 19670022650 value here (page 32)
const double SINGLERECRUIT_ISP_SL = 1.52 * SINGLERECRUIT_THRUST_SL / SINGLERECRUIT_FUEL_MASS; // picked 19670022650 value here (page 32)
const double SINGLERECRUIT_THRUST_VAC = 167e3; // picked wikipedia value here
const double SINGLERECRUIT_ISP_VAC = 1.52 * SINGLERECRUIT_THRUST_VAC / SINGLERECRUIT_FUEL_MASS; // picked wikipedia value here

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

// Periscope aparture sizes from 19740075935_1974075935 page 72
const double PERISCOPE_NARROW = 19.0 * RAD / 2.0;
const double PERISCOPE_WIDE = 175.0 * RAD / 2.0; // Actually 175 deg, but Orbiter only supports up to 160.0 deg, so this will be truncated to 160 deg FOV by Orbiter
const double PERISCOPE_ANGLE = (90.0 - 14.5) * RAD; // 19630012071 page 291
const VECTOR3 CAMERA_DIRECTION = _V(0, -sin(PERISCOPE_ANGLE), cos(PERISCOPE_ANGLE));
const VECTOR3 CAMERA_OFFSET = _V(0.0, -0.6, -0.2);

const int numRocketCamModes = 2;
const VECTOR3 ROCKET_CAMERA_DIRECTION[numRocketCamModes] = { _V(0, 0, -1), _V(0,0,1) };
const VECTOR3 ROCKET_CAMERA_OFFSET[numRocketCamModes] = { _V(0.4, 1.2, -LITTLEJOE_LENGTH / 2.0 + 3.0), _V(0.4, 1.2, LITTLEJOE_LENGTH / 2.0 + 1.0) };

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
	void clbkVisualCreated(VISHANDLE vis, int refcount);
	bool clbkLoadGenericCockpit(void);
	bool clbkPanelMouseEvent(int id, int event, int mx, int my, void* context);
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

	void AimEulerAngle(double pitch, double yaw);
	void DisableAutopilot(bool turnOff);
	void CreateAirfoilsLittleJoe(void);

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
	VECTOR3 SwapXY(VECTOR3 vIn);

	double OrbitArea(double angle, double ecc);
	double OrbitalFrameSlipAngle(VECTOR3 pos, VECTOR3 vel);
	double OrbitalFrameSlipAngle2(VECTOR3 pos, VECTOR3 vel);

	void SeparateTower(bool noAbortSep);
	void SeparateLittleJoe(void);
	void SeparateRingsAndAdapters(VECTOR3 redstoneDirection);
	void SetCameraSceneVisibility(WORD mode);

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

	void GetPixelDeviationForAltitude(double inputAltitude, double* deg0Pix, double* deg5Pix);
	void SetPeriscopeAltitude(double inputAltitude);

	void FitPanelToScreen(int w, int h);
	void AnimateDials(void);
	void RotateArmGroup(int groupNum, float x0, float y0, float length, float width, float angleR, float pointiness, float negLength = 0.0f, bool includeLatency = true);
	float ValueToAngle(float value, float minValue, float maxValue, float minAngle, float maxAngle);
	void RotateGlobe(float angularResolution, float viewAngularRadius, float longitude0, float latitude0, float rotationAngle = 0.0f);
	void ChangePanelNumber(int group, int num);

	void GetPanelRetroTimes(double met, int* rH, int* rM, int* rS, int* dH, int* dM, int* dS);

	// Mercury network
	bool InRadioContact(OBJHANDLE planet);
	double MnA2TrA(double MnA, double Ecc);
	double TrA2MnA(double TrA, double Ecc);
	double EccentricAnomaly(double ecc, double TrA);
	void myStrncpy(char* writeTo, const char* readFrom, int len);

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
	void LoadCapsule(const char* cbuf);
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
	static void vlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	static void vliftEscape(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftEscape(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	static void vliftLittleJoe(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftLittleJoe(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	THRUSTER_HANDLE th_castor[4], th_pollux[4], th_recruit[4], escape_engine, thruster_retro[3], thruster_posigrade[3], thruster_man_py[4],
		thruster_man_roll[2], thruster_auto_py[4], thruster_auto_py_1lb[4], thruster_auto_roll[2], thruster_auto_roll_1lb[2],
		pitchup, pitchdown, yawleft, yawright, bankleft, bankright;
	PROPELLANT_HANDLE castor_propellant[4], pollux_propellant[4], recruit_propellant[4], retro_propellant[3], posigrade_propellant[3], fuel_auto, fuel_manual, escape_tank;
	//THRUSTER_HANDLE th_main, th_rcs[14], th_group[4];
	PARTICLESTREAMSPEC contrail_main_castor, contrail_main_recruit;
	PSTREAM_HANDLE contrail_castor[4], contrail_recruit[4], contrail_vapour, rcsStream[18], exhaustCastor[2];
	bool contrailActive = true;
	bool suborbitalMission = true; // change to false for Mercury Atlas!
	bool boilerplateMission = false;

	MESHHANDLE booster,
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

	UINT Booster; // rocket mesh
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
	UINT DrogueDeployAnim, MainChuteDeployAnim, LandingBagDeployAnim, Rudder1Anim, Rudder2Anim, Rudder3Anim, Rudder4Anim, JetVaneAnim[4], PeriscopeDeployAnim, DestabiliserDeployAnim;
	ANIMATIONCOMPONENT_HANDLE DrogueDeployAnim1, MainChuteDeployAnim1, LandingBagDeployAnim1;
	int heatShieldGroup = 29;

	//CTRLSURFHANDLE Fins[4];

	ATTACHMENTHANDLE padAttach;

	// ReplacementTextures
	int numTextures = 0;
	char textureString[50][100] = { NULL }; // now supports up to 50 appended textures. Should be more than plenty enough
	//char texturePorthole[50] = { NULL };
	//int texturePortholeWidth, texturePortholeHeight;
	ATEX texPorthole, texBrailmap, texMBrail1, texMBrail2, texRidges, texVerrail1, texMetal, texVerrail2, texOrange, texPythr, texRollthst, texScanner, texScope, texSnorkel, texTrailmap, texTopboxes, texUsa, // all these present in Freedom 7 mesh
		texArtwork,
		texCrack, texGlass, texFoilwindow, texFoil, texWindowfr,
		texWfrfront, texWindow, texFlag,
		texBoilerplateCapsule,
		texMetalant, texAntridge, texScrew, texDialec,
		texMetalret, texDialecret, texBw,
		texLittlej, texBaixxo, texJoefins, texFinfron, texBodynum;
	int configTextureUserNum = -1; // if between 0 and 9, it is defined
	char configTextureUserName[10][20]; // up to 10 user defined capsules, supports up to length 20
	int configTextureUserBasis[10]; // The original frame to build upon
	bool configTextureUserEnable = false; // by default, don't load textures defined in config. Only if actual capsule is called
	bool scenarioTextureUserEnable = false;

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
	bool launchPerformed = false;
	double castorFireTime = 23.0; // 19670022650 page 14
	double boosterShutdownTime = 0.0;
	bool boosterSeparated = false;
	bool towerJettisoned = false;
	bool posigradeDampingActivated = false;
	double posigradeDampingTime = 0.0;
	bool turnAroundFinished = false;
	bool attitudeHold14deg = false;
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
	double previousThrusterLevel = 0.0;
	double eulerPitch = 0.0;
	double eulerYaw = 0.0;

	int showInfoOnHud = 0;	/* 0 = Both key commands and flight data
							   1 = Only flight data
							   2 = Nothing (only stock HUD)*/
	char contactBase[50];
	bool leftMFDwasOn = false;
	bool rightMFDwasOn = false;
	bool MercuryNetwork = true;

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
	double periscopeAltitude = 160.0;
	bool rocketCam = false;
	int rocketCamMode = 0;
	// Include panel, although technically Little Joe was never manned
	bool panelView = false;
	int armGroups[50] = { NULL }; // Support up to 50 for now. Real number more like 15
	int totalArmGroups = 0;
	int panelMeshGroupSide[100] = { 0 }; // 0 empty, 1 left, 2 right, -1 ignore
	float addScreenWidthValue = 0.0;
	int globeGroup = NULL;
	int globeVertices = NULL;
	float previousDialAngle[200] = { 0.0f }; // must be longer than total mesh group number
	float dialAngularSpeed = float(180.0 * RAD); // Degrees per second

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

	// Beware of editing these, as scenarios may be affected.
	enum capsulever { FREEDOM7, LIBERTYBELL7, FRIENDSHIP7, AURORA7, SIGMA7, FAITH7, FREEDOM7II, CAPSULEBIGJOE, CAPSULELITTLEJOE, CAPSULEBD } CapsuleVersion;
	enum vesselstate { LAUNCH, TOWERSEP, FLIGHT, REENTRY, REENTRYNODROGUE, REENTRYMAIN, ABORT, ABORTNORETRO } VesselStatus, PreviousVesselStatus;
	//enum droguestate { D_CLOSED, D_DEPLOYED, D_OPENING, D_REEFED, D_UNREEFING, D_OPENED } DrogueStatus;
	enum chutestate { CLOSED, DEPLOYED, OPENING, REEFED, UNREEFING, OPENED } DrogueStatus, MainChuteStatus, ReserveChuteStatus;
	enum landingbagstate { L_CLOSED, L_DEPLOYED, L_OPENING, L_OPENED } LandingBagStatus;
	enum periscopestate { P_CLOSED, P_DEPLOYED, P_OPENING, P_CLOSING } PeriscopeStatus, DestabiliserStatus;
	enum rcsstate { MANUAL, AUTOHIGH, AUTOLOW } RcsStatus;
	enum autopilotstate { AUTOLAUNCH, POSIGRADEDAMP, TURNAROUND, PITCHHOLD, REENTRYATT, LOWG } AutopilotStatus;
	enum filtertype { CLEAR, RED, YELLOW, GRAY } CurrentFilter;

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

	// Concept adapter stuff that has to be created, but is only used on Atlas
	bool conceptManouverUnit = false;
	bool conceptManouverUnitAttached = false;
	PROPELLANT_HANDLE conceptPropellant;
	THRUSTER_HANDLE conceptPosigrade[2], conceptRetrograde[2], conceptLinear[6];
};
