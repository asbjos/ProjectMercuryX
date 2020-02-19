// ==============================================================
// ORBITER vessel module: Redstone
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"

const VECTOR3 OFS_AB = { 0.0, 0.0, -1.9 };
const double THRUST_VAC_ESC = 231308.0; // Totalling 52000 lb (or 58500 lb), burn time 0.78
const double FUEL_MASS_ESC = 131.77; // 290.5 lb
const double ISP_VAC_ESC = 0.78 * THRUST_VAC_ESC / FUEL_MASS_ESC; // Calculated to give 0.78 sec burn time
const double THRUST_VAC_JET = 3492.0; // Tower jettison thruster, burntime 1.3 seconds. 785 lb force
const double FUEL_MASS_JET = 2.31; // 5.1 lb
const double ISP_VAC_JET = 1.3 * THRUST_VAC_JET / FUEL_MASS_JET; // Calculated to give 1.3 sec burn time
const double MERCURY_LENGTH_ABORT = 5.1604;
const double ABORT_DRY_MASS = 460.4 - FUEL_MASS_ESC - FUEL_MASS_JET;

static const DWORD touchdownPointsNumb = 7;
static TOUCHDOWNVTX touchdownPoint[touchdownPointsNumb] = {
	// pos, stiff, damping, mu, mu long
	{_V(-0.2, -0.2, -MERCURY_LENGTH_ABORT / 2.0), 2e3, 1e3, 0.3},
	{_V(0.2, -0.2,  -MERCURY_LENGTH_ABORT / 2.0), 2e3, 1e3, 0.3},
	{_V(0.2, 0.2, -MERCURY_LENGTH_ABORT / 2.0), 2e3, 1e3, 0.3},
	{_V(0.01, 0.01, MERCURY_LENGTH_ABORT / 2.0), 2e3, 1e3, 0.3},
	{_V(0.01,.300,-.018702), 2e3, 1e3, 0.3},
	{_V(-0.25980762, -.150, -.018702), 2e3, 1e3, 0.3},
	{_V(0.25980762, -.150, -.018702), 2e3, 1e3, 0.3}
};

PARTICLESTREAMSPEC exhaust_main;
PARTICLESTREAMSPEC exhaust_escape;

class MercuryAbort: public VESSEL4
{
public:
	MercuryAbort(OBJHANDLE hVessel, int flightmodel);
	~MercuryAbort();
	void clbkSetClassCaps(FILEHANDLE cfg);
	void clbkPostCreation();
	// int clbkConsumeBufferedKey(DWORD key, bool down, char* kstate);
private:
	bool bManualSeparate = false;
	double stage_sep;
	UINT stage = 0;
	MESHHANDLE tower;
	UINT Tower;
	
	PROPELLANT_HANDLE main_tank;
	THRUSTER_HANDLE main_engine;
	THGROUP_HANDLE main_engine_group;
	PSTREAM_HANDLE contrail_main;

	PROPELLANT_HANDLE escape_tank;
	THRUSTER_HANDLE escape_engine;
	THGROUP_HANDLE escape_engine_group;
	PSTREAM_HANDLE contrail_escape;
};


// ==============================================================
// API interface
// ==============================================================




MercuryAbort::MercuryAbort(OBJHANDLE hVessel, int flightmodel) : VESSEL4(hVessel, flightmodel)
{
	tower = oapiLoadMeshGlobal("ProjectMercury\\merc_tower");
}

MercuryAbort::~MercuryAbort()
{
}

