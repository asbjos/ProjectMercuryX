// ==============================================================
// ORBITER vessel module: Little Joe ramp
// ==============================================================

#define STRICT
#define ORBITER_MODULE
#define VESSELVER VESSEL3

#include "orbitersdk.h"

const double LC_MASS = 1e5; // guesstimate
const double LC_COG = 2.33;
const VECTOR3 ROCKET_POS = _V(0.0, LC_COG, 0.0);
const VECTOR3 ROCKET_DIR = _V(0.0, 0.0, 1.0);
const VECTOR3 PAD_OFFSET = _V(0, 0.0, 0.0);
const VECTOR3 MESH_OFFSET = _V(0.0, 0.0, 0.0) + _V(0, LC_COG, 0);
const VECTOR3 EXHAUST_POS = _V(0.0, 0.0, 0.0);
const VECTOR3 EXHAUST_DIR = _V(1.0, 0.0, 0.0);
//const VECTOR3 MESH_OFFSET = PAD_OFFSET + _V(0, LC_COG, 0);

// Camera settings
const int numCamera = 3; // lrgTwrDwn, lrgTwrFar, undrRckt, farAwy, farAwy2
const VECTOR3 CAMERA_UNDER_ROCKET = _V(0.0, ROCKET_POS.y - 0.0, 0.0); // 2
const VECTOR3 CAMERA_UNDER_ROCKET_DIR = _V(0, 1, 0);
const VECTOR3 CAMERA_FAR_AWAY = _V(-36.0, 9.3, -3.1) + MESH_OFFSET; // 3
const VECTOR3 CAMERA_FAR_AWAY_DIR = unit(_V(1, -0.1, 0));
const VECTOR3 CAMERA_FAR_AWAY_2 = _V(3.6, 7.0, 27.6) + MESH_OFFSET; // 4
const VECTOR3 CAMERA_FAR_AWAY_2_DIR = unit(_V(1, -0.1, -1.2));


class ProjectMercury : public VESSELVER
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
	VECTOR3 NormalXY(VECTOR3 vIn);
	//void DefineAnimation(void);

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
	double rocketAngle = 80.0;
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
	//double towerInStatus = 0.0;
	//enum towerstaus { CLOSE, MOVEAWAY, MOVECLOSE, AWAY } TowerInProcess;
};

#include "..\FunctionsForOrbiter2010.h"

ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel) : VESSELVER(hVessel, flightmodel)
{
	launchPad = oapiLoadMeshGlobal("ProjectMercury\\LJ_tower");

	//DefineAnimation();
	//TowerProcess = CLOSE;
	//TowerInProcess = AWAY;
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
	rocketAngle = 82.0; // DEG
	if (!oapiReadItem_float(cfg, "AttachmentAngle", rocketAngle))
	{
		rocketDirection = _V(0.0, cos(-78.0 * RAD), sin(-78.0 * RAD)); // if not available in config file
		oapiWriteLog("LC-5 could not read rocket attachment angle config.");
	}
	else
	{
		rocketDirection = _V(0.0, cos(-rocketAngle * RAD), sin(-rocketAngle * RAD));
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

	rocketAttach = CreateAttachment(false, rocketPosition + PAD_OFFSET, unit(rocketDirection), unit(NormalXY(rocketDirection)), "ROCKET", false);

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
}

void ProjectMercury::clbkPostCreation(void)
{
	oapiGetViewportSize(&ScreenWidth, &ScreenHeight, &ScreenColour);
	TextX0 = (int)(0.025 * ScreenWidth);
	TextY0 = (int)(0.225 * ScreenHeight);
	LineSpacing = (int)(0.025 * ScreenHeight);

	// Update animation state
	//SetAnimation(TowerMoveIn, towerInStatus);

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
		if (distance > fadeHeight&& distance < cutoffHeight) // start decreasing level
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
		else if (!_strnicmp(line, "ROCKETANGLE", 11))
		{
			rocketAngle = atof(line + 11);
			rocketDirection = _V(0.0, cos(-rocketAngle * RAD), sin(-rocketAngle * RAD));
			SetAttachmentParams(rocketAttach, rocketPosition + PAD_OFFSET, unit(rocketDirection), unit(NormalXY(rocketDirection)));
			oapiWriteLogV("Launcher angle set in scenario to %.2f deg", rocketAngle);
		}
		else
		{
			ParseScenarioLineEx(line, vs);
		}
	}
}

