#pragma once

// ==============================================================
//		Header file for Mercury Capsule Virtual Cockpit.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2020
// 
// This code is my own work.
// 
// Thank you to Rob Conley and the rest of the original Project
// Mercury team. Especially thank you for releasing the source.
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================

const VECTOR3 VC_INTERNAL_OFS = _V(0.0, 0.5, -0.8);
const VECTOR3 VC_FRAME_OFS = _V(0.0, -1.5, 5.0) + VC_INTERNAL_OFS;

// Create panel ID's as enum, as we don't care about the value, only that it's an unique ID. This way, they get autoassigned in increasing order. We don't need any variable.
const enum PANEL_ID { PANEL_ID_PERISCOPE_FILTER_SWITCH , PANEL_ID_LOAD_RESET, 
PANEL_ID_TOWJET_BUTTON, PANEL_ID_CAPSEP_BUTTON, PANEL_ID_RETSEQ_BUTTON, PANEL_ID_RETJET_BUTTON, PANEL_ID_TIMZER_BUTTON,
// Physical switches:
PANEL_ID_AUTORETJET_SWITCH_L, PANEL_ID_AUTORETJET_SWITCH_R,
PANEL_ID_RETDELAY_SWITCH_L, PANEL_ID_RETDELAY_SWITCH_R,
PANEL_ID_RETATT_SWITCH_L, PANEL_ID_RETATT_SWITCH_R,
PANEL_ID_ASCSMODE_SWITCH_L, PANEL_ID_ASCSMODE_SWITCH_R,
PANEL_ID_CNTRLMODE_SWITCH_L, PANEL_ID_CNTRLMODE_SWITCH_R,
// THandles:
PANEL_ID_MANUAL_THANDLE, PANEL_ID_ROLL_THANDLE, PANEL_ID_YAW_THANDLE, PANEL_ID_PITCH_THANDLE,
// Fireflies
PANEL_ID_FIREFLY
};

//
//bool ProjectMercury::clbkLoadVC(int id)
//{
//	periscope = false; // We are not there now
//	DelMesh(PeriscopeFilter); // So that it doesn't appear in cockpit
//
//	// Default cockpit values
//	SetCameraDefaultDirection(_V(0, 0, 1)); // forward
//	SetCameraOffset(VC_INTERNAL_OFS); // Adjust to fit to eyes (slightly above y = 0). And maybe check z. And ensure that this works also after ShiftCG()
//	SetCameraRotationRange(80.0 * RAD, 80.0 * RAD, 60.0 * RAD, 60.0 * RAD); // adjust to fit to what is visible inside helmet
//	oapiCameraSetCockpitDir(0, 0); // Rotate camera to desired direction
//	oapiCameraSetAperture(oldFOV);
//
//	sprintf(oapiDebugString(), "We're in VC %.2f. Mesh is %i", oapiGetSimTime(), VcFrame);
//
//
//	return true;
//}

