// ==============================================================
//				Source file for Mercury Scout.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2020
// 
// Made for Mercury Scout mesh by Ricardo Nunes ('4th rock')
// 
// Thank you to Ricardo Nunes for meshes, data, etc.
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================

#define STRICT
#define ORBITER_MODULE
#define VESSELVER VESSEL4

#include "orbitersdk.h"

#include "Scout.h"
#include "..\FunctionsForOrbiter2016.h"
#include "..\ProjectMercuryGeneric.h"
#include <time.h> // for seed in random function


ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel)
	: VESSELVER(hVessel, flightmodel)
{
	// Load exterior meshes
	// capsule is defined in loadstate
	stage[0] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_stage1_trans_no");
	stage[1] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_stage2_trans_no");
	stage[2] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_stage3_trans_no");
	stage[3] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_stage4_trans");
	skirt[0] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_skirt1_trans");
	skirt[1] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_skirt2_trans");
	skirt[2] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_skirt3_trans");
	fairing[0] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_fairing_1_trans");
	fairing[1] = oapiLoadMeshGlobal("ProjectMercury\\Scout\\scout_fairing_2_trans");
	circularFrameMesh = oapiLoadMeshGlobal("ProjectMercury\\FullCircle");

	VesselStatus = STAGE1;
	PreviousVesselStatus = STAGE1;
	FairingStatus = FAIRINGON;
	FailureMode = NONE;

	CGshifted = false;

	srand(UINT(time(NULL)));
}

ProjectMercury::~ProjectMercury()
{
	char cbuf[256];
	sprintf(cbuf, "%s Flight parameters:", GetName());
	oapiWriteLog(cbuf);

	if (historyReference == NULL) // spawned after launch
	{
		historyReference = GetSurfaceRef();

		oapiWriteLogV(" > FULL FLIGHT NOT RECORDED");
	}

	oapiWriteLogV(" > Apogee altitude: %.1f km (%.1f nm)", historyMaxAltitude / 1000.0, historyMaxAltitude / 1852.0);
	if (historyPerigee == 1e10) oapiWriteLogV(" > No perigee experienced", historyPerigee / 1000.0, historyPerigee / 1852.0);
	else oapiWriteLogV(" > Perigee altitude: %.1f km (%.1f nm)", historyPerigee / 1000.0, historyPerigee / 1852.0);
	oapiWriteLogV(" > Inclination angle: %.2f deg", historyInclination); // only true for full orbit

	oapiWriteLogV(" > Earth-fixed velocity: %.0f m/s (%.0f ft/s)", historyMaxEarthSpeed, historyMaxEarthSpeed / 0.3048);
	oapiWriteLogV(" > Space-fixed velocity: %.0f m/s (%.0f ft/s)", historyMaxSpaceSpeed, historyMaxSpaceSpeed / 0.3048);
}


