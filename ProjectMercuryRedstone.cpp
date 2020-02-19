// ==============================================================
//				Source file for Mercury Redstone.
//				Created by Asbj�rn "asbjos" Kr�ger
//					asbjorn.kruger@gmail.com
//						Made in 2019
// 
// Based on Project Mercury addon by "estar", with permission.
// This code is my own work.
// 
// Thank you to Rob Conley and the rest of the original Project
// Mercury team. Especially thank you for releasing the source.
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================

#define STRICT
#define ORBITER_MODULE

#include "orbitersdk.h"

#include "MercuryRedstone.h"
#include "MercuryCapsule.h"

ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel)
	: VESSEL4(hVessel, flightmodel)
{
	// Load exterior meshes
	// capsule is defined in loadstate
	redstone = oapiLoadMeshGlobal("ProjectMercury\\merc_redstone");
	
	MercuryGenericConstructor();
}

ProjectMercury::~ProjectMercury()
{
	
	WriteFlightParameters();
}

// --------------------------------------------------------------
// Set the capabilities of the vessel class
// --------------------------------------------------------------
void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	// physical vessel parameters
	SetSize(STAGE1_LENGTH + MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT);
	SetEmptyMass(MERCURY_MASS + STAGE1_DRY_MASS + ABORT_MASS);

	ReadConfigSettings(cfg);

	static const DWORD tchdwnLaunchPadNum = 4;
	static TOUCHDOWNVTX tchdwnLaunchPad[tchdwnLaunchPadNum] = {
		// pos, stiff, damping, mu, mu long
		{_V(0.0, -1.0, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0 - heightOverGround), 1e7, 1e5, 10},
		{_V(-0.7, 0.7, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0 - heightOverGround), 1e7, 1e5, 10},
		{_V(0.7, 0.7, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0 - heightOverGround), 1e7, 1e5, 10},
		{_V(0.0, 0.0, MERCURY_OFS_ABORT.z + MERCURY_LENGTH_ABORT / 2.0), 1e7, 1e5, 10},
	};
	SetTouchdownPoints(tchdwnLaunchPad, tchdwnLaunchPadNum);

	SetCW(1.0, 0.1, 0.3, 0.3);
	SetCrossSections(_V(38.4, 38.4, 4.04));
	SetRotDrag(_V(0.7, 0.7, 1.2));
	SetPMI(_V(33.0, 33.0, 20.51)); // actually much smaller z-component, but makes atmospheric flight unstable at >1x

	CreateCapsuleFuelTanks();

	redstone_propellant = CreatePropellantResource(STAGE1_FUEL_MASS);
	//SetPropellantMass(redstone_propellant, STAGE1_FUEL_MASS);

	SetDefaultPropellantResource(redstone_propellant);

	// main engine
	th_main = CreateThruster(REDSTONE_EXHAUST_POS, REDSTONE_EXHAUST_DRI, STAGE1_THRUST, redstone_propellant, STAGE1_ISP_VAC, STAGE1_ISP_SL);
	CreateThrusterGroup(&th_main, 1, THGROUP_MAIN);
	AddExhaust(th_main, 8, 1, REDSTONE_EXHAUST_POS, -REDSTONE_EXHAUST_DRI);

	// Dummy RCS for jet vanes and air rudders
	th_rcsDummyBooster[0] = CreateThruster(_V(0, 1, -STAGE1_LENGTH / 2), _V(0, 1, 0), 0, redstone_propellant, 0, 0);
	th_rcsDummyBooster[1] = CreateThruster(_V(0, -1, -STAGE1_LENGTH / 2), _V(0, -1, 0), 0, redstone_propellant, 0, 0);
	th_rcsDummyBooster[2] = CreateThruster(_V(1, 0, -STAGE1_LENGTH / 2), _V(1, 0, 0), 0, redstone_propellant, 0, 0);
	th_rcsDummyBooster[3] = CreateThruster(_V(-1, 0, -STAGE1_LENGTH / 2), _V(-1, 0, 0), 0, redstone_propellant, 0, 0);
	th_rcsDummyBooster[4] = CreateThruster(_V(1, 0, -STAGE1_LENGTH / 2), _V(0, 1, 0), 0, redstone_propellant, 0, 0);
	th_rcsDummyBooster[5] = CreateThruster(_V(1, 0, -STAGE1_LENGTH / 2), _V(0, -1, 0), 0, redstone_propellant, 0, 0);
	CreateThrusterGroup(&th_rcsDummyBooster[0], 1, THGROUP_ATT_PITCHUP);
	CreateThrusterGroup(&th_rcsDummyBooster[1], 1, THGROUP_ATT_PITCHDOWN);
	CreateThrusterGroup(&th_rcsDummyBooster[2], 1, THGROUP_ATT_YAWLEFT); // maybe switch later
	CreateThrusterGroup(&th_rcsDummyBooster[3], 1, THGROUP_ATT_YAWRIGHT);
	CreateThrusterGroup(&th_rcsDummyBooster[4], 1, THGROUP_ATT_BANKLEFT); // maybe switch later
	CreateThrusterGroup(&th_rcsDummyBooster[5], 1, THGROUP_ATT_BANKRIGHT);

	// Control surfaces for air rudders. dCl and delay values from DeltaGlider and/or Falcon 9 by BrianJ
	CreateAirfoilsRedstone(); // try this
	Rudders[0] = CreateControlSurface3(AIRCTRL_ELEVATOR, 0.515*2, rudderLift, _V(0.0, 0.0, -8.6), AIRCTRL_AXIS_XPOS, rudderDelay); // check Airctrl_Axis
	Rudders[1] = CreateControlSurface3(AIRCTRL_RUDDER, 0.515*2, rudderLift, _V(0.0, 0.0, -8.6), AIRCTRL_AXIS_YPOS, rudderDelay);
	// Rudders[2] = CreateControlSurface3(AIRCTRL_ELEVATOR, 0.515, 3, _V(0.0, 1.8, -8.6), AIRCTRL_AXIS_YPOS, 1.0);
	//Rudders[2] = CreateControlSurface3(AIRCTRL_AILERON, 0.515, 1.7, _V(0.0, 1.8, -8.6), AIRCTRL_AXIS_YPOS, 0.5);
	//Rudders[3] = CreateControlSurface3(AIRCTRL_FLAP, 0.515, 1.7, _V(1.8, 0.0, -8.6), AIRCTRL_AXIS_XPOS, 0.5);
	//DefineRudderAnimations();

	// Contrail stuff
	contrail_main = {
		0, 3.0, 100, 200, 0.1, 20.0, 2.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin, contrailEnd
	};
	contrail = AddExhaustStream(th_main, _V(0, 0.3, -20), &contrail_main);

	// associate a mesh for the visual
	Redstone = AddMesh(redstone, &REDSTONE_OFFSET);
	// capsule defined in LoadState
	AddDefaultMeshes();

	DefineRudderAnimations();
	DefineJetVaneAnimations();

	padAttach = CreateAttachment(true, _V(0.0, 0.0, REDSTONE_OFFSET.z - STAGE1_LENGTH / 2.0), _V(0, 0, 1), _V(0, 0, 1), "PAD", true);
}

