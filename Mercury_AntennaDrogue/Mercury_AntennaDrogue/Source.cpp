// ==============================================================
// ORBITER vessel module: Redstone
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"

const double MERCURY_LENGTH_ANTHOUSE = 0.687;
const double  MERCURY_CHUTE_DROGUE = 3.0;
const VECTOR3 MERCURY_OFS_ANTHOUSE = { 0.0, 0.0, MERCURY_LENGTH_ANTHOUSE / 2 };
const VECTOR3 MERCURY_OFS_DROGUE = { -0.12, 0.08, -.2 + MERCURY_LENGTH_ANTHOUSE / 2 };
const VECTOR3 CHUTE_ATTACK_POINT = MERCURY_OFS_DROGUE + _V(0, 0, 3.0); // guesstimate


static const DWORD touchdownPointsNumb = 4;
const VECTOR3 TOUCHDOWN_POINT0 = _V(-0.2, -0.2, -MERCURY_LENGTH_ANTHOUSE / 2.0);
const VECTOR3 TOUCHDOWN_POINT1 = _V(0.2, -0.2, -MERCURY_LENGTH_ANTHOUSE / 2.0);
const VECTOR3 TOUCHDOWN_POINT2 = _V(0.2, 0.2, -MERCURY_LENGTH_ANTHOUSE / 2.0);
const VECTOR3 TOUCHDOWN_POINT3 = _V(0.01, 0.01, MERCURY_CHUTE_DROGUE / 2.0);

class ProjectMercury : public VESSEL3
{
public:
	ProjectMercury(OBJHANDLE hVessel, int flightmodel);
	~ProjectMercury();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPostCreation(void);
	void DefineDestabiliserAnim(void);

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
	static void vlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	MESHHANDLE anthouse, drogue;
	UINT Anthouse, Drogue;
	UINT DestabiliserDeployAnim;
	/*const*/ double drogueLevel = 1.0; // debug, orbiter2010 orbiter2016
};

#include "..\..\FunctionsForOrbiter2010.h"

// ==============================================================
// API interface
// ==============================================================


ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel) : VESSEL3(hVessel, flightmodel)
{
	drogue = oapiLoadMeshGlobal("ProjectMercury\\merc_drogue");
}

ProjectMercury::~ProjectMercury()
{
}

void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	int CapsuleType;
	if (!oapiReadItem_int(cfg, "CapsuleType", CapsuleType))
	{
		anthouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
	}
	else // Found the value
	{
		switch (CapsuleType)
		{
		case 1: // Big Joe
			anthouse = oapiLoadMeshGlobal("ProjectMercury\\big_joe_antenna");
			oapiWriteLog("Antenna type Big Joe.");
			break;
		case 2: // Little Joe
			anthouse = oapiLoadMeshGlobal("ProjectMercury\\little_joe_antenna");
			oapiWriteLog("Antenna type Little Joe.");
			break;
		case 3: // Booster Development
			anthouse = oapiLoadMeshGlobal("ProjectMercury\\MR-BD");
			oapiWriteLog("Antenna type Booster Development.");
			break;
		default:
			anthouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
			break;
		}
	}

	SetCrossSections(_V(2.8, 2.8, MERCURY_CHUTE_DROGUE * 8));
	SetCW(1.5, 1.5, 0.3, 0.3);
	SetSize(9.0);
	SetEmptyMass(46.0); // MA-7 weight 2663.36 lb at reentry, 2557.70 lb at main chute deploy -> 47.93 kg (not considering attitude fuel)
	SetRotDrag(_V(0.7, 0.7, .1));
	SetPMI(_V(0.66, 0.66, .33));
	VersionDependentTouchdown(TOUCHDOWN_POINT0, TOUCHDOWN_POINT1, TOUCHDOWN_POINT2, TOUCHDOWN_POINT3, 2e3, 1e3, 0.3);
	Anthouse = AddMesh(anthouse, &MERCURY_OFS_ANTHOUSE);
	AddMesh(drogue, &MERCURY_OFS_DROGUE);
	DefineDestabiliserAnim();

	double diameter = 0.7;
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 0.1), vlift, NULL, diameter, diameter * diameter * PI / 4.0, 1.0); //
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 0.1), hlift, NULL, diameter, diameter * diameter * PI / 4.0, 1.0); // spherical symmetric

	CreateVariableDragElement(&drogueLevel, 2.365, CHUTE_ATTACK_POINT);

	//drogueLevel = 1.0;
}