// --------------------------------------------------------------
// Set the capabilities of the vessel class
// --------------------------------------------------------------
void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	// physical vessel parameters
	SetSize(STAGE1_LENGTH + STAGE2_LENGTH + STAGE3_LENGTH + STAGE4_LENGTH);
	SetEmptyMass(STAGE1_DRY_MASS + STAGE2_DRY_MASS + STAGE3_DRY_MASS + STAGE4_DRY_MASS);

	// Read height over ground from config
	if (!oapiReadItem_float(cfg, "HeightOverGround", heightOverGround))
	{
		heightOverGround = 5.8; // if not available in config file
		oapiWriteLog("Mercury could not read height over ground config.");
	}

	// Read rudder values from config
	if (!oapiReadItem_float(cfg, "RudderGainFactor", ampFactor))
	{
		ampFactor = 0.10;
		oapiWriteLog("Mercury could not read rudder gain factor config.");
	}
	if (!oapiReadItem_float(cfg, "RudderGainMinimum", ampAdder))
	{
		ampAdder = 0.05;
		oapiWriteLog("Mercury could not read rudder gain adder config.");
	}
	if (!oapiReadItem_float(cfg, "RudderDelayResponse", rudderDelay))
	{
		rudderDelay = 0.5;
		oapiWriteLog("Mercury could not read rudder delay time config.");
	}
	if (!oapiReadItem_float(cfg, "RudderLiftCoeffShift", rudderLift))
	{
		rudderLift = 1.7;
		oapiWriteLog("Mercury could not read rudder lift coeff config.");
	}

	// Read HUD x-pos of second column
	if (!oapiReadItem_int(cfg, "HUD2ndColumnPos", secondColumnHUDx))
	{
		secondColumnHUDx = 28;
		oapiWriteLog("Mercury could not read HUD column position config.");
	}

	if (!oapiReadItem_float(cfg, "TimeStepLimit", timeStepLimit))
	{
		timeStepLimit = 0.1;
		oapiWriteLog("Mercury could not read time step limit config for authentic autopilot.");
	}

	if (!oapiReadItem_bool(cfg, "MercuryNetwork", MercuryNetwork))
	{
		MercuryNetwork = true;
		oapiWriteLog("Mercury could not read network contact config.");
	}

	if (!oapiReadItem_float(cfg, "Stage4SpinRate", stage4SpinRate))
	{
		stage4SpinRate = 160.0;
		oapiWriteLog("Mercury could not read stage 4 spin rate config.");
	}

	const double depression = 0.3;
	const double totalStartMass = (STAGE1_TOTAL_MASS + STAGE2_TOTAL_MASS + STAGE3_TOTAL_MASS + STAGE4_TOTAL_MASS);
	const double stiffness = abs(- totalStartMass * G / (3.0 * depression)); // abs for sanity check, as I have a tendency to forget signs
	const double damping = 0.3 * 2.0 * sqrt(totalStartMass * stiffness);
	const VECTOR3 TOUCHDOWN_LAUNCH0 = _V(0.0, -1.0, STAGE1_OFFSET.z - STAGE1_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH1 = _V(-0.7, 0.7, STAGE1_OFFSET.z - STAGE1_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH2 = _V(0.7, 0.7, STAGE1_OFFSET.z - STAGE1_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH3 = _V(0.0, 0.0, STAGE4_OFFSET.z + STAGE4_LENGTH / 2.0);
	VersionDependentTouchdown(TOUCHDOWN_LAUNCH0, TOUCHDOWN_LAUNCH1, TOUCHDOWN_LAUNCH2, TOUCHDOWN_LAUNCH3, stiffness, damping, 10.0);

	// Rotation parameters
	SetRotDrag(_V(0.7, 0.7, 1.2));
	SetPMI(_V(27.40, 27.40, 0.11)); // 19620003288 page 43, but it lists as slug/ft2, while it should be slug*ft2

	// Propellants
	scout_propellant[3] = CreatePropellantResource(STAGE4_FUEL_MASS);
	scout_propellant[2] = CreatePropellantResource(STAGE3_FUEL_MASS);
	scout_propellant[1] = CreatePropellantResource(STAGE2_FUEL_MASS);
	scout_propellant[0] = CreatePropellantResource(STAGE1_FUEL_MASS);
	SetDefaultPropellantResource(scout_propellant[0]);

	// Engines
	th_main[3] = CreateThruster(STAGE4_ENGINE_POS, STAGE4_ENGINE_DIR, STAGE4_THRUST, scout_propellant[3], STAGE4_ISP);
	th_main[2] = CreateThruster(STAGE3_ENGINE_POS, STAGE3_ENGINE_DIR, STAGE3_THRUST, scout_propellant[2], STAGE3_ISP);
	th_main[1] = CreateThruster(STAGE2_ENGINE_POS, STAGE2_ENGINE_DIR, STAGE2_THRUST, scout_propellant[1], STAGE2_ISP);
	th_main[0] = CreateThruster(STAGE1_ENGINE_POS, STAGE1_ENGINE_DIR, STAGE1_THRUST, scout_propellant[0], STAGE1_ISP);
	CreateThrusterGroup(&th_main[0], 1, THGROUP_MAIN);
	SURFHANDLE texExh = oapiRegisterExhaustTexture("Exhaust2");
	AddExhaust(th_main[3], 8, 0.3, STAGE4_ENGINE_POS, -STAGE4_ENGINE_DIR, texExh);
	AddExhaust(th_main[2], 6, 0.7, STAGE3_ENGINE_POS, -STAGE3_ENGINE_DIR, texExh);
	AddExhaust(th_main[1], 4, 1.0, STAGE2_ENGINE_POS, -STAGE2_ENGINE_DIR, texExh);
	AddExhaust(th_main[0], 3, 1.2, STAGE1_ENGINE_POS, -STAGE1_ENGINE_DIR, texExh);

	// Control surfaces for air rudders. dCl and delay values from DeltaGlider and/or Falcon 9 by BrianJ
	CreateAirfoilsScout(); // try this
	double finArea = 9.5 * 9.5 / 2.0 * (0.0254 * 0.0254);
	Rudders[0] = CreateControlSurface3(AIRCTRL_ELEVATOR, finArea * 2, rudderLift, _V(0.0, 0.0, -STAGE1_LENGTH), AIRCTRL_AXIS_XPOS, rudderDelay); // check Airctrl_Axis
	Rudders[1] = CreateControlSurface3(AIRCTRL_RUDDER, finArea * 2, rudderLift, _V(0.0, 0.0, -STAGE1_LENGTH), AIRCTRL_AXIS_YPOS, rudderDelay);
	Rudders[2] = CreateControlSurface3(AIRCTRL_AILERON, finArea * 2, rudderLift, _V(0.0, -1.0, -STAGE1_LENGTH), AIRCTRL_AXIS_YPOS, rudderDelay);
	/*Rudders[2] = CreateControlSurface3(AIRCTRL_AILERON, finArea, 1.7, _V(0.0, 1.0, -5.0), AIRCTRL_AXIS_YPOS, 0.5);
	Rudders[3] = CreateControlSurface3(AIRCTRL_FLAP, finArea, 1.7, _V(1.0, 0.0, -5.0), AIRCTRL_AXIS_XPOS, 0.5);*/

	// Contrail stuff
	exhaustMain[0] = {
		0, 3.0, 50, 200, 0.1, 10.0, 2.0, 5.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, 1e-5, 0.3
	};
	exhaustStream[3] = AddExhaustStream(th_main[3], _V(0, 0.3, -20), &(exhaustMain[0]));
	exhaustStream[2] = AddExhaustStream(th_main[2], _V(0, 0.3, -20), &(exhaustMain[0])); // debug
	exhaustStream[1] = AddExhaustStream(th_main[1], _V(0, 0.3, -20), &(exhaustMain[0])); // debug
	exhaustStream[0] = AddExhaustStream(th_main[0], _V(0, 0.3, -20), &(exhaustMain[0])); // debug

	// associate a mesh for the visual
	Stage[3] = AddMesh(stage[3], &STAGE4_OFFSET);
	Stage[2] = AddMesh(stage[2], &STAGE3_OFFSET);
	Stage[1] = AddMesh(stage[1], &STAGE2_OFFSET);
	Stage[0] = AddMesh(stage[0], &STAGE1_OFFSET);
	Skirt[2] = AddMesh(skirt[2], &STAGE3_OFFSET);
	Skirt[1] = AddMesh(skirt[1], &STAGE3_OFFSET);
	Skirt[0] = AddMesh(skirt[0], &STAGE2_OFFSET);
	Fairing[1] = AddMesh(fairing[1], &FlipX(FAIRING_OFFSET));
	Fairing[0] = AddMesh(fairing[0], &FAIRING_OFFSET);

	padAttach = CreateAttachment(true, STAGE1_ENGINE_POS, _V(0, -1, 0), _V(0, 0, 1), "PAD", false);
}

void ProjectMercury::clbkPostCreation()
{
	switch (VesselStatus)
	{
	case STAGE1:
		// Do nothing
		break;
	case STAGE2:
		DelMesh(Stage[0]);
		DelThruster(th_main[0]);
		DelPropellantResource(scout_propellant[0]);
		break;
	case STAGE3:
		DelMesh(Stage[0]);
		DelMesh(Stage[1]);
		DelMesh(Skirt[0]);
		DelThruster(th_main[0]);
		DelThruster(th_main[1]);
		DelPropellantResource(scout_propellant[0]);
		DelPropellantResource(scout_propellant[1]);
		break;
	case STAGE4:
		DelMesh(Stage[0]);
		DelMesh(Stage[1]);
		DelMesh(Stage[2]);
		DelMesh(Skirt[0]);
		DelMesh(Skirt[1]);
		DelMesh(Skirt[2]);
		DelThruster(th_main[0]);
		DelThruster(th_main[1]);
		DelThruster(th_main[2]);
		DelPropellantResource(scout_propellant[0]);
		DelPropellantResource(scout_propellant[1]);
		DelPropellantResource(scout_propellant[2]);
		break;
	}

	if (FairingStatus == FAIRINGOFF)
	{
		DelMesh(Fairing[0]);
		DelMesh(Fairing[1]);
	}
}

void ProjectMercury::clbkPreStep(double simt, double simdt, double mjd)
{
	SetEmptyMass(EmptyMass()); // calculate drymass of current setting (launch, abort, flight ...)

	if (GroundContact() || (GetAttachmentStatus(padAttach) == NULL && previousFrameAttachment != NULL)) initPitch = GetPitch();

	VECTOR3 angVel;
	GetAngularVel(angVel);
	if (VesselStatus == STAGE4 && length(angVel) > 3.0 * stage4SpinRate / 60.0 * PI2)
	{
		oapiWriteLogV("Stop me, please! Attitude rate %.2f deg/s at T+%.1f", length(angVel) * DEG, simt - launchTime);
		SetAngularVel(_V(0, 0, 0)); // at high time acc the vessel gets erratic high ang.speed motion
	}

	// Actions
	if (separateFairingAction)
	{
		SeparateFairing();
		fairingSeparationTime = oapiGetSimTime();
		separateFairingAction = false;
	}
	if (staging1Action)
	{
		Staging(1);
		stageSeparationTime[0] = oapiGetSimTime();
		staging1Action = false;
	}
	if (staging2Action)
	{
		Staging(2);
		stageSeparationTime[1] = oapiGetSimTime();
		staging2Action = false;
	}
	if (staging3Action)
	{
		Staging(3);
		stageSeparationTime[2] = oapiGetSimTime();
		staging3Action = false;
	}

	// Solid rocket is not adjustable
	if (VesselStatus == STAGE1 && stageIgnitionTime[0] != NULL) SetThrusterLevel(th_main[0], 1.0);
	if (VesselStatus == STAGE2 && stageIgnitionTime[1] != NULL) SetThrusterLevel(th_main[1], 1.0);
	if (VesselStatus == STAGE3 && stageIgnitionTime[2] != NULL) SetThrusterLevel(th_main[2], 1.0);
	if (VesselStatus == STAGE4 && stageIgnitionTime[3] != NULL) SetThrusterLevel(th_main[3], 1.0);

	switch (VesselStatus)
	{
	case STAGE1:
		// Allow to be launched from attached pad
		OBJHANDLE attachedParent;
		attachedParent = GetAttachmentStatus(padAttach);
		if (attachedParent != NULL && oapiGetFocusObject() == attachedParent && GetThrusterGroupLevel(THGROUP_MAIN) != 0.0) // attached, pad in focus, engines on, and has been uninitialised. Launch is initiated from pad
		{
			oapiSetTimeAcceleration(1.0);
			autopilot = true;
			launchTime = oapiGetSimTime() + 0.0;
		}

		break;
	case STAGE2:
		if (!CGshifted && !GroundContact())
		{
			SetSize(STAGE2_LENGTH + STAGE3_LENGTH + STAGE4_LENGTH);
			//VersionDependentTouchdown(TOUCHDOWN_TOWSEP0, TOUCHDOWN_TOWSEP1, TOUCHDOWN_TOWSEP2, TOUCHDOWN_TOWSEP3, 1e7, 1e5, 10.0);

			ShiftCG(STAGE2_OFFSET); // only from STAGE1 or scenario
			SetPMI(_V(8.30, 8.30, 0.09)); // 19620003288 page 43
			SetCameraOffset(_V(0.0, 0.0, 0.0));
			CreateStage2RCS();
			CGshifted = true;

			PreviousVesselStatus = STAGE2;
		}
		break;
	case STAGE3:
		if (!CGshifted && !GroundContact())
		{
			SetSize(STAGE3_LENGTH + STAGE4_LENGTH);
			//VersionDependentTouchdown(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2, TOUCHDOWN_FLIGHT3, stiffness, damping, 10.0);
			if (PreviousVesselStatus == STAGE2) ShiftCG(STAGE3_OFFSET - STAGE2_OFFSET);
			else ShiftCG(STAGE3_OFFSET - STAGE1_OFFSET); // From scenario
			SetCameraOffset(_V(0.0, 0.0, 0.0));
			SetPMI(_V(1.26, 1.26, 0.077)); // 19620003288 page 43
			SetRotDrag(_V(0.25, 0.25, 0.01));
			CreateStage3RCS();
			CGshifted = true;
			PreviousVesselStatus = STAGE3;
		}
		break;
	case STAGE4:
		if (!CGshifted && !GroundContact())
		{
			SetSize(STAGE4_LENGTH);
			//VersionDependentTouchdown(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2, TOUCHDOWN_FLIGHT3, stiffness, damping, 10.0);
			if (PreviousVesselStatus == STAGE3) ShiftCG(STAGE4_OFFSET - STAGE3_OFFSET);
			else ShiftCG(STAGE4_OFFSET - STAGE1_OFFSET); // From scenario
			SetCameraOffset(_V(0.0, 0.0, 0.0));
			SetPMI(_V(0.26, 0.26, 0.029)); // 19620003288 page 43
			SetRotDrag(_V(0.25, 0.25, 0.0001)); // A spinning stage is now stopped at 160 km. Not good. Adding a zero for every simulation I'm not satisfied
			CGshifted = true;
			PreviousVesselStatus = STAGE3;
		}
		break;
	}

	DeleteRogueVessels();
	BaseContactManager();

	if (autopilot && !(VesselStatus == STAGE4 && GetPropellantMass(scout_propellant[3]) == 0.0))
	{
		ScoutAutopilot(simt - launchTime, simt, simdt);
	}

	if (simt - launchTime > 3.0 && simdt > timeStepLimit && autopilot && previousSimdt > timeStepLimit && (stageShutdownTime[2] == NULL || (!pitchYawEliminated && VesselStatus != STAGE4)))
	{
		AimEulerAngle(eulerPitch, eulerYaw);
	}

	previousSimdt = simdt;
	previousFrameAttachment = GetAttachmentStatus(padAttach);
}

void ProjectMercury::clbkPostStep(double simt, double simdt, double mjd)
{
	if (!GroundContact())
	{
		VECTOR3 angvel;
		GetAngularVel(angvel);
		integratedPitch += angvel.x * simdt;
		integratedYaw += angvel.y * simdt;
		integratedRoll += angvel.z * simdt;
	}

	//if (!GroundContact() && stageShutdownTime[1] == NULL) // debug, pitchlog
	//{
	//	if (pitchDataLogFile == NULL) pitchDataLogFile = oapiOpenFile("ScoutPitchLog.txt", FILE_OUT, ROOT); // debug
	//	// Debug
	//	char pitchLog[256];
	//	sprintf(pitchLog, "%.3f\t%.3f", simt - launchTime, GetPitch() * DEG);
	//	oapiWriteLine(pitchDataLogFile, pitchLog);
	//}

	if (VesselStatus == STAGE1 && !autopilot) // not autopilot
	{
		SetADCtrlMode(7); // enable adc
	}

	if (VesselStatus == STAGE1)
	{
		VECTOR3 t0;
		t0 = STAGE1_ENGINE_DIR;
		double deflPi = GetControlSurfaceLevel(AIRCTRL_ELEVATOR) * 0.5 * RAD;
		double deflYa = GetControlSurfaceLevel(AIRCTRL_RUDDER) * 0.5 * RAD;
		VECTOR3 thrustDirection = _V(t0.x * cos(deflYa) - (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) + t0.x * sin(deflYa));
		SetThrusterDir(th_main[0], thrustDirection);
	}

	if (VesselStatus == STAGE1 && stageShutdownTime[0] == NULL && GetPropellantMass(scout_propellant[0]) == 0.0) stageShutdownTime[0] = simt;
	if (VesselStatus == STAGE2 && stageShutdownTime[1] == NULL && GetPropellantMass(scout_propellant[1]) == 0.0) stageShutdownTime[1] = simt;
	if (VesselStatus == STAGE3 && stageShutdownTime[2] == NULL && GetPropellantMass(scout_propellant[2]) == 0.0) stageShutdownTime[2] = simt;
	if (VesselStatus == STAGE4 && stageShutdownTime[3] == NULL && GetPropellantMass(scout_propellant[3]) == 0.0) stageShutdownTime[3] = simt;

	if (VesselStatus == STAGE1 && stageIgnitionTime[0] == NULL && GetThrusterLevel(th_main[0]) != 0.0) stageIgnitionTime[0] = simt;
	if (VesselStatus == STAGE2 && stageIgnitionTime[1] == NULL && GetThrusterLevel(th_main[1]) != 0.0) stageIgnitionTime[1] = simt;
	if (VesselStatus == STAGE3 && stageIgnitionTime[2] == NULL && GetThrusterLevel(th_main[2]) != 0.0) stageIgnitionTime[2] = simt;
	if (VesselStatus == STAGE4 && stageIgnitionTime[3] == NULL && GetThrusterLevel(th_main[3]) != 0.0) stageIgnitionTime[3] = simt;

	if (sattelitePoweredOn && !GroundContact())
	{
		batteryLevel -= simdt;
		if (batteryLevel < 0.0) batteryLevel = 0.0;
	}
	else if (!sattelitePoweredOn && !GroundContact()) // low power mode
	{
		batteryLevel -= simdt * 0.05;
		if (batteryLevel < 0.0) batteryLevel = 0.0;
	}

	// History
	double longit, latit, radiusDontCare;
	GetEquPos(longit, latit, radiusDontCare);
	double getAlt = GetAltitude();

	if (historyInclination < latit * DEG)
	{
		historyInclination = latit * DEG;
	}

	if (VesselStatus == STAGE4 && GetPropellantMass(scout_propellant[3]) == 0.0 && getAlt < historyPerigee && historyBottomPrev < getAlt && historyBottomPrevPrev > historyBottomPrev) // we have perigee, and it's the absolute lowest
	{
		historyPerigee = getAlt;
	}
	historyBottomPrevPrev = historyBottomPrev;
	historyBottomPrev = getAlt;

	if (getAlt > historyMaxAltitude) historyMaxAltitude = getAlt;

	if (GetGroundspeed() > historyMaxEarthSpeed)
		historyMaxEarthSpeed = GetGroundspeed();

	VECTOR3 currentSpaceVelocity;
	GetRelativeVel(GetSurfaceRef(), currentSpaceVelocity);
	double currentSpaceSpeed = length(currentSpaceVelocity);
	if (currentSpaceSpeed > historyMaxSpaceSpeed)
		historyMaxSpaceSpeed = currentSpaceSpeed;
}

int ProjectMercury::clbkConsumeBufferedKey(DWORD key, bool down, char* kstate)
{
	if (!down) return 0; // only process keydown events

	if (KEYMOD_CONTROL(kstate)) // Ctrl+button
	{
		switch (key)
		{
		case OAPI_KEY_T: // hide on-screen text
			if (showInfoOnHud == 0)
			{
				showInfoOnHud = 1;
			}
			else if (showInfoOnHud == 1)
			{
				showInfoOnHud = 2;
			}
			else
			{
				showInfoOnHud = 0;
			}
			return 1;
		}
		return 0;
	}
	else if (KEYMOD_ALT(kstate))
	{
		return 0;
	}
	else if (KEYMOD_SHIFT(kstate))
	{
		return 0;
	}
	else // single buttons are pressed (no Ctrl, Shift, Alt)
	{
		switch (key)
		{
		case OAPI_KEY_SPACE: // Remove from final
			if (VesselStatus == STAGE1)
			{
				spaceLaunch = true;
				autopilot = false;

				SetThrusterGroupLevel(THGROUP_MAIN, 1.0);

				if (GroundContact())
				{
					launchTime = oapiGetSimTime();
					return 1;
				}
			}

			return 1;
		case OAPI_KEY_C: // Switch rocket camera
			if (VesselStatus == STAGE1 && GroundContact() && GetAttachmentStatus(padAttach) == NULL)
			{
				OBJHANDLE vessel, pad;
				pad = NULL;
				double closestDistance;
				closestDistance = 1e3;
				for (int i = 0; i < (int)oapiGetVesselCount(); i++)
				{
					vessel = oapiGetVesselByIndex(i);
					VECTOR3 dist;
					GetRelativePos(vessel, dist);

					if (oapiGetMass(vessel) > 1e4 && length(dist) < closestDistance && vessel != GetHandle())
					{
						pad = vessel;
						closestDistance = length(dist);
					}

					if (pad != NULL) // found something
					{
						oapiSetFocusObject(pad);
						return 1;
					}
				}
			}
			else if (rocketCam)
			{
				rocketCamMode = (rocketCamMode + 1) % NUM_ROCKET_CAMERA;
				SetCameraOffset(ROCKET_CAMERA_OFFSET[rocketCamMode]);
				SetCameraDefaultDirection(ROCKET_CAMERA_DIRECTION[rocketCamMode]);
				oapiCameraSetCockpitDir(0, 0);
			}

			return 1;
		case OAPI_KEY_J: // Staging
			if (VesselStatus == STAGE1)
			{
				staging1Action = true;
			}
			else if (VesselStatus == STAGE2)
			{
				staging2Action = true;
			}
			else if (VesselStatus == STAGE3)
			{
				staging3Action = true;
			}

			return 1;
		case OAPI_KEY_G: // Fairing
			if (FairingStatus == FAIRINGON)
			{
				separateFairingAction = true;
			}

			return 1;
		case OAPI_KEY_P: // Autolaunch
			if (VesselStatus == STAGE1 && GroundContact())
			{
				oapiSetTimeAcceleration(1.0);
				autopilot = true;
				SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
				launchTime = oapiGetSimTime() + 0.0;
			}
			else if (GetMass() == STAGE4_DRY_MASS)
			{
				sattelitePoweredOn = !sattelitePoweredOn;
			}
			else if (VesselStatus == STAGE3 || stageShutdownTime[1] != NULL)
			{
				oapiSetTimeAcceleration(1.0);
				autopilot = true;
			}

			return 1;
		case OAPI_KEY_M: // Manual / disable autopilot
			autopilot = false;
			if (VesselStatus == STAGE2 || VesselStatus == STAGE3)
			{
				SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 0.0);
				SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0.0);
				SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 0.0);
				SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT, 0.0);
				SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT, 0.0);
				SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT, 0.0);
			}

			return 1;
		}
	}
	return 0;
}

