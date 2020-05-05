// ==============================================================
// ORBITER vessel module: Redstone
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"
//#include <algorithm>

const double LC_MASS = 2e6; // guesstimate
const double LC_COG = 26.33;
const VECTOR3 ROCKET_POS = _V(0.0, -14.18 + LC_COG, 0.0);
const VECTOR3 ROCKET_DIR = _V(0.0, 0.0, 1.0);
const VECTOR3 PAD_OFFSET = _V(0, 0.0, 0.0);
const VECTOR3 MESH_OFFSET = _V(-30.8, 0.0, 3.6) + _V(0, LC_COG, 0);
const VECTOR3 EXHAUST_POS = _V(0.0, 0.0, 0.0);
const VECTOR3 EXHAUST_DIR = _V(1.0, 0.0, 0.0);
//const VECTOR3 MESH_OFFSET = PAD_OFFSET + _V(0, LC_COG, 0);

// Tower groups, omitting some and including some)
const int groupStart0 = 1;
const int groupEnd0 = 157;
const int groupStart1 = 169;
const int groupEnd1 = 182;
const int groupStart2 = 905;
const int groupEnd2 = 916;
const int groupStart3 = 925;
const int groupEnd3 = 1581;
const int groupStart4 = 1590;
const int groupEnd4 = 1597;
const int numGroups0 = groupEnd0 - groupStart0 + 1;
const int numGroups1 = groupEnd1 - groupStart1 + 1;
const int numGroups2 = groupEnd2 - groupStart2 + 1;
const int numGroups3 = groupEnd3 - groupStart3 + 1;
const int numGroups4 = groupEnd4 - groupStart4 + 1;
const int numGroups = numGroups0 + numGroups1 + numGroups2 +numGroups3 + numGroups4;
const double TOWER_SPEED = 0.5; // m/s
//const VECTOR3 TOWER_MOVE_AWAY = _V(0.0, 0.0, -7e1);
//const double TOWER_DURATION = length(TOWER_MOVE_AWAY) / TOWER_SPEED;
//const VECTOR3 TOWER_MOVE_IN = _V(-34.1, 0.0, 0.0);
const VECTOR3 TOWER_MOVE_IN = _V(-68.2, 0.0, 0.0);
const double TOWER_IN_DURATION = length(TOWER_MOVE_IN) / TOWER_SPEED;

// Camera settings
const int numCamera = 5; // lrgTwrDwn, lrgTwrFar, undrRckt, farAwy, farAwy2
const VECTOR3 CAMERA_LARGE_TOWER_DOWN = _V(65.0, 12.0, -3.1) + MESH_OFFSET; // 0
const VECTOR3 CAMERA_LARGE_TOWER_DOWN_DIR = _V(0, -1, 0);
const VECTOR3 CAMERA_LARGE_TOWER_FAR = _V(60.0, 19.0, -3.1) + MESH_OFFSET; // 1
const VECTOR3 CAMERA_LARGE_TOWER_FAR_DIR = unit(_V(-1, -0.8, 0));
const VECTOR3 CAMERA_UNDER_ROCKET = _V(0.0, ROCKET_POS.y - 8.0, 0.0); // 2
const VECTOR3 CAMERA_UNDER_ROCKET_DIR = _V(0, 1, 0);
const VECTOR3 CAMERA_FAR_AWAY = _V(-36.0, 9.3, -3.1) + MESH_OFFSET; // 3
const VECTOR3 CAMERA_FAR_AWAY_DIR = unit(_V(1, -0.1, 0));
const VECTOR3 CAMERA_FAR_AWAY_2 = _V(3.6, -7.0, 27.6) + MESH_OFFSET; // 4
const VECTOR3 CAMERA_FAR_AWAY_2_DIR = unit(_V(1, -0.1, -1.2));

