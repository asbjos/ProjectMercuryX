// ==============================================================
// ORBITER vessel module: Litle Joe Booster
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"

const double LITTLEJOE_LENGTH = 7.70;
const VECTOR3 LITTLEJOE_CASTOR_EXHAUST_POS = { 0.450, 0.450, 2.15 };
const VECTOR3 LITTLEJOE_RECRUIT_EXHAUST_POS = { 0.880, 0.0, 2.55 };
const double CASTOR_ANGLE = 11.0 * RAD;
const double RECRUIT_ANGLE = 12.0 * RAD;
const VECTOR3 LITTLEJOE_CASTOR_EXHAUST_DIR = { -sqrt(0.5) * sin(CASTOR_ANGLE), -sqrt(0.5) * sin(CASTOR_ANGLE), cos(CASTOR_ANGLE) };
const VECTOR3 LITTLEJOE_RECRUIT_EXHAUST_DIR = { -sin(RECRUIT_ANGLE), 0.0, cos(CASTOR_ANGLE) };

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

class ProjectMercury : public VESSEL3
{
public:
	ProjectMercury(OBJHANDLE hVessel, int flightmodel);
	~ProjectMercury();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPreStep(double simt, double simdt, double mjd);
	void clbkPostStep(double simt, double simdt, double mjd);

	void CreateAirfoilsLittleJoe(void);
	VECTOR3 FlipX(VECTOR3 vIn);
	VECTOR3 FlipY(VECTOR3 vIn);
	VECTOR3 SwapXY(VECTOR3 vIn);

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
private:

	static void vliftLittleJoe(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hliftLittleJoe(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);

	THRUSTER_HANDLE th_castor[4], th_pollux[4], th_recruit[4];
	PROPELLANT_HANDLE castor_propellant[4], pollux_propellant[4], recruit_propellant[4];
	PARTICLESTREAMSPEC contrail_main_castor, contrail_main_recruit;
	PSTREAM_HANDLE contrail_castor[4], contrail_recruit[4], contrail_vapour;

	bool lastBoostersOn = false;

	MESHHANDLE booster;
	UINT Booster;
	THRUSTER_HANDLE th_main;
	PARTICLESTREAMSPEC contrail_main;
	PSTREAM_HANDLE contrail;
	bool contrailActive = true;
	bool firstFrame = true;
	bool selfDestruct = false;
	double creationTime = 0.0;
	int frameNr = 0;
	int deleteNextFrame = 0;
	double explosionLevel = 0.0;
};

#include "..\FunctionsForOrbiter2010.h"

ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel) : VESSEL3(hVessel, flightmodel)
{
	booster = oapiLoadMeshGlobal("ProjectMercury\\LJ"); // debug
}

ProjectMercury::~ProjectMercury()
{
}