bool ProjectMercury::clbkDrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp)
{
	char cbuf[256];
	int yIndex = 0;
	TextX0 = int(hps->W * 0.025);
	TextY0 = int(hps->H * 0.225);
	LineSpacing = int(hps->H * 0.025);

	if (showInfoOnHud == 0)
	{
		// ==== Buttons text ====
		sprintf(cbuf, "Key commands:");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// Button Ctrl+T
		sprintf(cbuf, "Ctrl+T:Hide on-screen text");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// Key SPACE
		if (VesselStatus == STAGE1)
		{
			if (!spaceLaunch)
			{
				sprintf(cbuf, "SPACE: Engine on");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}

		// Key C
		if (GroundContact())
		{
			sprintf(cbuf, "C:     Set focus to ramp");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (rocketCam)
		{
			sprintf(cbuf, "C:     Swtich camera");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key M
		sprintf(cbuf, "M:     Disengage autopilot");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// Key J
		if (VesselStatus != STAGE4)
		{
			sprintf(cbuf, "J:     Separate stage");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key G
		if (FairingStatus == FAIRINGON)
		{
			sprintf(cbuf, "G:     Separate fairing");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key P
		if (GetMass() == STAGE4_DRY_MASS)
		{
			if (sattelitePoweredOn) sprintf(cbuf, "P:     Power down");
			else sprintf(cbuf, "P:     Power up");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else
		{
			sprintf(cbuf, "P:     Engage autopilot");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
	}

	yIndex = -4;
	double simt = oapiGetSimTime();

	if (showInfoOnHud < 2)
	{
		if (batteryLevel <= 0.0)
		{
			// This sets the x so that the text is centered
			char message[30];
			sprintf(message, "No battery charge");
			int stringWidth = skp->GetTextWidth(message, strlen(message));
			skp->Text((int)floor(hps->W / 2 - stringWidth / 2), TextY0 - 5 * LineSpacing, message, strlen(message));
		}
		else if (!sattelitePoweredOn)
		{
			// This sets the x so that the text is centered
			char message[30];
			sprintf(message, "Powered down");
			int stringWidth = skp->GetTextWidth(message, strlen(message));
			skp->Text((int)floor(hps->W / 2 - stringWidth / 2), TextY0 - 5 * LineSpacing, message, strlen(message));
		}
		else
		{
			// This sets the x so that the text is centered
			int stringWidth = skp->GetTextWidth(contactBase, strlen(contactBase));
			skp->Text((int)floor(hps->W / 2 - stringWidth / 2), TextY0 - 5 * LineSpacing, contactBase, strlen(contactBase));
		}

		double metAbs = simt - launchTime;
		int metSign = 1;
		char signChar[3];
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

		sprintf(cbuf, "T%s%02i %02i %02i", signChar, metH, metM, metS);
		skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		sprintf(cbuf, "Battery level: %.1f %%", batteryLevel / BATTERY_TOTAL_CHARGE * 100.0);
		skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		if (GetMass() != STAGE4_DRY_MASS && sattelitePoweredOn && batteryLevel > 0.0)
		{
			if (autopilot)
			{
				if (VesselStatus == STAGE1 && simt < stageSeparationTime[0] + launchTime)
				{
					sprintf(cbuf, "Stage separation in %.1f s", stageSeparationTime[0] + launchTime - simt);
					skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
				else if (VesselStatus == STAGE2 && simt < stageSeparationTime[1])
				{
					sprintf(cbuf, "Stage separation in %.1f s", stageSeparationTime[1] + launchTime - simt);
					skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
				else if (VesselStatus == STAGE3 && simt < stageSeparationTime[2])
				{
					sprintf(cbuf, "Stage separation in %.1f s", stageSeparationTime[2] + launchTime - simt);
					skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
			}

			// ==== Autopilot status ====
			if (autopilot)
			{
				sprintf(cbuf, "Autopilot: ON");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "Autopilot: OFF");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (FairingStatus == FAIRINGON) sprintf(cbuf, "Fairing: ON");
			else sprintf(cbuf, "Fairing: OFF");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			// Longitudinal acc:
			double m = GetMass();
			VECTOR3 F, W;
			GetForceVector(F);
			GetWeightVector(W);
			double longAcc = (F - W).z / m;
			if (longAcc > 21.0 * G) longAcc = 21.0 * G;
			if (longAcc < -9.0 * G) longAcc = -9.0 * G;
			sprintf(cbuf, "Longitudinal acceleration: %.1f g", longAcc / G);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			// Launch target pitch and yaw
			VECTOR3 angVel;
			GetAngularVel(angVel);

			sprintf(cbuf, "Pitch rate: % .2f\u00B0/s (aim: %.1f\u00B0)", angVel.x * DEG, pitchAim * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Yaw rate: % .2f\u00B0/s", angVel.y * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Roll rate: % .2f\u00B0/s", angVel.z * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  (% .1f RPM)", angVel.z / PI2 * 60.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			if (VesselStatus == STAGE2 && rcs_propellant[1] != NULL)
			{
				sprintf(cbuf, "Attitude fuel: %.1f %%", GetPropellantMass(rcs_propellant[1]) / STAGE2_RCS_FUEL_MASS * 100.0);
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else if (VesselStatus == STAGE3 && rcs_propellant[2] != NULL)
			{
				sprintf(cbuf, "Attitude fuel: %.1f %%", GetPropellantMass(rcs_propellant[2]) / STAGE3_RCS_FUEL_MASS * 100.0);
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			// Nothing more
		}
		else if (sattelitePoweredOn && batteryLevel > 0.0)
		{
			OBJHANDLE planet = GetSurfaceRef();
			OBJHANDLE base = oapiGetBaseByName(planet, "BDA");
			if (base == NULL)
			{
				sprintf(cbuf, "You have not activated Mercury bases. Please");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				sprintf(cbuf, "append the line:");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				sprintf(cbuf, "DIR Earth\\Base\\ProjectMercury CONTEXT MercuryBases");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				sprintf(cbuf, "to the Config\\Earth.cfg file after the");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				sprintf(cbuf, "BEGIN_SURFBASE line and before the DIR Earth\\Base");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				sprintf(cbuf, "line.");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "Station         Acquisition  Loss         ");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				for (int i = 0; i < baseContactLogLength; i++)
				{
					int idx = (i + baseContactLogIdx) % baseContactLogLength;

					if (baseContactLog[idx].x != NULL) // check if something is registered
					{
						double metContact = baseContactLog[idx].y - launchTime;
						if (metContact < 0.0) metContact = 0.0;
						int metHC = (int)floor(metContact / 3600.0);
						int metMC = (int)floor((metContact - metHC * 3600.0) / 60.0);
						int metSC = (int)floor((metContact - metHC * 3600.0 - metMC * 60.0));

						char contactTimeString[30], lossTimeString[30];

						sprintf(contactTimeString, "%02i %02i %02i", metHC, metMC, metSC);

						double metLoss = baseContactLog[idx].z - launchTime;
						int metHL = (int)floor(metLoss / 3600.0);
						int metML = (int)floor((metLoss - metHL * 3600.0) / 60.0);
						int metSL = (int)floor((metLoss - metHL * 3600.0 - metML * 60.0));

						sprintf(lossTimeString, "%02i %02i %02i", metHL, metML, metSL);

						if (baseContactLog[idx].x == 3) // Cape Canaveral
						{
							sprintf(cbuf, "%s  %s     %s", BASE_NAME_LIST[int(baseContactLog[idx].x)], contactTimeString, lossTimeString);
						}
						else // Mercury Tracking Site
						{
							sprintf(cbuf, "%s             %s     %s", BASE_NAME_LIST[int(baseContactLog[idx].x)], contactTimeString, lossTimeString);
						}
						skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
						yIndex += 1;
					}
				}
			}

			// Nothing more
		}
		else
		{
			sprintf(cbuf, "No power");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
	}

	return true;
}

void ProjectMercury::clbkRenderHUD(int mode, const HUDPAINTSPEC* hps, SURFHANDLE hDefaultTex)
{
	OBJHANDLE planetRef = oapiGetGbodyByName("Earth");
	if (planetRef == NULL) planetRef = GetSurfaceRef();
	bool radioContact = InRadioContact(planetRef);

	if (rocketCam)
	{
		return; // i.e. supress HUD. Else let HUD show
	}
	else
	{
		if (MercuryNetwork && (!radioContact || !sattelitePoweredOn || batteryLevel <= 0.0))
		{
			if (oapiGetMFDMode(MFD_LEFT) != MFD_NONE && oapiGetFocusObject() == GetHandle())
			{
				leftMFDwasOn = true;
				oapiToggleMFD_on(MFD_LEFT);
			}

			if (oapiGetMFDMode(MFD_RIGHT) != MFD_NONE && oapiGetFocusObject() == GetHandle())
			{
				rightMFDwasOn = true;
				oapiToggleMFD_on(MFD_RIGHT);
			}
		}
		else
		{
			// Let HUD show
			VESSELVER::clbkRenderHUD(mode, hps, hDefaultTex);

			if (leftMFDwasOn)
			{
				leftMFDwasOn = false;
				oapiToggleMFD_on(MFD_LEFT);
			}

			if (rightMFDwasOn)
			{
				rightMFDwasOn = false;
				oapiToggleMFD_on(MFD_RIGHT);
			}
		}
	}
}

bool ProjectMercury::clbkLoadPanel2D(int id, PANELHANDLE hPanel, DWORD viewW, DWORD viewH)
{
	const DWORD PANEL2D_WIDTH = 2160; // 2160
	const DWORD PANEL2D_HEIGHT = 1440; // 1440
	double defaultScale = (double)viewH / PANEL2D_HEIGHT; // nobody has a < 1 aspect ratio (I hope)
	double	panelScale = max(defaultScale, 1.0);
	SURFHANDLE panelTexture;

	switch (id)
	{
	case 0: // rocket camera
		if (true) // basically in launch, but different states between Redstone and Atlas
		{
			rocketCam = true;
			oldFOV = oapiCameraAperture();

			// Trying to replicate something like this: https://www.youtube.com/watch?v=9JFNE9HoXSw&feature=youtu.be&t=10
			SetCameraOffset(ROCKET_CAMERA_OFFSET[rocketCamMode]);
			SetCameraDefaultDirection(ROCKET_CAMERA_DIRECTION[rocketCamMode]);
			oapiCameraSetCockpitDir(0, 0); // Rotate camera to desired direction
			SetCameraRotationRange(0, 0, 0, 0); // Make camera fixed
			oapiCameraSetAperture(oldFOV);
			if (GroundContact()) oapiSetPanelNeighbours(-1, -1, 0, -1);
			else oapiSetPanelNeighbours(-1, -1, -1, -1);
			SetCameraSceneVisibility(MESHVIS_ALWAYS);

			// Here comes circular frame
			//const DWORD PANEL2D_WIDTH = 2160; // 2160
			//const DWORD PANEL2D_HEIGHT = 1440; // 1440
			panelTexture = oapiGetTextureHandle(circularFrameMesh, 1);

			SetPanelBackground(hPanel, 0, 0, circularFrameMesh, PANEL2D_WIDTH, PANEL2D_HEIGHT, 0UL, PANEL_ATTACH_TOP | PANEL_ATTACH_BOTTOM);

			//double defaultScale = (double)viewH / PANEL2D_HEIGHT; // nobody has a < 1 aspect ratio (I hope)
			//double	panelScale = max(defaultScale, 1.0);
			SetPanelScaling(hPanel, defaultScale, panelScale);
			// End circular frame

			return true;
		}
		else return false;
	default:
		return false;
	}
}

bool ProjectMercury::clbkLoadGenericCockpit(void)
{
	rocketCam = false;
	SetCameraDefaultDirection(_V(0, 0, 1));
	SetCameraOffset(_V(0.0, 0.0, 0.0));
	oapiCameraSetCockpitDir(0, 0); // Rotate camera to desired direction
	SetCameraRotationRange(0.8 * PI, 0.8 * PI, 0.4 * PI, 0.4 * PI); // SetCameraRotationRange-values are the default ones (page 524 in Orbitersdk\doc\API_Reference.pdf
	//oapiCameraSetAperture(oldFOV);
	SetCameraSceneVisibility(MESHVIS_EXTERNAL);
	return true;
}

void ProjectMercury::clbkLoadStateEx(FILEHANDLE scn, void* status)
{
	int i = 0;
	char* cbuf;

	while (oapiReadScenario_nextline(scn, cbuf))
	{
		if (!_strnicmp(cbuf, "STATE", 5))
		{
			sscanf_s(cbuf + 5, "%i", &VesselStatus); // consider changing back to normal sscanf (opposed to sscanf_s)
		}
		else if (!_strnicmp(cbuf, "FAIRING", 7))
		{
			sscanf_s(cbuf + 7, "%i", &FairingStatus);
		}
		else if (!_strnicmp(cbuf, "MET", 3))
		{
			int metTime;
			sscanf_s(cbuf + 3, "%i", &metTime);
			launchTime = oapiGetSimTime() - (double)metTime;
		}
		else if (!_strnicmp(cbuf, "FAILURE", 7)) // input -1 for random failure (or possibly none if HARDMODE is not defined)
		{
			int inputFailure;
			sscanf_s(cbuf + 7, "%i", &inputFailure);
			if (inputFailure == -1)
			{
				inputFailure = rand() % ((int)LASTENTRY - 1) + 1; // this should return a "random" integer between 1 and 12 (or the number of total modes in FailureMode enum)
			}

			FailureMode = (failure)inputFailure;
			// Actual failure determination is done in clbkPostCreation, as we then have result from HARDMODE
		}
		else if (!_strnicmp(cbuf, "HARDMODE", 8))
		{
			difficultyHard = true;
		}
		else ParseScenarioLineEx(cbuf, status);
	}
}

void ProjectMercury::clbkSaveState(FILEHANDLE scn)
{
	VESSELVER::clbkSaveState(scn); // write default parameters (orbital elements etc.)

	oapiWriteScenario_int(scn, "STATE", VesselStatus);

	oapiWriteScenario_int(scn, "FAIRING", FairingStatus);

	if (launchTime != 0.0) // if we have launched using autopilot
	{
		oapiWriteScenario_int(scn, "MET", int(oapiGetSimTime() - launchTime));
	}

	oapiWriteScenario_int(scn, "FAILURE", FailureMode);

	if (difficultyHard)
	{
		oapiWriteScenario_string(scn, "HARDMODE", "");
	}
}

void ProjectMercury::ScoutAutopilot(double met, double simt, double simdt)
{
	SetADCtrlMode(0); // disable adc

	double pitch = GetPitch();

	double yaw = integratedYaw;
	double bank = integratedRoll;

	pitchAim = PI05;
	double pitchRate = 0.0;
	VECTOR3 currentAngRate, currentAngAcc;
	GetAngularVel(currentAngRate);
	GetAngularAcc(currentAngAcc);
	double thrusterAngle = 0.0;
	eulerPitch = 0.0;
	eulerYaw = 0.0;
	bool spinStabilisation = false;
	double ApT = 1e3;

	pitchAim = PitchProgramAim(met) * RAD;

	double pitchdiff = pitchAim - pitch;
	if (pitchdiff > 0.5 * RAD)
		pitchRate = 0.5 * RAD + pitchdiff * 1.0; // guesstimate
	else if (pitchdiff < -0.5 * RAD)
		pitchRate = -0.5 * RAD + pitchdiff * 1.0;
	else
		pitchRate = 0.0;

	double stage3TotMass = STAGE3_TOTAL_MASS + STAGE3_RCS_FUEL_MASS + STAGE4_TOTAL_MASS;

	if (VesselStatus == STAGE2 && GetPropellantMass(scout_propellant[1]) == 0.0) // coasting stage 2
	{
		pitchAim = TargetEndBurnApogeePitch(386e3, STAGE3_ISP * log(stage3TotMass / (stage3TotMass - GetPropellantMass(scout_propellant[2]))));
		double pitchdiff = pitchAim - pitch;

		if (pitchdiff > 0.5 * RAD)
			pitchRate = 0.5 * RAD + pitchdiff * 0.2; // guesstimate
		else if (pitchdiff < -0.5 * RAD)
			pitchRate = -0.5 * RAD + pitchdiff * 0.2;
		else
			pitchRate = 0.0;
	}
	else if (VesselStatus == STAGE3 && stageShutdownTime[2] == NULL) // burning stage 3
	{
		pitchAim = TargetEndBurnApogeePitch(386e3, STAGE3_ISP * log(GetMass() / (GetMass() - GetPropellantMass(scout_propellant[2]))));

		double pitchdiff = pitchAim - pitch;

		if (pitchdiff > 0.5 * RAD)
			pitchRate = 0.5 * RAD + pitchdiff * 0.2; // guesstimate
		else if (pitchdiff < -0.5 * RAD)
			pitchRate = -0.5 * RAD + pitchdiff * 0.2;
		else
			pitchRate = 0.0;
	}
	else if (VesselStatus == STAGE3 || VesselStatus == STAGE4)
	{
		double Ap, Pe;
		OBJHANDLE planet = GetApDist(Ap);
		GetPeDist(Pe);
		double ecc = (Ap - Pe) / (Ap + Pe);
		double SMa = (Ap + Pe) / 2.0;
		double r = GetAltitude() + oapiGetSize(planet);
		double TrA = acos((SMa * (1.0 - ecc * ecc) / r - 1.0) / ecc);
		double pitchOfApogeeAtCurrPos = acos(-sin(TrA));
		pitchAim = PI05 - pitchOfApogeeAtCurrPos;
		spinStabilisation = true;

		double pitchdiff = pitchAim - pitch;

		if (pitchdiff > 0.5 * RAD)
			pitchRate = 0.5 * RAD; // guesstimate
		else if (pitchdiff < -0.5 * RAD)
			pitchRate = -0.5 * RAD;
		else
			pitchRate = 0.0;

		double MnA = TrA2MnA(TrA, ecc);
		ApT = (PI - MnA) * sqrt(pow(SMa, 3.0) / oapiGetMass(planet) / GGRAV);

		stageSeparationTime[2] = met + ApT - STAGE4_BURNTIME / 2.0;
	}


	if (VesselStatus == STAGE1 && met > stageSeparationTime[0] && simt - stageShutdownTime[0] > 5.0 && stageShutdownTime[0] != NULL)
	{
		staging1Action = true;
	}
	else if (VesselStatus == STAGE2 && met > stageSeparationTime[1] && simt - stageShutdownTime[1] > 5.0 && stageShutdownTime[1] != NULL)
	{
		separateFairingAction = true;
		staging2Action = true;
	}
	else if (FairingStatus == FAIRINGON && (VesselStatus == STAGE3 || VesselStatus == STAGE4)) // in case stage is jettisoned manually
	{
		separateFairingAction = true;
	}

	SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);
	double yawDiff = 0.0 - yaw;
	if (simdt > timeStepLimit) yawDiff = -GetSlipAngle() * DEG;
	double yawRate;
	if (yawDiff > 0.1)
		yawRate = 0.67 * RAD;
	else if (yawDiff < -0.1)
		yawRate = -0.67 * RAD;
	else
		yawRate = 0.0;

	if (simdt > timeStepLimit) // force attitude
	{
		eulerPitch = pitchAim - GetPitch();
	}
	else if (VesselStatus == STAGE1) // not time acc, so use organic rocket mechanics
	{
		if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x > pitchRate + 0.0005)
		{
			SetControlSurfaceLevel(AIRCTRL_ELEVATOR, -(currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
		}
		else if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x < pitchRate - 0.0005)
		{
			SetControlSurfaceLevel(AIRCTRL_ELEVATOR, (currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
		}
		else
		{
			SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
		}

		// Yaw
		if (simdt > timeStepLimit) // force attitude
		{
			if (eulerYaw == 0.0) eulerYaw = -yawDiff * RAD;
		}
		else // not time acc, so use organic rocket mechanics
		{
			if (currentAngRate.y > yawRate + 0.0005)
			{
				//thrusterAngle = 0.00075;
				SetControlSurfaceLevel(AIRCTRL_RUDDER, (currentAngRate.y * DEG * currentAngRate.y * DEG * ampFactor + ampAdder));
			}
			else if (currentAngRate.y < yawRate - 0.0005)
			{
				//thrusterAngle = -0.00075;
				SetControlSurfaceLevel(AIRCTRL_RUDDER, -(currentAngRate.y * DEG * currentAngRate.y * DEG * ampFactor + ampAdder));
			}
			else
			{
				SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
			}
		}
	}
	else if (!pitchYawEliminated && (VesselStatus == STAGE2 || VesselStatus == STAGE3))
	{
		if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x > pitchRate + 0.0005)
		{
			SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 1.0); // <-
			SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0.0);
		}
		else if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x < pitchRate - 0.0005)
		{
			SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 0.0);
			SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 1.0); // <-
		}
		else
		{
			SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 0.0);
			SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0.0);
		}

		// Yaw
		if (simdt > timeStepLimit) // force attitude
		{
			if (eulerYaw == 0.0) eulerYaw = -yawDiff * RAD;
		}
		else // not time acc, so use organic rocket mechanics
		{
			if (currentAngRate.y > yawRate + 0.0005)
			{
				SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT, 1.0); // <-
				SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 0.0);
			}
			else if (currentAngRate.y < yawRate - 0.0005)
			{
				SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT, 0.0);
				SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 1.0); // <-
			}
			else
			{
				SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT, 0.0);
				SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 0.0);
			}
		}
	}

	if (simdt > timeStepLimit)
	{
		SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
		SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);
	}
	else
	{
		if (spinStabilisation && !pitchYawEliminated && VesselStatus == STAGE3)
		{

			if (abs(pitch - pitchAim) < 0.25 * RAD)
			{
				if (currentAngRate.x > 0.01 * RAD)
				{
					SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 1.0); // <-
					SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0.0);
				}
				else if (currentAngRate.x < -0.01 * RAD)
				{
					SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 0.0);
					SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 1.0); // <-
				}
				else
				{
					SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 0.0);
					SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0.0);
					SetAngularVel(_V(0, 0, 0));
					pitchYawEliminated = true;
				}
			}
		}
		else if (pitchYawEliminated && VesselStatus == STAGE3)
		{
			SetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN, 0.0);
			SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0.0);
			SetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT, 0.0);
			SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 0.0);

			if (abs(currentAngRate.z) < PI2 * stage4SpinRate / 60.0) // 160 RPM
				SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT, 1.0);
			else
				SetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT, 0.0);

			if (ApT < STAGE4_BURNTIME / 2.0)
			{
				staging3Action = true;
			}
		}
	}

	
}

