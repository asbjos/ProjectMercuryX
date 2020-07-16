// ==============================================================
// ORBITER vessel module: Redstone Booster
// ==============================================================

#define STRICT
#define ORBITER_MODULE
#define VESSELVER VESSEL3

#include "Orbitersdk.h"

const double ATLAS_CORE_LENGTH = 20.9;
const double ATLAS_BOOSTER_LENGTH = 3.86;
const double MERCURY_LENGTH_CAPSULE = 2.3042;
const VECTOR3 MERCURY_OFS_CAPSULE = { 0.0, 0.0, (MERCURY_LENGTH_CAPSULE) / 2.0 + ATLAS_CORE_LENGTH / 2.0 };
const VECTOR3 ATLAS_ADAPTER_OFFSET = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - 1.55 };
const VECTOR3 ATLAS_BOOSTER_OFFSET = { 0.0, 0.0, MERCURY_OFS_CAPSULE.z - 20.2 };
const VECTOR3 CORE_EXHAUST_POS = { 0.0, 0.0, -ATLAS_CORE_LENGTH / 2.0 };
const VECTOR3 CORE_EXHAUST_DIR = { 0.0, 0.0, 1.0 };
const VECTOR3 BOOSTER_EXHAUST_POS = { 1.38, 0.0, ATLAS_BOOSTER_OFFSET.z - ATLAS_BOOSTER_LENGTH / 2.0 };
const VECTOR3 VERNIER_EXHAUST_POS = { 0.0, -1.90, -6.822 };
const VECTOR3 TURBINE_EXHAUST_POS = { -0.1868404, -0.60114, -1.689692 + ATLAS_BOOSTER_OFFSET.z };
const VECTOR3 BOOSTER_EXHAUST_DIR = { 0.0, 0.0, 1.0 };
const VECTOR3 VERNIER_EXHAUST_DIR = { 0.0, 0.134382, 0.99093 };
const VECTOR3 TURBINE_EXHAUST_DIR = { 0.0, sin(10 * RAD), cos(10 * RAD) }; // pick angle to fit

// Contrail conditions. Calibrated to Liberty Bell 7 launch video, from T+80 to T+95 s
const double contrailBegin = 0.35; // Air density for contrail to begin
const double contrailEnd = 0.1; // Air density for contrail to end
// Secondary contrail. From T+103 to T+141 s
const double contrailBegin2 = 6e-3;
const double contrailEnd2 = 1e-5;

const double CORE_MASS = 113050.0; // 28440, but now shaving off to get correct total weight (66 000 lbs = ) - 28089 kg
const double CORE_DRY_MASS = 2347.0; // Total booster dry weight 8 195 lbs
const double CORE_FUEL_MASS = CORE_MASS - CORE_DRY_MASS;
const double CORE_ISP_SL = 215.0 * G; // Sea Level (215), astronautix
const double CORE_ISP_VAC = 309.0 * G;
const double CORE_THRUST = 363218.0; // astronautix. Or 253.5 kN (19670022198 page 27)

const double VERNIER_THRUST_VAC = 2975.0; // https://www.scss.tcd.ie/Stephen.Farrell/ipn/background/Braeunig/specs/atlas.htm Or 4448 N (19630012071 page 97)
const double  VERNIER_ISP_SL = 172.0 * G;
const double  VERNIER_ISP_VAC = 231.0 * G;
const VECTOR3 VERNIER1_OFFSET = _V(0.023, -1.829, -6.614);
const VECTOR3 VERNIER2_OFFSET = _V(-0.053, 1.841, -6.614);

const double BOOSTER_MASS = 3050.0; // 28440, but now shaving off to get correct total weight (66 000 lbs = ) - 28089 kg
const double BOOSTER_DRY_MASS = 3050.0; // Total booster dry weight 8 195 lbs
//const double BOOSTER_FUEL_MASS = CORE_MASS - CORE_DRY_MASS; // uses core fuel
const double BOOSTER_ISP_SL = 248.0 * G; // Sea Level (248), astronautix
const double BOOSTER_ISP_VAC = 282.0 * G;
const double BOOSTER_THRUST = 1517422.0 / 2.0; // Two thrusters, totaling 1.5 MN

inline VECTOR3 FlipX(VECTOR3 vIn)
{
	VECTOR3 vOut;
	vOut.x = -vIn.x;
	vOut.y = vIn.y;
	vOut.z = vIn.z;
	return vOut;
}

