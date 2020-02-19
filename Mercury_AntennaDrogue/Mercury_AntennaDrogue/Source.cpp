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
static TOUCHDOWNVTX touchdownPoint[touchdownPointsNumb] = {
	// pos, stiff, damping, mu, mu long
	{_V(-0.2, -0.2, -MERCURY_LENGTH_ANTHOUSE / 2.0), 2e3, 1e3, 0.3},
	{_V(0.2, -0.2,  -MERCURY_LENGTH_ANTHOUSE / 2.0), 2e3, 1e3, 0.3},
	{_V(0.2, 0.2, -MERCURY_LENGTH_ANTHOUSE / 2.0), 2e3, 1e3, 0.3},
	{_V(0.01, 0.01, MERCURY_CHUTE_DROGUE / 2.0), 2e3, 1e3, 0.3},
};

class Mercury_Antenna : public VESSEL4
{
public:
	Mercury_Antenna(OBJHANDLE hVessel, int flightmodel);
	~Mercury_Antenna();
	void clbkSetClassCaps(FILEHANDLE cfg);

private:
	static void vlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	static void hlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd);
	MESHHANDLE anthouse, drogue;
	UINT Anthouse, Drogue;
	const double drogueLevel = 1.0;
};


// ==============================================================
// API interface
// ==============================================================




Mercury_Antenna::Mercury_Antenna(OBJHANDLE hVessel, int flightmodel) : VESSEL4(hVessel, flightmodel)
{
	drogue = oapiLoadMeshGlobal("ProjectMercury\\merc_drogue");
	anthouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
}

Mercury_Antenna::~Mercury_Antenna()
{
}

void Mercury_Antenna::clbkSetClassCaps(FILEHANDLE cfg)
{
	SetCrossSections(_V(2.8, 2.8, MERCURY_CHUTE_DROGUE * 8));
	SetCW(1.5, 1.5, 0.3, 0.3);
	SetSize(9.0);
	SetEmptyMass(46.0); // MA-7 weight 2663.36 lb at reentry, 2557.70 lb at main chute deploy -> 47.93 kg (not considering attitude fuel)
	SetRotDrag(_V(0.7, 0.7, .1));
	SetPMI(_V(0.66, 0.66, .33));
	SetTouchdownPoints(touchdownPoint, touchdownPointsNumb);
	AddMesh(anthouse, &MERCURY_OFS_ANTHOUSE);
	AddMesh(drogue, &MERCURY_OFS_DROGUE);

	double diameter = 0.7;
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 0.1), vlift, NULL, diameter, diameter * diameter * PI / 4.0, 1.0); //
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 0.1), hlift, NULL, diameter, diameter * diameter * PI / 4.0, 1.0); // spherical symmetric

	CreateVariableDragElement(&drogueLevel, 2.365, CHUTE_ATTACK_POINT);

	//drogueLevel = 1.0;
}

// Initialisation
DLLCLBK VESSEL* ovcInit(OBJHANDLE hvessel, int flightmodel)
{
	return new Mercury_Antenna(hvessel, flightmodel);
}

// Cleanup
DLLCLBK void ovcExit(VESSEL* vessel)
{
	if (vessel) delete (Mercury_Antenna*)vessel;
}

void Mercury_Antenna::vlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
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

void Mercury_Antenna::hlift(VESSEL* v, double beta, double M, double Re, void* context, double* cl, double* cm, double* cd)
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