bool ProjectMercury::clbkLoadPanel2D(int id, PANELHANDLE hPanel, DWORD viewW, DWORD viewH)
{
	if (oapiGetOrbiterVersion() < 100831)
	{
		oapiWriteLog("Sorry, panel, periscope and rocket cameras are disabled in Orbiter 2010 and earlier, as the views give crash to desktop. I'll try to find a solution.");
		return false; // Don't support periscope as for now, as it leads to a crash
	}

	const DWORD PANEL2D_WIDTH = 2160; // 2160
	const DWORD PANEL2D_HEIGHT = 1440; // 1440
	double defaultScale = (double)viewH / PANEL2D_HEIGHT; // nobody has a < 1 aspect ratio (I hope)
	double	panelScale = max(defaultScale, 1.0);

	// Button coordinates for click actions
	const int loadX = 190, loadY = 850;
	const int butX = 45, towjetY = 150, capsepY = 240, retseqY = 330, retjetY = 600;
	const int butRad = 30;
	const int autRetJet[2] = { 2060, 100 };
	const int retDel[2] = { 2060, 200 };
	const int retAtt[2] = { 2060, 300 };
	const int ASCSmde[2] = { 830, 1070 };
	const int ctrlMde[2] = { 830, 1170 };
	const int tHandleManual[2] = { 1330, 1140 };
	const int tHandleRoll[2] = { 830, 1340 };
	const int tHandleYaw[2] = { 1080, 1340 };
	const int tHandlePitch[2] = { 1330, 1340 };
	//const int switchX = 2060, fuelsrcY = 100, autostateY = 200, thrusterY = 300;

	if (id == 1 && (VesselStatus != FLIGHT && VesselStatus != REENTRY && VesselStatus != REENTRYNODROGUE && !(VesselStatus == LAUNCH && GroundContact()))) // must be in capsule mode
	{
		id = 2; // send to rocket cam
	}
	else if (id == 1 && (CapsuleVersion == FAITH7 || CapsuleVersion == FREEDOM7II || CapsuleVersion == CAPSULEBIGJOE || CapsuleVersion == CAPSULELITTLEJOE || CapsuleVersion == CAPSULEBD)) // these two did not feature a periscope
	{
		oapiWriteLog("You are trying to access the periscope, but Faith 7 and Freedom 7II (and boilerplates) did not include it in their capsules. Therefore it's not supported.");

		if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) // no rocket cam
			id = 0; // Did not include periscope, send back to panel
		else
			id = 2; // Did not include periscope, send to rocket cam
	}

	if (id == 2 && (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO))
	{
		id = 0; // We don't have any rocket cam, and therefore send to panel
	}

	switch (id)
	{
	case 0: // instrument panel
		// Just some camera specs. Not strictly neccessary.
		periscope = false;
		rocketCam = false;
		panelView = true;
		SetCameraDefaultDirection(_V(0, 0, 1));
		SetCameraOffset(_V(0.0, 0.0, 0.0));
		oapiCameraSetCockpitDir(0, 0); // Rotate camera to desired direction
		SetCameraRotationRange(0.8 * PI, 0.8 * PI, 0.4 * PI, 0.4 * PI); // SetCameraRotationRange-values are the default ones (page 524 in Orbitersdk\doc\API_Reference.pdf
		oapiCameraSetAperture(oldFOV);
		DelMesh(PeriscopeFilter); // So that it doesn't appear

		// Set neighbours
		if (CapsuleVersion == FAITH7 || CapsuleVersion == FREEDOM7II || CapsuleVersion == CAPSULEBIGJOE || CapsuleVersion == CAPSULELITTLEJOE || CapsuleVersion == CAPSULEBD)
			oapiSetPanelNeighbours(-1, -1, -1, 2);
		else
			oapiSetPanelNeighbours(-1, -1, -1, 1);

		// Here comes the panel
		SetPanelBackground(hPanel, 0, 0, cockpitPanelMesh, PANEL2D_WIDTH, PANEL2D_HEIGHT, 0UL, PANEL_ATTACH_TOP | PANEL_ATTACH_BOTTOM);

		SetPanelScaling(hPanel, defaultScale, panelScale);

		// Click to reset load indicator
		VersionDependentPanelClick(PANEL_ID_LOAD_RESET, _R(loadX - 150, loadY - 150, loadX + 150, loadY + 150), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);

		// Click to set TowJet, CapSep, RetSeq, RetJet.
		VersionDependentPanelClick(PANEL_ID_TOWJET_BUTTON, _R(butX - butRad, towjetY - butRad, butX + butRad, towjetY + butRad), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN | PANEL_MOUSE_LBPRESSED, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_CAPSEP_BUTTON, _R(butX - butRad, capsepY - butRad, butX + butRad, capsepY + butRad), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN | PANEL_MOUSE_LBPRESSED, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_RETSEQ_BUTTON, _R(butX - butRad, retseqY - butRad, butX + butRad, retseqY + butRad), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN | PANEL_MOUSE_LBPRESSED, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_RETJET_BUTTON, _R(butX - butRad, retjetY - butRad, butX + butRad, retjetY + butRad), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN | PANEL_MOUSE_LBPRESSED, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_TIMZER_BUTTON, _R(1875 - butRad, 340 - butRad, 1875 + butRad, 340 + butRad), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN | PANEL_MOUSE_LBPRESSED, hPanel, _R(0, 0, 0, 0), NULL);

		// Click to set swtiches. Left box and right box
		CreatePanelSwitchClick(PANEL_ID_AUTORETJET_SWITCH_L, PANEL_ID_AUTORETJET_SWITCH_R, autRetJet[0], autRetJet[1], hPanel);
		CreatePanelSwitchClick(PANEL_ID_RETDELAY_SWITCH_L, PANEL_ID_RETDELAY_SWITCH_R, retDel[0], retDel[1], hPanel);
		CreatePanelSwitchClick(PANEL_ID_RETATT_SWITCH_L, PANEL_ID_RETATT_SWITCH_R, retAtt[0], retAtt[1], hPanel);
		CreatePanelSwitchClick(PANEL_ID_ASCSMODE_SWITCH_L, PANEL_ID_ASCSMODE_SWITCH_R, ASCSmde[0], ASCSmde[1], hPanel);
		CreatePanelSwitchClick(PANEL_ID_CNTRLMODE_SWITCH_L, PANEL_ID_CNTRLMODE_SWITCH_R, ctrlMde[0], ctrlMde[1], hPanel);
		/*VersionDependentPanelClick(PANEL_ID_AUTORETJET_SWITCH_L, _R(autRetJet[0] - switchW / 2, autRetJet[1] - switchH / 2, autRetJet[0], autRetJet[1] + switchH / 2), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_AUTORETJET_SWITCH_R, _R(autRetJet[0], autRetJet[1] - switchH / 2, autRetJet[0] + switchW / 2, autRetJet[1] + switchH / 2), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_RETDELAY_SWITCH_L, _R(retDel[0] - switchW / 2, retDel[1] - switchH / 2, retDel[0], retDel[1] + switchH / 2), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_RETDELAY_SWITCH_R, _R(retDel[0], retDel[1] - switchH / 2, retDel[0] + switchW / 2, retDel[1] + switchH / 2), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_THRUSTER_SWITCH_L, _R(switchX - switchW / 2, thrusterY - switchH / 2, switchX, thrusterY + switchH / 2), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
		VersionDependentPanelClick(PANEL_ID_THRUSTER_SWITCH_R, _R(switchX, thrusterY - switchH / 2, switchX + switchW / 2, thrusterY + switchH / 2), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);*/

		// Click left mouse to push, click right mouse to push, T handles
		CreatePanelTHandleClick(PANEL_ID_MANUAL_THANDLE, tHandleManual[0], tHandleManual[1], hPanel);
		CreatePanelTHandleClick(PANEL_ID_ROLL_THANDLE, tHandleRoll[0], tHandleRoll[1], hPanel);
		CreatePanelTHandleClick(PANEL_ID_YAW_THANDLE, tHandleYaw[0], tHandleYaw[1], hPanel);
		CreatePanelTHandleClick(PANEL_ID_PITCH_THANDLE, tHandlePitch[0], tHandlePitch[1], hPanel);

		VersionDependentPanelClick(PANEL_ID_FIREFLY, _R(1080 - 250, 325 - 325, 1080 + 250, 325 + 325), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
		
		return true; // always available
	case 1: // periscope
		if (VesselStatus != FLIGHT && VesselStatus != REENTRY && VesselStatus != REENTRYNODROGUE && !(VesselStatus == LAUNCH && GroundContact())) // must be in capsule mode
		{
			return false;
		}
		else if (CapsuleVersion == FAITH7 || CapsuleVersion == FREEDOM7II || CapsuleVersion == CAPSULEBIGJOE || CapsuleVersion == CAPSULELITTLEJOE || CapsuleVersion == CAPSULEBD) // these two did not feature a periscope
		{
			oapiWriteLog("You are trying to access the periscope, but Faith 7 and Freedom 7II (and boilerplates) did not include it in their capsules. Therefore it's not supported.");
			return false;
		}

		// Here we implement the periscope. F8 to get to periscope, and no on screen information. Seems reasonable

		// First all the FOV and other basic stuff
		periscope = true;
		rocketCam = false;
		panelView = false;
		oldFOV = oapiCameraAperture();

		SetCameraDefaultDirection(CAMERA_DIRECTION);
		SetCameraOffset(CAMERA_OFFSET); // very approximate. Only important that it is above the water level after landing
		oapiCameraSetCockpitDir(0, 0); // Rotate camera to desired direction
		SetCameraRotationRange(0, 0, 0, 0); // Make camera fixed

		if (GroundContact()) oapiSetPanelNeighbours(-1, -1, 0, 2);
		else oapiSetPanelNeighbours(-1, -1, 0, -1);

		if (oapiCameraInternal())
		{
			if (narrowField)
			{
				oapiCameraSetAperture(PERISCOPE_NARROW);
			}
			else
			{
				oapiCameraSetAperture(PERISCOPE_WIDE);
			}
		}
		else
		{
			oapiWriteLog("We're in 2D panel, but in external view. How strange.");
		}
		// End FOV and other basic stuff

		// Here comes periscope indicators
		SetPanelBackground(hPanel, 0, 0, periscopeMesh, PANEL2D_WIDTH, PANEL2D_HEIGHT, 0UL, PANEL_ATTACH_TOP | PANEL_ATTACH_BOTTOM);

		SetPanelScaling(hPanel, defaultScale, panelScale);
		// End periscope indicators

		// And here finally comes the filters (clear, red, yellow, medium neutral density). MercuryFamiliarizationManual20May1962 page 365
		VECTOR3 filterOffset = CAMERA_OFFSET + CAMERA_DIRECTION;
		switch (CurrentFilter)
		{
		case CLEAR:
			DelMesh(PeriscopeFilter);
			break;
		case RED:
			DelMesh(PeriscopeFilter);
			PeriscopeFilter = AddMesh(periscopeFilterRed, &filterOffset);
			SetMeshVisibilityMode(PeriscopeFilter, MESHVIS_COCKPIT);
			break;
		case YELLOW:
			DelMesh(PeriscopeFilter);
			PeriscopeFilter = AddMesh(periscopeFilterYellow, &filterOffset);
			SetMeshVisibilityMode(PeriscopeFilter, MESHVIS_COCKPIT);
			break;
		case GRAY:
			DelMesh(PeriscopeFilter);
			PeriscopeFilter = AddMesh(periscopeFilterGray, &filterOffset);
			SetMeshVisibilityMode(PeriscopeFilter, MESHVIS_COCKPIT);
			break;
		}

		// Click to switch filters
		VersionDependentPanelClick(PANEL_ID_PERISCOPE_FILTER_SWITCH, _R(0, 0, PANEL2D_WIDTH, PANEL2D_HEIGHT), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
		return true;
	case 2: // rocket camera
		if (VesselStatus != FLIGHT && VesselStatus != REENTRY && VesselStatus != REENTRYNODROGUE && VesselStatus != ABORT && VesselStatus != ABORTNORETRO) // basically in launch, but different states between Redstone and Atlas
		{
			periscope = false;
			rocketCam = true;
			panelView = false;

			// Trying to replicate something like this: https://www.youtube.com/watch?v=9JFNE9HoXSw&feature=youtu.be&t=10
			SetCameraOffset(ROCKET_CAMERA_OFFSET[rocketCamMode]);
			SetCameraDefaultDirection(ROCKET_CAMERA_DIRECTION[rocketCamMode]);
			oapiCameraSetCockpitDir(0, 0); // Rotate camera to desired direction
			SetCameraRotationRange(0, 0, 0, 0); // Make camera fixed
			oapiCameraSetAperture(oldFOV);
			if (GroundContact()) oapiSetPanelNeighbours(-1, -1, 1, -1);
			else oapiSetPanelNeighbours(-1, -1, 0, -1);
			DelMesh(PeriscopeFilter); // So that it doesn't appear in generic cockpit
			SetCameraSceneVisibility(MESHVIS_ALWAYS);

			SetPanelBackground(hPanel, 0, 0, circularFrameMesh, PANEL2D_WIDTH, PANEL2D_HEIGHT, 0UL, PANEL_ATTACH_TOP | PANEL_ATTACH_BOTTOM);

			SetPanelScaling(hPanel, defaultScale, panelScale);
			// End circular frame

			return true;
		}
		else return false;
	default:
		return false;
	}
}

inline bool ProjectMercury::clbkPanelMouseEvent(int id, int event, int mx, int my, void* context)
{
	double simt = oapiGetSimTime();

	switch (id)
	{
	case PANEL_ID_PERISCOPE_FILTER_SWITCH:
		if (periscope && oapiCameraInternal())
		{
			CurrentFilter = filtertype(((int(CurrentFilter) + 1) % 4));

			VECTOR3 filterOffset = CAMERA_OFFSET + CAMERA_DIRECTION * 1.0;
			switch (CurrentFilter)
			{
			case CLEAR:
				DelMesh(PeriscopeFilter);
				break;
			case RED:
				DelMesh(PeriscopeFilter);
				PeriscopeFilter = AddMesh(periscopeFilterRed, &filterOffset);
				SetMeshVisibilityMode(PeriscopeFilter, MESHVIS_COCKPIT);
				break;
			case YELLOW:
				DelMesh(PeriscopeFilter);
				PeriscopeFilter = AddMesh(periscopeFilterYellow, &filterOffset);
				SetMeshVisibilityMode(PeriscopeFilter, MESHVIS_COCKPIT);
				break;
			case GRAY:
				DelMesh(PeriscopeFilter);
				PeriscopeFilter = AddMesh(periscopeFilterGray, &filterOffset);
				SetMeshVisibilityMode(PeriscopeFilter, MESHVIS_COCKPIT);
				break;
			}
		}
		else
		{
			oapiWriteLog("Debug debug debug. What is happening? clbkPanelMouseEvent");
		}

		return true;
	case PANEL_ID_LOAD_RESET:
		// Reset
		maxVesselAcceleration = -1e9; // neg inft.
		minVesselAcceleration = 1e9; // plus inft.

		return true;
	case PANEL_ID_TOWJET_BUTTON:
		SetIndicatorButtonStatus(indicatorButtonFirstGroup + 0, 1);
		indicatorButtonPressTime[0] = simt;

		separateTowerAction = true;
		return true;
	case PANEL_ID_CAPSEP_BUTTON:
		SetIndicatorButtonStatus(indicatorButtonFirstGroup + 1, 1);
		indicatorButtonPressTime[1] = simt;

		separateCapsuleAction = true;
		return true;
	case PANEL_ID_RETSEQ_BUTTON:
		SetIndicatorButtonStatus(indicatorButtonFirstGroup + 2, 1);
		indicatorButtonPressTime[2] = simt;

		InitiateRetroSequence();
		return true;
	case PANEL_ID_RETJET_BUTTON:
		SetIndicatorButtonStatus(indicatorButtonFirstGroup + 3, 1);
		indicatorButtonPressTime[3] = simt;

		prepareReentryAction = true;
		return true;
	case PANEL_ID_TIMZER_BUTTON:
		SetIndicatorButtonStatus(indicatorButtonFirstGroup + 4, 1);
		indicatorButtonPressTime[4] = simt;

		launchTime = simt; // Set this instant to T+0. Useful for when launching on a Fred18 Multistage booster, where we don't have contact with launcher.
		return true;
	case PANEL_ID_AUTORETJET_SWITCH_L:
		if (switchAutoRetroJet == 1) // if in OFF, switch to ARM
			switchAutoRetroJet = -1;
		return true;
	case PANEL_ID_AUTORETJET_SWITCH_R:
		if (switchAutoRetroJet == -1) // if in ARM, switch to OFF
			switchAutoRetroJet = 1;
		return true;
	case PANEL_ID_RETDELAY_SWITCH_L:
		if (switchRetroDelay == 1) // if INST, then NORM
			switchRetroDelay = -1;
		return true;
	case PANEL_ID_RETDELAY_SWITCH_R:
		if (switchRetroDelay == -1) // if NORM, then INST
			switchRetroDelay = 1;
		return true;
	case PANEL_ID_RETATT_SWITCH_L:
		if (switchRetroAttitude == 1) // if MAN, then AUTO
			switchRetroAttitude = -1;
		return true;
	case PANEL_ID_RETATT_SWITCH_R:
		if (switchRetroAttitude == -1) // if AUTO, then MAN
			switchRetroAttitude = 1;
		return true;
	case PANEL_ID_ASCSMODE_SWITCH_L:
		if (switchASCSMode == 1) // if FLY BY WIRE, then AUX DAMP
			switchASCSMode = 0;
		else if (switchASCSMode == 0) // if AUX DAMP, then NORM
		{
			switchASCSMode = -1;

			// Also set autopilotmode to high torque if in FLIGHT
			if (VesselStatus == FLIGHT && AutopilotStatus != RETROATTITUDE) AutopilotStatus = TURNAROUND; // if in retroattitude, it's important that we stay there.
		}
		return true;
	case PANEL_ID_ASCSMODE_SWITCH_R:
		if (switchASCSMode == -1) // if NORM, then AUX DAMP
			switchASCSMode = 0;
		else if (switchASCSMode == 0) // if AUX DAMP, then FLY BY WIRE
			switchASCSMode = 1;
		return true;
	case PANEL_ID_CNTRLMODE_SWITCH_L:
		if (switchControlMode == 1) // if RATE COMD, then AUTO
			switchControlMode = -1;
		return true;
	case PANEL_ID_CNTRLMODE_SWITCH_R:
		if (switchControlMode == -1) // if AUTO, then RATE COMD
			switchControlMode = 1;
		return true;
	case PANEL_ID_MANUAL_THANDLE:
		if (event == PANEL_MOUSE_RBDOWN || event == PANEL_MOUSE_RBPRESSED) // pull
			tHandleManualPushed = false;
		else if (event == PANEL_MOUSE_LBDOWN || event == PANEL_MOUSE_LBPRESSED) // push
			tHandleManualPushed = true;
		return true;
	case PANEL_ID_ROLL_THANDLE:
		if (event == PANEL_MOUSE_RBDOWN || event == PANEL_MOUSE_RBPRESSED) // pull
			tHandleRollPushed = false;
		else if (event == PANEL_MOUSE_LBDOWN || event == PANEL_MOUSE_LBPRESSED) // push
			tHandleRollPushed = true;
		return true;
	case PANEL_ID_YAW_THANDLE:
		if (event == PANEL_MOUSE_RBDOWN || event == PANEL_MOUSE_RBPRESSED) // pull
			tHandleYawPushed = false;
		else if (event == PANEL_MOUSE_LBDOWN || event == PANEL_MOUSE_LBPRESSED) // push
			tHandleYawPushed = true;
		return true;
	case PANEL_ID_PITCH_THANDLE:
		if (event == PANEL_MOUSE_RBDOWN || event == PANEL_MOUSE_RBPRESSED) // pull
			tHandlePitchPushed = false;
		else if (event == PANEL_MOUSE_LBDOWN || event == PANEL_MOUSE_LBPRESSED) // push
			tHandlePitchPushed = true;
		return true;
	case PANEL_ID_FIREFLY:
		if (event == PANEL_MOUSE_LBDOWN && oapiGetTimeAcceleration() <= 10.0)
		{
			OBJHANDLE earthHandle = oapiGetGbodyByName("Earth");
			if (GetSurfaceRef() == earthHandle)
			{
				VECTOR3 earthPos, sunPos, sunVel;
				GetRelativePos(earthHandle, earthPos);
				GetRelativePos(oapiGetGbodyByIndex(0), sunPos); // Sun must be index 0.
				double sunAngle = acos(dotp(earthPos, sunPos) / length(earthPos) / length(sunPos)) - PI05;
				double horizonAngle = acos(oapiGetSize(earthHandle) / length(earthPos));

				GetRelativeVel(oapiGetGbodyByIndex(0), sunVel);

				double sunVelAngle = acos(dotp(sunPos, sunVel) / length(sunPos) / length(sunVel)); // due to flipped directions of vectors, the angle is 0 if going away, and PI if towards.

				if (sunAngle + horizonAngle > 0.0 && sunAngle + horizonAngle < 5.0 * RAD && sunVelAngle > PI05) // sun is above horizon, but less than 5 degrees, so release fireflies
				{
					fireflyLevel = 1.0; // bang
					fireflyBangTime = simt;
					//sprintf(oapiDebugString(), "Bang. sunAng: %.2f\u00B0, horAng: %.2f\u00B0, sunHorAng: %.2f\u00B0", sunAngle * DEG, horizonAngle * DEG, (sunAngle + horizonAngle) * DEG);
				}
			}
		}

		return true;
	default:
		return false;
	}
	return false;
}


inline void ProjectMercury::clbkVisualCreated(VISHANDLE vis, int refcount)
{
	// Initialise periscope
	SetPeriscopeAltitude(periscopeAltitude);

	// Set instrument panel on far left and right
	FitPanelToScreen(ScreenWidth, ScreenHeight);
}
//
//inline void ProjectMercury::clbkVisualDestroyed(VISHANDLE vis, int refcount)
//{
//	oapiWriteLog("Debug destroy!");
//}

// Altitude in kilometres
void ProjectMercury::GetPixelDeviationForAltitude(double inputAltitude, double *deg0Pix, double *deg5Pix)
{
	// We assume quadratic dependency.
	// In real life it is probably a form of an exponential, as 0 alt gives full deviation, and inf alt gives 0.

	if (inputAltitude == NULL) inputAltitude = 160.0;
	
	// Limits from Mercury Familiarization Guide page 362 (chapter 12-5)
	if (inputAltitude < 50.0 * 1.852) inputAltitude = 50.0 * 1.852;
	if (inputAltitude > 250.0 * 1.852) inputAltitude = 250.0 * 1.852;

	*deg0Pix = 8.0e3 * pow(inputAltitude, -0.52);
	*deg5Pix = 2.4e4 * pow(inputAltitude, -0.65);
}

inline void ProjectMercury::SetPeriscopeAltitude(double inputAltitude)
{
	const int totalGroupNumber = 8;

	//											 0r	 5r	 0d	 5d	 0l	 5l	 0u	 5u
	const int reticleGroup[totalGroupNumber] = { 0, 1, 2, 3, 4, 5, 6, 7 };

	const double reticleWidth = 10.0;

	const double centreX = 2160.0 / 2.0;
	const double centreY = 1440.0 / 2.0;

	const int totalVertices = 4;
	static WORD vertexIndex[totalVertices] = { 0, 1, 2, 3 };

	double vertexDisplacement0, vertexDisplacement5;
	GetPixelDeviationForAltitude(periscopeAltitude, &vertexDisplacement0, &vertexDisplacement5);

	for (int i = 0; i < totalGroupNumber; i++)
	{
		double displacement = vertexDisplacement0;
		if (i % 2 == 1) displacement = vertexDisplacement5; // 5 degree displacement

		double displacementSign = 1.0;
		if (i >= 4) displacementSign = -1.0; // negative coordinate

		GROUPEDITSPEC ges;
		NTVERTEX newVertex[totalVertices];

		if (i == 2 || i == 3 || i == 6 || i == 7) // y
		{
			ges.flags = GRPEDIT_VTXCRDY;
			ges.nVtx = totalVertices;
			ges.vIdx = vertexIndex;

			newVertex[0].y = float(centreY + reticleWidth / 2.0 + displacementSign * displacement);
			newVertex[1].y = float(centreY + reticleWidth / 2.0 + displacementSign * displacement);
			newVertex[2].y = float(centreY - reticleWidth / 2.0 + displacementSign * displacement);
			newVertex[3].y = float(centreY - reticleWidth / 2.0 + displacementSign * displacement);

			ges.Vtx = newVertex;
			oapiEditMeshGroup(periscopeMesh, reticleGroup[i], &ges);
		}
		else
		{
			ges.flags = GRPEDIT_VTXCRDX;
			ges.nVtx = 4;
			ges.vIdx = vertexIndex;

			newVertex[0].x = float(centreX - reticleWidth / 2.0 + displacementSign * displacement);
			newVertex[1].x = float(centreX - reticleWidth / 2.0 + displacementSign * displacement);
			newVertex[2].x = float(centreX + reticleWidth / 2.0 + displacementSign * displacement);
			newVertex[3].x = float(centreX + reticleWidth / 2.0 + displacementSign * displacement);

			ges.Vtx = newVertex;
			oapiEditMeshGroup(periscopeMesh, reticleGroup[i], &ges);
		}
	}
}

inline void ProjectMercury::CreatePanelSwitchClick(int ID_L, int ID_R, int x, int y, PANELHANDLE hPanel)
{
	const int switchW = 200, switchH = 100;

	VersionDependentPanelClick(ID_L, _R(x - switchW / 2, y - switchH / 2, x, y + switchH / 2), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
	VersionDependentPanelClick(ID_R, _R(x, y - switchH / 2, x + switchW / 2, y + switchH / 2), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
}

inline void ProjectMercury::CreatePanelTHandleClick(int ID, int x, int y, PANELHANDLE hPanel)
{
	const int tHandleRad = 100;

	VersionDependentPanelClick(ID, _R(x - tHandleRad, y - tHandleRad, x + tHandleRad, y + tHandleRad), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_RBDOWN | PANEL_MOUSE_RBPRESSED | PANEL_MOUSE_LBDOWN | PANEL_MOUSE_LBPRESSED, hPanel, _R(0, 0, 0, 0), NULL);
}

// Fit the current panel mesh (1.5 ratio) to any screen ratio, by pushing elements to screen border.
// Well, that was a previous version. Now, it's simply recording group indecies.
void ProjectMercury::FitPanelToScreen(int w, int h)
{
	// Get panel mesh information
	const int totalGroupNumber = int(oapiMeshGroupCount(cockpitPanelMesh));

	int groupNumVtx[1000] = { 0 }; // number of vertexes in group. Large number to foresee largest possible mesh group vertex count

	for (int i = 0; i < totalGroupNumber; i++)
	{
		MESHGROUPEX *groupData = oapiMeshGroupEx(cockpitPanelMesh, i);

		groupNumVtx[i] = int(groupData->nVtx);
		
		if (groupData->Vtx->x < 2160 / 2 && abs(groupData->Vtx->y) < 2000) // left hand group
		{
			panelMeshGroupSide[i] = 1;
		}
		else if (abs(groupData->Vtx->y) < 2000) // right hand group
		{
			panelMeshGroupSide[i] = 2;
		}
		else
		{
			// Group outside of screen, and thus don't touch (the gray border for superwide screens)
			panelMeshGroupSide[i] = -1;
		}

		// Get all arm group indecies for AnimateDials. All arms have five verticies
		if (groupData->nVtx == 5)
		{
			armGroups[totalArmGroups] = i;
			totalArmGroups += 1;
		}

		if (groupData->nVtx > 50) // large mesh, probably globe
		{
			if (globeGroup == NULL) globeGroup = i;
			else oapiWriteLog("ERROR ERROR ERROR! More than one mesh group with more than 50 vertices! Please debug!");

			globeVertices = groupData->nVtx;
		}

		if (groupData->nVtx == 41)
		{
			if (abortIndicatorGroup == NULL)
			{
				abortIndicatorGroup = i;
				oapiWriteLogV("Found abort indicator at group %i", i);
			}
			else oapiWriteLog("ERROR ERROR ERROR! More than one mesh group with 41 vertices! Please debug!");
		}

		//oapiWriteLogV("Group %i has %i vertices", i, groupData->nVtx);
	}

	// Edit mesh
	double defaultScale = (double)h / 1440;
	addScreenWidthValue = float((w / defaultScale - 2160.0) / 2.0); // add or subtract offset value
	//for (int k = 0; k < totalGroupNumber; k++) // for every group
	//{
	//	GROUPEDITSPEC ges;
	//	NTVERTEX newVertex[1000]; // again must define a large vertex count, as I can't allocate memory from the mesh info collected earlier

	//	if (k == 0) // first mesh group, which is the background with window
	//	{
	//		ges.flags = GRPEDIT_VTXCRDX;
	//		ges.nVtx = groupNumVtx[k]; // SHOULD be 8

	//		WORD vertexIndex[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	//		ges.vIdx = vertexIndex;

	//		newVertex[0].x = 0.0f - addScreenWidthValue;
	//		newVertex[1].x = 680.0f - addScreenWidthValue;
	//		newVertex[2].x = 830.0f - addScreenWidthValue;
	//		newVertex[3].x = 1330.0f + addScreenWidthValue;
	//		newVertex[4].x = 1480.0f + addScreenWidthValue;
	//		newVertex[5].x = 2160.0f + addScreenWidthValue;
	//		newVertex[6].x = 2160.0f + addScreenWidthValue;
	//		newVertex[7].x = 0.0f - addScreenWidthValue;

	//		ges.Vtx = newVertex;
	//		oapiEditMeshGroup(cockpitPanelMesh, k, &ges);
	//	}
	//	else
	//	{
	//		//if (panelMeshGroupSide[k] == 1 || panelMeshGroupSide[k] == 2) // on one side of the screen
	//		//{
	//		//	float sign = -1.0;
	//		//	if (panelMeshGroupSide[k] == 2) sign = 1.0; // if right, add. If left, subtract

	//		//	ges.flags = GRPEDIT_VTXCRDADDX;
	//		//	ges.nVtx = groupNumVtx[k];

	//		//	// Create vertex idx array
	//		//	WORD vertexIndex[1000] = { NULL };
	//		//	for (int j = 0; j < groupNumVtx[k]; j++)
	//		//	{
	//		//		vertexIndex[j] = j;
	//		//	}
	//		//	ges.vIdx = vertexIndex;

	//		//	// Add value to every vertex
	//		//	for (int l = 0; l < groupNumVtx[k]; l++) // for every vertex
	//		//	{
	//		//		newVertex[l].x = sign * addScreenWidthValue;
	//		//	}
	//		//	ges.Vtx = newVertex;
	//		//	oapiEditMeshGroup(cockpitPanelMesh, k, &ges);
	//		//}
	//	}
	//}

	oapiWriteLogV("Panel mesh fit to screen by pushing mesh groups %.1f pixels.", addScreenWidthValue);
}

inline void ProjectMercury::AnimateDials(void)
{
	bool includeLatency = true;
	double simt = oapiGetSimTime();

	if (simt - animateDialsPreviousSimt > 1.0)
	{
		includeLatency = false; // a relatively long time since last animation step, so force no latency to show current value. This is to omit the clock jumping around for every small periscope/F8-switch.

		oapiWriteLogV("Panel simt: %.2f, prev: %.2f, diff: %.2f", simt, animateDialsPreviousSimt, simt - animateDialsPreviousSimt);
	}

	int idx = 0;

	// positions:
	float gMeterPos[2] =	{ 190, 850 };
	float fuelPos[2] =		{525, 1000};
	float descentPos[2] =	{525, 1290};
	float altPos[2] =		{190, 1250};
	float clockPos[2] =		{1820, 540};
	float earthPos[2] =		{ 1820, 1140 };
	float attitudeCenter[2] = { 1080, 800 };
	float pitchPos[2] =		{attitudeCenter[0] + 250, attitudeCenter[1]};
	float rollPos[2] =		{attitudeCenter[0] - 250, attitudeCenter[1]};
	float yawPos[2] =		{attitudeCenter[0], attitudeCenter[1] + 270};
	float attitudePos[2] =	{ attitudeCenter[0] - 1.0f, attitudeCenter[1] + 0.5f}; // additional small fixes to perfectly centre crosshairs.

	// Longitudinal acceleration
	float load = float(longitudinalAcc / G);
	RotateArmGroup(armGroups[idx], gMeterPos[0], gMeterPos[1], 100.0f, 10.0f, ValueToAngle(load, -9.0f, 21.0f, -280.0f, 50.0f), 0.9f);
	idx += 1;
	// Longitudinal acceleration memory high
	float loadP = float(maxVesselAcceleration / G);
	RotateArmGroup(armGroups[idx], gMeterPos[0], gMeterPos[1], 90.0f, 6.0f, ValueToAngle(loadP, -9.0f, 21.0f, -280.0f, 50.0f), 0.9f);
	idx += 1;
	// Longitudinal acceleration memory low
	float loadN = float(minVesselAcceleration / G);
	RotateArmGroup(armGroups[idx], gMeterPos[0], gMeterPos[1], 90.0f, 6.0f, ValueToAngle(loadN, -9.0f, 21.0f, -280.0f, 50.0f), 0.9f);
	idx += 1;

	// Fuel
	float fuelAuto = float(GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_AUTO);
	float fuelManual = float(GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_MAN);

	// Fuel auto
	RotateArmGroup(armGroups[idx], fuelPos[0] - 72, fuelPos[1], 60.0f, 8.0f, ValueToAngle(fuelAuto, 0.0f, 1.0f, 50.0f, -50.0f), 0.1f);
	idx += 1;

	// Fuel manual
	RotateArmGroup(armGroups[idx], fuelPos[0] + 72, fuelPos[1], 60.0f, 8.0f, ValueToAngle(fuelManual, 0.0f, 1.0f, 130.0f, 230.0f), 0.1f);
	idx += 1;

	// Descent
	VECTOR3 groundSpeedVec;
	GetAirspeedVector(FRAME_HORIZON, groundSpeedVec);
	float descent = float(-groundSpeedVec.y / 3.048); // feet/s
	if (GetAltitude() > 5e4) descent = 0.0f;
	RotateArmGroup(armGroups[idx], descentPos[0], descentPos[1], 95.0f, 10.0f, ValueToAngle(descent, 0.0f, 15.0f, 180.0f, 0.0f), 0.85f);
	idx += 1;

	// Altitude
	float altitude = float(GetAltitude() / 304.8); // ft
	RotateArmGroup(armGroups[idx], altPos[0], altPos[1], 135.0f, 10.0f, ValueToAngle(altitude, 0.0f, 100.0f, 120.0f, 450.0f), 0.85f);
	idx += 1;

	// Roll
	float roll = float(-normangle((GetBank() + rollOffset))); // negative because it was flipped
	RotateArmGroup(armGroups[idx], rollPos[0], rollPos[1], 115.0f, 12.0f, ValueToAngle(roll, float(-PI), float(PI), -270.0f, 90.0f), 0.8f);
	idx += 1;

	// Pitch
	float pitch = float(normangle((GetPitch() + pitchOffset)));
	RotateArmGroup(armGroups[idx], pitchPos[0], pitchPos[1], 115.0f, 12.0f, ValueToAngle(pitch, float(-PI), float(PI), 34.0f, 394.0f), 0.8f, 60.0f); // Negative length part
	idx += 1;

	// Yaw. Note that retroattitude is indicated yaw 0 degrees! That's why we add PI.
	float yaw = float(normangle((GetSlipAngle() + yawOffset + PI)));
	RotateArmGroup(armGroups[idx], yawPos[0], yawPos[1], 115.0f, 12.0f, ValueToAngle(yaw, float(-PI), float(PI), -270.0f, 90.0f), 0.8f, 40.0f); // Negative length part
	idx += 1;

	// Calculate solar time from MJD
	double mjd = oapiGetSimMJD();
	double jd = mjd + 2400000.5;
	// Don't take time zone into consideration. In communications log, they only reference GMT times, so that was probably the one on the dial.
	//jd -= 5.0 / 24.0; // Take time zone into consideration, to get local Florida time (UTC-5).
	// Actual calculation follow
	double Zint = floor(jd + 0.5);
	double Ffrac = jd + 0.5 - Zint;
	double Aval = 0.0;
	if (Zint < 2299161.0) Aval = Zint;
	else Aval = Zint + 1.0 + floor((Zint - 1867216.25) / 36524.25) - floor(floor((Zint - 1867216.25) / 36524.25) / 4.0);
	double Bval = Aval + 1524.0;
	double Cval = floor((Bval - 122.1) / 365.25);
	double Dval = floor(365.25 * Cval);
	double Eval = floor((Bval - Dval) / 30.6001);
	double dayDec = Bval - Dval - floor(30.6001 * Eval) + Ffrac;
	double decimal = dayDec - floor(dayDec);
	// And finally, the time of day
	float hour = float(decimal * 24.0);
	float minute = float((decimal - floor(hour) / 24.0) * 1440.0);
	float second = float((decimal - floor(hour) / 24.0 - floor(minute) / 1440.0) * 86400.0);

	// But the second hand ticks 5 steps per second (https://youtu.be/YiVoc6oPZbI?t=81), so truncate seconds to steps of 0.2 (=1/5).
	second = floor(second * 5.0f) / 5.0f;

	// Clock hour
	RotateArmGroup(armGroups[idx], clockPos[0] - 173, clockPos[1] - 116, 55.0f, 10.0f, ValueToAngle(hour, 0.0f, 24.0f, -90.0f, 270.0f), 0.7f, 0.0f, includeLatency); // Note that the hour dial did one revolution per day!
	idx += 1;
	// Clock minute
	RotateArmGroup(armGroups[idx], clockPos[0] - 173, clockPos[1] - 116, 70.0f, 10.0f, ValueToAngle(minute, 0.0f, 60.0f, -90.0f, 270.0f), 0.8f, 0.0f, includeLatency);
	idx += 1;
	// Clock second
	RotateArmGroup(armGroups[idx], clockPos[0] - 173, clockPos[1] - 116, 110.0f, 3.0f, ValueToAngle(second, 0.0f, 60.0f, -90.0f, 270.0f), 0.95f, 0.0f, includeLatency);
	idx += 1;

	// Attitude rate
	// Get attitude rate
	VECTOR3 angVel;
	GetAngularVel(angVel);
	// The limits of attitude rate display is a bit confusing. 
	// MercuryFamiliarizationManual20May1962 page 395 says "A zero to three volt signal level represents a rate level of decreasing 40 deg/sec to increasing 40 deg/sec".
	// Same manual, page 440, says (now for apparently a kind of tape recorder):
	//		"A 0 volt signal level represents a zero attitude rate.
	//		A -1.5 V signal ... a decreasing rate of 6 deg/sec
	//		and a plus 1.5 V signal ... an increasing rate of 6 deg/sec."
	// According to MA6_FlightOps.pdf page 1-40 (page 26 in pdf), roll rate switches from a one range (+6 deg/sec for last dot, total +8 deg/sec) while in flight,
	// to a new range (+15 deg/sec for last dot, total +20 deg/sec) after .05g activation.
	// This is only for roll, however. One may assume that pitch and yaw rates also are at the default (+6 deg/sec, tot +8 deg/sec) scale during flight.
	// In that case, the question whether pitch and way rates also scale during .05g program. I guess no.
	// Summary: -> MinMax +- 8 deg/s, with roll going to +- 20 deg/s when .05g
	// Epilogue: I don't like the limited range of 6-8 deg/s, so I'm setting it to always be the extended .05g range, i.e. 15 deg/s.
	float range = 15.0f;
	float rollRange = range;
	//if (AutopilotStatus == LOWG) rollRange = 15.0f; // if .05g, set range scale to MinMax 20 deg/s
	float pRate = float(angVel.x * DEG);
	if (pRate > range) pRate = range;
	if (pRate < -range) pRate = -range;
	float yRate = float(angVel.y * DEG);
	if (yRate > range) yRate = range;
	if (yRate < -range) yRate = -range;
	float rRate = float(angVel.z * DEG);
	if (rRate > rollRange) rRate = rollRange;
	if (rRate < -rollRange) rRate = -rollRange;

	float pPixelY = attitudePos[1] + 85.0f * pRate / range;
	float yPixelX = attitudePos[0] + 85.0f * yRate / range;
	float rPixelX = attitudePos[0] + 85.0f * rRate / rollRange;

	// Roll (actually translation, but use same rotate function)
	RotateArmGroup(armGroups[idx], rPixelX, attitudePos[1] - 10, 100.0f, 5.0f, float(-90.0 * RAD), 0.95f, 0.0f, false);
	idx += 1;

	// Pitch (actually translation, but use same rotate function)
	RotateArmGroup(armGroups[idx], attitudePos[0] - 60, pPixelY, 165.0f, 5.0f, 0.0f, 0.97f, 0.0f, false);
	idx += 1;

	// Yaw (actually translation, but use same rotate function)
	RotateArmGroup(armGroups[idx], yPixelX, attitudePos[1] + 10, 100.0f, 5.0f, float(90.0 * RAD), 0.95f, 0.0f, false);
	idx += 1;

	// Rotate globe texture
	double posLong, posLat, posRad, heading;
	OBJHANDLE refP = GetEquPos(posLong, posLat, posRad);
	VECTOR3 currVel;
	GetGroundspeedVector(FRAME_HORIZON, currVel);
	heading = atan2(currVel.x + oapiGetSize(refP) * PI2 / oapiGetPlanetPeriod(refP) * cos(posLat), currVel.z); // orbital heading = (ground vector + ground rotation) heading
	RotateGlobe(5.0f, 60.0f, float(posLong), float(posLat), float(heading - PI05));

	// Clock numerals
	double met = simt - launchTime;
	double metFlow = met; // keep met unchanged, as it's used for retrotime
	if (metFlow > (100.0 * 3600.0 - 1.0)) metFlow = fmod(metFlow, 100.0 * 3600.0); // overflow
	int metH = (int)floor(metFlow / 3600.0);
	int metM = (int)floor((metFlow - metH * 3600.0) / 60.0);
	int metS = (int)floor((metFlow - metH * 3600.0 - metM * 60.0));

	if (met < 0.0) // don't start clock until after liftoff
	{
		metH = 0;
		metM = 0;
		metS = 0;
	}

	int digitGrpNum = 13; // first digit group index. Update this number if we change previous groups later

	ChangePanelNumber(digitGrpNum, metH); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;
	ChangePanelNumber(digitGrpNum, metM); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;
	ChangePanelNumber(digitGrpNum, metS); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;

	int ret3H = 0, ret3M = 0, ret3S = 0, dRetH = 0, dRetM = 0, dRetS = 0;

	GetPanelRetroTimes(met, &ret3H, &ret3M, &ret3S, &dRetH, &dRetM, &dRetS);

	ChangePanelNumber(digitGrpNum, ret3H); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;
	ChangePanelNumber(digitGrpNum, ret3M); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;
	ChangePanelNumber(digitGrpNum, ret3S); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;

	ChangePanelNumber(digitGrpNum, dRetH); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;
	ChangePanelNumber(digitGrpNum, dRetM); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;
	ChangePanelNumber(digitGrpNum, dRetS); // edits both of the digits in pair (24 -> 2, 4)
	digitGrpNum += 2;

	ChangeIndicatorStatus();

	animateDialsPreviousSimt = simt;
}

inline void ProjectMercury::RotateArmGroup(int groupNum, float x0, float y0, float length, float width, float angleR, float pointiness, float negLength, bool includeLatency)
{
	GROUPEDITSPEC ges;
	NTVERTEX newVertex[5]; // We require five verticies

	// Find x-offset from screen width (same as in FitPanelToScreen())
	float xTraOffset = addScreenWidthValue;
	if (panelMeshGroupSide[groupNum] == 1) // left
	{
		xTraOffset *= -1.0f;
	}
	xTraOffset = 0.0f; // debug. I have now disabled the screen fit, as the dials are now centered, instead of being on the sides

	// Include latency in dial
	if (includeLatency)
	{
		float simdt = (float)oapiGetSimStep();
		if (abs(normangle(angleR - previousDialAngle[groupNum])) > simdt * dialAngularSpeed)
		{
			float signDirection = (angleR - previousDialAngle[groupNum]) / abs(angleR - previousDialAngle[groupNum]);
			angleR = previousDialAngle[groupNum] + simdt * dialAngularSpeed * signDirection;
		}
	}
	previousDialAngle[groupNum] = angleR; // save outside of the latency adding, as clock has dynamic latency, and thus always needs to be saved

	float armInner0x = -negLength * cos(angleR) + width / 2.0f * sin(-angleR) + x0 + xTraOffset;
	float armInner0y = -negLength * sin(angleR) + width / 2.0f * cos(angleR) + y0;

	float armInner1x = -negLength * cos(angleR) - width / 2.0f * sin(-angleR) + x0 + xTraOffset;
	float armInner1y = -negLength * sin(angleR) -width / 2.0f * cos(angleR) + y0;

	float armOuter0x = length * pointiness * cos(angleR) + width / 2.0f * sin(-angleR) + x0 + xTraOffset;
	float armOuter0y = length * pointiness * sin(angleR) + width / 2.0f * cos(angleR) + y0;

	float armOuter1x = length * pointiness * cos(angleR) - width / 2.0f * sin(-angleR) + x0 + xTraOffset;
	float armOuter1y = length * pointiness * sin(angleR) - width / 2.0f * cos(angleR) + y0;

	float handOuterx = length * cos(angleR) + x0 + xTraOffset;
	float handOutery = length * sin(angleR) + y0;

	ges.flags = GRPEDIT_VTXCRDX | GRPEDIT_VTXCRDY;
	ges.nVtx = 5;
	WORD vertexIndex[5] = { 0, 1, 2, 3, 4 };
	ges.vIdx = vertexIndex;

	newVertex[0].x = armInner0x;
	newVertex[0].y = armInner0y;

	newVertex[1].x = armInner1x;
	newVertex[1].y = armInner1y;

	newVertex[2].x = armOuter0x;
	newVertex[2].y = armOuter0y;

	newVertex[3].x = armOuter1x;
	newVertex[3].y = armOuter1y;

	newVertex[4].x = handOuterx;
	newVertex[4].y = handOutery;

	ges.Vtx = newVertex;

	oapiEditMeshGroup(cockpitPanelMesh, groupNum, &ges);
}

inline float ProjectMercury::ValueToAngle(float value, float minValue, float maxValue, float minAngle, float maxAngle)
{
	float factor = ((value - minValue) / (maxValue - minValue));

	// Truncate to range
	if (factor > 1.0) factor = 1.0;
	if (factor < 0.0) factor = 0.0;

	float resultingAngle = factor * (maxAngle - minAngle) + minAngle;
	resultingAngle *= float(RAD); // convert to radians

	return resultingAngle;
}

inline void ProjectMercury::RotateGlobe(float angularResolution, float viewAngularRadius, float longitude0, float latitude0, float rotationAngle)
{
	GROUPEDITSPEC ges;
	NTVERTEX newVertex[1000]; // again must define a large vertex count, as I can't allocate memory from the mesh info collected earlier

	ges.flags = GRPEDIT_VTXTEXU | GRPEDIT_VTXTEXV;
	ges.nVtx = globeVertices;

	// Create vertex idx array
	WORD vertexIndex[1000] = { NULL };
	for (int j = 0; j < globeVertices; j++)
	{
		vertexIndex[j] = j;
	}
	ges.vIdx = vertexIndex;

	double angResR = angularResolution * RAD;
	int numberOfLayers = int(viewAngularRadius / angularResolution);
	int numberPerLayer = int(360.0f / angularResolution);

	// The texture covers 1.5 * of the Earth, to avoid seams.
	// Dynamically switch between left part (regular) when 90 W < long < 90 E
	// and right part (extended) when 90 E < long < 90 W (around date-line, and thus seam).
	if (longitude0 > PI05) // seam-area let run into extended
	{
		newVertex[0].tu = float(longitude0 / PI2 + 0.5) / 1.5f; // add a full rotation of the Earth, which is 0.666 in uv coordinates
		newVertex[0].tv = float(-latitude0 / PI + 0.5);

		// Add value to every vertex
		for (int i = 0; i < numberOfLayers; i++)
		{
			for (int k = 0; k < numberPerLayer; k++)
			{
				int idx = i * numberPerLayer + 1 + k;

				// Cast to double, as we'll multiply with doubles
				double ip1 = double(i + 1);
				double dk = double(k);

				// Great circle calculations from here: https://www.movable-type.co.uk/scripts/latlong.html
				double pointLat = asin(sin(latitude0) * cos(ip1 * angResR) + cos(latitude0) * sin(ip1 * angResR) * cos(dk * angResR + rotationAngle));
				double pointLong = longitude0 + atan2(sin(dk * angResR + rotationAngle) * sin(ip1 * angResR) * cos(latitude0), cos(ip1 * angResR) - sin(latitude0) * sin(pointLat));

				newVertex[idx].tu = float((fmod(pointLong + PI2, PI2)) / PI2 + 0.5) / 1.5f; // squeze by 1.5 as we now have a map covering 540 deg (3PI), and not 360 deg (2PI), and also add full rotation of Earth
				newVertex[idx].tv = float(-pointLat / PI + 0.5); // latitude is normal
			}
		}
	}
	else if (longitude0 < -PI05) // seam-area add one revolution
	{
		newVertex[0].tu = float(longitude0 / PI2 + 0.5) / 1.5f + 2.0f/3.0f; // add a full rotation of the Earth, which is 0.666 in uv coordinates
		newVertex[0].tv = float(-latitude0 / PI + 0.5);

		// Add value to every vertex
		for (int i = 0; i < numberOfLayers; i++)
		{
			for (int k = 0; k < numberPerLayer; k++)
			{
				int idx = i * numberPerLayer + 1 + k;

				// Cast to double, as we'll multiply with doubles
				double ip1 = double(i + 1);
				double dk = double(k);

				// Great circle calculations from here: https://www.movable-type.co.uk/scripts/latlong.html
				double pointLat = asin(sin(latitude0) * cos(ip1 * angResR) + cos(latitude0) * sin(ip1 * angResR) * cos(dk * angResR + rotationAngle));
				double pointLong = longitude0 + atan2(sin(dk * angResR + rotationAngle) * sin(ip1 * angResR) * cos(latitude0), cos(ip1 * angResR) - sin(latitude0) * sin(pointLat));

				newVertex[idx].tu = float(((fmod(pointLong + PI2, PI2) + PI2) / PI2) / 1.5 - 1.0/3.0); // squeze by 1.5 as we now have a map covering 540 deg (3PI), and not 360 deg (2PI), and also let flow over
				newVertex[idx].tv = float(-pointLat / PI + 0.5); // latitude is normal
			}
		}
	}
	else // regular
	{
		newVertex[0].tu = float(longitude0 / PI2 + 0.5) / 1.5f;
		newVertex[0].tv = float(-latitude0 / PI + 0.5);

		// Add value to every vertex
		for (int i = 0; i < numberOfLayers; i++)
		{
			for (int k = 0; k < numberPerLayer; k++)
			{
				int idx = i * numberPerLayer + 1 + k;

				// Cast to double, as we'll multiply with doubles
				double ip1 = double(i + 1);
				double dk = double(k);

				// Great circle calculations from here: https://www.movable-type.co.uk/scripts/latlong.html
				double pointLat = asin(sin(latitude0) * cos(ip1 * angResR) + cos(latitude0) * sin(ip1 * angResR) * cos(dk * angResR + rotationAngle));
				double pointLong = longitude0 + atan2(sin(dk * angResR + rotationAngle) * sin(ip1 * angResR) * cos(latitude0), cos(ip1 * angResR) - sin(latitude0) * sin(pointLat));

				newVertex[idx].tu = float((fmod(pointLong + PI * 3.0, PI2) - PI) / PI2 + 0.5) / 1.5f; // squeze by 1.5 as we now have a map covering 540 deg (3PI), and not 360 deg (2PI)
				newVertex[idx].tv = float(-pointLat / PI + 0.5); // latitude is normal
			}
		}
	}

	ges.Vtx = newVertex;
	oapiEditMeshGroup(cockpitPanelMesh, globeGroup, &ges);
}

inline void ProjectMercury::ChangePanelNumber(int group, int num)
{
	float digitToShow1 = float(int(num / 10)); // 24 -> 2
	float digitToShow2 = float(num - int(num / 10) * 10); // 24 -> 4

	GROUPEDITSPEC ges;
	NTVERTEX newVertex[4]; // rectangle

	ges.flags = GRPEDIT_VTXTEXU | GRPEDIT_VTXTEXV;
	ges.nVtx = 4; // rectange

	// Create vertex idx array
	WORD vertexIndex[4] = { 0, 1, 2, 3 };
	ges.vIdx = vertexIndex;

	float texX0 = 1683.0f / 2048.0f;
	float texX1 = 1717.0f / 2048.0f;
	float texY0 = (305.0f + digitToShow1 * 60.4f) / 1024.0f; // 60 gives slightly offset value
	float texY1 = (351.0f + digitToShow1 * 60.4f) / 1024.0f;

	newVertex[0].tu = texX0;	newVertex[0].tv = texY0;
	newVertex[1].tu = texX1;	newVertex[1].tv = texY0;
	newVertex[2].tu = texX0;	newVertex[2].tv = texY1;
	newVertex[3].tu = texX1;	newVertex[3].tv = texY1;

	ges.Vtx = newVertex;
	oapiEditMeshGroup(cockpitPanelMesh, group, &ges);

	// And then the second group
	texY0 = (305 + digitToShow2 * 60) / 1024;
	texY1 = (351 + digitToShow2 * 60) / 1024;

	newVertex[0].tu = texX0;	newVertex[0].tv = texY0;
	newVertex[1].tu = texX1;	newVertex[1].tv = texY0;
	newVertex[2].tu = texX0;	newVertex[2].tv = texY1;
	newVertex[3].tu = texX1;	newVertex[3].tv = texY1;

	ges.Vtx = newVertex;
	oapiEditMeshGroup(cockpitPanelMesh, group + 1, &ges);
}

inline void ProjectMercury::ChangeIndicatorStatus(void)
{
	int GRAY = 0;
	int RED = 1;
	int GREEN = 2;

	int idx = 0;
	double simt = oapiGetSimTime();

	int indicatorStatus[13] = { 0 };

	// ABORT
	if (abort) indicatorStatus[idx] = RED;
	else indicatorStatus[idx] = GRAY;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// JETT TOWER
	// JETT TOWER and SEP CAPSULE telelights seem to go out 5 minutes after capsule separation, according to MA6_FlightPlan2.pdf page 38. But have found no other source saying the same.
	if (towerJettisoned) indicatorStatus[idx] = GREEN; // I don't implement any failures of tower sep, so don't implement any failure light. This may change when fuses are added
	else indicatorStatus[idx] = GRAY;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// SEP CAPSULE
	// JETT TOWER and SEP CAPSULE telelights seem to go out 5 minutes after capsule separation, according to MA6_FlightPlan2.pdf page 38. But have found no other source saying the same.
	if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORT || VesselStatus == ABORTNORETRO) indicatorStatus[idx] = GREEN;
	else if (boosterShutdownTime == 0.0) indicatorStatus[idx] = GRAY; // time between shutdown and sep is red, so gray if not shutdown yet
	else indicatorStatus[idx] = RED;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// RETRO SEQ
	if (engageRetro && VesselStatus == FLIGHT) indicatorStatus[idx] = GREEN;
	else if (retroStartTime == 0.0) indicatorStatus[idx] = GRAY; // uninitialised
	else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) indicatorStatus[idx] = GRAY;
	else indicatorStatus[idx] = RED;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// RETRO ATT
	double currP = GetPitch() + pitchOffset;
	double currY = GetSlipAngle() + yawOffset;
	if (engageRetro && abs(currP + 34.0 * RAD) < 15.0 * RAD && abs(normangle(currY + PI)) < 15.0 * RAD) indicatorStatus[idx] = GREEN; // within limits
	else if (retroStartTime == 0.0) indicatorStatus[idx] = GRAY; // haven't engaged retro
	else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) indicatorStatus[idx] = GRAY;
	else indicatorStatus[idx] = RED;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// FIRE RETRO
	if ((FailureMode != LOWGDEACTIVE && VesselStatus == REENTRY && vesselAcceleration > 0.05 * G) || AutopilotStatus == LOWG) indicatorStatus[idx] = GRAY; // Disable if LOWG light is on
	else if ((retroStartTime != 0.0 && retroStartTime < simt) || (VesselStatus == FLIGHT && (retroCoverSeparated[0] || retroCoverSeparated[1] || retroCoverSeparated[2]))) indicatorStatus[idx] = GREEN; // green if retro is fired
	else if (retroStartTime == 0.0) indicatorStatus[idx] = GRAY;
	else if (retroStartTime != 0.0 && retroStartTime - 15.0 < simt) indicatorStatus[idx] = RED; // red 15 sec before retrofire
	else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) indicatorStatus[idx] = GREEN; // green until LOWG
	else indicatorStatus[idx] = GRAY;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// JETT RETRO
	if ((FailureMode != LOWGDEACTIVE && VesselStatus == REENTRY && vesselAcceleration > 0.05 * G) || AutopilotStatus == LOWG) indicatorStatus[idx] = GRAY; // Disable if LOWG light is on //else if (retroStartTime == 0.0) indicatorStatus[idx] = GRAY; // uninitialised
	else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) indicatorStatus[idx] = GREEN; // Green until LOWG
	else if (retroStartTime != 0.0 && retroStartTime - 28.0 < simt) indicatorStatus[idx] = RED;
	else indicatorStatus[idx] = GRAY;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// RETRACT SCOPE
	if (PeriscopeStatus != P_CLOSED && (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) && simt - retroStartTime > (60.0 + 40.0) && retroStartTime != 0.0) indicatorStatus[idx] = RED;
	else if (PeriscopeStatus != P_CLOSED && (VesselStatus == FLIGHT || VesselStatus == REENTRYNODROGUE || (VesselStatus == LAUNCH && GroundContact()))) indicatorStatus[idx] = GREEN;
	else if (PeriscopeStatus == P_CLOSED) indicatorStatus[idx] = GRAY;
	else indicatorStatus[idx] = RED;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// .05 G
	if (FailureMode != LOWGDEACTIVE && VesselStatus == REENTRY && vesselAcceleration > 0.05 * G) indicatorStatus[idx] = GREEN;
	else if (AutopilotStatus == LOWG) indicatorStatus[idx] = GREEN;
	else indicatorStatus[idx] = GRAY;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// MAIN
	if (mainChuteDeployed && simt > mainChuteDeployTime + 2.0) indicatorStatus[idx] = GREEN;
	else if (!mainChuteDeployed) indicatorStatus[idx] = GRAY;
	else indicatorStatus[idx] = RED;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// LANDING BAG
	if (landingBagDeployed) indicatorStatus[idx] = GREEN;
	else if (mainChuteDeployed && simt - mainChuteDeployTime > 10.0) indicatorStatus[idx] = RED;
	else if (!landingBagDeployed) indicatorStatus[idx] = GRAY;
	else indicatorStatus[idx] = RED;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// FUEL QUAN
	double autoLevel = GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_AUTO;
	double manualLevel = GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_MAN;
	//if (!attitudeFuelAuto) // swap (just how I've defined it)
	//{
	//	autoLevel = GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_AUTO;
	//	manualLevel = GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_MAN;
	//}
	// Old addon had limit 0.25. Familiarization manual says "The pressure swich is set to actuate at a pre-determined low fuel level".
	// However, on MA-7 Carpenter said at 01 34 37: "Fuel is 62 and 68 %, ..., fuel quantity light is on".
	// MA-6  flightOps manual notes under fuel dial: "65 % - minimum auto control fuel for retrograde and reentry". This is in agreeing with MA-7 communications.
	// So we now set 0.65 to be limit.
	if (autoLevel < 0.65 || manualLevel < 0.65) indicatorStatus[idx] = RED;
	else indicatorStatus[idx] = GRAY;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	// RETRO WARN
	if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) indicatorStatus[idx] = GRAY;
	else if (retroWarnLight) indicatorStatus[idx] = RED;
	else indicatorStatus[idx] = GRAY;
	if (indicatorStatus[idx] != previousIndicatorStatus[idx]) SetIndicatorStatus(idx, indicatorStatus[idx]);
	previousIndicatorStatus[idx] = indicatorStatus[idx];
	idx += 1;

	indicatorButtonFirstGroup = idx;
	// Set all button lights. Works so that button is switched to active on click inside clbkPanelMouseEvent, and we here dim it
	double buttonLightTime = 0.05;
	for (int i = 0; i < 5; i++)
	{
		if (indicatorButtonPressState[i] != -1 && simt > indicatorButtonPressTime[i] + buttonLightTime) SetIndicatorButtonStatus(indicatorButtonFirstGroup + i, -1); // extinguish
		idx += 1;

		// It seems like I have to do something here, or else Orbiter fails to correctly place vertices. So call a stupid function or something.
		//double uselessValue = GetPitch();
		//uselessValue *= GGRAV;
	}

	physicalSwitchFirstGroup = idx;
	// Set all switch positions. Is switched every frame to the current relevant state
	SetPhysicalSwitchStatus(physicalSwitchFirstGroup + 0, switchAutoRetroJet);
	idx += 1;

	SetPhysicalSwitchStatus(physicalSwitchFirstGroup + 1, switchRetroDelay);
	idx += 1;

	SetPhysicalSwitchStatus(physicalSwitchFirstGroup + 2, switchRetroAttitude);
	idx += 1;

	SetPhysicalSwitchStatus(physicalSwitchFirstGroup + 3, switchASCSMode);
	idx += 1;

	SetPhysicalSwitchStatus(physicalSwitchFirstGroup + 4, switchControlMode);
	idx += 1;

	// Then we have 8 labels which we skip.
	idx += 8;

	// And then the "T" handles.
	SetTHandleState(abortIndicatorGroup + idx, tHandleManualPushed, 0);
	idx += 1;

	SetTHandleState(abortIndicatorGroup + idx, tHandleRollPushed, 1);
	idx += 1;

	SetTHandleState(abortIndicatorGroup + idx, tHandleYawPushed, 2);
	idx += 1;

	SetTHandleState(abortIndicatorGroup + idx, tHandlePitchPushed, 3);
	idx += 1;
}