double ProjectMercury::PitchProgramAim(double met)
{
	// Give a continous pitch aim, for autopilot if time acceleration

	// Solution is for several linear curves
	int i = 1;
	double pitch = initPitch * DEG; // initial pitch
	while (i < pitchRateEntries && met > MET[i])
	{
		pitch += pitchRateAim[i - 1] * (MET[i] - MET[i - 1]);
		i += 1;
	}
	pitch += pitchRateAim[i - 1] * (met - MET[i - 1]);
	return pitch;
}

double ProjectMercury::TargetEndBurnApogeePitch(double targetApogee, double dV)
{
	// Constants
	OBJHANDLE planet = GetSurfaceRef();
	double planetMu = oapiGetMass(planet) * GGRAV;
	double planetRad = oapiGetSize(planet);

	// Current elements
	ELEMENTS el;
	ORBITPARAM prm;
	GetElements(planet, el, &prm, 0, FRAME_EQU);
	double r = planetRad + GetAltitude();
	double E = EccentricAnomaly(el.e, prm.TrA); // Find eccentric anomaly at the set time

	// First get the current state vector. We can do this in orbital frame, as we do not care about inclination.
	VECTOR3 orbitalFramePos = _V(cos(prm.TrA), sin(prm.TrA), 0.0) * r;
	VECTOR3 orbitalFrameVel = _V(-sin(E), sqrt(1.0 - el.e * el.e) * cos(E), 0.0) * sqrt(planetMu * el.a) / r;

	// The pitch at burn
	double pitch = -30.0 * RAD; // arbitrary limit for bad burn

	double postApogee = 0.0;
	targetApogee += planetRad;
	bool badTrajectory = false;

	while (postApogee < targetApogee && pitch < 30.0 * RAD)
	{
		// Then add the burn dV to the state velocity
		VECTOR3 posUnit = unit(orbitalFramePos);
		double rotAngle = PI05 - pitch;
		VECTOR3 dVvector = _V(cos(rotAngle) * posUnit.x - sin(rotAngle) * posUnit.y, sin(rotAngle) * posUnit.x + cos(rotAngle) * posUnit.y, 0.0) * dV;
		VECTOR3 postVel = orbitalFrameVel + dVvector;
		VECTOR3 postPos = orbitalFramePos;

		// Now find the resulting apogee altitude
		double postBurnR = length(postPos);
		double postBurnV = length(postVel);
		double postBurnRadVel = dotp(postPos, postVel) / postBurnR;
		VECTOR3 postBurnEccV = (postPos * (postBurnV * postBurnV - planetMu / postBurnR) - postVel * postBurnR * postBurnRadVel) / planetMu;
		double postBurnEcc = length(postBurnEccV);
		double postBurnEnergy = length2(postVel) / 2.0 - planetMu / length(postPos);
		double postBurnSMa = -planetMu / (2.0 * postBurnEnergy);

		postApogee = postBurnSMa * (1.0 - postBurnEcc * postBurnEcc) / (1.0 - postBurnEcc);

		pitch += 0.5 * RAD;

		double postFlightAngle = PI05 - acos(dotp(unit(postVel), unit(postPos)));
		if ((r < targetApogee && postFlightAngle < 0.0) || (r > targetApogee&& postFlightAngle > 0.0))
		{
			badTrajectory = true;
		}
		else badTrajectory = false;

	}

	return pitch;
}

