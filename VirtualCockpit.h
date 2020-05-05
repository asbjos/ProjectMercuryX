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
		oapiWriteLog("Sorry, periscope and rocket cameras are disabled in Orbiter 2010 and earlier, as the views give crash to desktop. I'll try to find a solution.");
		return false; // Don't support periscope as for now, as it leads to a crash
	}

	const DWORD PANEL2D_WIDTH = 2160; // 2160
	const DWORD PANEL2D_HEIGHT = 1440; // 1440
	double defaultScale = (double)viewH / PANEL2D_HEIGHT; // nobody has a < 1 aspect ratio (I hope)
	double	panelScale = max(defaultScale, 1.0);
	//panelTexture = NULL;

	if (id == 1 && (VesselStatus != FLIGHT && VesselStatus != REENTRY && VesselStatus != REENTRYNODROGUE && !(VesselStatus == LAUNCH && GroundContact()))) // must be in capsule mode
	{
		id = 2; // send to rocket cam
	}
	else if (id == 1 && (CapsuleVersion == FAITH7 || CapsuleVersion == FREEDOM7II || CapsuleVersion == CAPSULEBIGJOE || CapsuleVersion == CAPSULELITTLEJOE || CapsuleVersion == CAPSULEBD)) // these two did not feature a periscope
	{
		if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) // no rocket cam
			id = 0; // Did not include periscope, send back to panel
		else
			id = 2; // Did not include periscope, send to rocket cam
	}

	//oapiWriteLog("Debug 1");
	//// Work with the surface
	//if (panelDynamicTexture) oapiDestroySurface(panelDynamicTexture);
	//panelDynamicTexture = oapiCreateSurfaceEx(12 * 75, 2 * 50, OAPISURFACE_SKETCHPAD | OAPISURFACE_UNCOMPRESS | OAPISURFACE_TEXTURE);	// surface for dynamic elements
	//SURFHANDLE panelTextures[] = { panelDynamicTexture };
	//SURFHANDLE panelTexes[2];

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
		//panelTexture = oapiGetTextureHandle(cockpitPanelMesh, 1);
		//panelTexes[0] = panelTexture;
		//panelTexes[1] = globeTexture;

		//oapiWriteLog("Debug 2");

		SetPanelBackground(hPanel, 0, 0, cockpitPanelMesh, PANEL2D_WIDTH, PANEL2D_HEIGHT, 0UL, PANEL_ATTACH_TOP | PANEL_ATTACH_BOTTOM);

		SetPanelScaling(hPanel, defaultScale, panelScale);
		// End periscope indicators
		//oapiWriteLog("Debug 10");

		// Click to reset load indicator
		VersionDependentPanelClick(id, _R(190 - 150, 850 - 185, 190 + 150, 850 + 185), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);


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
		//panelTexture = oapiGetTextureHandle(periscopeMesh, 1);

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
		VersionDependentPanelClick(id, _R(0, 0, PANEL2D_WIDTH, PANEL2D_HEIGHT), 0, PANEL_REDRAW_NEVER, PANEL_MOUSE_LBDOWN, hPanel, _R(0, 0, 0, 0), NULL);
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

			// Here comes circular frame
			//const DWORD PANEL2D_WIDTH = 2160; // 2160
			//const DWORD PANEL2D_HEIGHT = 1440; // 1440
			//panelTexture = oapiGetTextureHandle(circularFrameMesh, 1);

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
	//oapiWriteLog("Debug 9");
}

inline void ProjectMercury::clbkVisualCreated(VISHANDLE vis, int refcount)
{
	// Initialise periscope
	SetPeriscopeAltitude(periscopeAltitude);

	// Set instrument panel on far left and right
	FitPanelToScreen(ScreenWidth, ScreenHeight);
}

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