inline void ProjectMercury::SetIndicatorStatus(int indicatorNr, int status)
{
	GROUPEDITSPEC ges;
	NTVERTEX newVertex[41]; // up to 41 vertices in a group (either 4 for rectangle or 41 for circle)

	ges.flags = GRPEDIT_VTXTEXU | GRPEDIT_VTXTEXV;

	if (indicatorNr == 0) // circular abort
	{
		ges.nVtx = 41; // circle

		WORD vertexIndex[41];

		// Centre
		newVertex[0].tu = (538.0f + float(status) * 486.0f) / 2048.0f;
		newVertex[0].tv = 1703.0f / 2048.0f;

		vertexIndex[0] = 0;

		// Perimeter
		for (int i = 0; i < 40; i++)
		{
			vertexIndex[i + 1] = i + 1;

			double angle = double(i) * PI2 / 40.0;
			float x0 = 538.0f + 486.0f * float(status);
			float y0 = 1703.0f;
			
			newVertex[i + 1].tu = (150.0f * float(sin(angle)) + x0) / 2048.0f;
			newVertex[i + 1].tv = (150.0f * float(cos(angle)) + y0) / 2048.0f;

			// It seems like I have to do something here, or else Orbiter fails to correctly place vertices. So call a stupid function or something.
			double uselessValue = GetPitch();
			uselessValue *= GGRAV;
		}

		ges.vIdx = vertexIndex;
	}
	else // rectangle
	{
		ges.nVtx = 4; // rectangle

		// Create vertex idx array
		WORD vertexIndex[4] = { 0, 1, 2, 3 };
		ges.vIdx = vertexIndex;

		// Source 2048*2048 px^2
		float texX0 = (363.0f + float(status) * 486.0f) / 2048.0f; 
		float texX1 = (714.0f + float(status) * 486.0f) / 2048.0f;
		float texY0 = (185.0f + (12.0f - float(indicatorNr)) * 113.27f) / 2048.0f;
		float texY1 = (277.0f + (12.0f - float(indicatorNr)) * 113.27f) / 2048.0f;

		newVertex[0].tu = texX0;	newVertex[0].tv = texY0;
		newVertex[1].tu = texX1;	newVertex[1].tv = texY0;
		newVertex[2].tu = texX0;	newVertex[2].tv = texY1;
		newVertex[3].tu = texX1;	newVertex[3].tv = texY1;

		// It seems like I have to do something here, or else Orbiter fails to correctly place vertices. So call a stupid function or something.
		double uselessValue = GetPitch();
		uselessValue *= GGRAV;
	}

	ges.Vtx = newVertex;
	oapiEditMeshGroup(cockpitPanelMesh, abortIndicatorGroup + indicatorNr, &ges);
}

