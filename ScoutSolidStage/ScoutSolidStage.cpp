// ==============================================================
//				Source file for Mercury Scout Stages.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2020
// 
// Used to define stages for Scout, all in one file
// 
// Thanks to Ricardo Nunes ('4throck') for data.
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"



// Dimensions
const double STAGE1_LENGTH = 9.120;
const double STAGE2_LENGTH = 6.040;
const double STAGE3_LENGTH = 3.380;

const double STAGE1_DIAMETER = 1.010;
const double STAGE2_DIAMETER = 0.790;
const double STAGE3_DIAMETER = 0.780;

// Positions
const VECTOR3 STAGE1_OFFSET = _V(0.0, 0.0, 0.0); // Translated z=-5.319541
const VECTOR3 STAGE2_OFFSET = STAGE1_OFFSET + _V(0.0, 0.0, 9.412 + 3.66705 - 5.319541); // Translated z=-3.66705
const VECTOR3 STAGE3_OFFSET = STAGE1_OFFSET + _V(0.0, 0.0, 15.712 + 1.5266 - 5.319541); // Translated z=-1.5266

const VECTOR3 STAGE1_ENGINE_POS = STAGE1_OFFSET + _V(0.0, 0.0, -STAGE1_LENGTH / 2.0); // Fix this later! Debug
const VECTOR3 STAGE2_ENGINE_POS = STAGE2_OFFSET + _V(0.0, 0.0, -STAGE2_LENGTH / 2.0); // Fix this later! Debug
const VECTOR3 STAGE3_ENGINE_POS = STAGE3_OFFSET + _V(0.0, 0.0, -STAGE3_LENGTH / 2.0); // Fix this later! Debug

const VECTOR3 STAGE1_ENGINE_DIR = _V(0.0, 0.0, 1.0);
const VECTOR3 STAGE2_ENGINE_DIR = _V(0.0, 0.0, 1.0);
const VECTOR3 STAGE3_ENGINE_DIR = _V(0.0, 0.0, 1.0);

// Masses
const double STAGE1_DRY_MASS = 1900.0;
const double STAGE2_DRY_MASS = 695.0;
const double STAGE3_DRY_MASS = 294.0;
const double STAGE4_DRY_MASS = 30.0;

const double STAGE1_FUEL_MASS = 8805.0;
const double STAGE2_FUEL_MASS = 3729.0;
const double STAGE3_FUEL_MASS = 1000.0;

const double STAGE1_TOTAL_MASS = STAGE1_DRY_MASS + STAGE1_FUEL_MASS;
const double STAGE2_TOTAL_MASS = STAGE2_DRY_MASS + STAGE2_FUEL_MASS;
const double STAGE3_TOTAL_MASS = STAGE3_DRY_MASS + STAGE3_FUEL_MASS;

// Engine parameters
const double STAGE1_THRUST = 470934.0;
const double STAGE2_THRUST = 258915.0;
const double STAGE3_THRUST = 60497.0;

const double STAGE1_BURNTIME = 40.0; // Value from 4th rock. 19620003288 page 43 says 44.2
const double STAGE2_BURNTIME = 37.0; // Value from 4th rock. 19620003288 page 43 says 40.0
const double STAGE3_BURNTIME = 39.0; // Value from 4th rock. 19620003288 page 43 says 40.0

const double STAGE1_ISP = STAGE1_BURNTIME * STAGE1_THRUST / STAGE1_FUEL_MASS; // S95 document says 220 s, while this is 218 s
const double STAGE2_ISP = STAGE2_BURNTIME * STAGE2_THRUST / STAGE2_FUEL_MASS; // S95 document says 273 s, while this is 262 s
const double STAGE3_ISP = STAGE3_BURNTIME * STAGE3_THRUST / STAGE3_FUEL_MASS; // S95 document says 255 s, while this is 241 s


class ProjectMercury : public VESSEL3
{
public:
	ProjectMercury(OBJHANDLE hVessel, int flightmodel);
	~ProjectMercury();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPostCreation(void);
	void clbkPreStep(double simt, double simdt, double mjd);
	// int clbkConsumeBufferedKey(DWORD key, bool down, char* kstate);

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

	VECTOR3 FlipX(VECTOR3 vIn);
	VECTOR3 FlipY(VECTOR3 vIn);
	VECTOR3 SwapXY(VECTOR3 vIn);
	double OrbitArea(double angle, double ecc);
	double OrbitalFrameSlipAngle(VECTOR3 pos, VECTOR3 vel);
	double OrbitalFrameSlipAngle2(VECTOR3 pos, VECTOR3 vel);
	void AimEulerAngle(double pitch, double yaw);
	double GenerateRandom01(void);
	double GenerateRandomNorm(double a1, double a2);
	double GenerateRandomAngleNorm(double a1, double a2);
	double NormAngleDeg(double ang);
	bool InRadioContact(OBJHANDLE planet);
	void DeleteRogueVessels(void);
	double MnA2TrA(double MnA, double Ecc);
	double TrA2MnA(double TrA, double Ecc);
	double EccentricAnomaly(double ecc, double TrA);
	void myStrncpy(char* writeTo, const char* readFrom, int len);
private:
	MESHHANDLE mesh = NULL;
	UINT Mesh = NULL;
	enum failure { NONE, RETROCALCOFF, LASTENTRY } FailureMode;
	double launchTime;
	double timeOfError;
	char contactBase[100];
	int stuffCreated;
	OBJHANDLE createdVessel[50];
	bool createdAbove50km[50];
};