// Light sources
const VECTOR3 LIGHT1 = _V(56.0, -7.4, 27.3) + MESH_OFFSET;
const VECTOR3 LIGHT1_DIR = unit(_V(-1.0, 0.0, -1.0));
const VECTOR3 LIGHT2 = _V(4.4, -7.4, 26.6) + MESH_OFFSET;
const VECTOR3 LIGHT2_DIR = unit(_V(1.0, 0.0, -1.0));
const VECTOR3 LIGHT3 = _V(56.0, -7.4, -42.0) + MESH_OFFSET;
const VECTOR3 LIGHT3_DIR = unit(_V(-1.0, 0.0, 1.0));
const VECTOR3 LIGHT4 = _V(4.4, -7.4, -42.8) + MESH_OFFSET;
const VECTOR3 LIGHT4_DIR = unit(_V(1.0, 0.0, 1.0));

class ProjectMercury : public VESSEL3
{
public:
	ProjectMercury(OBJHANDLE hVessel, int flightmodel);
	~ProjectMercury();
	void clbkSetClassCaps(FILEHANDLE cfg); // read config for optional attach pos and dir
	void clbkPostCreation(void);
	void clbkPreStep(double simt, double simdt, double mjd);
	void clbkPostStep(double simt, double simdt, double mjd);
	void clbkLoadStateEx(FILEHANDLE scn, void* vs);
	void clbkSaveState(FILEHANDLE scn);
	int clbkConsumeBufferedKey(DWORD key, bool down, char* kstate);
	bool clbkLoadGenericCockpit(void);
	void clbkVisualCreated(VISHANDLE vis, int refcount);
	bool clbkLoadPanel2D(int id, PANELHANDLE hPanel, DWORD viewW, DWORD viewH);
	bool clbkDrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp);
	void clbkRenderHUD(int mode, const HUDPAINTSPEC* hps, SURFHANDLE hTex);

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

	void SwitchCamera(int camera);
	void AttachRocket(double simt, OBJHANDLE closestVessel, VESSEL* v);
	void GetClosestVessel(OBJHANDLE* closestVessel, double* distance);
	void DefineAnimation(void);

	DWORD ScreenWidth, ScreenHeight, ScreenColour;
	int TextX0, TextY0, LineSpacing;
	int secondColumnHUDx = 30; // The x-pos of the second collumn on the HUD
	VECTOR3 rocketPosition, rocketDirection;

private:
	PARTICLESTREAMSPEC exhaust[5];

	double exhaustLevel = 0.0;
	bool launching = false;
	double engineIgnitionTime = 0.0;
	double holdDownTime = 0.0;
	double lightLevel = 0.0;
	OBJHANDLE attachedObject;
	bool currentlyAttached;
	UINT LaunchPad;
	MESHHANDLE launchPad;
	ATTACHMENTHANDLE rocketAttach;
	UINT TowerMoveAway, TowerMoveIn;
	int cameraMode = 0; // smlTwrDwn, lrgTwrDwn, lrgTwrFar, undrRckt, farAwy, farAwy2

	int showInfoOnHud = 0;	/* 0 = Both key commands and flight data
							   1 = Only flight data
							   2 = Nothing (only stock HUD)*/


	//double towerStatus = 0.0;
	double towerInStatus = 0.0;
	enum towerstaus { CLOSE, MOVEAWAY, MOVECLOSE, AWAY } TowerInProcess;
};

#include "..\..\FunctionsForOrbiter2010.h"


// ==============================================================
// API interface
// ==============================================================


ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel) : VESSEL3(hVessel, flightmodel)
{
	launchPad = oapiLoadMeshGlobal("ProjectMercury\\K-Pad-5");

	DefineAnimation();
	//TowerProcess = CLOSE;
	TowerInProcess = AWAY;
}

ProjectMercury::~ProjectMercury()
{
}