void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	SetSize(LITTLEJOE_LENGTH / 2.0);

	// Read height over ground from config
	double heightOverGround;
	if (!oapiReadItem_float(cfg, "HeightOverGround", heightOverGround))
	{
		heightOverGround = 5.8; // if not available in config file
		oapiWriteLog("Little Joe booster could not read config");
	}
	static const DWORD touchdownPointsNumbers = 6;
	const VECTOR3 TOUCH0 = _V(0.1, -1.0, -LITTLEJOE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCH1 = _V(-0.7, 0.7, -LITTLEJOE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCH2 = _V(0.7, 0.7, -LITTLEJOE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCH3 = _V(0.1, -0.5, LITTLEJOE_LENGTH / 2.0);
	const VECTOR3 TOUCH4 = _V(-0.5, 0.5, LITTLEJOE_LENGTH / 2.0);
	const VECTOR3 TOUCH5 = _V(0.5, 0.5, LITTLEJOE_LENGTH / 2.0);
	VersionDependentTouchdown(TOUCH0, TOUCH1, TOUCH2, TOUCH3, 1e5, 1e5, 0.3);
	SetEmptyMass(LITTLEJOE_TOT_DRY_MASS);

	SetCW(1.0, 0.1, 0.3, 0.3);
	SetCrossSections(_V(38.4, 38.4, 4.04));
	SetRotDrag(_V(0.7, 0.7, 1.2));
	SetPMI(_V(33.0, 33.0, 10.51));
	SetLiftCoeffFunc(0);

	for (int i = 0; i < 4; i++)
	{
		castor_propellant[i] = CreatePropellantResource(SINGLECASTOR_FUEL_MASS);
		recruit_propellant[i] = CreatePropellantResource(SINGLERECRUIT_FUEL_MASS);
	}

	SetDefaultPropellantResource(castor_propellant[0]);

	// main engine
	th_castor[0] = CreateThruster(LITTLEJOE_CASTOR_EXHAUST_POS, LITTLEJOE_CASTOR_EXHAUST_DIR, SINGLECASTOR_THRUST_VAC, castor_propellant[0], SINGLECASTOR_ISP_VAC, SINGLECASTOR_ISP_SL);
	th_castor[1] = CreateThruster(FlipX(LITTLEJOE_CASTOR_EXHAUST_POS), FlipX(LITTLEJOE_CASTOR_EXHAUST_DIR), SINGLECASTOR_THRUST_VAC, castor_propellant[1], SINGLECASTOR_ISP_VAC, SINGLECASTOR_ISP_SL);
	th_castor[2] = CreateThruster(FlipY(FlipX(LITTLEJOE_CASTOR_EXHAUST_POS)), FlipY(FlipX(LITTLEJOE_CASTOR_EXHAUST_DIR)), SINGLECASTOR_THRUST_VAC, castor_propellant[2], SINGLECASTOR_ISP_VAC, SINGLECASTOR_ISP_SL);
	th_castor[3] = CreateThruster(FlipY(LITTLEJOE_CASTOR_EXHAUST_POS), FlipY(LITTLEJOE_CASTOR_EXHAUST_DIR), SINGLECASTOR_THRUST_VAC, castor_propellant[3], SINGLECASTOR_ISP_VAC, SINGLECASTOR_ISP_SL);

	th_recruit[0] = CreateThruster(LITTLEJOE_RECRUIT_EXHAUST_POS, LITTLEJOE_RECRUIT_EXHAUST_DIR, SINGLERECRUIT_THRUST_VAC, recruit_propellant[0], SINGLERECRUIT_ISP_VAC, SINGLERECRUIT_ISP_SL);
	th_recruit[1] = CreateThruster(FlipX(LITTLEJOE_RECRUIT_EXHAUST_POS), FlipX(LITTLEJOE_RECRUIT_EXHAUST_DIR), SINGLERECRUIT_THRUST_VAC, recruit_propellant[1], SINGLERECRUIT_ISP_VAC, SINGLERECRUIT_ISP_SL);
	th_recruit[2] = CreateThruster(FlipY(SwapXY(LITTLEJOE_RECRUIT_EXHAUST_POS)), FlipY(SwapXY(LITTLEJOE_RECRUIT_EXHAUST_DIR)), SINGLERECRUIT_THRUST_VAC, recruit_propellant[2], SINGLERECRUIT_ISP_VAC, SINGLERECRUIT_ISP_SL);
	th_recruit[3] = CreateThruster(SwapXY(LITTLEJOE_RECRUIT_EXHAUST_POS), SwapXY(LITTLEJOE_RECRUIT_EXHAUST_DIR), SINGLERECRUIT_THRUST_VAC, recruit_propellant[3], SINGLERECRUIT_ISP_VAC, SINGLERECRUIT_ISP_SL);

	CreateThrusterGroup(th_recruit, 4, THGROUP_MAIN);
	AddExhaust(th_castor[0], 8, 1, LITTLEJOE_CASTOR_EXHAUST_POS, -LITTLEJOE_CASTOR_EXHAUST_DIR);
	AddExhaust(th_castor[1], 8, 1, FlipX(LITTLEJOE_CASTOR_EXHAUST_POS), -FlipX(LITTLEJOE_CASTOR_EXHAUST_DIR));
	AddExhaust(th_castor[2], 8, 1, FlipY(FlipX(LITTLEJOE_CASTOR_EXHAUST_POS)), -FlipY(FlipX(LITTLEJOE_CASTOR_EXHAUST_DIR)));
	AddExhaust(th_castor[3], 8, 1, FlipY(LITTLEJOE_CASTOR_EXHAUST_POS), -FlipY(LITTLEJOE_CASTOR_EXHAUST_DIR));

	AddExhaust(th_recruit[0], 4, 1, LITTLEJOE_RECRUIT_EXHAUST_POS, -LITTLEJOE_RECRUIT_EXHAUST_DIR);
	AddExhaust(th_recruit[1], 4, 1, FlipX(LITTLEJOE_RECRUIT_EXHAUST_POS), -FlipX(LITTLEJOE_RECRUIT_EXHAUST_DIR));
	AddExhaust(th_recruit[2], 4, 1, FlipY(FlipX(LITTLEJOE_RECRUIT_EXHAUST_POS)), -FlipY(FlipX(LITTLEJOE_RECRUIT_EXHAUST_DIR)));
	AddExhaust(th_recruit[3], 4, 1, FlipY(LITTLEJOE_RECRUIT_EXHAUST_POS), -FlipY(LITTLEJOE_RECRUIT_EXHAUST_DIR));

	// Contrail stuff
	contrail_main_castor = {
		0, 3.0, 100, 200, 0.1, 20.0, 2.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin, contrailEnd
	};
	contrail_castor[0] = AddExhaustStream(th_castor[0], _V(0, 0.0, -20), &contrail_main_castor);

	contrail_main_recruit = {
		0, 3.0, 100, 200, 0.1, 20.0, 2.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin, contrailEnd
	};
	contrail_recruit[0] = AddExhaustStream(th_recruit[0], _V(0, 0.0, -15), &contrail_main_recruit);

	Booster = AddMesh(booster);

	// create huge clouds and fireballs
	PARTICLESTREAMSPEC explode1 = {
		0, 10.0, 20, 2.0, 10.0, 45.0, 50.0, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_LIN, 0, 1,
		PARTICLESTREAMSPEC::ATM_PLOG, 0.0, 3.0
	};
	PARTICLESTREAMSPEC explode2 = {
		0, 10.0, 20, 2.0, 10.0, 1.0, 50.0, 1.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_LIN, 0, 1,
		PARTICLESTREAMSPEC::ATM_PLOG, 0.0, 3.0
	};

	AddParticleStream(&explode1, _V(0.0, 0.0, 0.0), _V(0.0, 0.0, 1.0), &explosionLevel);
	AddParticleStream(&explode2, _V(0.0, 0.0, 0.0), _V(0.0, 0.0, -1.0), &explosionLevel);

	// Control surfaces for air rudders. dCl and delay values from DeltaGlider and/or Falcon 9 by BrianJ
	CreateAirfoilsLittleJoe(); // try this
}

void ProjectMercury::clbkPreStep(double simt, double simdt, double mjd)
{
	if (selfDestruct && simt - creationTime > 3.2) // delete
	{
		//oapiWriteLog("It has been a privelige exploding with you.");
		oapiDeleteVessel(GetHandle());
	}
	else if (selfDestruct && simt - creationTime > 3.0) // allow astronaut to escape
	{
		//oapiWriteLog("Destructing");
		explosionLevel = 1.0;
		// Maybe add other damage models (for when the stage reenters the atmosphere)
	}

	if (GetThrusterGroupLevel(THGROUP_MAIN) != 0.0)
	{
		SetThrusterLevel(th_recruit[0], 1.0);
		SetThrusterLevel(th_recruit[1], 1.0);
		SetThrusterLevel(th_recruit[2], 1.0);
		SetThrusterLevel(th_recruit[3], 1.0);

		SetThrusterLevel(th_castor[0], 1.0);
		SetThrusterLevel(th_castor[2], 1.0);
	}

	if (lastBoostersOn)
	{
		SetThrusterLevel(th_castor[1], 1.0);
		SetThrusterLevel(th_castor[3], 1.0);
	}

	if (GetThrusterLevel(th_castor[1]) != 0.0)
	{
		lastBoostersOn = true;
	}
}

void ProjectMercury::clbkPostStep(double simt, double simdt, double mjd)
{
	if (GetAtmDensity() < contrailEnd)
	{
		DelExhaustStream(contrail);

		contrailActive = false;
	}
	else if (!contrailActive) // If in dense atmosphere and contrail has not been activated
	{
		contrail = AddExhaustStream(th_main, _V(0, 0.3, -20), &contrail_main);
		contrailActive = true;
	}

	if (frameNr < 2 && !selfDestruct) // after first couble of frames, so that Mercury has time to get into new condition
	{
		// explosionLevel = 1.0;

		frameNr += 1;

		// firstFrame = false;
		// Find out if Mercury is in abort mode

		char* name = GetName(); // Our full name
		char parentName[256];
		sprintf(parentName, "%.*s", strlen(name) - 8, name); // " booster" has length 17

		double MERCURY_LENGTH_CAPSULE = 2.3042;
		double MERCURY_LENGTH_ABORT = 5.1604;

		OBJHANDLE parent = oapiGetVesselByName(parentName);
		if (parent != NULL)
		{
			if (oapiGetSize(parent) == MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT) // in abort mode
			{
				oapiWriteLog("Little Joe abort condition. Self-destruct!");
				creationTime = simt;
				selfDestruct = true;
			}
			else
			{
				oapiWriteLog("No abort. Regular staging.");
				//oapiWriteLogV("Size is: %.2f", oapiGetSize(parent));
			}
		}
		else
		{
			oapiWriteLog("No Little Joe parent found");
		}
	}
}

// Initialisation
DLLCLBK VESSEL* ovcInit(OBJHANDLE hvessel, int flightmodel)
{
	return new ProjectMercury(hvessel, flightmodel);
}

// Cleanup
DLLCLBK void ovcExit(VESSEL* vessel)
{
	if (vessel) delete (ProjectMercury*)vessel;
}

void ProjectMercury::CreateAirfoilsLittleJoe(void)
{
	ClearAirfoilDefinitions(); // delete previous airfoils
	SetRotDrag(_V(0.25, 0.25, 0.1)); // from BrianJ's Falcon 9
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 2.0), vliftLittleJoe, NULL, 1.893, 2.81, 1.0); // 19670022650 page 4. The centre of pressure is estimated, bases on Little Joe II (Apollo)
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 2.0), hliftLittleJoe, NULL, 1.893, 2.81, 1.0); // spherical symmetric
}