void ProjectMercury::clbkPostCreation()
{
	CapsuleGenericPostCreation();

	if (VesselStatus == TOWERSEP)
	{
		DelMesh(Tower);
	}
	else if (VesselStatus == ABORT)
	{
		// Airfoil-stuff
		CreateAirfoilsEscape();

		DelThruster(th_main);
		DelPropellantResource(redstone_propellant);
		DelMesh(Redstone);
		DelMesh(Adaptcover1);
		DelMesh(Adaptcover2);
		DelMesh(Adaptcover3);
		DelMesh(Adaptring1);
		DelMesh(Adaptring2);
		DelMesh(Adaptring3);

		CreateAbortThrusters();
		// SetThrusterGroupLevel(THGROUP_MAIN, 1.0); // Probably already fired. No scenario saved in the second it fires.

		//oapiWriteLog("PostCreation ABORT");
	}
	else if (VesselStatus == ABORTNORETRO)
	{
		// Airfoil-stuff
		CreateAirfoilsEscape();

		DelThruster(th_main);
		DelPropellantResource(redstone_propellant);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);
		DelMesh(Redstone);
		DelMesh(Adaptcover1);
		DelMesh(Adaptcover2);
		DelMesh(Adaptcover3);
		DelMesh(Adaptring1);
		DelMesh(Adaptring2);
		DelMesh(Adaptring3);
		DelMesh(Retro);
		DelMesh(Retrocover1);
		DelMesh(Retrocover2);
		DelMesh(Retrocover3);
		DelMesh(Retrostrap1);
		DelMesh(Retrostrap2);
		DelMesh(Retrostrap3);
		DelMesh(Explosivebolt);
		
		CreateAbortThrusters();

		//oapiWriteLog("PostCreation ABORTNORETRO");
	}
	else if (VesselStatus == FLIGHT)
	{
		// Airfoil-stuff
		CreateAirfoils();

		DelThruster(th_main);
		DelPropellantResource(redstone_propellant);
		DelPropellantResource(escape_tank);
		DelMesh(Tower);
		DelMesh(Redstone);
		DelMesh(Adaptcover1);
		DelMesh(Adaptcover2);
		DelMesh(Adaptcover3);
		DelMesh(Adaptring1);
		DelMesh(Adaptring2);
		DelMesh(Adaptring3);

		// CreatePosigradeRockets(); // Probably already fired. No scenario saved in the second they fire.
		CreateRetroRockets();
		//CreateRCS();
	}
	else if (VesselStatus == REENTRY)
	{
		CreateAirfoils();

		DelThruster(th_main);
		DelPropellantResource(redstone_propellant);
		DelPropellantResource(escape_tank);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);

		//CreateRCS();

		DelMesh(Tower);
		DelMesh(Redstone);
		DelMesh(Adaptcover1);
		DelMesh(Adaptcover2);
		DelMesh(Adaptcover3);
		DelMesh(Adaptring1);
		DelMesh(Adaptring2);
		DelMesh(Adaptring3);
		DelMesh(Retro);
		DelMesh(Retrocover1);
		DelMesh(Retrocover2);
		DelMesh(Retrocover3);
		DelMesh(Retrostrap1);
		DelMesh(Retrostrap2);
		DelMesh(Retrostrap3);
		DelMesh(Explosivebolt);
	}
	else if (VesselStatus == REENTRYNODROGUE)
	{
		CreateAirfoils();

		DelThruster(th_main);
		DelPropellantResource(redstone_propellant);
		DelPropellantResource(escape_tank);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);

		//CreateRCS();

		DelMesh(Tower);
		DelMesh(Redstone);
		DelMesh(Adaptcover1);
		DelMesh(Adaptcover2);
		DelMesh(Adaptcover3);
		DelMesh(Adaptring1);
		DelMesh(Adaptring2);
		DelMesh(Adaptring3);
		DelMesh(Retro);
		DelMesh(Retrocover1);
		DelMesh(Retrocover2);
		DelMesh(Retrocover3);
		DelMesh(Retrostrap1);
		DelMesh(Retrostrap2);
		DelMesh(Retrostrap3);
		DelMesh(Explosivebolt);

		DelMesh(Antennahouse);
		DelMesh(Droguecover);
		// Drogue-mesh has not been created, and thus needs no deletion
	}
}

void ProjectMercury::clbkPreStep(double simt, double simdt, double mjd)
{
	// Automatic abort
	if (VesselStatus == LAUNCH && abortConditionsMet)
		LaunchAbort();

	// Actions
	if (separateTowerAction)
	{
		TowerSeparation();
		separateTowerAction = false;
	}
	else if (separateCapsuleAction)
	{
		CapsuleSeparate();
		separateCapsuleAction = false;
	}
	else if (prepareReentryAction)
	{
		PrepareReentry();
		prepareReentryAction = false;
	}
	else if (separateRetroCoverAction[0])
	{
		SeparateRetroCoverN(1);
		separateRetroCoverAction[1 - 1] = false;
	}
	else if (separateRetroCoverAction[1])
	{
		SeparateRetroCoverN(2);
		separateRetroCoverAction[2 - 1] = false;
	}
	else if (separateRetroCoverAction[2])
	{
		SeparateRetroCoverN(3);
		separateRetroCoverAction[3 - 1] = false;
	}
	else if (separateDrogueCoverAction)
	{
		SeparateDrogueCover();
		separateDrogueCoverAction = false;
	}
	else if (separateDrogueAction)
	{
		SeparateDrogue();
		separateDrogueAction = false;
	}
	else if (separateMainChuteAction)
	{
		SeparateMainChute();
		separateMainChuteAction = false;
	}

	SetEmptyMass(EmptyMass()); // calculate drymass of current setting (launch, abort, flight ...)

	switch (VesselStatus)
	{
	case LAUNCH:
		// Allow to be launched from LC5.The trigger is if engine is lit from an attached vessel
		OBJHANDLE attachedParent;
		attachedParent = GetAttachmentStatus(padAttach);
		if (attachedParent != NULL && oapiGetFocusObject() == attachedParent && GetThrusterGroupLevel(THGROUP_MAIN) != 0.0) // attached, pad in focus, engines on, and has been uninitialised. Launch is initiated from pad
		{
			oapiSetTimeAcceleration(1.0);
			autoPilot = true;
			launchTime = oapiGetSimTime() + 0.0;
			integratedSpeed = 0.0;
		}
		
		break;
	case TOWERSEP:
		if (!CGshifted && !GroundContact())
		{
			SetSize(STAGE1_LENGTH + MERCURY_LENGTH_CAPSULE);
			SetTouchdownPoints(tchdwnTowSep, tchdwnTowSepNum);
			CGshifted = true; // Yeah, not bothering with CG here

			//oapiWriteLog("Shift to TOWERSEP");
			PreviousVesselStatus = TOWERSEP;
		}
		break;
	case FLIGHT:
		if (!CGshifted && !GroundContact())
		{
			CreateAirfoils();

			SetSize(MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_RETRO);
			SetTouchdownPoints(tchdwnFlight, tchdwnFlightNum);
			ShiftCG(MERCURY_OFS_CAPSULE + MERCURY_CAPSULE_OFFSET); // only from TOWERSEP or scenario
			SetCameraOffset(_V(0.0, 0.0, 0.0));
			ShiftCentreOfMass(_V(0.0, 0.0, -0.6)); // Allign RCS to CentreOfMass
			SetCrossSections(_V(3.5, 3.37, 2.8));
			// SetPMI(_V(0.68, 0.63, 0.303)); // original
			// SetPMI(_V(0.39, 0.39, 0.35)); // according to pitch acc data doi:10.1002/j.2161-4296.1962.tb02524.x
			SetPMI(_V(0.39, 0.39, 0.28)); // Set to match the preceeding data
			CGshifted = true;
			CreateRCS();
			//oapiWriteLog("RCS created AFTER CoG-shift");
			PreviousVesselStatus = FLIGHT;
		}
		break;
	case REENTRY:
		if (!CGshifted && !GroundContact())
		{
			CreateAirfoils();

			double oldSize = GetSize();
			SetSize(MERCURY_LENGTH_CAPSULE);
			SetTouchdownPoints(tchdwnFlight, tchdwnFlightNum);

			if (PreviousVesselStatus == ABORT || PreviousVesselStatus == ABORTNORETRO)
			{
				ShiftCG(-_V(0.0, 0.0, (77.8 - 19.3) * 0.0254) + MERCURY_CAPSULE_OFFSET);  // 19670022650 page 36 (CG of escape setup at 77.8 in, compared to 19.3 in for spacecraft)
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			}
			else if (PreviousVesselStatus == LAUNCH) // spawned in REENTRY from scenario
			{
				ShiftCG(MERCURY_OFS_CAPSULE + MERCURY_CAPSULE_OFFSET);
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			} // else from FLIGHT and don't need to shift

			SetPMI(_V(0.39, 0.39, 0.28)); // Set to match the preceeding data
			SetCrossSections(_V(2.8, 2.8, 2.8));
			CGshifted = true;
			CreateRCS();
			//oapiWriteLog("RCS created AFTER CoG-shift");

			//oapiWriteLog("Shift to REENTRY");
			PreviousVesselStatus = REENTRY;
		}
		else if (drogueDeployed)
		{
			AnimateDrogueChute(simt, simdt);
		}
		break;
	case REENTRYNODROGUE:
		if (!CGshifted && !GroundContact())
		{
			CreateAirfoils();

			double oldSize = GetSize();
			SetSize(MERCURY_LENGTH_CAPSULE);
			SetTouchdownPoints(tchdwnFlight, tchdwnFlightNum);

			if (PreviousVesselStatus == LAUNCH) // scenario spawned at REENTRYNODROGUE state
			{
				ShiftCG(MERCURY_OFS_CAPSULE + MERCURY_CAPSULE_OFFSET);
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			} // else don't shift, as came from REENTRY

			SetPMI(_V(0.39, 0.39, 0.28)); // Set to match the preceeding data
			SetCrossSections(_V(2.8, 2.8, 2.8));
			CGshifted = true;
			CreateRCS();
			//oapiWriteLog("RCS created AFTER CoG-shift");

			//oapiWriteLog("Shift to REENTRYNODROGUE");
			PreviousVesselStatus = REENTRYNODROGUE;
		}
		else if (!mainChuteSeparated && GroundContact())
		{
			separateMainChuteAction = true;
			oapiWriteLog("sep main on contact");
		}
		else if (reserveChuteDeployed && !reserveChuteSeparated && GroundContact())
		{
			oapiWriteLog("Sep reserve on contact");
			separateMainChuteAction = true;
		}

		if (mainChuteDeployed)
		{
			AnimateMainChute(simt, simdt);
		}
		
		if (landingBagDeployed)
		{
			AnimateLandingBag(simt, simdt);
		}
		break;
	case ABORT:
		if (!CGshifted && !GroundContact())
		{
			CreateAirfoilsEscape();

			SetSize(MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT);
			SetTouchdownPoints(tchdwnAbort, tchdwnAbortNum);
			if (PreviousVesselStatus == LAUNCH) // either from aborting or from scenario load
			{
				ShiftCG(MERCURY_OFS_CAPSULE + _V(0.0, 0.0, (77.8 - 19.3) * 0.0254)); // 19670022650 page 36 (CG of escape setup at 77.8 in, compared to 19.3 in for spacecraft)
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			}
			SetPMI(_V(5.0, 5.0, 0.4)); // 19670022650 page 5 and 6 (5650 slug*ft^2 and total mass 3400 pounds)
			SetCrossSections(_V(4.3, 4.3, 3.17)); // grossly approx
			CGshifted = true;
			//oapiWriteLogV("Tot abort mass is %.2f kg", GetMass());

			//oapiWriteLog("Shift to ABORT");
			PreviousVesselStatus = ABORT;
		}

		escapeLevel = GetThrusterLevel(escape_engine);

		if (inFlightAbort && vesselAcceleration < 0.25 * G && (simt - abortTime) > 2.0) // When aborting from inflight, and acc<0.25g
		{
			InflightAbortSeparate();
			//oapiWriteLog("Finished 0.25G separation");
		}
		break;
	case ABORTNORETRO:
		if (!CGshifted && !GroundContact())
		{
			CreateAirfoilsEscape();
			SetSize(MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT);
			SetTouchdownPoints(tchdwnAbort, tchdwnAbortNum);
			if (PreviousVesselStatus == LAUNCH) // either abort or from scenario
			{
				ShiftCG(MERCURY_OFS_CAPSULE +_V(0.0, 0.0, (77.8 - 19.3) * 0.0254));
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			}
			SetPMI(_V(5.0, 5.0, 0.4)); // 19670022650 page 5 and 6 (5650 slug*ft^2 and total mass 3400 pounds)
			SetCrossSections(_V(4.3, 4.3, 3.17)); // grossly approx
			CGshifted = true;

			//oapiWriteLog("Shift to ABORTNORETRO");
			//oapiWriteLogV("Tot abortnoretro mass is %.2f kg", GetMass());
			PreviousVesselStatus = ABORTNORETRO;
		}

		escapeLevel = GetThrusterLevel(escape_engine);

		if (vesselAcceleration < 0.25 * G && (simt - abortTime) > 2.0) // When aborting from inflight, and acc<0.25g
		{
			//oapiWriteLog("Low G key");
			OffPadAbortSeparate();
		}
	}

	DeleteRogueVessels();
}