inline void ProjectMercury::SetIndicatorButtonStatus(int buttonNr, int status)
{
	GROUPEDITSPEC ges;
	NTVERTEX newVertex[4]; // verticies per group, 4 for rectangle

	ges.flags = GRPEDIT_VTXTEXU; // just switch x pos, as constant y

	ges.nVtx = 4; // rectangle

	// Create vertex idx array
	WORD vertexIndex[4] = { 0, 1, 2, 3 };
	ges.vIdx = vertexIndex;

	// Source 256*256 px^2
	float butRad = 0.2f;
	float butOfs = 57.0f / 256.0f;
	float texX0 = 0.5f + float(status) * butOfs - butRad;
	float texX1 = 0.5f + float(status) * butOfs + butRad;

	newVertex[0].tu = texX0;
	newVertex[1].tu = texX1;
	newVertex[2].tu = texX0;
	newVertex[3].tu = texX1;

	// It seems like I have to do something here, or else Orbiter fails to correctly place vertices. So call a stupid function or something.
	//double uselessValue = GetPitch();
	//uselessValue *= GGRAV;

	ges.Vtx = newVertex;
	oapiEditMeshGroup(cockpitPanelMesh, abortIndicatorGroup + buttonNr, &ges);

	indicatorButtonPressState[buttonNr - indicatorButtonFirstGroup] = status;
}