inline VECTOR3 ProjectMercury::FlipX(VECTOR3 vIn)
{
	VECTOR3 vOut;
	vOut.x = -vIn.x;
	vOut.y = vIn.y;
	vOut.z = vIn.z;
	return vOut;
}

inline VECTOR3 ProjectMercury::FlipY(VECTOR3 vIn)
{
	VECTOR3 vOut;
	vOut.x = vIn.x;
	vOut.y = -vIn.y;
	vOut.z = vIn.z;
	return vOut;
}

inline VECTOR3 ProjectMercury::SwapXY(VECTOR3 vIn)
{
	VECTOR3 vOut;
	vOut.x = vIn.y;
	vOut.y = vIn.x;
	vOut.z = vIn.z;
	return vOut;
}

void ProjectMercury::vliftLittleJoe(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	// The papers on Little Joe aerodynamics are strangely classified. The Little Joe II (Apollo) aerodynamics are luckily not, so we use them here.

	// 19680013484 page 73, the pitching moment is approximately linear, with slope 0.16 * aoa(deg).
	// As the Little Joe was very stable, following the wind, we only care about this small AoA regime anyway.
	*cm = -0.16 * aoa * DEG;

	// Similarly, page 76, we see that the lift coefficient is approximately linear, with slope 0.26 * aoa(deg).
	*cl = -0.26 * aoa * DEG;
	//sprintf(oapiDebugString(), "aoa: %.1f, cm: %.2f, cl: %.2f", aoa, cm, cl);

	// Finally, the drag coefficient varies greatly both with AoA and mach, but as with the Mercury capsule, we assume AoA zero, and create only a mach dependence.
	if (M > 6.8) M = 6.8; // max of this interpolation
	if (M < 0.0) M = 0.0; // min of this interpolation
	*cd = -2.6154644599e-06 * pow(M, 15) + 1.3925595645e-04 * pow(M, 14) + -3.3479687526e-03 * pow(M, 13) + 4.8015136575e-02 * pow(M, 12) + -4.5705296374e-01 * pow(M, 11) + 3.0391668768e+00 * pow(M, 10) + -1.4462570476e+01 * pow(M, 9) + 4.9598487277e+01 * pow(M, 8) + -1.2169523577e+02 * pow(M, 7) + 2.0891394075e+02 * pow(M, 6) + -2.4064585332e+02 * pow(M, 5) + 1.7341363369e+02 * pow(M, 4) + -7.0416769088e+01 * pow(M, 3) + 1.5178741115e+01 * pow(M, 2) + -2.3548590922e+00 * pow(M, 1) + 8.8797126179e-01 * pow(M, 0);

	* cd *= 0.5;
}