void ProjectMercury::clbkPostStep(double simt, double simdt, double mjd)
{
	// Animate rudders
	if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP)
	{
		SetAnimation(Rudder1Anim, 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_RUDDER)); // calibrate these
		SetAnimation(Rudder2Anim, 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_ELEVATOR));
		SetAnimation(Rudder3Anim, 0.5 - 0.5 * GetControlSurfaceLevel(AIRCTRL_RUDDER));
		SetAnimation(Rudder4Anim, 0.5 - 0.5 * GetControlSurfaceLevel(AIRCTRL_ELEVATOR));

		SetAnimation(JetVaneAnim[0], 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_RUDDER));
		SetAnimation(JetVaneAnim[1], 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_ELEVATOR));
		SetAnimation(JetVaneAnim[2], 0.5 - 0.5 * GetControlSurfaceLevel(AIRCTRL_RUDDER));
		SetAnimation(JetVaneAnim[3], 0.5 - 0.5 * GetControlSurfaceLevel(AIRCTRL_ELEVATOR));
		//SetAnimation(Rudder3Anim, 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_AILERON));
		//SetAnimation(Rudder4Anim, 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_FLAP));

		/*SetAnimation(Rudder1Anim, 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_AILERON));
		SetAnimation(Rudder2Anim, 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_AILERON));
		SetAnimation(Rudder3Anim, 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_AILERON));
		SetAnimation(Rudder4Anim, 0.5 + 0.5 * GetControlSurfaceLevel(AIRCTRL_AILERON));*/
	}

	if (VesselStatus == LAUNCH && enableAbortConditions)
		CheckAbortConditions(simt, simdt);

	// Redstone contrail
	if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP) && GetAtmDensity() < contrailEnd)
	{
		DelExhaustStream(contrail);

		contrailActive = false;
	}
	else if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP) && !contrailActive) // If in dense atmosphere and contrail has not been activated
	{
		contrail = AddExhaustStream(th_main, _V(0, 0.3, -20), &contrail_main);
		contrailActive = true;
	}

	// Autopilot-stuff
	if (autoPilot && VesselStatus == LAUNCH)
	{
		if (AutopilotStatus == AUTOLAUNCH)
		{
			RedstoneAutopilot(simt, simdt);
		}
		else if (simt - boosterShutdownTime > 0.5)
		{
			separateTowerAction = true;
		}
	}
	else if (autoPilot && VesselStatus == TOWERSEP)
	{
		if (simt - boosterShutdownTime > 9.5 && vesselAcceleration < 0.25 * G)
		{
			separateCapsuleAction = true;
		}
	}
	else
	{
		CapsuleAutopilotControl(simt, simdt);
	}

	if (VesselStatus == LAUNCH && !autoPilot) // not autopilot
	{
		SetADCtrlMode(7); // enable adc
		double PitchUp = GetThrusterGroupLevel(THGROUP_ATT_PITCHUP);
		double PitchDown = GetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN);
		double TotalPitch = PitchUp - PitchDown;

		double YawRight = GetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT);
		double YawLeft = GetThrusterGroupLevel(THGROUP_ATT_YAWLEFT);
		double TotalYaw = YawRight - YawLeft;

		double flameArea = 0.46 * 0.46 * PI;
		double pitchVaneArea = 2 * 0.025 * sin(27.5 * RAD * GetControlSurfaceLevel(AIRCTRL_RUDDER));
		double yawVaneArea = 2 * 0.025 * sin(27.5 * RAD * GetControlSurfaceLevel(AIRCTRL_ELEVATOR));

		VECTOR3 thrustDirection;
		thrustDirection.z = 1 - ((abs(pitchVaneArea) + abs(yawVaneArea)) / flameArea);
		thrustDirection.x = -pitchVaneArea / flameArea;
		thrustDirection.y = -yawVaneArea / flameArea;
		SetThrusterDir(th_main, thrustDirection);
	}

	double longit, latit, radiusDontCare;
	GetEquPos(longit, latit, radiusDontCare);
	double getAlt = GetAltitude();
	FlightReentryAbortControl(simt, simdt, latit, longit, getAlt);

	if ((VesselStatus == LAUNCH || VesselStatus == ABORT || VesselStatus == ABORTNORETRO) && vesselAcceleration > historyMaxLaunchAcc)
		historyMaxLaunchAcc = vesselAcceleration;

	if (PMIcheck) // debugging stuff. Remove from final product
	{
		if (simt >= PMItime + 1.0 && PMIn == 1)
		{
			SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT, 0.0);
			VECTOR3 angVel;
			GetAngularVel(angVel);
			oapiWriteLogV("Bankleft: %.2f %.2f %.2f", DEG * angVel.x, DEG * angVel.y, DEG * angVel.z);
			PMIn++;
			SetAngularVel(_V(0.0, 0.0, 0.0));
			SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 1.0);
			PMItime = simt;
		}
		else if (simt >= PMItime + 1.0 && PMIn == 2)
		{
			SetThrusterGroupLevel(THGROUP_ATT_YAWLEFT, 0.0);
			VECTOR3 angVel;
			GetAngularVel(angVel);
			oapiWriteLogV("Yawleft: %.2f %.2f %.2f", DEG * angVel.x, DEG * angVel.y, DEG * angVel.z);
			PMIn++;
			SetAngularVel(_V(0.0, 0.0, 0.0));
			SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT, 1.0);
			PMItime = simt;
		}
		else if (simt >= PMItime + 1.0 && PMIn == 3)
		{
			SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT, 0.0);
			VECTOR3 angVel;
			GetAngularVel(angVel);
			oapiWriteLogV("Bankleft: %.2f %.2f %.2f", DEG * angVel.x, DEG * angVel.y, DEG * angVel.z);
			PMIn++;
			SetAngularVel(_V(0.0, 0.0, 0.0));
		}
	}
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
		case OAPI_KEY_J:
			if (VesselStatus == LAUNCH)
			{
				autoPilot = false; // maybe change this later when I have added escape rocket engines
				LaunchAbort();
			}
			else if (VesselStatus == TOWERSEP)
			{
				separateCapsuleAction = true;
				autoPilot = false;
			}
			else if (VesselStatus == ABORT)
			{
				autoPilot = false;

				// Engage Reentry-mode
				InflightAbortSeparate();
			}
			else if (VesselStatus == FLIGHT)
			{
				// Engage reentry-mode
				prepareReentryAction = true;
			}
			else if (VesselStatus == ABORTNORETRO) // If on-pad abort, where just tower and capsule eject
			{
				autoPilot = false;

				// Engage reentry-mode, but with no retro-pack to separate
				OffPadAbortSeparate();
			}
			else if (VesselStatus == REENTRY && !drogueDeployed)
			{
				// Drouge and main chute
				DeployDrogue();
			}
			else if (VesselStatus == REENTRY && drogueDeployed)
			{
				autoPilot = false;
				separateDrogueAction = true;
			}
			else if (VesselStatus == REENTRYNODROGUE && !mainChuteDeployed && !mainChuteSeparated)
			{
				DeployMainChute();
				//oapiWriteLog("DeployMainKey");
			}
			else if (VesselStatus == REENTRYNODROGUE && !landingBagDeployed)
			{
				DeployLandingBag();
				//oapiWriteLog("DeployLandingBagKey");
			}
			else if (VesselStatus == REENTRYNODROGUE && mainChuteDeployed && !mainChuteSeparated)
			{
				separateMainChuteAction = true;
				//oapiWriteLog("SepMainKey");
			}
			else if (VesselStatus == REENTRYNODROGUE && mainChuteSeparated && !reserveChuteDeployed)
			{
				mainChuteDeployed = false;
				DeployMainChute();
				reserveChuteDeployed = true;
				//oapiWriteLog("DeployReserveKey");
			}
			return 1;
		case OAPI_KEY_SPACE: // Remove from final
			if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP)
			{
				spaceLaunch = !spaceLaunch;
				autoPilot = false;

				if (spaceLaunch)
					SetThrusterLevel(th_main, 1.0);
				else
					SetThrusterLevel(th_main, 0.0);
			}
			else if (VesselStatus == FLIGHT)
			{
				engageRetro = true;
				retroStartTime = oapiGetSimTime() + 30.0; // retrosequence starts 30 sec before firing
				char cbuf[256];
				sprintf(cbuf, "Retrosequence T+%.0f", retroStartTime - 30.0 - launchTime);
				oapiWriteLog(cbuf);
			}

			return 1;
		case OAPI_KEY_P:
			if (GroundContact() && VesselStatus == LAUNCH)
			{
				autoPilot = true;
				SetThrusterLevel(th_main, 1.0);
				launchTime = oapiGetSimTime();
				integratedSpeed = 0.0;
				integratedLongitudinalSpeed = 0.0;
			}
			else if (VesselStatus == FLIGHT || VesselStatus == REENTRY) // switch fuel tank for attitude control
			{
				SwitchPropellantSource();
			}
			else if (VesselStatus == REENTRYNODROGUE)
			{
				engageFuelDump = true;
			}

			return 1;

		case OAPI_KEY_K:
			if (VesselStatus == LAUNCH && GetThrusterLevel(th_main) == 0.0) // C++ should exit if first condition is false, omiting a crash for when th_main is deleted
			{
				separateTowerAction = true;
			}
			else if (VesselStatus == FLIGHT) // engage retro attitude
			{
				//PMIcheck = true;
				//// Test of PMI
				//SetAngularVel(_V(0, 0, 0));
				//SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT, 1.0);
				//PMItime = oapiGetSimTime();
				//PMIn++;

				AutopilotStatus = TURNAROUND;
				autoPilot = true;
			}
			else if (VesselStatus == REENTRY)
			{
				AutopilotStatus = REENTRYATT;
				autoPilot = true;
			}
			
			return 1;
		case OAPI_KEY_G:
			if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE)
			{
				if (RcsStatus == AUTOLOW)
				{
					RcsStatus = MANUAL;
					SwitchPropellantSource();
				}
				else if (RcsStatus == MANUAL)
				{
					RcsStatus = AUTOHIGH;
					SwitchPropellantSource();
				}
				else
					RcsStatus = AUTOLOW; // don't switch propellant, as we're from Autohigh->Autolow

				SwitchAttitudeMode();
			}
			return 1;
		case OAPI_KEY_C: // switch to launch complex
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
				}
			}

			return 1;
		case OAPI_KEY_M: // turn on manual mode
			if (enableAbortConditions)
				oapiWriteLog("Automatic abort turned off by key");
			enableAbortConditions = false;
			DisableAutopilot(true);
			abortDamping = false;
			engageFuelDump = false;

			return 1;
		}
	}
	return 0;
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

		// Button Ctrl+T
		sprintf(cbuf, "Ctrl+T\t:Hide on-screen text");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;
		

		// Key J
		if (VesselStatus == LAUNCH)
		{
			sprintf(cbuf, "J\t:ABORT");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == TOWERSEP)
		{
			sprintf(cbuf, "J\t:Separate capsule");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == ABORT)
		{
			sprintf(cbuf, "J\t:Separate escape tower");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == FLIGHT)
		{
			sprintf(cbuf, "J\t:Separate retropack");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == ABORTNORETRO)
		{
			sprintf(cbuf, "J\t:Separate escape tower");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRY && !drogueDeployed)
		{
			sprintf(cbuf, "J\t:Deploy drogue chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRY && drogueDeployed)
		{
			sprintf(cbuf, "J\t:Separate drogue chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE && !mainChuteDeployed && !mainChuteSeparated)
		{
			sprintf(cbuf, "J\t:Deploy main chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE && !landingBagDeployed)
		{
			sprintf(cbuf, "J\t:Deploy landing bag");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE && mainChuteDeployed && !mainChuteSeparated)
		{
			sprintf(cbuf, "J\t:Separate main chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE && mainChuteSeparated && !reserveChuteDeployed)
		{
			sprintf(cbuf, "J\t:Deploy reserve chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key SPACE
		if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP)
		{
			if (spaceLaunch)
			{
				sprintf(cbuf, "SPACE\t:Engine off");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "SPACE\t:Engine on");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
		else if (VesselStatus == FLIGHT)
		{
			sprintf(cbuf, "SPACE\t:Fire retros");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key P
		if (GroundContact() && VesselStatus == LAUNCH)
		{
			sprintf(cbuf, "P\t:Automatic launch");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "C\t:Set focus on launch pad");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == FLIGHT || VesselStatus == REENTRY) // switch fuel tank for attitude control
		{
			if (attitudeFuelAuto)
			{
				sprintf(cbuf, "P\t:Set propellant source MANUAL");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "P\t:Set propellant source AUTO");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
		else if (VesselStatus == REENTRYNODROGUE)
		{
			if (!engageFuelDump)
			{
				sprintf(cbuf, "P\t:Fuel dump");
				skp->Text(TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			
		}

		// Key K
		if (VesselStatus == LAUNCH && GetThrusterLevel(th_main) == 0.0) // C++ should exit if first condition is false, omiting a crash for when th_main is deleted
		{
			sprintf(cbuf, "K\t:Separate escape tower");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == FLIGHT || VesselStatus == REENTRY)
		{
			sprintf(cbuf, "K\t:Engage automatic attitude");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key G
		if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE)
		{
			if (RcsStatus == AUTOLOW)
			{
				sprintf(cbuf, "G\t:Switch to attitude MANUAL");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else if (RcsStatus == MANUAL)
			{
				sprintf(cbuf, "G\t:Switch to attitude AUTOHIGH");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "G\t:Switch to attitude AUTOLOW");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}

		// Key M
		sprintf(cbuf, "M\t:Disengage autopilot");
		skp->Text(TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;
	}

	yIndex = -4;
	double simt = oapiGetSimTime();

	if (showInfoOnHud < 2)
	{
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
		int metM = (int)floor(metAbs / 60.0);
		int metS = (int)floor((metAbs- metM * 60.0));

		sprintf(cbuf, "T%s%02i %02i", signChar, metM, metS);
		skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// ==== Autopilot status ====
		if (autoPilot && VesselStatus != REENTRYNODROGUE)
		{
			sprintf(cbuf, "Autopilot: ON");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (!autoPilot && VesselStatus != REENTRYNODROGUE)
		{
			sprintf(cbuf, "Autopilot: OFF");
			skp->Text(secondColumnHUDx* TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		
		if (VesselStatus == FLIGHT)
		{
			sprintf(cbuf, "Retrosequence time:");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  %02i %02i", 4, 44);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		if (attitudeFuelAuto && (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE))
		{
			sprintf(cbuf, "Manual fuel: %.1f %%", GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_MAN * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Auto fuel: %.1f %%", GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_AUTO * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE)
		{
			sprintf(cbuf, "Manual fuel: %.1f %%", GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_MAN * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Auto fuel: %.1f %%", GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_AUTO * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Next event
		if (autoPilot)
		{
			if (VesselStatus == LAUNCH)
			{
				if (AutopilotStatus == AUTOLAUNCH)
				{
					sprintf(cbuf, "Awaiting cutoff");
					skp->Text(secondColumnHUDx * TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;

					sprintf(cbuf, "  remaining impulse: %.1f m/s", integratedSpeedLimit - integratedSpeed);
					skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
				else
				{
					sprintf(cbuf, "Separate escape tower in %.2f s", 0.5 + boosterShutdownTime - simt);
					skp->Text(secondColumnHUDx * TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
			}
			else if (VesselStatus == TOWERSEP)
			{
				sprintf(cbuf, "Separate capsule in %.2f s", 9.5 + boosterShutdownTime - simt);
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				WriteHUDAutoFlightReentry(skp, simt, &yIndex, cbuf);
			}
		}

		if (abort && (VesselStatus == FLIGHT || VesselStatus == REENTRY) && !drogueDeployed && abortDamping)
		{
			VECTOR3 angVel;
			GetAngularVel(angVel);
			sprintf(cbuf, "Holding reentry roll: %.2f\u00B0/s", angVel.z * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Launch target pitch and yaw
		if (autoPilot && VesselStatus == LAUNCH)
		{
			sprintf(cbuf, "Current pitch:");
			skp->Text(secondColumnHUDx * TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  %.2f\u00B0 (target: %.2f\u00B0)", GetPitch() * DEG, currentPitchAim);
			skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Current yaw:");
			skp->Text(secondColumnHUDx * TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  %.2f\u00B0 (target: %.2f\u00B0)", integratedYaw, 0.0);
			skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == LAUNCH && enableAbortConditions)
		{
			VECTOR3 angVel;
			GetAngularVel(angVel);

			sprintf(cbuf, "Current pitch rate: %.2f\u00B0/s", angVel.x * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Current yaw rate: %.2f\u00B0/s", angVel.y * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Chute status
		if (VesselStatus == REENTRY && drogueDeployed)
		{
			sprintf(cbuf, "Drogue chute deployed");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE)
		{
			if (mainChuteDeployed)
			{
				sprintf(cbuf, "Main chute deploy (%.2f)", GetAnimation(MainChuteDeployAnim));
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (landingBagDeployed)
			{
				sprintf(cbuf, "Landing bag deploy (%.2f)", GetAnimation(LandingBagDeployAnim));
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}

		if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE)
		{
			if (attitudeFuelAuto)
			{
				sprintf(cbuf, "Attitude fuel AUTO");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "Attitude fuel MANUAL");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (RcsStatus == MANUAL)
			{
				sprintf(cbuf, "Attitude mode MANUAL");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else if (RcsStatus == AUTOHIGH)
			{
				sprintf(cbuf, "Attitude mode AUTOHIGH");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "Attitude mode AUTOLOW");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (engageFuelDump)
			{
				sprintf(cbuf, "Dumping attitude fuel");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
	}

	return false;
}


void ProjectMercury::clbkLoadStateEx(FILEHANDLE scn, void* status)
{
	int i = 0;
	char *cbuf;

	while (oapiReadScenario_nextline(scn, cbuf))
	{
		if (!_strnicmp(cbuf, "STATE", 5))
		{
			sscanf_s(cbuf + 5, "%i", &VesselStatus); // consider changing back to normal sscanf (opposed to sscanf_s)
		}
		else if (!_strnicmp(cbuf, "CAPSULE", 7))
		{
			sscanf_s(cbuf + 7, "%i", &CapsuleVersion);
			capsuleDefined = true;
			if (CapsuleVersion == FREEDOM7)
			{
				capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Freedom7");
				heatShieldGroup = 32;
			}
			else if (CapsuleVersion == LIBERTYBELL7)
			{
				capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_LibertyBell7");
				heatShieldGroup = 35;
			}
			else if (CapsuleVersion == FRIENDSHIP7)
			{
				capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Friendship7");
				heatShieldGroup = 25;
			}
			else if (CapsuleVersion == AURORA7)
			{
				capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Aurora7");
				heatShieldGroup = 25;
			}
			else if (CapsuleVersion == SIGMA7)
			{
				capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Sigma7");
				heatShieldGroup = 25;
			}
			else if (CapsuleVersion == FAITH7)
			{
				capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Faith7");
				heatShieldGroup = 29;
			}
			else
			{
				capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Freedom7II");
				heatShieldGroup = 29;
			}

			Capsule = AddMesh(capsule, &MERCURY_OFS_CAPSULE);
		}
		else if (!_strnicmp(cbuf, "AUTOMODE", 8))
		{
			sscanf_s(cbuf + 8, "%i", &AutopilotStatus);
		}
		else if (!_strnicmp(cbuf, "SUBORBITAL", 10))
		{
			int readToBool;
			sscanf_s(cbuf + 10, "%i", &readToBool);
			suborbitalMission = (bool)readToBool;
		}
		else if (!_strnicmp(cbuf, "ABORTBUS", 8))
		{
			int readToBool;
			sscanf_s(cbuf + 8, "%i", &readToBool);
			enableAbortConditions = (bool)readToBool;
		}
		else if (!_strnicmp(cbuf, "MET", 3))
		{
			int metTime;
			sscanf_s(cbuf + 3, "%i", &metTime);
			launchTime = oapiGetSimTime() - (double)metTime;
		}
		else ParseScenarioLineEx(cbuf, status);
	}
}

void ProjectMercury::clbkSaveState(FILEHANDLE scn)
{
	VESSEL4::clbkSaveState(scn); // write default parameters (orbital elements etc.)

	oapiWriteScenario_int(scn, "STATE", VesselStatus);

	oapiWriteScenario_int(scn, "CAPSULE", CapsuleVersion);

	oapiWriteScenario_int(scn, "AUTOMODE", AutopilotStatus);

	oapiWriteScenario_int(scn, "SUBORBITAL", suborbitalMission);

	oapiWriteScenario_int(scn, "ABORTBUS", enableAbortConditions);
	
	if (launchTime != 0.0) // if we have launched using autopilot
	{
		oapiWriteScenario_int(scn, "MET", oapiGetSimTime() - launchTime);
	}
}

// ==============================================================
// Custom Vessel Functions
// ==============================================================

void ProjectMercury::RedstoneAutopilot(double simt, double simdt)
{
	SetADCtrlMode(0); // disable adc

	double met = simt - launchTime;
	//double pitch = GetPitch() * DEG;
	//double yaw = GetYaw() * DEG;
	//double bank = GetBank() * DEG;
	double pitch = integratedPitch;
	if (met > 40.0)
	{
		pitch = GetPitch() * DEG;
	}
	double yaw = integratedYaw;
	double bank = integratedRoll;
	double aim = 90.0;
	double pitchRate = 0.0;
	VECTOR3 currentAngRate;
	GetAngularVel(currentAngRate);
	double thrusterAngle = 0.0;

	for (int i = 0; i < 19; i++)
	{
		if (met > MET[i])
		{
			aim = aimPitch[i];
		}
	}
	currentPitchAim = aim;

	// not necessary as we now get pitch > 90 when on the back
	//if (pitch > 90.0) // If rocket turns on its back, it will keep pitching up in negative direction
	//	pitch = 180.0 - pitch;

	double pitchDiff = aim - pitch;
	if (pitchDiff > 0.1)
		pitchRate = 0.67 * RAD;
	else if (pitchDiff < -0.1)
		pitchRate = -0.67 * RAD;
	else
		pitchRate = 0.0;

	if (currentAngRate.x > pitchRate + 0.0005)
	{
		//thrusterAngle = 0.00075;
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, -(currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
	}
	else if (currentAngRate.x < pitchRate - 0.0005)
	{
		//thrusterAngle = -0.00075;
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, (currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
	}
	else
	{
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
	}

	// SetThrusterDir(th_main, _V(0, thrusterAngle, 1));

	// Automatically null yaw
	double yawDiff = 0.0 - yaw;
	double yawRate;
	if (yawDiff > 0.1)
		yawRate = 0.67 * RAD;
	else if (yawDiff < -0.1)
		yawRate = -0.67 * RAD;
	else
		yawRate = 0.0;

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

	double flameArea = 0.46 * 0.46 * PI;
	double pitchVaneArea = 2 * 0.025 * sin(27.5 * RAD * GetControlSurfaceLevel(AIRCTRL_RUDDER));
	double yawVaneArea = 2 * 0.025 * sin(27.5 * RAD * GetControlSurfaceLevel(AIRCTRL_ELEVATOR));

	VECTOR3 thrustDirection;
	thrustDirection.z = 1 - ((abs(pitchVaneArea) + abs(yawVaneArea)) / flameArea);
	thrustDirection.x = -pitchVaneArea / flameArea;
	thrustDirection.y = -yawVaneArea / flameArea;
	SetThrusterDir(th_main, thrustDirection);

	VECTOR3 Force;
	GetForceVector(Force);
	integratedSpeed += simdt * length(Force) / GetMass();
	integratedLongitudinalSpeed += simdt * Force.z / GetMass();

	if (integratedSpeed > integratedSpeedLimit) // Default 2130, gives Ap of 185-190 km, like MR3 and MR4
	{
		SetThrusterLevel(th_main, 0.0);

		historyCutOffAlt = GetAltitude();
		VECTOR3 currentSpaceVelocity;
		GetRelativeVel(GetSurfaceRef(), currentSpaceVelocity);
		historyCutOffVel = length(currentSpaceVelocity);
		VECTOR3 currentSpaceLocation;
		GetRelativePos(GetSurfaceRef(), currentSpaceLocation);
		historyCutOffAngl = -acos(dotp(currentSpaceLocation, currentSpaceVelocity) / length(currentSpaceLocation) / length(currentSpaceVelocity)) * DEG + 90.0;

		AutopilotStatus = POSIGRADEDAMP;
		boosterShutdownTime = simt;
	}

	// debug
	char vaneLog[256];
	double latit, longit, radit;
	GetEquPos(longit, latit, radit);
	OBJHANDLE plt = GetSurfaceRef();
	double range = oapiGetSize(plt) * oapiOrthodome(historyLaunchLong, historyLaunchLat, longit, latit);
	VECTOR3 posG, velG, velE;
	GetRelativeVel(plt, velG);
	double ApD;
	GetApDist(ApD);
	double ApA = ApD - oapiGetSize(plt);
	sprintf(vaneLog, "%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f", met, GetAltitude(), range, length(velG), GetGroundspeed(), integratedSpeed, GetPitch() * DEG, vesselAcceleration / G, GetThrusterMax(th_main) / 4.448, (GetPropellantMaxMass(redstone_propellant) - GetPropellantMass(redstone_propellant)) / 0.4536, ApA / 1852.0);
	oapiWriteLine(jetVaneLogFile, vaneLog);
}

void ProjectMercury::DefineRudderAnimations(void)
{
	// Max deflection from 19670028606 page 56 (vanes +- 27.5 deg, rudders +- 11 deg)
	// -y
	static UINT rudder1groups[1] = { 0 };
	static MGROUP_ROTATE Rudder1(
		Redstone,
		rudder1groups, 1,
		_V(0.0, -1.470, -8.595),
		_V(0, -1, 0),
		22*RAD
	);
	Rudder1Anim = CreateAnimation(0.5);
	AddAnimationComponent(Rudder1Anim, 0.0, 1.0, &Rudder1);

	// -x
	static UINT rudder2groups[1] = { 1 };
	static MGROUP_ROTATE Rudder2(
		Redstone,
		rudder2groups, 1,
		_V(-1.470, 0.0, -8.595),
		_V(-1, 0, 0),
		22 * RAD
	);
	Rudder2Anim = CreateAnimation(0.5);
	AddAnimationComponent(Rudder2Anim, 0.0, 1.0, &Rudder2);

	// +y
	static UINT rudder3groups[1] = { 2 };
	static MGROUP_ROTATE Rudder3(
		Redstone,
		rudder3groups, 1,
		_V(0.0, 1.470, -8.595),
		_V(0, 1, 0),
		22 * RAD
	);
	Rudder3Anim = CreateAnimation(0.5);
	AddAnimationComponent(Rudder3Anim, 0.0, 1.0, &Rudder3);

	// +x
	static UINT rudder4groups[1] = { 3 };
	static MGROUP_ROTATE Rudder4(
		Redstone,
		rudder4groups, 1,
		_V(1.470, 0.0, -8.595),
		_V(1, 0, 0),
		22 * RAD
	);
	Rudder4Anim = CreateAnimation(0.5);
	AddAnimationComponent(Rudder4Anim, 0.0, 1.0, &Rudder4);

	//oapiWriteLog("DefineRudderAnimations");
}

void ProjectMercury::DefineJetVaneAnimations(void)
{
	// Max deflection from 19670028606 page 56 (vanes +- 27.5 deg, rudders +- 11 deg)
	// -y
	static UINT vane1groups[1] = { 4 };
	static MGROUP_ROTATE Vane1(
		Redstone,
		vane1groups, 1,
		_V(0.0, -0.546, -9.063),
		_V(0, -1, 0),
		55 * RAD
	);
	JetVaneAnim[0] = CreateAnimation(0.5);
	AddAnimationComponent(JetVaneAnim[0], 0.0, 1.0, &Vane1);

	// -x
	static UINT vane2groups[1] = { 5 };
	static MGROUP_ROTATE Vane2(
		Redstone,
		vane2groups, 1,
		_V(-0.546, 0.0, -9.063),
		_V(-1, 0, 0),
		55 * RAD
	);
	JetVaneAnim[1] = CreateAnimation(0.5);
	AddAnimationComponent(JetVaneAnim[1], 0.0, 1.0, &Vane2);

	// +y
	static UINT vane3groups[1] = { 6 };
	static MGROUP_ROTATE Vane3(
		Redstone,
		vane3groups, 1,
		_V(0.0, 0.546, -9.063),
		_V(0, 1, 0),
		55 * RAD
	);
	JetVaneAnim[2] = CreateAnimation(0.5);
	AddAnimationComponent(JetVaneAnim[2], 0.0, 1.0, &Vane3);

	// +x
	static UINT vane4groups[1] = { 7 };
	static MGROUP_ROTATE Vane4(
		Redstone,
		vane4groups, 1,
		_V(0.546, 0.0, -9.063),
		_V(1, 0, 0),
		55 * RAD
	);
	JetVaneAnim[3] = CreateAnimation(0.5);
	AddAnimationComponent(JetVaneAnim[3], 0.0, 1.0, &Vane4);

	//oapiWriteLog("DefineJetVaneAnimations");
}

void ProjectMercury::CreateAirfoilsRedstone(void)
{
	ClearAirfoilDefinitions(); // delete previous airfoils
	SetRotDrag(_V(0.25, 0.25, 0.1)); // from BrianJ's Falcon 9
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 0.1), vliftRedstone, NULL, 1.89, 1.89 * 1.89 * PI / 4.0, 1.0); // 1.923 m from CG to CP post escape burn (19670022650 page 36)
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 0.1), hliftRedstone, NULL, 1.89, 1.89 * 1.89 * PI / 4.0, 1.0); // spherical symmetric
}

double ProjectMercury::EmptyMass(void)
{
	double mass;

	if (VesselStatus == TOWERSEP)
		mass = MERCURY_MASS + STAGE1_DRY_MASS;
	else if (VesselStatus == FLIGHT)
		mass = CAPSULE_MASS + RETRO_MASS_EMPTY;
	else if (VesselStatus == REENTRY)
		mass = CAPSULE_MASS;
	else if (VesselStatus == REENTRYNODROGUE)
		mass = LANDING_MASS;
	else if (VesselStatus == ABORT)
		mass = MERCURY_MASS + ABORT_MASS - ABORT_MASS_FUEL;
	else if (VesselStatus == ABORTNORETRO)
		mass = MERCURY_MASS + ABORT_MASS - ABORT_MASS_FUEL - RETRO_MASS;
	else // LAUNCH
		mass = MERCURY_MASS + STAGE1_DRY_MASS + ABORT_MASS;

	return mass;
}

void ProjectMercury::TowerSeparation(void)
{
	if (VesselStatus == LAUNCH || VesselStatus == ABORT) // only occasions with tower
	{
		SeparateTower(true);

		if (VesselStatus == LAUNCH)
		{
			VesselStatus = TOWERSEP;
			CGshifted = false;
		}
		else
		{
			VesselStatus = REENTRY;
			CGshifted = false;
		}
	}
}

void ProjectMercury::CheckAbortConditions(double simt, double simdt) // These conditions are from 19670028606 page 87
{
	double met = simt - launchTime;

	if (met < 0.1)
		return;
	
	// Attitude error
	VECTOR3 angVel;
	GetAngularVel(angVel);
	integratedPitch += angVel.x * DEG * simdt;
	integratedYaw += angVel.y * DEG * simdt;
	integratedRoll += angVel.z * DEG * simdt;

	// These are currently dependent on autopilot, to let the user launch in any direction if needed
	if (autoPilot && abs(integratedPitch - currentPitchAim) > 5.0)
	{
		abortConditionsMet = true;
		oapiWriteLogV("Abort due to error in pitch at T+%.1f. It was %.1f deg", met, integratedPitch);
	}
	if (autoPilot && abs(integratedYaw) > 5.0)
	{
		abortConditionsMet = true;
		oapiWriteLogV("Abort due to error in yaw at T+%.1f. It was %.1f deg", met, integratedYaw);
	}
	if(autoPilot && abs(integratedRoll) > 10.0)
	{
		abortConditionsMet = true;
		oapiWriteLogV("Abort due to error in roll at T+%.1f. It was %.1f deg", met, integratedRoll);
	}

	// Angular velocity

	if (abs(angVel.x * DEG) > 5.0)
	{
		abortConditionsMet = true;
		oapiWriteLogV("Abort due to error in pitch rate at T+%.1f. It was %.1f deg/s", met, angVel.x * DEG);
	}
	if (abs(angVel.y * DEG) > 5.0)
	{
		abortConditionsMet = true;
		oapiWriteLogV("Abort due to error in yaw rate at T+%.1f. It was %.1f deg/s", met, angVel.y * DEG);
	}

}

void ProjectMercury::LaunchAbort(void)
{
	abort = true;

	if (VesselStatus == LAUNCH)
	{
		SeparateRedstone();
		SeparateRingsAndAdapters(_V(0.0, -1.0, 0.0)); // Adapters and rings

		SeparateRetroPack(false);
		//oapiWriteLog("SeparateRetroFromLaunchAbortPad");
		for (int i = 1; i <= 3; i++)
		{
			SeparateRetroCoverN(i);
		}

		CreateAbortThrusters();
		SetPropellantMass(escape_tank, ABORT_MASS_FUEL);
		SetDefaultPropellantResource(escape_tank);
		SetThrusterGroupLevel(THGROUP_MAIN, 1.0);

		VesselStatus = ABORTNORETRO;
		CGshifted = false;

		abortTime = oapiGetSimTime();

		//oapiWriteLog("LaunchAbort PostLaunch");
	}

	// Separate potential attached pad
	OBJHANDLE padHandle = GetAttachmentStatus(padAttach);
	if (padHandle != NULL)
	{
		VESSEL* v = oapiGetVesselInterface(padHandle);
		ATTACHMENTHANDLE padAtt = v->GetAttachmentHandle(false, 0);
		bool success = v->DetachChild(padAtt, 0.0);
		if (success)
			oapiWriteLog("Separated due to abort");
		else
			oapiWriteLog("Failed to separate at abort");
	}
}

void ProjectMercury::CapsuleSeparate(void)
{
	if (VesselStatus == TOWERSEP)
	{
		VECTOR3 redstoneDirection = _V(0.0, -1.0, 0.0);

		SeparateRedstone();
		SeparateRingsAndAdapters(redstoneDirection); // Adapters and rings

		CreatePosigradeRockets();
		SetPropellantMass(posigrade_propellant[0], POSIGRADE_MASS_FUEL);
		SetPropellantMass(posigrade_propellant[1], POSIGRADE_MASS_FUEL);
		SetPropellantMass(posigrade_propellant[2], POSIGRADE_MASS_FUEL);
		CreateRetroRockets();
		SetPropellantMass(retro_propellant[0], RETRO_MASS_FUEL);
		SetPropellantMass(retro_propellant[1], RETRO_MASS_FUEL);
		SetPropellantMass(retro_propellant[2], RETRO_MASS_FUEL);
		//CreateRCS();
		SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);
		SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);

		VesselStatus = FLIGHT;
		CGshifted = false;
		//oapiWriteLog("CapsuleSeparate");
	}
}

MATRIX3 ProjectMercury::RotationMatrix(VECTOR3 angles, bool xyz)
{
	// Courtesy of fred18 https://www.orbiter-forum.com/showthread.php?t=38725
	MATRIX3 m;
	MATRIX3 RM_X, RM_Y, RM_Z;
	RM_X = _M(1, 0, 0, 0, cos(angles.x), -sin(angles.x), 0, sin(angles.x), cos(angles.x));
	RM_Y = _M(cos(angles.y), 0, sin(angles.y), 0, 1, 0, -sin(angles.y), 0, cos(angles.y));
	RM_Z = _M(cos(angles.z), -sin(angles.z), 0, sin(angles.z), cos(angles.z), 0, 0, 0, 1);
	if (!xyz)
	{
		m = mul(RM_Z, mul(RM_Y, RM_X));
	}
	else
	{
		m = mul(RM_X, mul(RM_Y, RM_Z));
	}
	return m;
}

void ProjectMercury::SeparateTower(bool noAbortSep)
{
	VESSELSTATUS2 vs;
	VESSELSTATUS2::FUELSPEC fuel;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	char name[256];
	VECTOR3 towerDirection = { 0,-1, 0 };
	double towerVelocity = 0.0;

	VECTOR3 relativeOffset;
	GetStatusEx(&vs);

	if (VesselStatus == LAUNCH)
		Local2Rel(ABORT_OFFSET, vs.rpos);
	else // abort mode
		Local2Rel(ABORT_OFFSET - MERCURY_OFS_CAPSULE - _V(0.0, 0.0, (77.8 - 19.3) * 0.0254), vs.rpos); // _V(0.0, 0.0, (77.8 - 19.3) * 0.0254)

	GlobalRot(towerDirection, relativeOffset);
	vs.rvel += relativeOffset * towerVelocity;

	vs.flag = VS_FUELLIST;
	vs.fuel = &fuel;
	vs.nfuel = 2;
	vs.fuel->idx = 0;
	vs.fuel->level = 1.0;
	if (noAbortSep)
	{
		vs.fuel->idx = 1;
		vs.fuel->level = 1.0;
	}
	else
	{
		vs.fuel->idx = 1;
		vs.fuel->level = 0.0;
	}

	strcpy(name, GetName());
	strcat(name, " abort tower");

	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Abort", &vs);
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Tower);
	DelPropellantResource(escape_tank);
	DelThruster(escape_engine);
	towerJetTime = oapiGetSimTime();
}

void ProjectMercury::SeparateRedstone(void)
{
	VESSELSTATUS2 vs;
	VESSELSTATUS2::FUELSPEC fuel;
	VESSELSTATUS2::THRUSTSPEC thrust;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	GetStatusEx(&vs);
	char name[256];
	VECTOR3 redstoneOffset = REDSTONE_OFFSET;
	VECTOR3 redstoneDirection = { 0,-1, 0 };
	double redstoneVelocity = 0.0;

	vs.flag = VS_FUELLIST;
	vs.fuel = &fuel;
	vs.nfuel = 1;
	vs.fuel->idx = 0;
	vs.fuel->level = GetFuelMass() / GetMaxFuelMass();

	if (GroundContact())
	{
		double latit = vs.surf_lat;
		double longi = vs.surf_lng;
		double headi = vs.surf_hdg;

		MATRIX3 rot1 = RotationMatrix(_V(0 * RAD, (90 * RAD - longi), 0 * RAD), TRUE);
		MATRIX3 rot2 = RotationMatrix(_V(-latit + 0 * RAD, 0, 0 * RAD), TRUE);
		MATRIX3 rot3 = RotationMatrix(_V(0, 0, 180 * RAD + headi), TRUE);
		MATRIX3 rot4 = RotationMatrix(_V(0, 0, 0), TRUE); // Direction

		MATRIX3 RotMatrix_Def = mul(rot1, mul(rot2, mul(rot3, rot4)));

		vs.arot.x = atan2(RotMatrix_Def.m23, RotMatrix_Def.m33);
		vs.arot.y = -asin(RotMatrix_Def.m13);
		vs.arot.z = atan2(RotMatrix_Def.m12, RotMatrix_Def.m11);
		vs.vrot.x = STAGE1_LENGTH / 2.0 + heightOverGround;
	}
	else
	{
		VECTOR3 relativeOffset;
		Local2Rel(redstoneOffset, vs.rpos);
		GlobalRot(redstoneDirection, relativeOffset);
		vs.rvel += relativeOffset * redstoneVelocity;
		inFlightAbort = true;
	}

	if (GetThrusterLevel(th_main) != 0.0 && oapiGetSimTime() - launchTime < 30.0) // 19670028606 page 87, keep engine running if less than T+30
	{
		vs.flag = VS_THRUSTLIST;
		vs.thruster = &thrust;
		vs.nthruster = 1;
		vs.thruster->idx = 0;
		vs.thruster->level = 1.0;
		oapiWriteLog("Keep Redstone burning due to T < +30");
	}

	strcpy(name, GetName());
	strcat(name, " Redstone booster");
	int k = 0;
	while (oapiGetVesselByName(name) != NULL)
	{
		k += 1;
		strcat(name, "1");
	}

	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "Vessels\\ProjectMercury\\Mercury_RedstoneBooster", &vs);;
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;

	// The thrusterlist spec doesnt seem to work, sadly
	VESSEL* v = oapiGetVesselInterface(createdVessel[stuffCreated - 1]);
	double levl = GetPropellantMass(redstone_propellant);
	v->SetPropellantMass(v->GetPropellantHandleByIndex(0), levl);
	if (GetThrusterLevel(th_main) != 0.0 && oapiGetSimTime() - launchTime < 30.0)
	{
		v->SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
	}

	DelThruster(th_main);
	DelThruster(th_rcsDummyBooster[0]);
	DelThruster(th_rcsDummyBooster[1]);
	DelThruster(th_rcsDummyBooster[2]);
	DelThruster(th_rcsDummyBooster[3]);
	DelThruster(th_rcsDummyBooster[4]);
	DelThruster(th_rcsDummyBooster[5]);
	DelControlSurface(Rudders[0]);
	DelControlSurface(Rudders[1]);
	DelControlSurface(Rudders[2]);
	DelControlSurface(Rudders[3]);
	DelPropellantResource(redstone_propellant);
	DelMesh(Redstone);
	//oapiWriteLog("SeparateRedstone");
}

void ProjectMercury::SeparateRingsAndAdapters(VECTOR3 redstoneDirection)
{
	VESSELSTATUS2 vs;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	char name[256];
	VECTOR3 vel = _V(0, .95, 0);
	VECTOR3 relativeOffset1;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER1, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset1);
	VECTOR3 vel2;
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 1");
	oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover1", &vs);
	DelMesh(Adaptcover1);

	vel = _V(-0.823, -0.475, 0);
	VECTOR3 relativeOffset2;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER2, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset2);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 2");
	oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover2", &vs);
	DelMesh(Adaptcover2);

	vel = _V(0.823, -0.475, 0);
	VECTOR3 relativeOffset3;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER3, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset3);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 3");
	oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover3", &vs);
	DelMesh(Adaptcover3);

	vel = _V(-0.823, 0.475, 0);
	VECTOR3 relativeOffset4;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING1, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset4);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 1");
	oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing1", &vs);
	DelMesh(Adaptring1);

	vel = _V(0.0, -0.95, 0);
	VECTOR3 relativeOffset5;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING2, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset5);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 2");
	oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing2", &vs);
	DelMesh(Adaptring2);

	vel = _V(0.823, 0.475, 0);
	VECTOR3 relativeOffset6;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING3, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset6);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 3");
	oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing3", &vs);
	DelMesh(Adaptring3);

	//oapiWriteLog("SeparateRingsAndAdapters");
}

// --------------------------------------------------------------
// Vessel initialisation
// --------------------------------------------------------------
DLLCLBK VESSEL* ovcInit(OBJHANDLE hvessel, int flightmodel)
{
	jetVaneLogFile = oapiOpenFile("MR-3_PitchData.txt", FILE_OUT, ROOT);

	return new ProjectMercury(hvessel, flightmodel);
}

// --------------------------------------------------------------
// Vessel cleanup
// --------------------------------------------------------------
DLLCLBK void ovcExit(VESSEL* vessel)
{
	oapiCloseFile(jetVaneLogFile, FILE_OUT);

	if (vessel) delete (ProjectMercury*)vessel;
}

void ProjectMercury::vliftRedstone(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
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

void ProjectMercury::hliftRedstone(VESSEL* v, double beta, double M, double Re, void* context, double* cl, double* cm, double* cd)
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

	//*cd = 0.55 * (0.11 + oapiGetInducedDrag(*cl, 0.16, 0.2));
}