void ProjectMercury::clbkPostCreation(void)
{
	SetAnimation(DestabiliserDeployAnim, 1.0); // destabiliser is always deployed
}

void ProjectMercury::DefineDestabiliserAnim(void)
{
	static UINT destabiliserGroups[1] = { 0 };

	static MGROUP_ROTATE DestabiliserDeploy(
		Anthouse,
		destabiliserGroups, 1,
		_V(0.0, -0.230, 0.265),
		_V(1.0, 0.0, 0.0),
		float(150.0 * RAD)
	);

	DestabiliserDeployAnim = CreateAnimation(0.0);
	AddAnimationComponent(DestabiliserDeployAnim, 0.0, 1.0, &DestabiliserDeploy);
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

void ProjectMercury::vlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	static const double step = RAD * 22.5;
	static const double istep = 1.0 / step;
	static const int nabsc = 17;
	static const double CL[nabsc] = { 0, 0.1, 0.2, 0.1, 0, 0.1, 0.2, 0.1, 0, -0.1, -0.2, -0.1, 0, -0.1, -0.2, -0.1, 0 };

	aoa += PI;
	int idx = max(0, min(15, (int)(aoa * istep)));
	double d = aoa * istep - idx;
	*cl = CL[idx] + (CL[idx + 1] - CL[idx]) * d;
	*cm = 0.0;
	//*cd = 0.55 * (0.11 + oapiGetInducedDrag(*cl, 0.16, 0.2));

	static const  double mach[12] = {
		0.0, 0.50, 0.7, 0.90, 1.00, 1.15, 1.5, 2.0, 3.0, 5.0, 7.0, 9.6
	};
	static const double cdp[12] = { // drag coeff at 0 AoA (tip first) for different mach numbers.
		0.63, 0.64, 0.64, 0.72, 0.92, 0.9, 0.78, 0.66, 0.46, 0.3, 0.23, 0.18
	};

	double aoastep = 30.0 * RAD;
	//beta += PI;
	idx = max(0, min(11, (int)(aoa / aoastep)));
	d = aoa / aoastep - idx;

	int i = 0;
	while (i < 14 && M > mach[i])
	{
		i++;
	}

	if (i == 12)
	{
		*cd = cdp[11];
	}
	else if (i == 0)
	{
		*cd = cdp[0];
	}
	else
	{
		*cd = cdp[i - 1] + (cdp[i] - cdp[i - 1]) * (M - mach[i - 1]) / (mach[i] - mach[i - 1]);
	}

	*cd *= 0.5;
}

void ProjectMercury::hlift(VESSEL* v, double beta, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	static const double step = RAD * 22.5;
	static const double istep = 1.0 / step;
	static const int nabsc = 17;
	static const double CL[nabsc] = { 0, 0.1, 0.2, 0.1, 0, 0.1, 0.2, 0.1, 0, -0.1, -0.2, -0.1, 0, -0.1, -0.2, -0.1, 0 };

	beta += PI;
	int idx = max(0, min(15, (int)(beta * istep)));
	double d = beta * istep - idx;
	*cl = CL[idx] + (CL[idx + 1] - CL[idx]) * d;
	*cm = 0.0;

	static const  double mach[12] = {
		0.0, 0.50, 0.7, 0.90, 1.00, 1.15, 1.5, 2.0, 3.0, 5.0, 7.0, 9.6
	};
	static const double cdp[12] = { // drag coeff at 0 AoA (tip first) for different mach numbers.
		0.63, 0.64, 0.64, 0.72, 0.92, 0.9, 0.78, 0.66, 0.46, 0.3, 0.23, 0.18
	};

	double aoastep = 30.0 * RAD;
	//beta += PI;
	idx = max(0, min(11, (int)(beta / aoastep)));
	d = beta / aoastep - idx;

	int i = 0;
	while (i < 14 && M > mach[i])
	{
		i++;
	}

	if (i == 12)
	{
		*cd = cdp[11];
	}
	else if (i == 0)
	{
		*cd = cdp[0];
	}
	else
	{
		*cd = cdp[i - 1] + (cdp[i] - cdp[i - 1]) * (M - mach[i - 1]) / (mach[i] - mach[i - 1]);
	}

	*cd *= 0.5;
}