void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	if (!oapiReadItem_vec(cfg, "AttachmentPosition", rocketPosition))
	{
		rocketPosition = ROCKET_POS; // if not available in config file
		oapiWriteLog("LC-5 could not read rocket attachment position config.");
	}

	if (!oapiReadItem_vec(cfg, "AttachmentDirection", rocketDirection))
	{
		rocketDirection = ROCKET_DIR; // if not available in config file
		oapiWriteLog("LC-5 could not read rocket attachment direction config.");
	}

	double heightOverGround;
	if (!oapiReadItem_float(cfg, "COG", heightOverGround))
	{
		heightOverGround = 0.0;
		oapiWriteLog("LC-5 could not read launchpad COG config");
	}
	static const DWORD tchdwnNum = 4;
	const double depression = -0.1;
	const double stiffness = abs(-LC_MASS * G / (3 * depression)); // abs for sanity check, as I have a tendency to forget signs
	const double damping = 0.3 * 2 * sqrt(LC_MASS * stiffness);
	const VECTOR3 TOUCHDOWN_POINTS0 = _V(-40.0, depression - heightOverGround, -30.0);
	const VECTOR3 TOUCHDOWN_POINTS1 = _V(40.0, depression - heightOverGround, -30.0);
	const VECTOR3 TOUCHDOWN_POINTS2 = _V(0.0, depression - heightOverGround, 50.0);
	const VECTOR3 TOUCHDOWN_POINTS3 = _V(0.0, 10.0, 0.0);

	SetSize(50.0);
	VersionDependentTouchdown(TOUCHDOWN_POINTS0, TOUCHDOWN_POINTS1, TOUCHDOWN_POINTS2, TOUCHDOWN_POINTS3, stiffness, damping, 10.0);
	SetEmptyMass(LC_MASS);

	rocketAttach = CreateAttachment(false, rocketPosition + PAD_OFFSET, unit(rocketDirection), _V(0, 0, 1), "ROCKET", true);

	LaunchPad = AddMesh(launchPad, &MESH_OFFSET);
	SetMeshVisibilityMode(LaunchPad, MESHVIS_ALWAYS | MESHVIS_EXTPASS);

	SURFHANDLE cont3 = oapiRegisterParticleTexture("Contrail3");
	SURFHANDLE cont4 = oapiRegisterParticleTexture("Contrail4");

	/*exhaust[0] = {
		0, 5.0, 30, 200, 1.0, 0.7, 20, 10.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PLIN, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1
	};

	exhaust[1] = {
		0, 5.0, 60, 300, 0.3, 0.3, 2.0, 5.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_PLIN, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1,
		cont3
	};*/

	exhaust[2] = {
		0, 1.0, 45, 5.0, 10.0, 0.7, 4.0, 5.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PLIN, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1,
		cont4
	};

	/*AddParticleStream(&exhaust[0], EXHAUST_POS, EXHAUST_DIR, &exhaustLevel);
	AddParticleStream(&exhaust[1], EXHAUST_POS, EXHAUST_DIR, &exhaustLevel);*/
	AddParticleStream(&exhaust[2], EXHAUST_POS, EXHAUST_DIR, &exhaustLevel);

	SetCameraOffset(CAMERA_FAR_AWAY_2);
	SetCameraDefaultDirection(CAMERA_FAR_AWAY_2_DIR);

	// Pad lights
	COLOUR4 col_d = { (float)(0.95),(float)(0.95),(float)(0.95),0 };
	COLOUR4 col_s = { (float)(0.95),(float)(0.95),(float)(0.95),0 };
	COLOUR4 col_a = { (float)(0.95),(float)(0.95),(float)(0.95),0 };
	LightEmitter* light1 = AddSpotLight(LIGHT1, LIGHT1_DIR, 150, 0.5, 0.03, 0.002, 60 * RAD, 90 * RAD, col_d, col_s, col_a);
	light1->SetIntensityRef(&lightLevel);
	LightEmitter* light2 = AddSpotLight(LIGHT2, LIGHT2_DIR, 150, 0.5, 0.03, 0.002, 60 * RAD, 90 * RAD, col_d, col_s, col_a);
	light2->SetIntensityRef(&lightLevel);
	LightEmitter* light3 = AddSpotLight(LIGHT3, LIGHT3_DIR, 150, 0.5, 0.03, 0.002, 60 * RAD, 90 * RAD, col_d, col_s, col_a);
	light3->SetIntensityRef(&lightLevel);
	LightEmitter* light4 = AddSpotLight(LIGHT4, LIGHT4_DIR, 150, 0.5, 0.03, 0.002, 60 * RAD, 90 * RAD, col_d, col_s, col_a);
	light4->SetIntensityRef(&lightLevel);
}