void MercuryAbort::clbkSetClassCaps(FILEHANDLE cfg)
{
	// SURFHANDLE tex = oapiRegisterExhaustTexture("Exhaust2");
	// ==============================================================
	SetSize(MERCURY_LENGTH_ABORT / 2.0);
	SetTouchdownPoints(touchdownPoint, touchdownPointsNumb);
	SetEmptyMass(ABORT_DRY_MASS);
	// ==============================================================
	// This is from the altanis launch config and is what makes the V-2 launch vertically
	// The Z values you use to set the rocket up into the air and align with the height of 
	// your launch platform. In this case the Pad 39A.
	//	vessel->SetTouchdownPoints (_V(0,-1.0,OFS_AB.z-26.0), _V(-.7,.7,OFS_AB.z-26.0), _V(.7,.7,OFS_AB.z-26.0));
	// ==============================================================
	// This from running the mesh through the shipedit program
	SetPMI(_V(1.36, 1.37, 1.04));
	SetCrossSections(_V(0.81, 0.93, 0.37));
	// ==============================================================
	// This is based off of the SRB and Main Shuttle Tank as 
	// they are the nearest thing to V-2 that I have stats for.
	SetCW(0.1, 0.3, 1.4, 1.4);
	SetRotDrag(_V(0.7, 0.7, 1.2));
	// SetPitchMomentScale(0);
	// SetBankMomentScale(0);
	// SetLiftCoeffFunc(0);
	// ==============================================================
	// ClearMeshes(true);
	// ClearExhaustRefs();
	// ClearAttExhaustRefs();

	Tower = AddMesh(tower);

	VECTOR3 m_exhaust_pos[3];
	VECTOR3 m_exhaust_ref[3];

	VECTOR3 esc_exhaust_pos[3];
	VECTOR3 esc_exhaust_ref[3];

	main_tank = CreatePropellantResource(FUEL_MASS_JET);
	escape_tank = CreatePropellantResource(FUEL_MASS_ESC);
	main_engine = CreateThruster(_V(0.0, 0.78*0.0254, 0.0), _V(0.0, 0.0, 1.0), THRUST_VAC_JET, main_tank, ISP_VAC_JET, ISP_VAC_JET);
	escape_engine = CreateThruster(_V(0.0, 0.78*0.0254, -0.056106) / sqrt(0.04 * 0.04 + 0.056106 * 0.056106) * 0.78 * 0.0254, _V(0.0, 0.0, 1.0), THRUST_VAC_ESC, escape_tank, ISP_VAC_ESC, ISP_VAC_ESC); // create position offset of 0.78 inches (19670022198 page 6)
	m_exhaust_pos[0] = _V(0, 0.04, 0.0);
	m_exhaust_ref[0] = _V(.433, .25, .866);
	esc_exhaust_pos[0] = _V(0.0,.300,-.018702);
	esc_exhaust_ref[0] = _V(0,-.300,.946);
	// main_engine[0] = CreateThruster(m_exhaust_pos, m_exhaust_ref, THRUST_VAC_JET, main_tank, ISP_VAC_JET, ISP_VAC_JET);
	// escape_engine[0] = CreateThruster(esc_exhaust_pos, esc_exhaust_ref, THRUST_VAC_ESC, escape_tank, ISP_VAC_ESC, ISP_VAC_ESC);
	m_exhaust_pos[1] = _V(0, 0.04, 0.0);
	m_exhaust_ref[1] = _V(0, -.500, .866);
	esc_exhaust_pos[1] = _V(-0.25980762,-.150,-.018702);
	esc_exhaust_ref[1] = _V(0.25980762,.150,.946);
	// main_engine[1] = CreateThruster(m_exhaust_pos, m_exhaust_ref, THRUST_VAC_JET, main_tank, ISP_VAC_JET, ISP_VAC_JET);
	// escape_engine[1] = CreateThruster(esc_exhaust_pos, esc_exhaust_ref, THRUST_VAC_ESC, escape_tank, ISP_VAC_ESC, ISP_VAC_ESC);
	m_exhaust_pos[2] = _V(0, 0.04, 0.0);
	m_exhaust_ref[2] = _V(-.433, .25, .866);
	esc_exhaust_pos[2] = _V(0.25980762,-.150,-.018702);
	esc_exhaust_ref[2] = _V(-0.25980762,.150,.946);
	// main_engine[2] = CreateThruster(m_exhaust_pos, m_exhaust_ref, THRUST_VAC_JET, main_tank, ISP_VAC_JET, ISP_VAC_JET);
	// escape_engine[2] = CreateThruster(esc_exhaust_pos, esc_exhaust_ref, THRUST_VAC_ESC, escape_tank, ISP_VAC_ESC, ISP_VAC_ESC);

	main_engine_group = CreateThrusterGroup(&main_engine, 1, THGROUP_MAIN);
	escape_engine_group = CreateThrusterGroup(&escape_engine, 1, THGROUP_USER);

	SURFHANDLE tex = oapiRegisterExhaustTexture("Exhaust2");
	EXHAUSTSPEC ex_main[3] = {
		{main_engine, NULL, &m_exhaust_pos[0], &m_exhaust_ref[0], 1.0, 0.05, 0.0, 0.0, tex},
		{main_engine, NULL, &m_exhaust_pos[1], &m_exhaust_ref[1], 1.0, 0.05, 0.0, 0.0, tex},
		{main_engine, NULL, &m_exhaust_pos[2], &m_exhaust_ref[2], 1.0, 0.05, 0.0, 0.0, tex}
	};
	EXHAUSTSPEC ex_escape[3] = {
		{main_engine, NULL, &esc_exhaust_pos[0], &esc_exhaust_ref[0], 1.0, 0.05, 0.0, 0.0, tex},
		{main_engine, NULL, &esc_exhaust_pos[1], &esc_exhaust_ref[1], 1.0, 0.05, 0.0, 0.0, tex},
		{main_engine, NULL, &esc_exhaust_pos[2], &esc_exhaust_ref[2], 1.0, 0.05, 0.0, 0.0, tex}
	};

	AddExhaust(ex_main);
	AddExhaust(ex_escape);

	exhaust_main = {
		0, 0.1, 120, 100, 0.5, 2, 10, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1
	};
	exhaust_escape = {
		0, 0.5, 120, 100, 0.5, 2, 10, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1
	};
	contrail_main = AddExhaustStream(main_engine, &exhaust_main);
	contrail_escape = AddExhaustStream(escape_engine, &exhaust_escape);
}

void MercuryAbort::clbkPostCreation()
{
	// oapiWriteLogV("Escape fuel %.3f, jettison fuel %.3f", GetPropellantMass(escape_tank) / GetPropellantMaxMass(escape_tank), GetPropellantMass(main_tank) / GetPropellantMaxMass(main_tank));

	if (GetPropellantMass(escape_tank) == 0.0)
	{
		oapiWriteLog("Firing tower jett (we have aborted)");
		SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
	}
	else
	{
		oapiWriteLog("Firing tower escape (nominal flight)");
		SetThrusterGroupLevel(escape_engine_group, 1.0);
		SetThrusterGroupLevel(THGROUP_MAIN, 1.0); // https://web.archive.org/web/20140222235747/http://www.mercury-redstone3.com/?page_id=54 somewhere on this page says that both are fired for regular tower sep
	}
}

// Initialisation
DLLCLBK VESSEL* ovcInit(OBJHANDLE hvessel, int flightmodel)
{
	return new MercuryAbort(hvessel, flightmodel);
}

// Cleanup
DLLCLBK void ovcExit(VESSEL* vessel)
{
	if (vessel) delete (MercuryAbort*)vessel;
}