inline VECTOR3 FlipY(VECTOR3 vIn)
{
	VECTOR3 vOut;
	vOut.x = vIn.x;
	vOut.y = -vIn.y;
	vOut.z = vIn.z;
	return vOut;
}

class ProjectMercury : public VESSELVER
{
public:
	ProjectMercury(OBJHANDLE hVessel, int flightmodel);
	~ProjectMercury();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPreStep(double simt, double simdt, double mjd);
	void clbkPostStep(double simt, double simdt, double mjd);
	void clbkPostCreation();

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
	MESHHANDLE atlas, atlasIce, atlasIce2, atlasIce3, coreAdapter, coreBooster;
	UINT Atlas, AtlasIce, AtlasIce2, AtlasIce3, CoreAdapter, CoreBooster;
	THRUSTER_HANDLE th_main, th_booster[2], th_vernier[2], thCluster[5], th_main_vent;
	UINT exMain, exBooster[2], exVernier[2];
	PARTICLESTREAMSPEC contrail_main, contrail_second;
	PSTREAM_HANDLE contrail, contrail2, turbineExhaustContrail, boosterExhaustContrail[2];
	PROPELLANT_HANDLE atlas_propellant;
	enum icemesh { ICE0, ICE1, ICE2, ICE3 } AtlasIceStatus;
	bool contrailActive = true;
	bool contrail2Active = true;
	bool firstFrame = true;
	bool selfDestruct = false;
	double creationTime = 0.0;
	int frameNr = 0;
	int deleteNextFrame = 0;
	double explosionLevel = 0.0;
	double turbineContrailLevel = 0.0;

	bool boosterAttached = false;

	// Constants for unused functionsForOrbiter2016
	int TextX0 = 0, TextY0 = 0;
	int secondColumnHUDx = 0;
	int LineSpacing = 0;
};

#include "..\..\FunctionsForOrbiter2010.h"

ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel) : VESSELVER(hVessel, flightmodel)
{
	atlas = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main");
	atlasIce = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main_ice");
	atlasIce2 = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main_ice2");
	atlasIce3 = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main_ice3");
	coreAdapter = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_adapt");
	coreBooster = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_boost");
}

ProjectMercury::~ProjectMercury()
{
}