void ProjectMercury::clbkPostCreation(void)
{
	oapiGetViewportSize(&ScreenWidth, &ScreenHeight, &ScreenColour);
	TextX0 = (int)(0.025 * ScreenWidth);
	TextY0 = (int)(0.225 * ScreenHeight);
	LineSpacing = (int)(0.025 * ScreenHeight);

	// Update animation state
	SetAnimation(TowerMoveIn, towerInStatus);

	if (GetAttachmentStatus(rocketAttach) != NULL) // something is spawned attached
	{
		currentlyAttached = true;
		launching = true;
		engineIgnitionTime = 0.0;
		oapiWriteLog("Attached at spawn.");
	}
	else
	{
		currentlyAttached = false;
	}
}

void ProjectMercury::clbkPreStep(double simt, double simdt, double mjd)
{
	OBJHANDLE closestVessel = NULL;
	double distance = 1e10;
	GetClosestVessel(&closestVessel, &distance);

	if (closestVessel != NULL && distance < 100) // actually detected something, and it's within range
	{
		VESSEL* v = oapiGetVesselInterface(closestVessel);

		exhaustLevel = v->GetThrusterGroupLevel(THGROUP_MAIN);

		if (exhaustLevel != 0.0 && !launching)
		{
			AttachRocket(simt, closestVessel, v);
		}
		else if (exhaustLevel == 0.0)
		{
			launching = false; // set back to normal for next launch (engines are shut off)
		}

		if (launching && simt - engineIgnitionTime > holdDownTime && GetAttachmentStatus(rocketAttach) != NULL) // time to release
		{
			bool success2 = DetachChild(rocketAttach, 0.0);
			if (success2)
				oapiWriteLog("Released!");
			else
				oapiWriteLog("Failed to release.");
		}

		double fadeHeight = 16.0; // when attached, the distance is 14.9 m
		double cutoffHeight = 17.0;
		if (distance > fadeHeight && distance < cutoffHeight) // start decreasing level
		{
			exhaustLevel = 1.0 - fadeHeight / (fadeHeight - cutoffHeight) + distance / (fadeHeight - cutoffHeight);
		}
		else if (distance > cutoffHeight)
		{
			exhaustLevel = 0.0;
		}
	}
	else
	{
		exhaustLevel = 0.0;
		launching = false; // set back to normal for next launch
	}
}

void ProjectMercury::clbkPostStep(double simt, double simdt, double mjd)
{
	/*double delta = simdt / TOWER_DURATION;
	if (TowerProcess == MOVEAWAY)
		towerStatus += delta;
	else if (TowerProcess == MOVECLOSE)
		towerStatus -= delta;
	else if (TowerProcess == CLOSE)
		towerStatus = 0.0;
	else
		towerStatus = 1.0;

	if (towerStatus > 1.0)
		TowerProcess = AWAY;
	else if (towerStatus < 0.0)
		TowerProcess = CLOSE;

	SetAnimation(TowerMoveAway, towerStatus);*/

	// Move in
	double delta = simdt / TOWER_IN_DURATION;
	if (TowerInProcess == MOVEAWAY)
		towerInStatus += delta;
	else if (TowerInProcess == MOVECLOSE)
		towerInStatus -= delta;
	else if (TowerInProcess == CLOSE)
		towerInStatus = 0.0;
	else
		towerInStatus = 1.0;

	if (towerInStatus > 1.0)
		TowerInProcess = AWAY;
	else if (towerInStatus < 0.0)
		TowerInProcess = CLOSE;

	SetAnimation(TowerMoveIn, towerInStatus);

	if (cameraMode == 1)
	{
		SetCameraOffset(CAMERA_LARGE_TOWER_DOWN + TOWER_MOVE_IN * towerInStatus);
	}
	else if (cameraMode == 2)
	{
		SetCameraOffset(CAMERA_LARGE_TOWER_FAR + TOWER_MOVE_IN * towerInStatus);
	}
}