void ProjectMercury::clbkSaveState(FILEHANDLE scn)
{
	VESSELVER::clbkSaveState(scn); // write default parameters (orbital elements etc.)
}

int ProjectMercury::clbkConsumeBufferedKey(DWORD key, bool down, char* kstate)
{
	if (!down) return 0; // only process keydown events

	if (!KEYMOD_CONTROL(kstate) && !KEYMOD_ALT(kstate) && !KEYMOD_SHIFT(kstate))
	{
		if (key == OAPI_KEY_V)
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
		else if (key == OAPI_KEY_G) // rocket tilt up
		{
			if (rocketAngle < 90.0)
			{
				rocketAngle += 1.0;
				rocketDirection = _V(0.0, cos(-rocketAngle * RAD), sin(-rocketAngle * RAD));
				SetAttachmentParams(rocketAttach, rocketPosition + PAD_OFFSET, unit(rocketDirection), unit(NormalXY(rocketDirection)));
			}
			return 1;
		}
		else if (key == OAPI_KEY_K) // rocket tilt down
		{
			if (rocketAngle > 70.0)
			{
				rocketAngle -= 1.0;
				rocketDirection = _V(0.0, cos(-rocketAngle * RAD), sin(-rocketAngle * RAD));
				SetAttachmentParams(rocketAttach, rocketPosition + PAD_OFFSET, unit(rocketDirection), unit(NormalXY(rocketDirection)));
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

		sprintf(cbuf, "Ctrl+T:Hide on-screen text");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "V:     Attach closest rocket");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "C:     Switch camera");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "P:     Launch attached rocket");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "G/K:   Tilt rocket up/down");
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

		sprintf(cbuf, "Tilt: %.0f\u00B0", rocketAngle);
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

			// orbiter2010 orbiter2016 debug
			/*sprintf(cbuf, "  Height: %.2f km", v->GetAltitude(ALTMODE_GROUND) / 1000.0);
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;*/

			double longR, latR, radR, longP, latP, radP;
			v->GetEquPos(longR, latR, radR);
			OBJHANDLE plt = GetEquPos(longP, latP, radP);
			double angleDownrange = oapiOrthodome(longR, latR, longP, latP);
			sprintf(cbuf, "  Downrange: %.2f km", angleDownrange * oapiGetSize(plt) / 1000.0);
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			/*sprintf(cbuf, "  Speed: %.1f m/s", v->GetGroundspeed());
			skp->Text(TextX0 * secondColumnHUDx, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;*/

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
		SetCameraOffset(CAMERA_UNDER_ROCKET);
		SetCameraDefaultDirection(CAMERA_UNDER_ROCKET_DIR);
	}
	else if (camera == 2)
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
		if (vesselI != GetHandle() && length(pos) < *distance && oapiGetMass(vesselI) > 1e3) // must be large enough to fit to pad
		{
			*closestVessel = vesselI;
			*distance = length(pos);
		}
	}
}

VECTOR3 ProjectMercury::NormalXY(VECTOR3 vIn)
{
	VECTOR3 vOut;
	/*vOut.x = vIn.z;
	vOut.y = vIn.y;
	vOut.z = -vIn.x;*/

	vOut.x = vIn.x;
	vOut.y = -vIn.z;
	vOut.z = vIn.y;


	//vOut.x = vIn.y;
	//vOut.y = -vIn.x;
	//vOut.z = vIn.z;
	return vOut;
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