void ProjectMercury::Staging(int StageToSeparate)
{
	if (StageToSeparate == 1 && VesselStatus == STAGE1)
	{
		SeparateStage(StageToSeparate);

		CreateThrusterGroup(&th_main[StageToSeparate], 1, THGROUP_MAIN);
		SetDefaultPropellantResource(scout_propellant[StageToSeparate]);

		VesselStatus = STAGE2;
		CGshifted = false;

		SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
	}
	else if (StageToSeparate == 2 && VesselStatus == STAGE2)
	{
		SeparateStage(StageToSeparate);

		CreateThrusterGroup(&th_main[StageToSeparate], 1, THGROUP_MAIN);
		SetDefaultPropellantResource(scout_propellant[StageToSeparate]);

		VesselStatus = STAGE3;
		CGshifted = false;

		SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
	}
	else if (StageToSeparate == 3 && VesselStatus == STAGE3)
	{
		SeparateStage(StageToSeparate);

		CreateThrusterGroup(&th_main[StageToSeparate], 1, THGROUP_MAIN);
		SetDefaultPropellantResource(scout_propellant[StageToSeparate]);

		VesselStatus = STAGE4;
		CGshifted = false;

		SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
	}
}

void ProjectMercury::SeparateStage(int stageNum)
{
	if (stageNum < 1 || stageNum > 3)
	{
		oapiWriteLog("Error in staging, stage number out of range!");
		return;
	}

	VESSELSTATUS2 vs;
	VESSELSTATUS2::FUELSPEC fuel;
	VESSELSTATUS2::THRUSTSPEC thrust;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	GetStatusEx(&vs);
	char name[256];

	vs.flag = VS_FUELLIST;
	vs.fuel = &fuel;
	vs.nfuel = 1;
	vs.fuel->idx = 0;
	vs.fuel->level = GetFuelMass() / GetMaxFuelMass();

	if (GetThrusterLevel(th_main[stageNum - 1]) != 0.0) // Keep engine running
	{
		vs.flag = VS_THRUSTLIST;
		vs.thruster = &thrust;
		vs.nthruster = 1;
		vs.thruster->idx = 0;
		vs.thruster->level = 1.0;
	}

	strcpy(name, GetName());

	if (stageNum == 1)
	{
		Local2Rel(_V(0,0,0), vs.rpos);
		strcat(name, " Stage 1");
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Scout\\Stage1", &vs);
	}
	else if (stageNum == 2)
	{
		Local2Rel(_V(0,0,0), vs.rpos);
		strcat(name, " Stage 2");
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Scout\\Stage2", &vs);
		DelMesh(Skirt[0]);
	}
	else if (stageNum == 3)
	{
		Local2Rel(_V(0,0,0), vs.rpos);
		strcat(name, " Stage 3");
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Scout\\Stage3", &vs);
		DelMesh(Skirt[1]);
		DelMesh(Skirt[2]);
	}

	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;

	// The thrusterlist spec doesnt seem to work, sadly
	VESSEL* v = oapiGetVesselInterface(createdVessel[stuffCreated - 1]);
	double levl = GetPropellantMass(scout_propellant[stageNum - 1]);
	v->SetPropellantMass(v->GetPropellantHandleByIndex(0), levl);
	if (GetThrusterLevel(th_main[stageNum - 1]) != 0.0)
	{
		v->SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
	}

	DelThruster(th_main[stageNum - 1]);
	DelControlSurface(Rudders[0]);
	DelControlSurface(Rudders[1]);
	DelControlSurface(Rudders[2]);
	DelControlSurface(Rudders[3]);
	DelPropellantResource(scout_propellant[stageNum - 1]);

	if (stageNum == 2 || stageNum == 3)
	{
		DelThruster(th_rcs[0]);
		DelThruster(th_rcs[1]);
		DelThruster(th_rcs[2]);
		DelThruster(th_rcs[3]);
		DelThruster(th_rcs[4]);
		DelThruster(th_rcs[5]);
		DelThruster(th_rcs[6]);
		DelThruster(th_rcs[7]);
		DelPropellantResource(rcs_propellant[stageNum - 1]);
	}

	DelMesh(Stage[stageNum - 1]);
}