void ProjectMercury::clbkLoadStateEx(FILEHANDLE scn, void* vs)
{
	char* line;

	while (oapiReadScenario_nextline(scn, line))
	{
		if (!_strnicmp(line, "HOLDTIME", 8))
		{
			sscanf_s(line + 8, "%lf", &holdDownTime);
		}
		else if (!_strnicmp(line, "TOWERSTATUS", 11))
		{
			sscanf_s(line + 11, "%i", &TowerInProcess);
		}
		else if (!_strnicmp(line, "TOWERPROCESS", 12))
		{
			sscanf_s(line + 12, "%lf", &towerInStatus);
		}
		else
		{
			ParseScenarioLineEx(line, vs);
		}
	}
}

void ProjectMercury::clbkSaveState(FILEHANDLE scn)
{
	VESSEL3::clbkSaveState(scn); // write default parameters (orbital elements etc.)

	oapiWriteScenario_int(scn, "TOWERSTATUS", TowerInProcess);

	oapiWriteScenario_float(scn, "TOWERPROCESS", towerInStatus);
}

int ProjectMercury::clbkConsumeBufferedKey(DWORD key, bool down, char* kstate)
{
	if (!down) return 0; // only process keydown events

	if (!KEYMOD_CONTROL(kstate) && !KEYMOD_ALT(kstate) && !KEYMOD_SHIFT(kstate))
	{
		if (key == OAPI_KEY_K)
		{
			if (TowerInProcess == MOVEAWAY || TowerInProcess == AWAY)
				TowerInProcess = MOVECLOSE;
			else
				TowerInProcess = MOVEAWAY;

			return 1;
		}
		else if (key == OAPI_KEY_V)
		{
			OBJHANDLE closestVessel = NULL;
			double distance = 1e10;

			GetClosestVessel(&closestVessel, &distance);

			if (distance < 150)
			{
				VESSEL* v = oapiGetVesselInterface(closestVessel);

				if (v->GetThrusterGroupLevel(THGROUP_MAIN) == 0.0)
					AttachRocket(oapiGetSimTime(), closestVessel, v);
			}

			return 1;
		}
		else if (key == OAPI_KEY_C)
		{
			cameraMode = (cameraMode + 1) % numCamera;

			SwitchCamera(cameraMode);

			oapiCameraSetCockpitDir(0.0, 0.0);
			//oapiSetHUDMode(HUD_NONE);

			return 1;
		}
		else if (key == OAPI_KEY_P) // ignite attached vessel
		{
			OBJHANDLE attachedRocket = GetAttachmentStatus(rocketAttach);

			if (attachedRocket != NULL)
			{
				VESSEL* v = oapiGetVesselInterface(attachedRocket);
				v->SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
				attachedObject = attachedRocket;
				currentlyAttached = true;
			}
			else
			{
				attachedObject = NULL;
				currentlyAttached = false;
			}

			return 1;
		}
		else if (key == OAPI_KEY_SPACE) // lights on/off
		{
			if (lightLevel == 0.0)
			{
				lightLevel = 1.0;
			}
			else
			{
				lightLevel = 0.0;
			}

			return 1;
		}
	}
	else if (KEYMOD_CONTROL(kstate) && !KEYMOD_ALT(kstate) && !KEYMOD_SHIFT(kstate)) // Ctrl + ...
	{
		if (key == OAPI_KEY_T)
		{
			showInfoOnHud = (showInfoOnHud + 1) % 3;
			return 1;
		}
	}

	return 0;
}
bool ProjectMercury::clbkLoadPanel2D(int id, PANELHANDLE hPanel, DWORD viewW, DWORD viewH)
{
	return true;
}

bool ProjectMercury::clbkLoadGenericCockpit(void)
{
	// Disable MFD view

	return false;
}

