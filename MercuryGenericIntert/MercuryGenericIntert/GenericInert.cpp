// ==============================================================
// ORBITER vessel module: Redstone
// ==============================================================

#define STRICT
#define ORBITER_MODULE
#define VESSELVER VESSEL3

#include "orbitersdk.h"

class ProjectMercury : public VESSELVER
{
public:
	ProjectMercury(OBJHANDLE hVessel, int flightmodel);
	~ProjectMercury();
	void clbkSetClassCaps(FILEHANDLE cfg);
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
private:
	MESHHANDLE mesh;
	UINT Mesh;

	int TextX0, secondColumnHUDx, LineSpacing, TextY0;
};

#include "..\..\FunctionsForOrbiter2010.h"

// ==============================================================
// API interface
// ==============================================================




ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel) : VESSELVER(hVessel, flightmodel)
{
}

ProjectMercury::~ProjectMercury()
{
}

void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	double mass, size;
	int objectType;
	oapiReadItem_int(cfg, "ObjectType", objectType);

	switch (objectType)
	{
	case 1:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover1");
		mass = 10.0;
		size = 1.0;
		break;
	case 2:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover2");
		mass = 10.0;
		size = 1.0;
		break;
	case 3:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover3");
		mass = 10.0;
		size = 1.0;
		break;
	case 4:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring1");
		mass = 20.0;
		size = 1.0;
		break;
	case 5:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring2");
		mass = 20.0;
		size = 1.0;
		break;
	case 6:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring3");
		mass = 20.0;
		size = 1.0;
		break;
	case 7:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_droguecover");
		mass = 2.0;
		size = 0.3;
		break;
	case 8:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_retro");
		mass = 100.0;
		size = 1.0;
		break;
	case 9:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover1");
		mass = 1.0;
		size = 0.3;
		break;
	case 10:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover2");
		mass = 1.0;
		size = 0.3;
		break;
	case 11:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover3");
		mass = 1.0;
		size = 0.3;
		break;
	case 12:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_strap1");
		mass = 4.0;
		size = 1.0;
		break;
	case 13:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_strap2");
		mass = 4.0;
		size = 1.0;
		break;
	case 14:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\merc_strap3");
		mass = 4.0;
		size = 1.0;
		break;
	case 15:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_fairing_1_trans");
		mass = 20.0;
		size = 3.0;
		break;
	case 16:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_fairing_2_trans");
		mass = 20.0;
		size = 3.0;
		break;
	case 17:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\ManouverConcept\\posigrade");
		mass = 167.4;
		size = 2.0;
		break;
	case 18:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\ManouverConcept\\concept_retrograde_caps1_trans");
		mass = 2.0;
		size = 0.5;
		break;
	case 19:
		mesh = oapiLoadMeshGlobal("ProjectMercury\\ManouverConcept\\concept_retrograde_caps2_trans");
		mass = 2.0;
		size = 0.5;
		break;
	default:
		char cbuf[256];
		sprintf(cbuf, "ProjectMercuryInert could not read type for %s", GetName());
		oapiWriteLog(cbuf);
		mass = 1.0;
		size = 1.0;
	}

	const double depression = 0.1;
	const double stiffness = abs(-mass * G / (3 * depression)); // abs for sanity check, as I have a tendency to forget signs
	const double damping = 0.9 * 2 * sqrt(mass * stiffness);
	const VECTOR3 TOUCH0 = _V(0.0, -size / 2.0, -depression);
	const VECTOR3 TOUCH1 = _V(-size * sqrt(0.5), size * sqrt(0.5), -depression);
	const VECTOR3 TOUCH2 = _V(size * sqrt(0.5), size * sqrt(0.5), -depression);
	const VECTOR3 TOUCH3 = _V(0.0, 0.0, size);

	SetSize(size / 2.0);
	VersionDependentTouchdown(TOUCH0, TOUCH1, TOUCH2, TOUCH3, stiffness, damping, 1e1);
	SetEmptyMass(mass);
	SetPMI(_V(1.36, 1.37, 1.04));
	SetCrossSections(_V(size * size / 4 * PI, size * size / 4 * PI, size * size / 4 * PI));
	SetCW(1.0, 1.0, 1.0, 1.0);
	SetRotDrag(_V(0.7, 0.7, 1.2));

	Mesh = AddMesh(mesh);
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