void ProjectMercury::SeparateFairing(void)
{
	if (FairingStatus == FAIRINGON)
	{
		VESSELSTATUS2 vs;
		vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
		vs.version = 2;

		double fairingSepSpeed = 1.0;
		VECTOR3 offset;
		switch (VesselStatus)
		{
		case STAGE1:
			offset = FAIRING_OFFSET - STAGE1_OFFSET;
			break;
		case STAGE2:
			offset = FAIRING_OFFSET - STAGE2_OFFSET;
			break;
		case STAGE3:
			offset = FAIRING_OFFSET - STAGE3_OFFSET;
			break;
		case STAGE4:
			offset = FAIRING_OFFSET - STAGE4_OFFSET;
			break;
		default:
			offset = FAIRING_OFFSET;
			break;
		}

		char name[256];
		VECTOR3 vel = _V(-fairingSepSpeed, 0, 0);
		VECTOR3 relativeOffset1;
		GetStatusEx(&vs);
		Local2Rel(offset, vs.rpos);
		VECTOR3 vel2;
		GlobalRot(vel, vel2);
		vs.rvel += vel2;
		strcpy(name, GetName());
		strcat(name, " Fairing 1");
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Scout\\Fairing1", &vs);
		if (GetAltitude() > 5e4)
			createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(Fairing[0]);

		vel = _V(fairingSepSpeed, 0.0, 0);
		VECTOR3 relativeOffset2;
		GetStatusEx(&vs);
		Local2Rel(FlipX(offset), vs.rpos);
		GlobalRot(vel, vel2);
		vs.rvel += vel2;
		strcpy(name, GetName());
		strcat(name, " Fairing 2");
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Scout\\Fairing2", &vs);
		if (GetAltitude() > 5e4)
			createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(Fairing[1]);

		FairingStatus = FAIRINGOFF;
	}
}