// Fit the current panel mesh (1.5 ratio) to any screen ratio, by pushing elements to screen border
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

		//oapiWriteLogV("Group %i has %i vertices", i, groupData->nVtx);
	}

	// Edit mesh
	double defaultScale = (double)h / 1440;
	addScreenWidthValue =float((w / defaultScale - 2160.0) / 2.0); // add or subtract offset value
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
	int idx = 0;

	// positions:
	float gMeterPos[2] =	{ 190, 850 };
	float fuelPos[2] =		{525, 1000};
	float descentPos[2] =	{525, 1290};
	float altPos[2] =		{190, 1250};
	float clockPos[2] =		{1820, 540};
	float earthPos[2] = { 1820, 1140 };
	float attitudeCenter[2] = { 1080, 800 };
	float pitchPos[2] =		{attitudeCenter[0] + 250, attitudeCenter[1]};
	float rollPos[2] =		{attitudeCenter[0] - 250, attitudeCenter[1]};
	float yawPos[2] =		{attitudeCenter[0], attitudeCenter[1] + 270};
	float attitudePos[2] = { attitudeCenter[0], attitudeCenter[1] };

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
	if (!attitudeFuelAuto) // swap (just how I've defined it)
	{
		fuelAuto = float(GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_AUTO);
		fuelManual = float(GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_MAN);
	}

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

	// Yaw
	float yaw = float(normangle((GetSlipAngle() + yawOffset)));
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

	// Clock hour
	RotateArmGroup(armGroups[idx], clockPos[0] - 173, clockPos[1] - 116, 55.0f, 10.0f, ValueToAngle(hour, 0.0f, 24.0f, -90.0f, 270.0f), 0.7f, 0.0f, false); // Note that the hour dial did one revolution per day!
	idx += 1;
	// Clock minute
	RotateArmGroup(armGroups[idx], clockPos[0] - 173, clockPos[1] - 116, 70.0f, 10.0f, ValueToAngle(minute, 0.0f, 60.0f, -90.0f, 270.0f), 0.8f, 0.0f, false);
	idx += 1;
	// Clock second
	RotateArmGroup(armGroups[idx], clockPos[0] - 173, clockPos[1] - 116, 110.0f, 3.0f, ValueToAngle(second, 0.0f, 60.0f, -90.0f, 270.0f), 0.95f, 0.0f, false);
	idx += 1;

	// Attitude rate
	// Get attitude rate
	VECTOR3 angVel;
	GetAngularVel(angVel);
	// MinMax +- 40 deg/s
	float pRate = float(angVel.x * DEG);
	if (pRate > 40.0f) pRate = 40.0f;
	if (pRate < -40.0f) pRate = -40.0f;
	float yRate = float(angVel.y * DEG);
	if (yRate > 40.0f) yRate = 40.0f;
	if (yRate < -40.0f) yRate = -40.0f;
	float rRate = float(angVel.z * DEG);
	if (rRate > 40.0f) rRate = 40.0f;
	if (rRate < -40.0f) rRate = -40.0f;

	float pPixelY = attitudePos[1] + pRate * 3.0f;
	float yPixelX = attitudePos[0] + yRate * 3.0f;
	float rPixelX = attitudePos[0] + rRate * 3.0f;

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
	//oapiGetEquPos(GetHandle(), )
	GetEquPos(posLong, posLat, posRad);
	oapiGetHeading(GetHandle(), &heading);
	VECTOR3 currPos, currVel;
	GetRelativePos(GetSurfaceRef(), currPos);
	GetRelativeVel(GetSurfaceRef(), currVel);
	heading -= OrbitalFrameSlipAngle2(currPos, currVel);
	RotateGlobe(5.0f, 60.0f, float(posLong), float(posLat), float(heading - PI05));

	// Clock numerals
	double simt = oapiGetSimTime();
	double met = simt - launchTime;
	double metFlow = met; // keep met unchanged, as it's used for retrotime
	if (metFlow > (99 * 3600 + 59 * 60 + 59)) metFlow = fmod(metFlow, 100 * 3600); // overflow
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
		if (abs(normangle(angleR - previousDialAngle[groupNum])) > simdt* dialAngularSpeed)
		{
			float signDirection = (angleR - previousDialAngle[groupNum]) / abs(angleR - previousDialAngle[groupNum]);
			angleR = previousDialAngle[groupNum] + simdt * dialAngularSpeed * signDirection;
		}
		previousDialAngle[groupNum] = angleR;
	}

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
				newVertex[idx].tv = -pointLat / PI + 0.5; // latitude is normal
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
				newVertex[idx].tv = -pointLat / PI + 0.5; // latitude is normal
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
				newVertex[idx].tv = -pointLat / PI + 0.5; // latitude is normal
			}
		}
	}

	ges.Vtx = newVertex;
	oapiEditMeshGroup(cockpitPanelMesh, globeGroup, &ges);
}

inline void ProjectMercury::ChangePanelNumber(int group, int num)
{
	float digitToShow1 = int(num / 10); // 24 -> 2
	float digitToShow2 = num - int(num / 10) * 10; // 24 -> 4

	GROUPEDITSPEC ges;
	NTVERTEX newVertex[4]; // rectangle

	ges.flags = GRPEDIT_VTXTEXU | GRPEDIT_VTXTEXV;
	ges.nVtx = 4; // rectange

	// Create vertex idx array
	WORD vertexIndex[4] = { 0, 1, 2, 3 };
	ges.vIdx = vertexIndex;

	float texX0 = 1683.0f / 2048.0f;
	float texX1 = 1717.0f / 2048.0f;
	float texY0 = (305.0f + digitToShow1 * 60.0f) / 1024.0f;
	float texY1 = (351.0f + digitToShow1 * 60.0f) / 1024.0f;

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
