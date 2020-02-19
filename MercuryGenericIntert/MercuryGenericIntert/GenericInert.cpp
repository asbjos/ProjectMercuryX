// ==============================================================
// ORBITER vessel module: Redstone
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"

class MercuryGeneric : public VESSEL4
{
public:
	MercuryGeneric(OBJHANDLE hVessel, int flightmodel);
	~MercuryGeneric();
	void clbkSetClassCaps(FILEHANDLE cfg);
	// int clbkConsumeBufferedKey(DWORD key, bool down, char* kstate);
private:
	MESHHANDLE mesh;
	UINT Mesh;
};


// ==============================================================
// API interface
// ==============================================================




MercuryGeneric::MercuryGeneric(OBJHANDLE hVessel, int flightmodel) : VESSEL4(hVessel, flightmodel)
{
}

MercuryGeneric::~MercuryGeneric()
{
}

void MercuryGeneric::clbkSetClassCaps(FILEHANDLE cfg)
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
	default:
		char cbuf[256];
		sprintf(cbuf, "MercuryGenericInert could not read type for %s", GetName());
		oapiWriteLog(cbuf);
		mass = 1.0;
		size = 1.0;
	}

	static const DWORD tchdnNum = 4;
	const double depression = 0.1;
	const double stiffness = abs(-mass * G / (3 * depression)); // abs for sanity check, as I have a tendency to forget signs
	const double damping = 0.9 * 2 * sqrt(mass * stiffness);
	static TOUCHDOWNVTX tchdwn[tchdnNum] = {
		// pos, stiff, damping, mu, mu long
		{_V(0.0, -size / 2.0, -depression), stiffness, damping, 1e1},
		{_V(-size * sqrt(0.5), size * sqrt(0.5), -depression), stiffness, damping, 1e1},
		{_V(size * sqrt(0.5), size * sqrt(0.5), -depression), stiffness, damping, 1e1},
		{_V(0.0, 0.0, size), stiffness, damping, 1e1},
	};

	SetSize(size / 2.0);
	SetTouchdownPoints(tchdwn, tchdnNum);
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
	return new MercuryGeneric(hvessel, flightmodel);
}

// Cleanup
DLLCLBK void ovcExit(VESSEL* vessel)
{
	if (vessel) delete (MercuryGeneric*)vessel;
}