inline void ProjectMercury::SetPhysicalSwitchStatus(int switchNr, int status)
{
	GROUPEDITSPEC ges;
	NTVERTEX newVertex[4]; // verticies per group, 4 for rectangle

	ges.flags = GRPEDIT_VTXTEXU; // just switch x pos, as constant y

	ges.nVtx = 4; // rectangle

	// Create vertex idx array
	WORD vertexIndex[4] = { 0, 1, 2, 3 };
	ges.vIdx = vertexIndex;

	// Source 2048*1024 px^2
	float butWidth = 350.0f / 2048.0f;
	float aspectRatio = 0.5f;
	float butHeight = (350.0f * aspectRatio) / 1024.0f;
	float switchOffset = 349.0f / 2048.0f;
	float texX0 = 0.5f + float(status) * switchOffset - butWidth / 2.0f;
	float texX1 = 0.5f + float(status) * switchOffset + butWidth / 2.0f;

	newVertex[0].tu = texX0;
	newVertex[1].tu = texX1;
	newVertex[2].tu = texX0;
	newVertex[3].tu = texX1;

	// It seems like I have to do something here, or else Orbiter fails to correctly place vertices. So call a stupid function or something.
	//double uselessValue = GetPitch();
	//uselessValue *= GGRAV;

	ges.Vtx = newVertex;
	oapiEditMeshGroup(cockpitPanelMesh, abortIndicatorGroup + switchNr, &ges);

	//physicalSwitchState[switchNr - physicalSwitchFirstGroup] = status;
}