void ProjectMercury::CreateStage2RCS(void)
{
	rcs_propellant[1] = CreatePropellantResource(STAGE2_RCS_FUEL_MASS);
	th_rcs[0] = CreateThruster(STAGE2_RCS_PD_POS, STAGE2_RCS_PD_DIR, STAGE2_RCS_THRUST, rcs_propellant[1], STAGE2_RCS_ISP);
	th_rcs[1] = CreateThruster(STAGE2_RCS_PU_POS, STAGE2_RCS_PU_DIR, STAGE2_RCS_THRUST, rcs_propellant[1], STAGE2_RCS_ISP);
	th_rcs[2] = CreateThruster(STAGE2_RCS_YL_POS, STAGE2_RCS_YL_DIR, STAGE2_RCS_THRUST, rcs_propellant[1], STAGE2_RCS_ISP);
	th_rcs[3] = CreateThruster(STAGE2_RCS_YR_POS, STAGE2_RCS_YR_DIR, STAGE2_RCS_THRUST, rcs_propellant[1], STAGE2_RCS_ISP);
	th_rcs[4] = CreateThruster(STAGE2_RCS_RUL_POS, STAGE2_RCS_RUL_DIR, STAGE2_RCS_THRUST_ROLL, rcs_propellant[1], STAGE2_RCS_ISP);
	th_rcs[5] = CreateThruster(STAGE2_RCS_RDL_POS, STAGE2_RCS_RDL_DIR, STAGE2_RCS_THRUST_ROLL, rcs_propellant[1], STAGE2_RCS_ISP);
	th_rcs[6] = CreateThruster(STAGE2_RCS_RUR_POS, STAGE2_RCS_RUR_DIR, STAGE2_RCS_THRUST_ROLL, rcs_propellant[1], STAGE2_RCS_ISP);
	th_rcs[7] = CreateThruster(STAGE2_RCS_RDR_POS, STAGE2_RCS_RDR_DIR, STAGE2_RCS_THRUST_ROLL, rcs_propellant[1], STAGE2_RCS_ISP);
	THRUSTER_HANDLE rl[2] = { th_rcs[4], th_rcs[5] };
	THRUSTER_HANDLE rr[2] = { th_rcs[6], th_rcs[7] };
	CreateThrusterGroup(&th_rcs[0], 1, THGROUP_ATT_PITCHUP);
	CreateThrusterGroup(&th_rcs[1], 1, THGROUP_ATT_PITCHDOWN);
	CreateThrusterGroup(&th_rcs[2], 1, THGROUP_ATT_YAWLEFT);
	CreateThrusterGroup(&th_rcs[3], 1, THGROUP_ATT_YAWRIGHT);
	CreateThrusterGroup(rl, 2, THGROUP_ATT_BANKLEFT);
	CreateThrusterGroup(rr, 2, THGROUP_ATT_BANKRIGHT);

	exhaustRCS = {
		0, 0.05, 20, 1, 0.01, 0.1, 1, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1
	};

	rcsStream[0] = AddExhaustStream(th_rcs[0], &exhaustRCS);
	rcsStream[1] = AddExhaustStream(th_rcs[1], &exhaustRCS);
	rcsStream[2] = AddExhaustStream(th_rcs[2], &exhaustRCS);
	rcsStream[3] = AddExhaustStream(th_rcs[3], &exhaustRCS);
	rcsStream[4] = AddExhaustStream(th_rcs[4], &exhaustRCS);
	rcsStream[5] = AddExhaustStream(th_rcs[5], &exhaustRCS);
	rcsStream[6] = AddExhaustStream(th_rcs[6], &exhaustRCS);
	rcsStream[7] = AddExhaustStream(th_rcs[7], &exhaustRCS);
}