#include "..\FunctionsForOrbiter2010.h"
#include "..\ProjectMercuryGeneric.h"

ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel) : VESSEL3(hVessel, flightmodel)
{
}

ProjectMercury::~ProjectMercury()
{
}

void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	double emptyMass, length, fuelMass;
	int stageNr;
	PROPELLANT_HANDLE prp;
	THRUSTER_HANDLE th;
	oapiReadItem_int(cfg, "Stage", stageNr);
	oapiWriteLogV("Creating Scout stage nr. %i", stageNr);

	switch (stageNr)
	{
	case 1:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_stage1_trans_no");
		emptyMass = STAGE1_DRY_MASS;
		length = STAGE1_LENGTH;
		fuelMass = STAGE1_FUEL_MASS;
		prp = CreatePropellantResource(fuelMass);
		th = CreateThruster(STAGE1_ENGINE_POS, STAGE1_ENGINE_DIR, STAGE1_THRUST, prp, STAGE1_ISP);
		break;
	case 2:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_stage2_trans_no");
		emptyMass = STAGE2_DRY_MASS;
		length = STAGE2_LENGTH;
		fuelMass = STAGE2_FUEL_MASS;
		prp = CreatePropellantResource(fuelMass);
		th = CreateThruster(STAGE2_ENGINE_POS, STAGE2_ENGINE_DIR, STAGE2_THRUST, prp, STAGE2_ISP);

		AddMesh(oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_skirt1_trans"));
		break;
	case 3:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_stage3_trans_no");
		emptyMass = STAGE3_DRY_MASS;
		length = STAGE3_LENGTH;
		fuelMass = STAGE3_FUEL_MASS;
		prp = CreatePropellantResource(fuelMass);
		th = CreateThruster(STAGE3_ENGINE_POS, STAGE3_ENGINE_DIR, STAGE3_THRUST, prp, STAGE3_ISP);

		AddMesh(oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_skirt2_trans"));
		AddMesh(oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_skirt3_trans"));
		break;
	default:
		char cbuf[256];
		sprintf(cbuf, "ScoutStageGeneric could not read type for %s", GetName());
		oapiWriteLog(cbuf);
		emptyMass = 1.0;
		length = 1.0;
		fuelMass = 1.0;
		prp = CreatePropellantResource(fuelMass);
		th = CreateThruster(_V(0, 0, -1), _V(0, 0, 1), 1.0, prp, 1.0);
		break;
	}

	CreateThrusterGroup(&th, 1, THGROUP_MAIN);
	oapiWriteLogV("fuelMass: %.2f", fuelMass);

	//static const DWORD tchdnNum = 4;
	const double depression = 0.1;
	const double stiffness = abs(-emptyMass * G / (3 * depression)); // abs for sanity check, as I have a tendency to forget signs
	const double damping = 0.9 * 2 * sqrt(emptyMass * stiffness);
	const VECTOR3 TOUCH0 = _V(0.0, -length / 2.0, -depression);
	const VECTOR3 TOUCH1 = _V(-length * sqrt(0.5), length * sqrt(0.5), -depression);
	const VECTOR3 TOUCH2 = _V(length * sqrt(0.5), length * sqrt(0.5), -depression);
	const VECTOR3 TOUCH3 = _V(0.0, 0.0, length);
	//static TOUCHDOWNVTX tchdwn[tchdnNum] = {
	//	// pos, stiff, damping, mu, mu long
	//	{TOUCH0, stiffness, damping, 1e1},
	//	{TOUCH1, stiffness, damping, 1e1},
	//	{TOUCH2, stiffness, damping, 1e1},
	//	{TOUCH3, stiffness, damping, 1e1},
	//};
	//SetTouchdownPoints(tchdwn, tchdnNum);
	VersionDependentTouchdown(TOUCH0, TOUCH1, TOUCH2, TOUCH3, stiffness, damping, 1e1);
	//SetTouchdownPoints(TOUCH0, TOUCH1, TOUCH2);

	SetSize(length / 2.0);
	SetEmptyMass(emptyMass);
	SetPMI(_V(1.36, 1.37, 1.04));
	SetCrossSections(_V(length * length / 4 * PI, length * length / 4 * PI, length * length / 4 * PI));
	SetCW(1.0, 1.0, 1.0, 1.0);
	SetRotDrag(_V(0.7, 0.7, 1.2));

	Mesh = AddMesh(mesh);
}

void ProjectMercury::clbkPostCreation(void)
{
}

void ProjectMercury::clbkPreStep(double simt, double simdt, double mjd)
{
}

// --------------------------------------------------------------
// Vessel initialisation
// --------------------------------------------------------------
DLLCLBK VESSEL* ovcInit(OBJHANDLE hvessel, int flightmodel)
{
	return new ProjectMercury(hvessel, flightmodel);
}

// --------------------------------------------------------------
// Vessel cleanup
// --------------------------------------------------------------
DLLCLBK void ovcExit(VESSEL* vessel)
{
	//oapiCloseFile(jetVaneLogFile, FILE_OUT);

	if (vessel) delete (ProjectMercury*)vessel;
}
