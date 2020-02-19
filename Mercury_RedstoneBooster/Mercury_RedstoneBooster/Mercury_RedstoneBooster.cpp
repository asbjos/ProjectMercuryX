// ==============================================================
// ORBITER vessel module: Redstone Booster
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"

const double STAGE1_LENGTH = 18.2;
const VECTOR3 REDSTONE_EXHAUST_POS = { 0.0, 0.0, - STAGE1_LENGTH / 2.0 };
const VECTOR3 REDSTONE_EXHAUST_DRI = { 0.0, 0.0, 1.0 };

// Contrail conditions. Calibrated to Liberty Bell 7 launch video, from T+80 to T+95 s
const double contrailBegin = 0.35; // Air density for contrail to begin
const double contrailEnd = 0.1; // Air density for contrail to end

const double STAGE1_MASS = 28089; // 28440, but now shaving off to get correct total weight (66 000 lbs)
const double STAGE1_DRY_MASS = 3717; // Total booster dry weight 8 195 lbs
const double STAGE1_FUEL_MASS = STAGE1_MASS - STAGE1_DRY_MASS;
const double STAGE1_ISP_SL = 235 * G; // Sea Level (235), 216 according to NTRS
const double STAGE1_ISP_VAC = 247 * G;
const double STAGE1_THRUST = 395900; // 414340, or 395892 (89 000 lbs)

class Mercury_RedstoneBooster : public VESSEL4
{
public:
	Mercury_RedstoneBooster(OBJHANDLE hVessel, int flightmodel);
	~Mercury_RedstoneBooster();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPreStep(double simt, double simdt, double mjd);
	void clbkPostStep(double simt, double simdt, double mjd);
private:
	MESHHANDLE redstoneB;
	UINT RedstoneB;
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

Mercury_RedstoneBooster::Mercury_RedstoneBooster(OBJHANDLE hVessel, int flightmodel) : VESSEL4(hVessel, flightmodel)
{
	redstoneB = oapiLoadMeshGlobal("ProjectMercury\\merc_redstone");
}

Mercury_RedstoneBooster::~Mercury_RedstoneBooster()
{
}

void Mercury_RedstoneBooster::clbkSetClassCaps(FILEHANDLE cfg)
{
	SetSize(STAGE1_LENGTH / 2.0);

	// Read height over ground from config
	double heightOverGround;
	if (!oapiReadItem_float(cfg, "HeightOverGround", heightOverGround))
	{
		heightOverGround = 5.8; // if not available in config file
		oapiWriteLog("Redstone could not read config");
	}
	static const DWORD touchdownPointsNumbers = 6;
	static TOUCHDOWNVTX touchdownPointies[touchdownPointsNumbers] = {
		// pos, stiff, damping, mu, mu long
		{_V(0.1, -1.0, -STAGE1_LENGTH / 2.0 - heightOverGround), 1e5, 1e5, 0.3},
		{_V(-0.7, 0.7, -STAGE1_LENGTH / 2.0 - heightOverGround), 1e5, 1e5, 0.3},
		{_V(0.7, 0.7, -STAGE1_LENGTH / 2.0 - heightOverGround), 1e5, 1e5,  0.3},
		{_V(0.1, -0.5, STAGE1_LENGTH / 2.0), 1e5, 1e5, 0.3},
		{_V(-0.5, 0.5, STAGE1_LENGTH / 2.0), 1e5, 1e5, 0.3},
		{_V(0.5, 0.5, STAGE1_LENGTH / 2.0), 1e5, 1e5, 0.3},
	};
	SetTouchdownPoints(touchdownPointies, touchdownPointsNumbers);
	SetEmptyMass(STAGE1_DRY_MASS);

	SetCW(1.0, 0.1, 0.3, 0.3);
	SetCrossSections(_V(38.4, 38.4, 4.04));
	SetRotDrag(_V(0.7, 0.7, 1.2));
	SetPMI(_V(33.0, 33.0, 10.51));
	SetLiftCoeffFunc(0);

	PROPELLANT_HANDLE hpr = CreatePropellantResource(STAGE1_FUEL_MASS);

	th_main = CreateThruster(REDSTONE_EXHAUST_POS, REDSTONE_EXHAUST_DRI, STAGE1_THRUST, hpr, STAGE1_ISP_VAC, STAGE1_ISP_SL);
	CreateThrusterGroup(&th_main, 1, THGROUP_MAIN);
	AddExhaust(th_main, 8, 1, REDSTONE_EXHAUST_POS, -REDSTONE_EXHAUST_DRI);

	contrail_main = {
		0, 3.0, 100, 200, 0.1, 20.0, 2.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin, contrailEnd
	};

	contrail = AddExhaustStream(th_main, _V(0, 0.3, -20), &contrail_main);

	RedstoneB = AddMesh(redstoneB);

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
}

void Mercury_RedstoneBooster::clbkPreStep(double simt, double simdt, double mjd)
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
}

void Mercury_RedstoneBooster::clbkPostStep(double simt, double simdt, double mjd)
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
		sprintf(parentName, "%.*s", strlen(name) - 17, name); // " Redstone booster" has length 17

		double MERCURY_LENGTH_CAPSULE = 2.3042;
		double MERCURY_LENGTH_ABORT = 5.1604;

		OBJHANDLE parent = oapiGetVesselByName(parentName);
		if (parent != NULL)
		{
			if (oapiGetSize(parent) == MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT) // in abort mode
			{
				oapiWriteLog("Redstone abort condition. Self-destruct!");
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
			oapiWriteLog("No Redstone parent found");
		}
	}
}

// Initialisation
DLLCLBK VESSEL* ovcInit(OBJHANDLE hvessel, int flightmodel)
{
	return new Mercury_RedstoneBooster(hvessel, flightmodel);
}

// Cleanup
DLLCLBK void ovcExit(VESSEL* vessel)
{
	if (vessel) delete (Mercury_RedstoneBooster*)vessel;
}