void ProjectMercury::CreateStage3RCS(void)
{
	rcs_propellant[2] = CreatePropellantResource(STAGE3_RCS_FUEL_MASS);
	th_rcs[0] = CreateThruster(STAGE3_RCS_PD_POS, STAGE3_RCS_PD_DIR, STAGE3_RCS_THRUST, rcs_propellant[2], STAGE3_RCS_ISP);
	th_rcs[1] = CreateThruster(STAGE3_RCS_PU_POS, STAGE3_RCS_PU_DIR, STAGE3_RCS_THRUST, rcs_propellant[2], STAGE3_RCS_ISP);
	th_rcs[2] = CreateThruster(STAGE3_RCS_YL_POS, STAGE3_RCS_YL_DIR, STAGE3_RCS_THRUST, rcs_propellant[2], STAGE3_RCS_ISP);
	th_rcs[3] = CreateThruster(STAGE3_RCS_YR_POS, STAGE3_RCS_YR_DIR, STAGE3_RCS_THRUST, rcs_propellant[2], STAGE3_RCS_ISP);
	th_rcs[4] = CreateThruster(STAGE3_RCS_RUL_POS, STAGE3_RCS_RUL_DIR, STAGE3_RCS_THRUST_ROLL, rcs_propellant[2], STAGE3_RCS_ISP);
	th_rcs[5] = CreateThruster(STAGE3_RCS_RDL_POS, STAGE3_RCS_RDL_DIR, STAGE3_RCS_THRUST_ROLL, rcs_propellant[2], STAGE3_RCS_ISP);
	th_rcs[6] = CreateThruster(STAGE3_RCS_RUR_POS, STAGE3_RCS_RUR_DIR, STAGE3_RCS_THRUST_ROLL, rcs_propellant[2], STAGE3_RCS_ISP);
	th_rcs[7] = CreateThruster(STAGE3_RCS_RDR_POS, STAGE3_RCS_RDR_DIR, STAGE3_RCS_THRUST_ROLL, rcs_propellant[2], STAGE3_RCS_ISP);
	THRUSTER_HANDLE rl[2] = { th_rcs[4], th_rcs[5] };
	THRUSTER_HANDLE rr[2] = { th_rcs[6], th_rcs[7] };
	CreateThrusterGroup(&th_rcs[0], 1, THGROUP_ATT_PITCHUP);
	CreateThrusterGroup(&th_rcs[1], 1, THGROUP_ATT_PITCHDOWN);
	CreateThrusterGroup(&th_rcs[2], 1, THGROUP_ATT_YAWLEFT);
	CreateThrusterGroup(&th_rcs[3], 1, THGROUP_ATT_YAWRIGHT);
	CreateThrusterGroup(rl, 2, THGROUP_ATT_BANKLEFT);
	CreateThrusterGroup(rr, 2, THGROUP_ATT_BANKRIGHT);

	exhaustRCS = {
		0, 0.05, 20, 1, 0.01, 0.1, 1, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1
	};

	rcsStream[0] = AddExhaustStream(th_rcs[0], &exhaustRCS);
	rcsStream[1] = AddExhaustStream(th_rcs[1], &exhaustRCS);
	rcsStream[2] = AddExhaustStream(th_rcs[2], &exhaustRCS);
	rcsStream[3] = AddExhaustStream(th_rcs[3], &exhaustRCS);
	rcsStream[4] = AddExhaustStream(th_rcs[4], &exhaustRCS);
	rcsStream[5] = AddExhaustStream(th_rcs[5], &exhaustRCS);
	rcsStream[6] = AddExhaustStream(th_rcs[6], &exhaustRCS);
	rcsStream[7] = AddExhaustStream(th_rcs[7], &exhaustRCS);
}

void ProjectMercury::BaseContactManager(void)
{
	double simt = oapiGetSimTime();

	OBJHANDLE planet = GetSurfaceRef();
	OBJHANDLE base;

	for (int i = 0; i < NUMBASES; i++)
	{
		base = oapiGetBaseByName(planet, BASE_NAME_LIST[i]);
		if (base != NULL) // sanity check
		{
			double baseLongitude, baseLatitude, baseRadius;
			oapiGetBaseEquPos(base, &baseLongitude, &baseLatitude, &baseRadius);

			VECTOR3 capsuleBase, capsulePlanet;
			GetRelativePos(base, capsuleBase);
			double distToBase2 = length2(capsuleBase);
			GetRelativePos(planet, capsulePlanet);
			double radius2 = length2(capsulePlanet);

			// Simple extention of Pythagoras. 64e8 is 8e4^2. So within 80 km, we have contact. Useful for launchpad situations and landings.
			// Even if outside visual range, we can still be within radio contact. 80 km limit is arbitrary.
			// Hide landing sites and contingency sites until after reentry. Arbitrarily set finished reentry at sub 35 km (still in visible plasma at 40 km).
			if (distToBase2 < 64e8 || distToBase2 < radius2 - baseRadius * baseRadius)
			{
				if (!baseInContact[i]) // just appeared
				{
					baseContactTime[i][baseContactsNum[i]] = simt;
				}

				baseInContact[i] = true;
			}
			else if (baseInContact[i])
			{
				baseLossTime[i][baseContactsNum[i]] = simt;

				if (sattelitePoweredOn)
				{
					baseContactLog[baseContactLogIdx] = { double(i), baseContactTime[i][baseContactsNum[i]], baseLossTime[i][baseContactsNum[i]] }; // Conversion from int to double, and then back to int later. Hope this goes well.
					baseContactLogIdx = (baseContactLogIdx + 1) % baseContactLogLength;
				}

				baseContactsNum[i] += 1;
				baseInContact[i] = false;
			}
		}
	}
}

void ProjectMercury::CreateAirfoilsScout(void)
{
	ClearAirfoilDefinitions(); // delete previous airfoils
	ClearControlSurfaceDefinitions();
	SetRotDrag(_V(0.25, 0.25, 0.1)); // from BrianJ's Falcon 9
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 0.1), vliftScout, NULL, STAGE1_DIAMETER, STAGE1_DIAMETER * STAGE1_DIAMETER * PI / 4.0, 1.0);
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 0.1), hliftScout, NULL, STAGE1_DIAMETER, STAGE1_DIAMETER * STAGE1_DIAMETER * PI / 4.0, 1.0); // spherical symmetric
}

double ProjectMercury::EmptyMass(void)
{
	double emptyMass = 0.0;
	
	if (VesselStatus == STAGE1) emptyMass = STAGE1_DRY_MASS + STAGE2_DRY_MASS + STAGE3_DRY_MASS + STAGE4_DRY_MASS;
	else if (VesselStatus == STAGE2) emptyMass = STAGE2_DRY_MASS + STAGE3_DRY_MASS + STAGE4_DRY_MASS;
	else if (VesselStatus == STAGE3) emptyMass = STAGE3_DRY_MASS + STAGE4_DRY_MASS;
	else emptyMass = STAGE4_DRY_MASS;

	if (FairingStatus == FAIRINGON) emptyMass += FAIRING_DRY_MASS * 2.0;

	return emptyMass;

}

void ProjectMercury::SetCameraSceneVisibility(WORD mode)
{
	if (mode == MESHVIS_ALWAYS)
	{
		oapiWriteLogV("Set ALWAYS, status: %i", VesselStatus);
	}
	else if (mode == MESHVIS_EXTERNAL)
	{
		oapiWriteLogV("Set EXTERNAL, status: %i", VesselStatus);
	}
	else
	{
		oapiWriteLogV("Unknown MESHVIS, status: %i", VesselStatus);
	}

	if (VesselStatus == STAGE1 && GetMeshVisibilityMode(Stage[0]) != mode) SetMeshVisibilityMode(Stage[0], mode);
	if ((VesselStatus != STAGE3 && VesselStatus != STAGE4) && GetMeshVisibilityMode(Stage[1]) != mode) SetMeshVisibilityMode(Stage[1], mode);
	if ((VesselStatus != STAGE4) && GetMeshVisibilityMode(Stage[2]) != mode) SetMeshVisibilityMode(Stage[2], mode);
	if (GetMeshVisibilityMode(Stage[3]) != mode) SetMeshVisibilityMode(Stage[3], mode);

	if (FairingStatus == FAIRINGON)
	{
		if (GetMeshVisibilityMode(Fairing[0]) != mode) SetMeshVisibilityMode(Fairing[0], mode);
		if (GetMeshVisibilityMode(Fairing[1]) != mode) SetMeshVisibilityMode(Fairing[1], mode);
	}
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
	//oapiCloseFile(pitchDataLogFile, FILE_OUT);
	if (vessel) delete (ProjectMercury*)vessel;
}

inline void ProjectMercury::vliftScout(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	static const double step = RAD * 22.5;
	static const double istep = 1.0 / step;
	static const int nabsc = 17;
	static const double CL[nabsc] = { 0, 0.1, 0.2, 0.1, 0, 0.1, 0.2, 0.1, 0, -0.1, -0.2, -0.1, 0, -0.1, -0.2, -0.1, 0 };

	aoa += PI;
	int idx = max(0, min(15, (int)(aoa * istep)));
	double d = aoa * istep - idx;
	*cl = CL[idx] + (CL[idx + 1] - CL[idx]) * d;
	*cm = 0.0; // Autopilot can't handle a pitching moment.

	// Cd from 19620003288 page 241
	static const int numMach = 14;
	static const  double mach[numMach] = {
		0.00, 0.50, 0.75, 0.90, 1.00, 1.10, 1.20, 1.30, 1.40, 1.50, 2.00, 2.50, 3.50, 5.00
	};
	static const double cdp[numMach] = { // drag coeff at 0 AoA (tip first) for different mach numbers.
		0.24, 0.29, 0.35, 0.41, 0.49, 0.63, 0.62, 0.60, 0.58, 0.56, 0.48, 0.41, 0.33, 0.31
	};

	int i = 0;
	while (i < numMach && M > mach[i])
	{
		i++;
	}

	if (i == numMach)
	{
		*cd = cdp[numMach - 1];
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

void ProjectMercury::hliftScout(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
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

	// Cd from 19620003288 page 241
	static const int numMach = 14;
	static const  double mach[numMach] = {
		0.00, 0.50, 0.75, 0.90, 1.00, 1.10, 1.20, 1.30, 1.40, 1.50, 2.00, 2.50, 3.50, 5.00
	};
	static const double cdp[numMach] = { // drag coeff at 0 AoA (tip first) for different mach numbers.
		0.24, 0.29, 0.35, 0.41, 0.49, 0.63, 0.62, 0.60, 0.58, 0.56, 0.48, 0.41, 0.33, 0.31
	};

	int i = 0;
	while (i < numMach && M > mach[i])
	{
		i++;
	}

	if (i == numMach)
	{
		*cd = cdp[numMach - 1];
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