bool ProjectMercury::clbkDrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp)
{
	char cbuf[256];
	int yIndex = 0;

	if (showInfoOnHud == 0)
	{
		// ==== Buttons text ====
		sprintf(cbuf, "Key commands:");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "Ctrl+T\t:Hide on-screen text");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "K\t:Move tower in/out");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "V\t:Attach closest rocket");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "C\t:Switch camera");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "P\t:Launch attached rocket");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "SPACE\t:Lights on/off");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;
	}

	yIndex = 0;

	double simt = oapiGetSimTime();

	if (showInfoOnHud < 2)
	{
		sprintf(cbuf, "Camera:");
		skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		if (cameraMode == 1)
			sprintf(cbuf, "  Gantry down");
		else if (cameraMode == 2)
			sprintf(cbuf, "  Gantry tele");
		else if (cameraMode == 3)
			sprintf(cbuf, "  Engine");
		else if (cameraMode == 4)
			sprintf(cbuf, "  Far 1");
		else if (cameraMode == 5)
			sprintf(cbuf, "  Far 2");
		else
			sprintf(cbuf, "  Tower");
		skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		OBJHANDLE closestVessel = NULL;
		double distance = 1e10;
		GetClosestVessel(&closestVessel, &distance);
		char ccbuf[256];
		if (closestVessel != NULL)
		{
			sprintf(cbuf, "Closest rocket:");
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			oapiGetObjectName(closestVessel, ccbuf, 256);
			sprintf(cbuf, "  %s", ccbuf);
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			if (engineIgnitionTime != 0.0)
			{
				double metAbs = simt - engineIgnitionTime - holdDownTime;
				int metSign = 1;
				char signChar[1];
				sprintf(signChar, "+");
				if (metAbs < 0.0)
				{
					metSign = -1;
					sprintf(signChar, "-");
				}
				metAbs = abs(metAbs) - 0.5 * ((double)metSign - 1.0); // if negative, add one extra second. Else don't add anything (to get count -3, -2, -1, 0, 1)
				int metH = (int)floor(metAbs / 3600.0);
				int metM = (int)floor((metAbs - metH * 3600.0) / 60.0);
				int metS = (int)floor((metAbs - metH * 3600.0 - metM * 60.0));

				sprintf(cbuf, "  T%s%02i %02i %02i", signChar, metH, metM, metS);
				skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			sprintf(cbuf, "  Distance %.1f m", distance);
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			if (GetAttachmentStatus(rocketAttach) == closestVessel)
				sprintf(cbuf, "  ATTACHED");
			else
				sprintf(cbuf, "  NOT ATTACHED");
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			VESSEL* v = oapiGetVesselInterface(closestVessel);
			if (v->GetThrusterGroupLevel(THGROUP_MAIN) != 0.0)
				sprintf(cbuf, "  ENGINES ON");
			else
				sprintf(cbuf, "  ENGINES OFF");
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			VersionDependentPadHUD(skp, simt, &yIndex, cbuf, v);

			VECTOR3 planetVec, rocketVec;
			GetRelativePos(GetSurfaceRef(), planetVec);
			GetRelativePos(closestVessel, rocketVec);
			double elevationAngle = acos(dotp(planetVec, rocketVec) / (length(planetVec) * length(rocketVec))) - PI05;
			sprintf(cbuf, "  Elevation: %.1f\u00B0", elevationAngle * DEG);
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else
		{
			sprintf(cbuf, "No rockets detected");
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
	}

	return true;
}

void ProjectMercury::clbkRenderHUD(int mode, const HUDPAINTSPEC* hps, SURFHANDLE hTex)
{
	// Do nothing to disable HUD
}


void ProjectMercury::clbkVisualCreated(VISHANDLE vis, int refcount)
{
	// Delete faulty mesh groups

	const int numFlatPlaneGroups = 3;
	UINT flatPlaneGroups[numFlatPlaneGroups] = { 917, 918, 919 }; // maybe also 158
	GROUPEDITSPEC ges;
	DEVMESHHANDLE lnchPad = GetDevMesh(vis, LaunchPad);
	ges.flags = GRPEDIT_ADDUSERFLAG;
	ges.UsrFlag = 3; // hide/delete mesh group
	for (int i = 0; i < numFlatPlaneGroups; i++)
	{
		oapiEditMeshGroup(lnchPad, flatPlaneGroups[i], &ges);
	}

	const int group1Start = 187, group1End = 202;
	const int group2Start = 647, group2End = 693;
	const int numSmallFeaturesGroups = group1End - group1Start + 1 + group2End - group2Start + 1;
	UINT smallFeaturesGroups[numSmallFeaturesGroups];
	for (int k = group1Start; k < group1End + 1; k++)
	{
		smallFeaturesGroups[k - group1Start] = k;
	}
	for (int L = group2Start; L < group2End + 1; L++)
	{
		smallFeaturesGroups[L - group2Start + group1End - group1Start + 1] = L;
	}

	for (int i = 0; i < numSmallFeaturesGroups; i++)
	{
		oapiEditMeshGroup(lnchPad, smallFeaturesGroups[i], &ges);
	}
}

void ProjectMercury::SwitchCamera(int camera)
{
	if (camera == 1)
	{
		SetCameraOffset(CAMERA_LARGE_TOWER_DOWN);
		SetCameraDefaultDirection(CAMERA_LARGE_TOWER_DOWN_DIR);
	}
	else if (camera == 2)
	{
		SetCameraOffset(CAMERA_LARGE_TOWER_FAR);
		SetCameraDefaultDirection(CAMERA_LARGE_TOWER_FAR_DIR);
	}
	else if (camera == 3)
	{
		SetCameraOffset(CAMERA_UNDER_ROCKET);
		SetCameraDefaultDirection(CAMERA_UNDER_ROCKET_DIR);
	}
	else if (camera == 4)
	{
		SetCameraOffset(CAMERA_FAR_AWAY);
		SetCameraDefaultDirection(CAMERA_FAR_AWAY_DIR);
	}
	else
	{
		SetCameraOffset(CAMERA_FAR_AWAY_2);
		SetCameraDefaultDirection(CAMERA_FAR_AWAY_2_DIR);
	}
}

void ProjectMercury::AttachRocket(double simt, OBJHANDLE closestVessel, VESSEL* v)
{
	launching = true;
	engineIgnitionTime = simt;
	bool success = AttachChild(closestVessel, rocketAttach, v->GetAttachmentHandle(true, 0));
	if (success)
	{
		oapiWriteLog("Attached!");
		attachedObject = closestVessel;
		currentlyAttached = true;
	}
	else
	{
		oapiWriteLog("Failed to attach.");
	}
}

void ProjectMercury::GetClosestVessel(OBJHANDLE* closestVessel, double* distance)
{
	VECTOR3 pos;
	OBJHANDLE vesselI;
	for (int i = 0; i < (int)oapiGetVesselCount(); i++)
	{
		vesselI = oapiGetVesselByIndex(i);
		GetRelativePos(vesselI, pos);
		if (vesselI != GetHandle() && length(pos) < *distance && oapiGetMass(vesselI) > 1e4) // must be large enough to fit to pad
		{
			*closestVessel = vesselI;
			*distance = length(pos);
		}
	}
}

void ProjectMercury::DefineAnimation(void)
{
	static UINT towerGroups[numGroups];
	for (int i = 0; i < numGroups0; i++)
	{
		towerGroups[i] = groupStart0 + i;
	}

	for (int i = 0; i < numGroups1; i++)
	{
		towerGroups[i + numGroups0] = groupStart1 + i;
	}

	for (int i = 0; i < numGroups2; i++)
	{
		towerGroups[i + numGroups0 + numGroups1] = groupStart2 + i;
	}

	for (int i = 0; i < numGroups3; i++)
	{
		towerGroups[i + numGroups0 + numGroups1 + numGroups2] = groupStart3 + i;
	}

	for (int i = 0; i < numGroups4; i++)
	{
		towerGroups[i + numGroups0 + numGroups1 + numGroups2 + numGroups3] = groupStart4 + i;
	}

	static MGROUP_TRANSLATE towerMove(
		LaunchPad,
		towerGroups, numGroups,
		TOWER_MOVE_IN
	);
	TowerMoveIn = CreateAnimation(0.5);
	AddAnimationComponent(TowerMoveIn, 0.0, 1.0, &towerMove);
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