void ProjectMercury::hliftLittleJoe(VESSEL* v, double beta, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	// The papers on Little Joe aerodynamics are strangely classified. The Little Joe II (Apollo) aerodynamics are luckily not, so we use them here.

	// 19680013484 page 73, the pitching moment is approximately linear, with slope 0.16 * aoa(deg).
	// As the Little Joe was very stable, following the wind, we only care about this small AoA regime anyway.
	*cm = 0.0; // we're in hlift

	// Similarly, page 76, we see that the lift coefficient is approximately linear, with slope 0.26 * aoa(deg).
	*cl = -0.26 * beta * DEG;

	// Finally, the drag coefficient varies greatly both with AoA and mach, but as with the Mercury capsule, we assume AoA zero, and create only a mach dependence.
	if (M > 6.8) M = 6.8; // max of this interpolation
	if (M < 0.0) M = 0.0; // min of this interpolation
	*cd = -2.6154644599e-06 * pow(M, 15) + 1.3925595645e-04 * pow(M, 14) + -3.3479687526e-03 * pow(M, 13) + 4.8015136575e-02 * pow(M, 12) + -4.5705296374e-01 * pow(M, 11) + 3.0391668768e+00 * pow(M, 10) + -1.4462570476e+01 * pow(M, 9) + 4.9598487277e+01 * pow(M, 8) + -1.2169523577e+02 * pow(M, 7) + 2.0891394075e+02 * pow(M, 6) + -2.4064585332e+02 * pow(M, 5) + 1.7341363369e+02 * pow(M, 4) + -7.0416769088e+01 * pow(M, 3) + 1.5178741115e+01 * pow(M, 2) + -2.3548590922e+00 * pow(M, 1) + 8.8797126179e-01 * pow(M, 0);

	* cd *= 0.5;
}