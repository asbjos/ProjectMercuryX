// ==============================================================
// ORBITER vessel module: Redstone
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"

const double LENGTH_CHUTE = 1.0;
const double  MERCURY_CHUTE = 20.0;
const VECTOR3 CHUTE_ATTACK_POINT = _V(0, 0, 3.0); // random number


static const DWORD touchdownPointsNumb = 4;
const double depression = 0.3;
const double stiffness = abs(-47.0 * G / (3 * depression)); // abs for sanity check, as I have a tendency to forget signs
const double damping = 0.3 * 2 * sqrt(47.0 * stiffness);
const VECTOR3 TOUCH0 = _V(0.0, -3.5, -LENGTH_CHUTE / 2.0 + depression);
const VECTOR3 TOUCH1 = _V(-2.5, 2.5, -LENGTH_CHUTE / 2.0 + depression);
const VECTOR3 TOUCH2 = _V(2.5, 2.5, -LENGTH_CHUTE / 2.0 + depression);
const VECTOR3 TOUCH3 = _V(0.0, 0.0, LENGTH_CHUTE / 2.0);

class ProjectMercury : public VESSEL3
{
public:
	ProjectMercury(OBJHANDLE hVessel, int flightmodel);
	~ProjectMercury();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPreStep(double simt, double simdt, double mjd);

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
	MESHHANDLE chute;
	UINT Chute;
	/*const */double chuteLevel = 1.0; // debug orbiter2016 orbiter2010
};

#include "..\..\FunctionsForOrbiter2010.h"

// ==============================================================
// API interface
// ==============================================================




ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel) : VESSEL3(hVessel, flightmodel)
{
	chute = oapiLoadMeshGlobal("ProjectMercury\\merc_chute");
}

ProjectMercury::~ProjectMercury()
{
}

void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	//SetCrossSections(_V(2.8, 2.8, LENGTH_CHUTE * 8));
	//SetCW(1.5, 1.5, 0.3, 0.3);
	SetSize(10.0);
	SetEmptyMass(68.0); // MA-7 weight 2557 lb at main chute deploy, 2407.83 lb at floating -> 68 kg
	VersionDependentTouchdown(TOUCH0, TOUCH1, TOUCH2, TOUCH3, stiffness, damping, 1e1);
	SetTouchdownPoints(TOUCH0, TOUCH1, TOUCH2);
	AddMesh(chute);

	double diameter = 0.7; // need to create airfoil for variabledragelement to work
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 0.1), vlift, NULL, diameter, diameter * diameter * PI / 4.0, 1.0); //
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 0.1), hlift, NULL, diameter, diameter * diameter * PI / 4.0, 1.0); // spherical symmetric

	CreateVariableDragElement(&chuteLevel, 218.0, CHUTE_ATTACK_POINT);
}

void ProjectMercury::clbkPreStep(double simt, double simdt, double mjd)
{
	if (GroundContact())
	{
		oapiWriteLog("Delete chute on contact");
		oapiDeleteVessel(GetHandle());
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