void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	SetSize(ATLAS_CORE_LENGTH / 2.0);

	if (!oapiReadItem_bool(cfg, "Booster", boosterAttached))
	{
		boosterAttached = false;
		oapiWriteLog("Atlas core could not read booster config");
	}

	bool conceptVersion = false;
	if (!oapiReadItem_bool(cfg, "NoAdapter", conceptVersion))
	{
		conceptVersion = false;
	}

	// Read height over ground from config
	double heightOverGround;
	if (!oapiReadItem_float(cfg, "HeightOverGround", heightOverGround))
	{
		heightOverGround = 5.8; // if not available in config file
		oapiWriteLog("Atlas core could not read height config");
	}
	static const DWORD touchdownPointsNumbers = 6;
	const VECTOR3 TOUCH_POINT0 = _V(0.1, -1.0, -ATLAS_CORE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCH_POINT1 = _V(-0.7, 0.7, -ATLAS_CORE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCH_POINT2 = _V(0.7, 0.7, -ATLAS_CORE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCH_POINT3 = _V(0.1, -0.5, ATLAS_CORE_LENGTH / 2.0);

	VersionDependentTouchdown(TOUCH_POINT0, TOUCH_POINT1, TOUCH_POINT2, TOUCH_POINT3, 1e5, 1e5, 0.3);
	SetEmptyMass(CORE_DRY_MASS);

	SetCW(1.0, 0.1, 0.3, 0.3);
	SetCrossSections(_V(38.4, 38.4, 4.04));
	SetRotDrag(_V(0.7, 0.7, 1.2));
	SetPMI(_V(33.0, 33.0, 10.51));
	SetLiftCoeffFunc(0);

	atlas_propellant = CreatePropellantResource(CORE_FUEL_MASS);

	// main engine
	th_main = CreateThruster(CORE_EXHAUST_POS, CORE_EXHAUST_DIR, CORE_THRUST, atlas_propellant, CORE_ISP_VAC, CORE_ISP_SL);
	th_vernier[0] = CreateThruster(VERNIER_EXHAUST_POS, VERNIER_EXHAUST_DIR, VERNIER_THRUST_VAC, atlas_propellant, VERNIER_ISP_VAC, VERNIER_ISP_SL); // -y
	th_vernier[1] = CreateThruster(FlipY(VERNIER_EXHAUST_POS), FlipY(VERNIER_EXHAUST_DIR), VERNIER_THRUST_VAC, atlas_propellant, VERNIER_ISP_VAC, VERNIER_ISP_SL); // +y
	th_booster[0] = CreateThruster(BOOSTER_EXHAUST_POS, BOOSTER_EXHAUST_DIR, BOOSTER_THRUST, atlas_propellant, BOOSTER_ISP_VAC, BOOSTER_ISP_SL);
	th_booster[1] = CreateThruster(FlipX(BOOSTER_EXHAUST_POS), FlipX(BOOSTER_EXHAUST_DIR), BOOSTER_THRUST, atlas_propellant, BOOSTER_ISP_VAC, BOOSTER_ISP_SL);
	thCluster[0] = th_main;
	thCluster[1] = th_vernier[0];
	thCluster[2] = th_vernier[1];
	thCluster[3] = th_booster[0];
	thCluster[4] = th_booster[1];
	CreateThrusterGroup(thCluster, 5, THGROUP_MAIN);
	SURFHANDLE tex = oapiRegisterExhaustTexture("Exhaust2");
	exMain = AddExhaust(th_main, 8, 1, tex);
	exVernier[0] = AddExhaust(th_vernier[0], 2, 0.2, tex);
	exVernier[1] = AddExhaust(th_vernier[1], 2, 0.2, tex);
	exBooster[0] = AddExhaust(th_booster[0], 15, 2, tex);
	exBooster[1] = AddExhaust(th_booster[1], 15, 2, tex);

	SURFHANDLE cont = oapiRegisterParticleTexture("Contrail3");
	PARTICLESTREAMSPEC turbineExhaust = {
		0, 3.0, 100, 50.0, 0.2, 0.2, 3.0, 0.5, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 0.5,
		PARTICLESTREAMSPEC::ATM_PLOG, 0.2, 1.2,
		cont
	};

	PARTICLESTREAMSPEC boosterExhaust = {
		0, 2.0, 100, 80.0, 0.1, 0.1, 1.0, 5.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 0.1,
		PARTICLESTREAMSPEC::ATM_PLOG, 0.05, 0.9,
		cont
	};

	turbineExhaustContrail = AddParticleStream(&turbineExhaust, TURBINE_EXHAUST_POS + _V(0.0, 0.0, -2.0), -TURBINE_EXHAUST_DIR, &turbineContrailLevel);
	boosterExhaustContrail[0] = AddParticleStream(&boosterExhaust, BOOSTER_EXHAUST_POS, -BOOSTER_EXHAUST_DIR, &turbineContrailLevel);
	boosterExhaustContrail[1] = AddParticleStream(&boosterExhaust, FlipX(BOOSTER_EXHAUST_POS), -BOOSTER_EXHAUST_DIR, &turbineContrailLevel);

	// Contrail stuff
	contrail_main = {
		0, 5.0, 100, 200, 0.1, 20.0, 3.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin, contrailEnd
	};
	contrail = AddExhaustStream(th_main, _V(0, 0.3, -25), &contrail_main);

	contrail_second = {
		0, 15.0, 200, 30, 0.1, 20.0, 2.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PLIN, 0, 50,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin2, contrailEnd2
	};
	contrail2 = AddExhaustStream(th_main, _V(0, 0.3, -30), &contrail_second);

	AtlasIce = AddMesh(atlasIce);
	AtlasIceStatus = ICE3;
	if (!conceptVersion) CoreAdapter = AddMesh(coreAdapter, &ATLAS_ADAPTER_OFFSET);

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


	if (boosterAttached)
	{
		CoreBooster = AddMesh(coreBooster, &ATLAS_BOOSTER_OFFSET);
		SetEmptyMass(CORE_DRY_MASS + BOOSTER_DRY_MASS);
	}
	else
	{
		DelThruster(th_booster[0]);
		DelThruster(th_booster[1]);
		DelExhaustStream(turbineExhaustContrail);
		DelExhaust(exBooster[0]);
		DelExhaust(exBooster[1]);
		thCluster[0] = th_main;
		thCluster[1] = th_vernier[0];
		thCluster[2] = th_vernier[1];
		thCluster[3] = NULL;
		thCluster[4] = NULL;
		CreateThrusterGroup(thCluster, 3, THGROUP_MAIN);
	}

	th_main_vent = CreateThruster(CORE_EXHAUST_POS, CORE_EXHAUST_DIR, 33.0, atlas_propellant, 1.0, 1.0);

	PARTICLESTREAMSPEC fuel_venting_spec = { // stolen/borrowed from NASSP (Orbitersdk/samples/ProjectApollo/src_saturn/sivb.cpp)
	0,		// flag
	0.8,	// size
	20,		// rate
	1,	    // velocity
	0.5,    // velocity distribution
	20,		// lifetime
	0.15,	// growthrate
	0.5,    // atmslowdown 
	PARTICLESTREAMSPEC::DIFFUSE,
	PARTICLESTREAMSPEC::LVL_FLAT, 0.02, 0.02,
	PARTICLESTREAMSPEC::ATM_FLAT, 1.0, 1.0
	};

	AddExhaustStream(th_main_vent, &fuel_venting_spec);
}

void ProjectMercury::clbkPreStep(double simt, double simdt, double mjd)
{
	if (selfDestruct && simt - creationTime > 3.2) // delete
	{
		oapiWriteLog("It has been a privelige exploding with you.");
		oapiDeleteVessel(GetHandle());
	}
	else if (selfDestruct && simt - creationTime > 3.0) // allow astronaut to escape
	{
		explosionLevel = 1.0;
		// Maybe add other damage models (for when the stage reenters the atmosphere)
	}

	if (GetAtmDensity() < 1e-6 && AtlasIceStatus == ICE1)
	{
		DelMesh(AtlasIce);
		DelMesh(AtlasIce2);
		DelMesh(AtlasIce3);
		Atlas = AddMesh(atlas);
		AtlasIceStatus = ICE0;
	}
	else if (GetAtmDensity() < 1e-4 && AtlasIceStatus == ICE2)
	{
		DelMesh(Atlas);
		DelMesh(AtlasIce);
		DelMesh(AtlasIce2);
		AtlasIce3 = AddMesh(atlasIce3);
		AtlasIceStatus = ICE1;
	}
	else if (GetAtmDensity() < 1e-2 && AtlasIceStatus == ICE3)
	{
		DelMesh(Atlas);
		DelMesh(AtlasIce);
		DelMesh(AtlasIce3);
		AtlasIce2 = AddMesh(atlasIce2);
		AtlasIceStatus = ICE2;
	}
}

void ProjectMercury::clbkPostStep(double simt, double simdt, double mjd)
{
	turbineContrailLevel = GetThrusterLevel(th_main);

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

	// Secondary Atlas contrail
	if (GetAtmDensity() < contrailEnd2)
	{
		DelExhaustStream(contrail2);

		contrail2Active = false;
	}
	else if (!contrail2Active) // If in dense atmosphere and contrail has not been activated
	{
		contrail2 = AddExhaustStream(th_main, _V(0, 0.3, -20), &contrail_second);
		contrail2Active = true;
	}

	if (frameNr < 2 && !selfDestruct) // after first couble of frames, so that Mercury has time to get into new condition
	{
		frameNr += 1;

		// Find out if Mercury is in abort mode

		char* name = GetName(); // Our full name
		char parentName[256];
		sprintf(parentName, "%.*s", strlen(name) - 11, name); // " Atlas core" has length 11

		double MERCURY_LENGTH_CAPSULE = 2.3042;
		double MERCURY_LENGTH_ABORT = 5.1604;

		OBJHANDLE parent = oapiGetVesselByName(parentName);
		if (parent != NULL)
		{
			if (oapiGetSize(parent) == MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT) // in abort mode
			{
				oapiWriteLog("Atlas abort condition. Self-destruct!");
				creationTime = simt;
				selfDestruct = true;
			}
			else
			{
				oapiWriteLog("No abort. Regular staging.");
			}
		}
		else
		{
			oapiWriteLog("No Atlas parent found");
		}
	}

	if (turbineContrailLevel == 0.0) // engines off, then vent
	{
		SetThrusterLevel(th_main_vent, 0.1);
	}
}

void ProjectMercury::clbkPostCreation()
{
	// char* name = GetName(); // Our full name
	// char parentName[256];
	// sprintf(parentName, "%.*s", strlen(name) - 11, name); // " Atlas core" has length 11

	if (GetThrusterLevel(th_main) == 1.0)
	{
		oapiWriteLog("Atlas abort condition. Self-destruct!");
		creationTime = oapiGetSimTime();
		selfDestruct = true;
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