inline void ProjectMercury::SetTHandleState(int groupIdx, bool pushed, int handleNum)
{
	GROUPEDITSPEC ges;
	NTVERTEX newVertex[4]; // verticies per group, 4 for rectangle

	ges.flags = GRPEDIT_VTXTEXU | GRPEDIT_VTXTEXV; // edit both u and v coord

	ges.nVtx = 4; // rectangle

	// Create vertex idx array
	WORD vertexIndex[4] = { 0, 1, 2, 3 };
	ges.vIdx = vertexIndex;

	// Source 1024*1024 px^2
	float zoom = 0.9f;
	if (!pushed) zoom *= 1.2f; // pulled towards pilot, so increase size

	float imgX = 0.225f;
	float imgY = 0.225f;
	switch (handleNum)
	{
	case 0:
		imgX = 0.5f - imgX + 0.05f;
		imgY = 0.5f - imgY;
		break;
	case 1:
		imgX = 0.5f - imgX + 0.05f;
		imgY = 0.5f + imgY;
		break;
	case 2:
		imgX = 0.5f + imgX + 0.05f;
		imgY = 0.5f - imgY;
		break;
	case 3:
		imgX = 0.5f + imgX + 0.05f;
		imgY = 0.5f + imgY;
		break;
	default:
		break;
	}

	/*float butWidth = 350.0f / 2048.0f;
	float aspectRatio = 0.5f;
	float butHeight = (350.0f * aspectRatio) / 1024.0f;
	float switchOffset = 349.0f / 2048.0f;*/
	float texX0 = imgX - 0.25f / zoom;
	float texX1 = imgX + 0.25f / zoom;
	float texY0 = imgY - 0.25f / zoom;
	float texY1 = imgY + 0.25f / zoom;

	newVertex[0].tu = texX0; newVertex[0].tv = texY0;
	newVertex[1].tu = texX1; newVertex[1].tv = texY0;
	newVertex[2].tu = texX0; newVertex[2].tv = texY1;
	newVertex[3].tu = texX1; newVertex[3].tv = texY1;

	// It seems like I have to do something here, or else Orbiter fails to correctly place vertices. So call a stupid function or something.
	//double uselessValue = GetPitch();
	//uselessValue *= GGRAV;

	ges.Vtx = newVertex;
	oapiEditMeshGroup(cockpitPanelMesh, groupIdx, &ges);

	//physicalSwitchState[switchNr - physicalSwitchFirstGroup] = status;
}

inline void ProjectMercury::clbkFocusChanged(bool getfocus, OBJHANDLE hNewVessel, OBJHANDLE hOldVessel)
{
	if (getfocus) // we gained focus (if else, we lost focus, and then don't need to update)
	{
		for (int i = 0; i < 13; i++)
			previousIndicatorStatus[i] = -1; // reset. If this is not done, a panel will inherit the state of the previously viewed panel.

		for (int i = 0; i < 20; i++)
			indicatorButtonPressState[i] = 0; // reset. (-1 is default off)
	}
}