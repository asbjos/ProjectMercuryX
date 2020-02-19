// ==============================================================
//				Source file for Mercury Redstone.
//				Created by Asbjørn "asbjos" Krüger
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

#include "MercuryAtlas.h"
#include "..\..\MercuryCapsule.h"

#include <fstream> // debug, for appending entry data to file

ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel)
	: VESSEL4(hVessel, flightmodel)
{
	// Load exterior meshes
	// capsule is defined in loadstate
	atlasAdapter = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_adapt");
	atlas = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main");
	atlasBooster = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_boost");
	
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
	SetSize(ATLAS_CORE_LENGTH + MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT);
	SetEmptyMass(MERCURY_MASS + CORE_DRY_MASS + BOOSTER_DRY_MASS + ABORT_MASS);

	ReadConfigSettings(cfg);

	static const DWORD tchdwnLaunchPadNum = 4;
	static TOUCHDOWNVTX tchdwnLaunchPad[tchdwnLaunchPadNum] = {
		// pos, stiff, damping, mu, mu long
		{_V(0.0, -1.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0 - heightOverGround), 1e7, 1e5, 10},
		{_V(-0.7, 0.7, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0 - heightOverGround), 1e7, 1e5, 10},
		{_V(0.7, 0.7, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0 - heightOverGround), 1e7, 1e5, 10},
		{_V(0.0, 0.0, MERCURY_OFS_ABORT.z + MERCURY_LENGTH_ABORT / 2.0), 1e7, 1e5, 10},
	};
	SetTouchdownPoints(tchdwnLaunchPad, tchdwnLaunchPadNum);

	SetCW(1.0, 0.1, 0.3, 0.3);
	SetCrossSections(_V(80.15, 85.27, 10.89));
	SetRotDrag(_V(0.7, 0.7, 1.2));
	//SetPMI(_V(33.0, 33.0, 20.51)); // actually much smaller z-component, but makes atmospheric flight unstable at >1x
	SetPMI(_V(39.7, 39.7, 1.45)); // From legacy Atlas.h

	CreateCapsuleFuelTanks();

	atlas_propellant = CreatePropellantResource(CORE_FUEL_MASS);
	//SetPropellantMass(redstone_propellant, STAGE1_FUEL_MASS);

	SetDefaultPropellantResource(atlas_propellant);

	// main engine
	th_main = CreateThruster(CORE_EXHAUST_POS, CORE_EXHAUST_DIR, CORE_THRUST, atlas_propellant, CORE_ISP_VAC, CORE_ISP_SL);
	th_vernier[0] = CreateThruster(VERNIER_EXHAUST_POS, VERNIER_EXHAUST_DIR, VERNIER_THRUST_VAC, atlas_propellant, VERNIER_ISP_VAC, VERNIER_ISP_SL); // -y
	th_vernier[1] = CreateThruster(FlipY(VERNIER_EXHAUST_POS), FlipY(VERNIER_EXHAUST_DIR), VERNIER_THRUST_VAC, atlas_propellant, VERNIER_ISP_VAC, VERNIER_ISP_SL); // +y
	th_booster[0] = CreateThruster(BOOSTER_EXHAUST_POS, BOOSTER_EXHAUST_DIR, BOOSTER_THRUST, atlas_propellant, BOOSTER_ISP_VAC, BOOSTER_ISP_SL);
	th_booster[1] = CreateThruster(FlipX(BOOSTER_EXHAUST_POS), FlipX(BOOSTER_EXHAUST_DIR), BOOSTER_THRUST, atlas_propellant, BOOSTER_ISP_VAC, BOOSTER_ISP_SL);
	thCluster[0] = th_main;
	thCluster[1] = th_vernier[0];
	thCluster[2] = th_vernier[1];
	thCluster[3] = th_booster[0];
	thCluster[4] = th_booster[1];
	CreateThrusterGroup(thCluster, 5, THGROUP_MAIN);
	SURFHANDLE tex = oapiRegisterExhaustTexture("Exhaust2");
	exMain = AddExhaust(th_main, 15, 1, tex);
	exVernier[0] = AddExhaust(th_vernier[0], 2, 0.2, tex);
	exVernier[1] = AddExhaust(th_vernier[1], 2, 0.2, tex);
	exBooster[0] = AddExhaust(th_booster[0], 30, 2, tex);
	exBooster[1] = AddExhaust(th_booster[1], 30, 2, tex);

	COLOUR4 col_d = { (float)(1.0),(float)(0.7),(float)(0.0),0 };
	COLOUR4 col_s = { (float)(1.0),(float)(0.7),(float)(0.0),0 };
	COLOUR4 col_a = { (float)(1.0),(float)(0.7),(float)(0.0),0 };

	// Main
	LightEmitter* main_light = AddPointLight(_V(0.0, 0.0, BOOSTER_EXHAUST_POS.z), 100.0, 0.5, 0.03, 0.002, col_d, col_s, col_a);
	main_light->SetIntensityRef(&turbineContrailLevel);

	SURFHANDLE cont = oapiRegisterParticleTexture("Contrail3");
	PARTICLESTREAMSPEC turbineExhaust = {
		0, 3.0, 100, 50.0, 0.2, 0.2, 3.0, 10.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 0.5,
		PARTICLESTREAMSPEC::ATM_PLOG, 0.2, 1.2,
		cont
	};

	PARTICLESTREAMSPEC boosterExhaust = {
		0, 2.0, 100, 80.0, 0.1, 0.1, 1.0, 5.0, PARTICLESTREAMSPEC::EMISSIVE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 0.1,
		PARTICLESTREAMSPEC::ATM_PLOG, 0.05, 0.9,
		cont
	};

	turbineExhaustContrail = AddParticleStream(&turbineExhaust, TURBINE_EXHAUST_POS + _V(0.0, 0.0, -2.0), -TURBINE_EXHAUST_DIR, &turbineContrailLevel);
	boosterExhaustContrail[0] = AddParticleStream(&boosterExhaust, BOOSTER_EXHAUST_POS, -BOOSTER_EXHAUST_DIR, &turbineContrailLevel);
	boosterExhaustContrail[1] = AddParticleStream(&boosterExhaust, FlipX(BOOSTER_EXHAUST_POS), -BOOSTER_EXHAUST_DIR, &turbineContrailLevel);

	// Contrail stuff
	contrail_main = {
		0, 5.0, 100, 200, 0.1, 20.0, 3.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin, contrailEnd
	};
	contrail = AddExhaustStream(th_main, _V(0, 0.3, -25), &contrail_main);

	contrail_second = {
		0, 15.0, 200, 30, 0.1, 20.0, 2.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PLIN, 0, 50,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin2, contrailEnd2
	};
	contrail2 = AddExhaustStream(th_main, _V(0, 0.3, -30), &contrail_second);

	// Dummy RCS for jet vanes and air rudders
	th_rcsDummyBooster[0] = CreateThruster(_V(0, 1, -ATLAS_CORE_LENGTH / 2), _V(0, 1, 0), 0,	atlas_propellant, 0, 0);
	th_rcsDummyBooster[1] = CreateThruster(_V(0, -1, -ATLAS_CORE_LENGTH / 2), _V(0, -1, 0), 0,	atlas_propellant, 0, 0);
	th_rcsDummyBooster[2] = CreateThruster(_V(1, 0, -ATLAS_CORE_LENGTH / 2), _V(1, 0, 0), 0,	atlas_propellant, 0, 0);
	th_rcsDummyBooster[3] = CreateThruster(_V(-1, 0, -ATLAS_CORE_LENGTH / 2), _V(-1, 0, 0), 0,	atlas_propellant, 0, 0);
	th_rcsDummyBooster[4] = CreateThruster(_V(1, 0, -ATLAS_CORE_LENGTH / 2), _V(0, 1, 0), 0,	atlas_propellant, 0, 0);
	th_rcsDummyBooster[5] = CreateThruster(_V(1, 0, -ATLAS_CORE_LENGTH / 2), _V(0, -1, 0), 0,	atlas_propellant, 0, 0);
	CreateThrusterGroup(&th_rcsDummyBooster[0], 1, THGROUP_ATT_PITCHUP);
	CreateThrusterGroup(&th_rcsDummyBooster[1], 1, THGROUP_ATT_PITCHDOWN);
	CreateThrusterGroup(&th_rcsDummyBooster[2], 1, THGROUP_ATT_YAWLEFT); // maybe switch later
	CreateThrusterGroup(&th_rcsDummyBooster[3], 1, THGROUP_ATT_YAWRIGHT);
	CreateThrusterGroup(&th_rcsDummyBooster[4], 1, THGROUP_ATT_BANKLEFT); // maybe switch later
	CreateThrusterGroup(&th_rcsDummyBooster[5], 1, THGROUP_ATT_BANKRIGHT);

	// Control surfaces for air rudders. dCl and delay values from DeltaGlider and/or Falcon 9 by BrianJ
	CreateAirfoilsAtlas(); // try this
	Verniers[0] = CreateControlSurface3(AIRCTRL_ELEVATOR, 0.515 * 2, rudderLift, _V(0.0, 0.0, -8.6), AIRCTRL_AXIS_XPOS, rudderDelay); // check Airctrl_Axis
	Verniers[1] = CreateControlSurface3(AIRCTRL_RUDDER, 0.515 * 2, rudderLift, _V(0.0, 0.0, -8.6), AIRCTRL_AXIS_YPOS, rudderDelay);
	Verniers[2] = CreateControlSurface3(AIRCTRL_AILERON, 0.515 * 2, rudderLift, _V(0.0, 1.0, -8.6), AIRCTRL_AXIS_YPOS, rudderDelay);
	// Rudders[2] = CreateControlSurface3(AIRCTRL_ELEVATOR, 0.515, 3, _V(0.0, 1.8, -8.6), AIRCTRL_AXIS_YPOS, 1.0);
	//Rudders[2] = CreateControlSurface3(AIRCTRL_AILERON, 0.515, 1.7, _V(0.0, 1.8, -8.6), AIRCTRL_AXIS_YPOS, 0.5);
	//Rudders[3] = CreateControlSurface3(AIRCTRL_FLAP, 0.515, 1.7, _V(1.8, 0.0, -8.6), AIRCTRL_AXIS_XPOS, 0.5);
	//DefineRudderAnimations();

	// associate a mesh for the visual
	Atlas = AddMesh(atlas, &ATLAS_CORE_OFFSET);
	AtlasBooster = AddMesh(atlasBooster, &ATLAS_BOOSTER_OFFSET);
	AtlasAdapter = AddMesh(atlasAdapter, &ATLAS_ADAPTER_OFFSET);
	// capsule defined in LoadState
	AddDefaultMeshes();

	//DefineRudderAnimations();
	//DefineJetVaneAnimations();
	DefineVernierAnimations();

	padAttach = CreateAttachment(true, _V(0.0, 0.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0), _V(0, 0, 1), _V(0, 0, 1), "PAD", true);
}

void ProjectMercury::clbkPostCreation()
{
	CapsuleGenericPostCreation();

	if (VesselStatus == TOWERSEP)
	{
		DelMesh(Tower);
	}
	else if (VesselStatus == LAUNCHCORE)
	{
		DelMesh(AtlasBooster);
		DelThruster(th_booster[0]);
		DelThruster(th_booster[1]);
		DelExhaustStream(turbineExhaustContrail);
		DelExhaustStream(boosterExhaustContrail[0]);
		DelExhaustStream(boosterExhaustContrail[1]);
		ClearLightEmitters();
	}
	else if (VesselStatus == LAUNCHCORETOWERSEP)
	{
		DelMesh(AtlasBooster);
		DelThruster(th_booster[0]);
		DelThruster(th_booster[1]);
		DelExhaustStream(turbineExhaustContrail);
		DelExhaustStream(boosterExhaustContrail[0]);
		DelExhaustStream(boosterExhaustContrail[1]);
		ClearLightEmitters();
		DelMesh(Tower);
	}
	else if (VesselStatus == ABORT)
	{
		// Airfoil-stuff
		CreateAirfoilsEscape();
		DelControlSurface(Verniers[0]);
		DelControlSurface(Verniers[1]);
		DelControlSurface(Verniers[2]);

		DelThruster(th_main);
		DelThruster(th_booster[0]);
		DelThruster(th_booster[1]);
		DelThruster(th_vernier[0]);
		DelThruster(th_vernier[1]);
		DelPropellantResource(atlas_propellant);
		DelMesh(Atlas);
		DelMesh(AtlasAdapter);
		DelMesh(AtlasBooster);
		DelMesh(Adaptcover1);
		DelMesh(Adaptcover2);
		DelMesh(Adaptcover3);
		DelMesh(Adaptring1);
		DelMesh(Adaptring2);
		DelMesh(Adaptring3);

		CreateAbortThrusters();
	}
	else if (VesselStatus == ABORTNORETRO)
	{
		// Airfoil-stuff
		CreateAirfoilsEscape();
		DelControlSurface(Verniers[0]);
		DelControlSurface(Verniers[1]);
		DelControlSurface(Verniers[2]);

		DelThruster(th_main);
		DelThruster(th_booster[0]);
		DelThruster(th_booster[1]);
		DelThruster(th_vernier[0]);
		DelThruster(th_vernier[1]);
		DelPropellantResource(atlas_propellant);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);
		DelMesh(Atlas);
		DelMesh(AtlasAdapter);
		DelMesh(AtlasBooster);
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
	}
	else if (VesselStatus == FLIGHT)
	{
		// Airfoil-stuff
		CreateAirfoils();
		DelControlSurface(Verniers[0]);
		DelControlSurface(Verniers[1]);
		DelControlSurface(Verniers[2]);

		DelThruster(th_main);
		DelThruster(th_booster[0]);
		DelThruster(th_booster[1]);
		DelThruster(th_vernier[0]);
		DelThruster(th_vernier[1]);
		DelPropellantResource(atlas_propellant);
		DelPropellantResource(escape_tank);
		DelMesh(Tower);
		DelMesh(Atlas);
		DelMesh(AtlasAdapter);
		DelMesh(AtlasBooster);
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
		DelControlSurface(Verniers[0]);
		DelControlSurface(Verniers[1]);
		DelControlSurface(Verniers[2]);

		DelThruster(th_main);
		DelThruster(th_booster[0]);
		DelThruster(th_booster[1]);
		DelThruster(th_vernier[0]);
		DelThruster(th_vernier[1]);
		DelPropellantResource(atlas_propellant);
		DelPropellantResource(escape_tank);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);

		//CreateRCS();

		DelMesh(Tower);
		DelMesh(Atlas);
		DelMesh(AtlasAdapter);
		DelMesh(AtlasBooster);
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
		DelControlSurface(Verniers[0]);
		DelControlSurface(Verniers[1]);
		DelControlSurface(Verniers[2]);

		DelThruster(th_main);
		DelThruster(th_booster[0]);
		DelThruster(th_booster[1]);
		DelThruster(th_vernier[0]);
		DelThruster(th_vernier[1]);
		DelPropellantResource(atlas_propellant);
		DelPropellantResource(escape_tank);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);

		//CreateRCS();

		DelMesh(Tower);
		DelMesh(Atlas);
		DelMesh(AtlasAdapter);
		DelMesh(AtlasBooster);
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
	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE) && abortConditionsMet)
		LaunchAbort();
	else if (VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP && abortConditionsMet)
		CapsuleSeparate();

	// Actions
	if (separateTowerAction)
	{
		TowerSeparation();
		separateTowerAction = false;
	}
	else if (separateBoosterAction)
	{
		SeparateAtlasBooster(true);
		separateBoosterAction = false;
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
		// Allow to be launched from LC14. The trigger is if engine is lit from an attached vessel
		OBJHANDLE attachedParent;
		attachedParent = GetAttachmentStatus(padAttach);
		if (attachedParent != NULL && oapiGetFocusObject() == attachedParent && GetThrusterGroupLevel(THGROUP_MAIN) != 0.0) // attached, pad in focus, engines on, and has been uninitialised. Launch is initiated from pad
		{
			oapiSetTimeAcceleration(1.0);
			autoPilot = true;
			launchTime = oapiGetSimTime() + 3.0;
			integratedSpeed = 0.0;
		}

		if (GetThrusterGroupLevel(THGROUP_MAIN) != 0.0)
		{
			turbineContrailLevel = 1.0;
		}
		else
		{
			turbineContrailLevel = 0.0;
		}
		break;
	case TOWERSEP:
		if (!CGshifted && !GroundContact())
		{
			SetSize(ATLAS_CORE_LENGTH + MERCURY_LENGTH_CAPSULE);
			SetTouchdownPoints(tchdwnTowSep, tchdwnTowSepNum);
			CGshifted = true; // Yeah, not bothering with CG here

			PreviousVesselStatus = TOWERSEP;
		}
		break;
	case LAUNCHCORE:
		if (!CGshifted && !GroundContact())
		{
			PreviousVesselStatus = LAUNCHCORE;
		}
		break;
	case LAUNCHCORETOWERSEP:
		if (!CGshifted && !GroundContact())
		{
			SetSize(ATLAS_CORE_LENGTH + MERCURY_LENGTH_CAPSULE);
			SetTouchdownPoints(tchdwnTowSep, tchdwnTowSepNum);
			CGshifted = true; // Yeah, not bothering with CG here

			PreviousVesselStatus = LAUNCHCORETOWERSEP;
		}
		break;
	case FLIGHT:
		if (!CGshifted && !GroundContact())
		{
			CreateAirfoils();

			SetSize(MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_RETRO);
			SetTouchdownPoints(tchdwnFlight, tchdwnFlightNum);
			ShiftCG(MERCURY_OFS_CAPSULE); // only from TOWERSEP or scenario
			SetCameraOffset(_V(0.0, 0.0, 0.0));
			ShiftCentreOfMass(_V(0.0, 0.0, -0.6)); // Allign RCS to CentreOfMass
			SetCrossSections(_V(3.5, 3.37, 2.8));
			// SetPMI(_V(0.68, 0.63, 0.303)); // original
			// SetPMI(_V(0.39, 0.39, 0.35)); // according to pitch acc data doi:10.1002/j.2161-4296.1962.tb02524.x
			SetPMI(_V(0.39, 0.39, 0.28)); // Set to match the preceeding data
			CGshifted = true;
			CreateRCS();
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
				ShiftCG(-_V(0.0, 0.0, (77.8 - 19.3) * 0.0254));  // 19670022650 page 36 (CG of escape setup at 77.8 in, compared to 19.3 in for spacecraft)
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			}
			else if (PreviousVesselStatus == LAUNCH) // spawned in REENTRY from scenario
			{
				ShiftCG(MERCURY_OFS_CAPSULE);
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			} // else from FLIGHT and don't need to shift

			SetPMI(_V(0.39, 0.39, 0.28)); // Set to match the preceeding data
			SetCrossSections(_V(2.8, 2.8, 2.8));
			CGshifted = true;
			CreateRCS();

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
				ShiftCG(MERCURY_OFS_CAPSULE);
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			} // else don't shift, as came from REENTRY

			SetPMI(_V(0.39, 0.39, 0.28)); // Set to match the preceeding data
			SetCrossSections(_V(2.8, 2.8, 2.8));
			CGshifted = true;
			CreateRCS();

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
			if (PreviousVesselStatus == LAUNCH || PreviousVesselStatus == LAUNCHCORE) // either from aborting or from scenario load
			{
				ShiftCG(MERCURY_OFS_CAPSULE + _V(0.0, 0.0, (77.8 - 19.3) * 0.0254)); // 19670022650 page 36 (CG of escape setup at 77.8 in, compared to 19.3 in for spacecraft)
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			}
			SetPMI(_V(5.0, 5.0, 0.4)); // 19670022650 page 5 and 6 (5650 slug*ft^2 and total mass 3400 pounds)
			SetCrossSections(_V(4.3, 4.3, 3.17)); // grossly approx
			CGshifted = true;

			PreviousVesselStatus = ABORT;
		}

		escapeLevel = GetThrusterLevel(escape_engine);

		if (inFlightAbort && vesselAcceleration < 0.25 * G && (simt - abortTime) > 2.0) // When aborting from inflight, and acc<0.25g
		{
			InflightAbortSeparate();
		}
		break;
	case ABORTNORETRO:
		if (!CGshifted && !GroundContact())
		{
			CreateAirfoilsEscape();
			SetSize(MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT);
			SetTouchdownPoints(tchdwnAbort, tchdwnAbortNum);
			if (PreviousVesselStatus == LAUNCH || PreviousVesselStatus == LAUNCHCORE) // either abort or from scenario
			{
				ShiftCG(MERCURY_OFS_CAPSULE + _V(0.0, 0.0, (77.8 - 19.3) * 0.0254));
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			}
			SetPMI(_V(5.0, 5.0, 0.4)); // 19670022650 page 5 and 6 (5650 slug*ft^2 and total mass 3400 pounds)
			SetCrossSections(_V(4.3, 4.3, 3.17)); // grossly approx
			CGshifted = true;

			PreviousVesselStatus = ABORTNORETRO;
		}

		escapeLevel = GetThrusterLevel(escape_engine);

		if (vesselAcceleration < 0.25 * G && (simt - abortTime) > 2.0) // When aborting from inflight, and acc<0.25g
		{
			OffPadAbortSeparate();
		}
	}

	DeleteRogueVessels();
}

void ProjectMercury::clbkPostStep(double simt, double simdt, double mjd)
{
	// Animate verniers
	if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP)
	{
		double vernPi = GetControlSurfaceLevel(AIRCTRL_ELEVATOR);
		double vernYa = GetControlSurfaceLevel(AIRCTRL_RUDDER);
		double vernRo = GetControlSurfaceLevel(AIRCTRL_AILERON);

		if (vernRo != 0.0)
		{
			SetAnimation(Vernier1AnimY, 0.5 + 0.5 * vernRo);
			SetAnimation(Vernier2AnimY, 0.5 - 0.5 * vernRo);
		}
		else // Not within T+2 s to T+15 s, or possibly when manual
		{
			SetAnimation(Vernier1AnimX, 0.5 + 0.5 * vernPi); // calibrate these
			SetAnimation(Vernier1AnimY, 0.5 - 0.5 * vernYa);
			SetAnimation(Vernier2AnimX, 0.5 + 0.5 * vernPi);
			SetAnimation(Vernier2AnimY, 0.5 - 0.5 * vernYa);
		}
	}

	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP) && enableAbortConditions)
		CheckAbortConditions(simt, simdt);

	// First Atlas contrail
	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP) && GetAtmDensity() < contrailEnd)
	{
		DelExhaustStream(contrail);

		contrailActive = false;
	}
	else if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP) && !contrailActive) // If in dense atmosphere and contrail has not been activated
	{
		contrail = AddExhaustStream(th_main, _V(0, 0.3, -20), &contrail_main);
		contrailActive = true;
	}

	// Secondary Atlas contrail
	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP) && GetAtmDensity() < contrailEnd2)
	{
		DelExhaustStream(contrail2);

		contrail2Active = false;
	}
	else if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP) && !contrail2Active) // If in dense atmosphere and contrail has not been activated
	{
		contrail2 = AddExhaustStream(th_main, _V(0, 0.3, -20), &contrail_second);
		contrail2Active = true;
	}

	// Autopilot-stuff
	if (autoPilot && (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP))
	{
		if (AutopilotStatus == AUTOLAUNCH)
		{
			AtlasAutopilot(simt, simdt);
		}
		else if (VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP)
		{
			if (simt > boosterShutdownTime + 1.0 && vesselAcceleration < 0.25 * G) // capsule sep at 1 sec after cutoff (19620004691 page 34)
			{
				separateCapsuleAction = true;
			}
		}
	}
	else
	{
		CapsuleAutopilotControl(simt, simdt);
	}

	if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP) && !autoPilot) // not autopilot
	{
		SetADCtrlMode(7); // enable adc

		AtlasEngineDir();
	}

	double longit, latit, radiusDontCare;
	GetEquPos(longit, latit, radiusDontCare);
	double getAlt = GetAltitude();

	FlightReentryAbortControl(simt, simdt, latit, longit, getAlt); // Various functions and checks for controlling abort, retrofire, and to record flight parameters

	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP || VesselStatus == ABORT || VesselStatus == ABORTNORETRO) && vesselAcceleration > historyMaxLaunchAcc)
		historyMaxLaunchAcc = vesselAcceleration;

	if (historyInclination < latit * DEG)
	{
		historyInclination = latit * DEG;

		if (simt - boosterShutdownTime < 5000.0) // less than one orbit
		{
			//timeFromCutOffToMaxLat = simt - boosterShutdownTime;
			maxLatitudeTime = simt - launchTime;
		}
	}

	if (VesselStatus == FLIGHT && getAlt < historyPerigee && historyBottomPrev < getAlt && historyBottomPrevPrev > historyBottomPrev) // we have perigee, and it's the absolute lowest
	{
		historyPerigee = getAlt;
	}
	historyBottomPrevPrev = historyBottomPrev;
	historyBottomPrev = getAlt;

	if (VesselStatus == FLIGHT)
	{
		if (latit > historyCutOffLat&& historyCutOffLat > previousFrameLatitude)
		{
			orbitCount += 1;
			historyPeriod = simt - previousOrbitSimt;
			previousOrbitSimt = simt;
		}

		previousFrameLatitude = latit;
	}

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
	bool TargetBaseInput(void* id, char* str, void* data);

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
			if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE)
			{
				autoPilot = false; // maybe change this later when I have added escape rocket engines
				oapiWriteLog("Abort by key");
				LaunchAbort();
			}
			else if (VesselStatus == TOWERSEP)
			{
				separateBoosterAction = true;
				autoPilot = false;
			}
			else if (VesselStatus == LAUNCHCORETOWERSEP)
			{
				separateCapsuleAction = true;
				//autoPilot = false;
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
			}
			else if (VesselStatus == REENTRYNODROGUE && !landingBagDeployed)
			{
				DeployLandingBag();
			}
			else if (VesselStatus == REENTRYNODROGUE && mainChuteDeployed && !mainChuteSeparated)
			{
				separateMainChuteAction = true;
			}
			else if (VesselStatus == REENTRYNODROGUE && mainChuteSeparated && !reserveChuteDeployed)
			{
				mainChuteDeployed = false;
				DeployMainChute();
				reserveChuteDeployed = true;
			}
			return 1;
		case OAPI_KEY_SPACE: // Remove from final
			if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP ||  VesselStatus == TOWERSEP)
			{
				spaceLaunch = !spaceLaunch;
				autoPilot = false;

				if (spaceLaunch)
					SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
				else
					SetThrusterGroupLevel(THGROUP_MAIN, 0.0);

				if (GroundContact() && GetAttachmentStatus(padAttach) != NULL)
				{
					launchTime = oapiGetSimTime() + 3.0;
				}
				else if (GroundContact())
				{
					launchTime = oapiGetSimTime();
				}
			}
			else if (VesselStatus == FLIGHT)
			{
				engageRetro = true;
				retroStartTime = oapiGetSimTime() + 30.0; // retrosequence starts 30 sec before firing
				char cbuf[256];
				sprintf(cbuf, "Retrosequence %.0f", retroStartTime - 30.0);
				oapiWriteLog(cbuf);
			}

			return 1;
		case OAPI_KEY_P:
			if (GroundContact() && VesselStatus == LAUNCH)
			{
				oapiSetTimeAcceleration(1.0);
				autoPilot = true;
				SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
				launchTime = oapiGetSimTime() + 3.0;
				integratedSpeed = 0.0;
			}
			else if (VesselStatus == LAUNCHCORETOWERSEP) // allow autopilot from a manual launch
			{
				autoPilot = true;
				AutopilotStatus = AUTOLAUNCH;
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
			if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP) // TOWERSEP should never happen, but oh well ...
			{
				separateBoosterAction = true;
			}
			else if (VesselStatus == LAUNCHCORE) // C++ should exit if first condition is false, omiting a crash for when th_main is deleted
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
		case OAPI_KEY_M: // turn on manual mode
			if (enableAbortConditions)
				oapiWriteLog("Automatic abort turned off by key");
			enableAbortConditions = false;
			DisableAutopilot(true);
			abortDamping = false;
			engageFuelDump = false;

			if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP)
			{
				SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);
				SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
				SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
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
		case OAPI_KEY_B: // set base index when in FLIGHT
			oapiOpenInputBox("Set target base / coordinate", TargetBaseInput, 0, 20, (void*)this);

			return 1;
		}
	}
	return 0;
}

bool TargetBaseInput(void* id, char* str, void* data)
{
	return ((ProjectMercury*)data)->SetTargetBaseIdx(str);
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
		if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE)
		{
			sprintf(cbuf, "J\t:ABORT");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP)
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
		if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP)
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
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}

		// Key K
		if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP))
		{
			sprintf(cbuf, "K\t:Separate booster stage");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == LAUNCHCORE)
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
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// Key B
		sprintf(cbuf, "B\t:Set landing target");
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
		int metH = (int)floor(metAbs / 3600.0);
		int metM = (int)floor((metAbs - metH * 3600.0) / 60.0);
		int metS = (int)floor((metAbs - metH * 3600.0 - metM * 60.0));

		sprintf(cbuf, "T%s%02i %02i %02i", signChar, metH, metM, metS);
		skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
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
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		if (VesselStatus == FLIGHT)
		{
			sprintf(cbuf, "Orbit number %i", orbitCount);
			skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			if (landingComputing)
			{
				// New method for retrosequence time
				double deltaT = 50.0; // 100 is a bit much, or then the 5.0 deg closeness is a bit harsh
				int totalIterations = 0;
				double time = 0.0;
				double previousAngDist = PI;
				bool closeSolution = false;
				bool targetFound = false;
				double landingLong, landingLat;

				// Current state:
				ELEMENTS el;
				ORBITPARAM prm;
				GetElements(GetSurfaceRef(), el, &prm, oapiGetSimMJD(), FRAME_EQU);
				double longAtNow, latAtNow, radAtNow;
				GetEquPos(longAtNow, latAtNow, radAtNow);

				while (time < 5500.0 && !closeSolution && (!engageRetro || simt < retroStartTime))
				{
					GetLandingPointIfRetroInXSeconds(time, el, prm, longAtNow, &landingLong, &landingLat);

					if (noMissionLandLat)
					{
						missionLandLat = landingLat * DEG; // only target the landing longitude
					}

					if (oapiOrthodome(landingLong, landingLat, missionLandLong * RAD, missionLandLat * RAD) < 5.0 * RAD)
					{
						closeSolution = true;
						time -= deltaT; // correct for later addition
					}
					time += deltaT;
					totalIterations += 1;
				}

				double angDistToTarget = 9.0 * RAD;
				bool endIteration = false;
				double minAngDist = 10 * RAD;
				double minAngDistTime = 0.0;
				double minAngDistLong, minAngDistLat, previousLandingLong, previousLandingLat;
				if (closeSolution) // investigate further
				{
					time -= 50.0; // roll back and search finer
					while (!endIteration && angDistToTarget < 10.0 * RAD)
					{
						GetLandingPointIfRetroInXSeconds(time, el, prm, longAtNow, &landingLong, &landingLat);

						if (noMissionLandLat)
						{
							missionLandLat = landingLat * DEG; // only target the landing longitude
						}

						angDistToTarget = oapiOrthodome(landingLong, landingLat, missionLandLong * RAD, missionLandLat * RAD);

						if (angDistToTarget < 2.0 * RAD && angDistToTarget > previousAngDist)
						{
							// SUCCESS. But keep searching, as the position jumps back and forth a bit
							targetFound = true;
							if (minAngDist > previousAngDist)
							{
								minAngDist = previousAngDist;
								minAngDistTime = time - 1.0;
								minAngDistLong = previousLandingLong;
								minAngDistLat = previousLandingLat;
							}
							else // We found a local minimum, but have a previous smaller value, i.e. are past the global minimum
							{
								// Find angular distance from point (target) to line (through last two projected landing points)
								double crossrange = abs((previousLandingLat - landingLat) * missionLandLong * RAD - (previousLandingLong - landingLong) * missionLandLat * RAD + previousLandingLong * landingLat - previousLandingLat * landingLong) / sqrt(pow(previousLandingLat - landingLat, 2.0) + pow(previousLandingLong - landingLong, 2.0));
								if (minAngDist < crossrange * 1.3)
									endIteration = true;
							}
						}
						else if (angDistToTarget > 2.0 * RAD && targetFound) // duplicates the one five lines above
							endIteration = true;

						time += 1.0;
						previousAngDist = angDistToTarget;
						previousLandingLat = landingLat;
						previousLandingLong = landingLong;
						totalIterations += 1;
					}
				}

				sprintf(cbuf, "Retrosequence:");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				if (noMissionLandLat)
				{
					sprintf(cbuf, "  Long: %.2f\u00B0", missionLandLong);
					skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
				else
				{
					sprintf(cbuf, "  Lat: %.2f\u00B0, long: %.2f\u00B0", missionLandLat, missionLandLong);
					skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}

				if (minAngDistTime != 0.0)
				{
					double metRetroTime = minAngDistTime - 30.0 + metAbs - 6.6; // -30 to account for retroseq to retroburn. -6.6 is empirical, mostly from retroburn not being instantanious
					int ret3H = (int)floor(metRetroTime / 3600.0);
					int ret3M = (int)floor((metRetroTime - ret3H * 3600.0) / 60.0);
					int ret3S = (int)floor((metRetroTime - ret3H * 3600.0 - ret3M * 60.0));
					sprintf(cbuf, "  %02i %02i %02i (%.0f km)", ret3H, ret3M, ret3S, minAngDist * oapiGetSize(GetSurfaceRef()) / 1e3);
					skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
				else
				{
					sprintf(cbuf, "  NO SOLUTION FOUND");
					skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
			}

			// Historical retroseq-times
			double retroseqTime = retroTimes[0];
			char retroseqName[256];
			sprintf(retroseqName, retroNames[0]);
			int modi = 0;
			for (int i = 0; i < 29; i++)
			{
				if (metAbs > retroTimes[i])
				{
					retroseqTime = retroTimes[i + 1];
					sprintf(retroseqName, "%s", retroNames[i + 1]);
					modi = i + 1;
				}
			}

			sprintf(cbuf, "Next retrosequence time:");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			int retH = (int)floor(retroseqTime / 3600.0);
			int retM = (int)floor((retroseqTime - retH * 3600.0) / 60.0);
			int retS = (int)floor((retroseqTime - retH * 3600.0 - retM * 60.0));

			sprintf(cbuf, "  %02i %02i %02i (%s)", retH, retM, retS, retroseqName);
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
		if (autoPilot && (VesselStatus == LAUNCH || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP))
		{
			if (AutopilotStatus == AUTOLAUNCH)
			{
				sprintf(cbuf, "Awaiting cutoff");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				sprintf(cbuf, "  remaining impulse: %.1f m/s", integratedSpeedLimit - integratedSpeed);
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
		else if (autoPilot && (VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP))
		{
			sprintf(cbuf, "Separate capsule in %.2f s", 1.0 + boosterShutdownTime - simt);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else
		{
			WriteHUDAutoFlightReentry(skp, simt, &yIndex, cbuf); // sketchy obsobsobsobos debug
		}

		if (abort && (VesselStatus == FLIGHT || VesselStatus == REENTRY) && !drogueDeployed && abortDamping)
		{
			VECTOR3 angVel;
			GetAngularVel(angVel);
			sprintf(cbuf, "Holding reentry roll: %.2f\u00B0/s", angVel.z * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP)
		{
			VECTOR3 angVel;
			GetAngularVel(angVel);

			double targetPitchRate = AtlasPitchControl();
			if (VesselStatus == LAUNCHCORETOWERSEP)
				sprintf(cbuf, "Pitch rate: %.2f\u00B0/s (target: %.2f\u00B0/s)", angVel.x * DEG, targetPitchRate * DEG);
			else
				sprintf(cbuf, "Pitch rate: %.2f\u00B0/s", angVel.x * DEG);

			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Yaw rate: %.2f\u00B0/s", angVel.y * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Roll rate: %.2f\u00B0/s", angVel.z* DEG);
			skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
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
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
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
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "Attitude mode AUTOLOW");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (engageFuelDump)
			{
				sprintf(cbuf, "Dumping attitude fuel");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
	}

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
			autoPilot = true;
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
		else if (!_strnicmp(cbuf, "HIGHROLLLIMIT", 13)) // for launching towards high latitudes, so that we need to roll faster
		{
			rollLimit = 10.0;
		}
		else if (!_strnicmp(cbuf, "MET", 3))
		{
			int metTime;
			sscanf_s(cbuf + 3, "%i", &metTime);
			launchTime = oapiGetSimTime() - (double)metTime;
		}
		else if (!_strnicmp(cbuf, "ORBITNR", 7)) // maybe obsolete. will have to figure out later (depending on next retrosequence calculation)
		{
			sscanf_s(cbuf + 7, "%i", &orbitCount);
		}
		else if (!_strnicmp(cbuf, "INCLINATION", 11))
		{
			sscanf_s(cbuf + 11, "%lf", &targetInclination);
			launchTargetPosition = false;
		}
		else if (!_strnicmp(cbuf, "APOGEE", 6))
		{
			sscanf_s(cbuf + 6, "%lf", &targetApogee);
			missionApogee = targetApogee;
			limitApogee = true;
		}
		else if (!_strnicmp(cbuf, "PERIGEE", 7))
		{
			sscanf_s(cbuf + 7, "%lf", &targetPerigee);
			missionPerigee = targetPerigee;
		}
		else if (!_strnicmp(cbuf, "MISSIONORBITNR", 14))
		{
			sscanf_s(cbuf + 14, "%i", &missionOrbitNumber);
		}
		else if (!_strnicmp(cbuf, "MISSIONLANDLAT", 14))
		{
			sscanf_s(cbuf + 14, "%lf", &missionLandLat);
		}
		else if (!_strnicmp(cbuf, "MISSIONLANDLONG", 15))
		{
			double userInput;
			sscanf_s(cbuf + 15, "%lf", &userInput);
			missionLandLong = fmod(userInput, 360.0); // range 0 to 360 deg

			launchTargetPosition = true; // we (probably) have all input data to initiate a target position launch
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

	if (VesselStatus == FLIGHT)
	{
		oapiWriteScenario_int(scn, "ORBITNR", orbitCount);

	}
}

bool ProjectMercury::SetTargetBaseIdx(char* rstr)
{
	oapiWriteLogV("Input target. You wrote: >%s<", rstr);
	// First assume input is base
	OBJHANDLE baseHandle = oapiGetBaseByName(GetSurfaceRef(), rstr);

	if (baseHandle != NULL) // got a base
	{
		oapiGetBaseEquPos(baseHandle, &missionLandLong, &missionLandLat);
		missionLandLong *= DEG;
		missionLandLat *= DEG;
		noMissionLandLat = false;
		oapiWriteLogV("Found base at %.2f N %.2f E", missionLandLat, missionLandLong);
		landingComputing = true;
		return true;
	}
	else // typed in a coordinate
	{
		// Look for lat and long, separated by space
		char* strPos;
		strPos = strchr(rstr, ' ');
		oapiWriteLogV("No base with that name. StrPos: %d", strPos - rstr + 1);

		if (strPos != NULL) // found two values
		{
			oapiWriteLogV("You wrote two coordinates.");
			char latText[20];
			char longText[20];
			strncpy(latText, rstr, strPos - rstr + 1); // maybe strPos - rstr + 1
			oapiWriteLogV("Part one: %s", latText);
			missionLandLat = atof(latText);
			oapiWriteLogV("Lat: %.2f N", missionLandLat);
			strncpy(longText, rstr + int(strPos - rstr + 1), strlen(rstr) - int(strPos - rstr + 1)); // maybe etc.
			oapiWriteLogV("Part two: %s", longText);
			missionLandLong = atof(longText);
			oapiWriteLogV("Long: %.2f E", missionLandLong);
			noMissionLandLat = false;
			landingComputing = true;
			return true;
		}
		else// only longitude is input (no space detected)
		{
			char* strPosU, * strPosL;
			strPosU = strchr(rstr, 'N');
			strPosL = strchr(rstr, 'n');

			if (strPosU != NULL || strPosL != NULL) // disable landing computer. Saves framerate etc.
			{
				landingComputing = false;
				oapiWriteLog("Disabled landing computing");
				return true;
			}
			else
			{
				missionLandLong = atof(rstr); // atof returns 0.0 if no valid number, which is a good backup solution for erranous input
				noMissionLandLat = true;
				oapiWriteLogV("You only wrote long: %.2f", missionLandLong);
				landingComputing = true;
				return true;
			}
			
		}
	}


	//int targetIdx;

	//sscanf_s(rstr, "%i", &targetIdx);

	//if (targetIdx >= 30)
	//{
	//	currentLandingTargetIdx = 29;
	//	return true;
	//}
	//else if (targetIdx < 0)
	//{
	//	return false;
	//}
	//else
	//{
	//	currentLandingTargetIdx = targetIdx;
	//	return true;
	//}
	return false;
}

// ==============================================================
// Custom Vessel Functions
// ==============================================================

void ProjectMercury::AtlasAutopilot(double simt, double simdt)
{
	// Debug
	if (pitchDataLogFile == NULL)
		pitchDataLogFile = oapiOpenFile("MA-6_PitchLog.txt", FILE_OUT, ROOT); // debug

	SetADCtrlMode(0); // disable adc

	double pitch = integratedPitch;
	double yaw = integratedYaw;
	double bank = integratedRoll;
	double met = simt - launchTime;
	double aim = 90.0;
	double pitchRate = 0.0;
	VECTOR3 currentAngRate;
	GetAngularVel(currentAngRate);
	double thrusterAngle = 0.0;
	bool controlAttitude = false;

	for (int i = 0; i < 12; i++)
	{
		if (met > MET[i])
		{
			pitchRate = aimPitchover[i] * (-RAD);
		}
	}

	double BECO = 128.6; // T+130.1 from 19620004691 page 34. T+128.6 from 19630002114 page 21. An earlier BECO time results in longer burntime.

	// Different actions during launch
	if (met > BECO + 26.0 && !GroundContact()) // time from 19930074071 page 54
	{
		controlAttitude = true;
		pitch = GetPitch() * DEG; // integrated pitch becomes off by up to five degrees
	}

	if (met > BECO + 4.0 && VesselStatus == LAUNCH && !GroundContact()) // time from 19930074071 page 54
	{
		separateBoosterAction = true;
	}

	if (met > BECO + 20.0 && VesselStatus == LAUNCHCORE && !GroundContact()) // time from 19930074071 page 25
	{
		separateTowerAction = true;
	}

	double bottomPitch = -4.0;

	// attempt to dynamically set bottomPitch to get zero vertical velocity component at cutoff
	VECTOR3 currentSpaceVelocity;
	GetRelativeVel(GetSurfaceRef(), currentSpaceVelocity);
	double currentOrbitalVelocity = length(currentSpaceVelocity);
	double planetMu = oapiGetMass(GetSurfaceRef()) * GGRAV;
	double planetRad = oapiGetSize(GetSurfaceRef());

	VECTOR3 velocity;
	GetGroundspeedVector(FRAME_HORIZON, velocity);
	double verticalSpeed = velocity.y;
	double targetOrbitalVelocity = sqrt(planetMu * (2.0 / (missionPerigee * 1000.0 + planetRad) - 2.0 / (missionApogee * 1000.0 + planetRad + missionPerigee * 1000.0 + planetRad))); // switch from currentRadius to missionPerigee
	double omegaP = AtlasPitchControl();
	// This solution is excellent for having cutoff at set height, but not so good for achieving zero flight angle. Therefore use the solution down to a few seconds before cutoff, where we turn to the basic zero y-vel solution

	if (controlAttitude) // final pitch and yaw program to enter target orbit
	{
		double pitchDiff;

		double verticalSpeed = velocity.y;
		VECTOR3 currentVelocity, currentPosition;
		GetRelativePos(GetSurfaceRef(), currentPosition);
		GetRelativeVel(GetSurfaceRef(), currentVelocity);
		double currentRadius = length(currentPosition);
		double currentSpeed = length(currentVelocity);
		double deltaV = targetOrbitalVelocity - currentSpeed;
		double timeToCutoff = GetMass() * CORE_ISP_VAC / CORE_THRUST * (1.0 - exp(-deltaV / CORE_ISP_VAC));

		double requiredVerticalAcc = verticalSpeed / timeToCutoff;
		if (timeToCutoff < 15.0)
		{
			if (abs(verticalSpeed) < deltaV)
				bottomPitch = -asin(verticalSpeed / deltaV) * DEG;
			else if (verticalSpeed < 0.0)
				bottomPitch = 90.0;
			else
				bottomPitch = -90.0;

			pitchDiff = bottomPitch - pitch;

			if (pitchDiff > 0.1)
				pitchRate = 0.5 * RAD; // guesstimate
			else if (pitchDiff < -0.1)
				pitchRate = -0.5 * RAD;
			else
				pitchRate = 0.0;
		}
		else
		{
			pitchRate = omegaP;
		}

		if (abs(pitchRate) > 2.0 * RAD)
		{
			pitchRate = pitchRate / abs(pitchRate) * 2.0 * RAD;
		}
		VECTOR3 currentAngAcc;
		GetAngularAcc(currentAngAcc);

		// Disabling any checks for abort; only use attitude rate for ASIS
		//if (met - BECO - 26.0 < 15.0) // give rocket 15.0 seconds to find new aim
		//	currentPitchAim = pitch;
		//else
		//	currentPitchAim = bottomPitch; // to ASIS computer for pitch check

		if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x > pitchRate + 0.0005)
		{
			SetControlSurfaceLevel(AIRCTRL_ELEVATOR, -(currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
		}
		else if (abs(currentAngAcc.x < 0.75) * RAD && currentAngRate.x < pitchRate - 0.0005)
		{
			SetControlSurfaceLevel(AIRCTRL_ELEVATOR, (currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
		}
		else
		{
			SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
		}

		// Autopilot for heading. Either target an inclination or a specific position after N orbits
		double long1, lat1, radi1;
		GetEquPos(long1, lat1, radi1);
		long1 = fmod(long1, PI2) * DEG;
		lat1 = lat1 * DEG;
		double targetAzimuth;
		if (launchTargetPosition)
		{
			// Target cut-off azimuth from 19980227091 and simplified from https://blog.wolfram.com/2017/02/24/hidden-figures-modern-approaches-to-orbit-and-reentry-calculations/
			targetAzimuth = AtlasTargetCutOffAzimuth(simt, radi1, long1, lat1, false);
		}
		else
		{
			targetAzimuth = asin(cos(targetInclination * RAD) / cos(lat1 * RAD)) * DEG;
		}

		velocity.x += cos(lat1 * RAD) * PI2 * oapiGetSize(GetSurfaceRef()) / oapiGetPlanetPeriod(GetSurfaceRef());
		double currentAzimuth = atan(velocity.x / velocity.z) * DEG;

		double azimuthDiff = targetAzimuth - currentAzimuth;
		double targetSlipAngle = 0.0;
		if (abs(azimuthDiff) > 4.0) // don't bother fixing an impossible case
			targetSlipAngle = 0.0;
		else if (azimuthDiff < -0.20)
			targetSlipAngle = -4.0;
		else if (azimuthDiff < -0.005)
			targetSlipAngle = -1.0;
		else if (azimuthDiff > 0.20)
			targetSlipAngle = 4.0;
		else if (azimuthDiff > 0.005)
			targetSlipAngle = 1.0;
		else
			targetSlipAngle = 0.0;

		double yawDiff = targetSlipAngle - OrbitalFrameSlipAngle(currentPosition, currentVelocity) * DEG;
		double yawRate;
		if (yawDiff < -0.05)
			yawRate = 0.5 * RAD; // guesstimate
		else if (yawDiff > 0.05)
			yawRate = -0.5 * RAD;
		else
			yawRate = 0.0;

		if (currentAngRate.y > yawRate + 0.0005)
		{
			SetControlSurfaceLevel(AIRCTRL_RUDDER, (currentAngRate.y * DEG * currentAngRate.y * DEG * ampFactor + ampAdder));
		}
		else if (currentAngRate.y < yawRate - 0.0005)
		{
			SetControlSurfaceLevel(AIRCTRL_RUDDER, -(currentAngRate.y * DEG * currentAngRate.y * DEG * ampFactor + ampAdder));
		}
		else
		{
			SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
		}

		//currentYawAim = -targetSlipAngle; // the reading is flipped, nut sure why

	}
	else if (met > MET[1] && pitchProgram) // after T+15 s and finalised roll
	{
		if (rollProgram)
		{
			currentRollAim = integratedRoll;

			char cbuf[256];
			double targetRoll = AtlasTargetCutOffAzimuth(simt, missionPerigee * 1000.0 + 6.371e6, historyLaunchLong * DEG, historyLaunchLat * DEG, false);
			sprintf(cbuf, "Roll program ended. Attained heading was %.3f deg. Target was %.3f deg. Error %.3f deg. Finishing roll rate %.2f deg/s", 105.0 - integratedRoll, targetRoll, 105.0 - integratedRoll - targetRoll, currentAngRate.z * DEG);
			oapiWriteLog(cbuf);
		}
		rollProgram = false;
		SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);

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

	}
	else if (met > 2.0) // roll program, time from 19930074071 page 54
	{
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);

		double initialHeading = 105.0;
		double targetHeading = 72.55;
		double wantedApogee = 2.65e5;
		if (limitApogee)
			wantedApogee = targetApogee * 1000.0;

		if (launchTargetPosition)
		{
			targetHeading = AtlasTargetCutOffAzimuth(simt, missionPerigee * 1000.0 + planetRad, historyLaunchLong * DEG, historyLaunchLat * DEG, false);
		}
		else
		{
			targetHeading = asin(cos(targetInclination * RAD) / cos(historyLaunchLat)) * DEG;
		}

		// Include Earth's rotation
		double orbitVel = sqrt(planetMu / (planetRad + missionPerigee * 1000.0));
		targetHeading = atan((orbitVel * sin(targetHeading * RAD) - PI2 * planetRad / oapiGetPlanetPeriod(GetSurfaceRef()) * cos(historyLaunchLat)) / (orbitVel * cos(targetHeading * RAD))) * DEG;

		double targetRoll = initialHeading - targetHeading;
		currentRollAim = targetRoll;

		rollProgram = true; // is this doing anything?

		double rollRate;

		double rollDiff = targetRoll - integratedRoll;

		double highLimit = 10.0;
		double medLimit = 2.5;
		double lowLimit = 0.1;
		double lowerLimit = 0.01;
		double highRate = 4.0 * RAD;
		double medRate = 2.0 * RAD;
		double lowRate = 1.0 * RAD;
		double lowerRate = 0.05 * RAD;

		if (rollLimit == 10.0)
		{
			highRate = 8.0 * RAD;
		}

		if (rollDiff > highLimit)
			rollRate = highRate;
		else if (rollDiff > medLimit)
			rollRate = medRate;
		else if (rollDiff > lowLimit)
			rollRate = lowRate;
		else if (rollDiff > lowerLimit)
			rollRate = lowerRate;
		else if (rollDiff < -highLimit)
			rollRate = -highRate;
		else if (rollDiff < -medLimit)
			rollRate = -medRate;
		else if (rollDiff < -lowLimit)
			rollRate = -lowRate;
		else if (rollDiff < -lowerLimit)
			rollRate = -lowerRate;
		else
			rollRate = 0.0;

		VECTOR3 currentAngAcc;
		GetAngularAcc(currentAngAcc);

		if (abs(currentAngAcc.z) < 2.0 * RAD && currentAngRate.z > rollRate + 0.0005)
		{
			SetControlSurfaceLevel(AIRCTRL_AILERON, -(currentAngRate.z * DEG * currentAngRate.z * DEG * ampFactor + ampAdder));
		}
		else if (abs(currentAngAcc.z) < 2.0 * RAD && currentAngRate.z < rollRate - 0.0005)
		{
			SetControlSurfaceLevel(AIRCTRL_AILERON, (currentAngRate.z* DEG* currentAngRate.z* DEG* ampFactor + ampAdder));
		}
		else
		{
			SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);
		}

		// Check if roll program is finished
		if (rollDiff < 0.1 && currentAngRate.z * DEG < 0.05)
		{
			pitchProgram = true;
		}
		else // allow the program to regret the decision if has traveled out of corridor
		{
			pitchProgram = false;
		}
	}
	else // keep 90 deg first 2 seconds
	{
		double pitchDiff = 90.0 - GetPitch() * DEG;

		double factor;
		if (pitch < 90.0)
			factor = 1.0;
		else
			factor = -1.0;

		if (pitchDiff > 0.1)
			pitchRate = factor * 0.01 * RAD;
		else if (pitchDiff < -0.1)
			pitchRate = factor * -0.01 * RAD;
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
	}

	// Automatically null yaw
	if (!controlAttitude)
	{
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
	}

	if (met > BECO&& VesselStatus == LAUNCH)
	{
		SetThrusterLevel(th_booster[0], 0.0);
		SetThrusterLevel(th_booster[1], 0.0);

		// Ensure good staging by stopping engine gimbal
		SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
		SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);
	}

	AtlasEngineDir();

	VECTOR3 Force;
	GetForceVector(Force);
	integratedSpeed += simdt * length(Force) / GetMass();

	if (targetOrbitalVelocity < currentOrbitalVelocity + 3.4) // posigrades give additional 3.4 m/s dV
	{
		char cbuf[256];
		SetThrusterGroupLevel(THGROUP_MAIN, 0.0);
		AutopilotStatus = POSIGRADEDAMP;
		boosterShutdownTime = simt;
		sprintf(cbuf, "SECO T+%.1f", met);
		oapiWriteLog(cbuf);

		historyCutOffAlt = GetAltitude();
		VECTOR3 currentSpaceVelocity;
		GetRelativeVel(GetSurfaceRef(), currentSpaceVelocity);
		historyCutOffVel = length(currentSpaceVelocity);
		VECTOR3 currentSpaceLocation;
		GetRelativePos(GetSurfaceRef(), currentSpaceLocation);
		
		historyCutOffAngl = -acos(dotp(currentSpaceLocation, currentSpaceVelocity) / length(currentSpaceLocation) / length(currentSpaceVelocity)) * DEG + 90.0;

		historyPerigee = historyCutOffAlt; // set as starting point

		double radiusNoCare;
		GetEquPos(historyCutOffLong, historyCutOffLat, radiusNoCare);

		// change to ice-less skin. This costs a fair amount of FPS though
		/*SURFHANDLE metSkin = oapiLoadTexture("merc_atlas3.dds");
		SURFHANDLE iceSkin = oapiGetTextureHandle(atlas, 1);
		oapiBlt(iceSkin, metSkin, 0, 0, 0, 0, 512, 2048);
		oapiReleaseTexture(iceSkin);*/
	}

	// Debug
	char pitchLog[256];
	sprintf(pitchLog, "%.3f\t%.3f", simt - launchTime - boosterShutdownTime, GetPitch() * DEG);
	oapiWriteLine(pitchDataLogFile, pitchLog);
}

double ProjectMercury::OrbitalFrameSlipAngle(VECTOR3 pos, VECTOR3 vel)
{
	VECTOR3 axis1, axis2, axis3;
	axis3 = vel;
	axis3 = unit(axis3);
	VECTOR3 vv, vm;
	vv = pos;
	vm = crossp(axis3, vv);    // direction of orbital momentum
	axis2 = unit(crossp(vm, axis3));
	axis1 = crossp(axis2, axis3);
	MATRIX3 Rref = _M(axis1.x, axis2.x, axis3.x, axis1.y, axis2.y, axis3.y, axis1.z, axis2.z, axis3.z);

	MATRIX3 srot;
	GetRotationMatrix(srot);

	// map ship's local axes into reference frame
	VECTOR3 shipx = { srot.m11, srot.m21, srot.m31 };
	VECTOR3 shipy = { srot.m12, srot.m22, srot.m32 };
	VECTOR3 shipz = { srot.m13, srot.m23, srot.m33 };
	shipx = tmul(Rref, shipx);
	shipy = tmul(Rref, shipy);
	shipz = tmul(Rref, shipz);

	VECTOR3 euler;
	euler.x = -atan2(shipy.x, shipx.x); // roll angle
	euler.y = atan2(shipz.y, shipz.z);   // pitch angle
	euler.z = asin(shipz.x);            // yaw angle

	return euler.z;
}

double ProjectMercury::AtlasPitchControl(void)
{
	double planetRad = oapiGetSize(GetSurfaceRef());
	double planetMu = oapiGetMass(GetSurfaceRef()) * GGRAV;

	// New attempt at launch autopilot from doi:10.21236/AD0293892, An Explicit Solution to the Powered Flight Dynamics of a Rocket Vehicle
	double betaP = GetPitch();
	VECTOR3 currentVelocity, currentPosition;
	GetRelativePos(GetSurfaceRef(), currentPosition);
	GetRelativeVel(GetSurfaceRef(), currentVelocity);
	double currentRadius = length(currentPosition);
	double currentSpeed = length(currentVelocity);
	VECTOR3 velocity;
	GetGroundspeedVector(FRAME_HORIZON, velocity);
	double verticalSpeed = velocity.y;
	double targetOrbitalVelocity = sqrt(planetMu * (2.0 / (missionPerigee * 1000.0 + planetRad) - 2.0 / (missionApogee * 1000.0 + planetRad + missionPerigee * 1000.0 + planetRad))); // switch from currentRadius to missionPerigee
	//double timeToCutoff = CORE_ISP_VAC * GetMass() / CORE_THRUST * (exp((targetOrbitalVelocity - currentSpeed) / CORE_ISP_VAC) - 1.0);
	double deltaV = targetOrbitalVelocity - currentSpeed;
	double timeToCutoff = GetMass() * CORE_ISP_VAC / CORE_THRUST * (1.0 - exp(-deltaV / CORE_ISP_VAC));
	double currentFlightPathAngle = -acos(dotp(currentPosition, currentVelocity) / currentRadius / currentSpeed) + PI05;
	double omegaPStar = -currentSpeed * cos(currentFlightPathAngle) / currentRadius;
	double gEp = G * pow(planetRad / currentRadius, 2.0) - pow(currentSpeed * cos(currentFlightPathAngle), 2.0) / currentRadius;
	double DVe = deltaV; // assuming no loss. Actual defined as (fig 2.3) Vf - V + VL - VLf
	double betaPAvg = (0.0 - verticalSpeed) / DVe;
	double accAtCutoff = CORE_THRUST / (GetMass() - CORE_THRUST / CORE_ISP_VAC * timeToCutoff); // DOUBLECHECK THIS!
	double Q6 = CORE_ISP_VAC / DVe - CORE_ISP_VAC / (accAtCutoff * timeToCutoff); // CORE_ISP_VAC is here used as exhaust velocity. Note that I neglect contribution from verniers.
	double omegaPr = (planetRad + missionPerigee * 1000.0 - currentRadius - timeToCutoff * (Q6 * (0.0 - verticalSpeed) + verticalSpeed)) / (Q6 - 0.5) / pow(timeToCutoff, 2.0) / CORE_ISP_VAC; // DOUBLECHECK THIS!

	// Limit and/or smooth omegaPr

	double betaG = gEp / (CORE_THRUST / GetMass());
	double omegaG = 2.0 * omegaPStar - gEp / CORE_ISP_VAC;
	double deltaBeta = betaPAvg - (1.0 - Q6) * omegaPr * timeToCutoff + betaG - betaP; // check what this is, as it is to be multiplied by gain function later
	double gainFunction = 0.2; // set to appropiate value. Pitchrate = gain * pitchDiff
	double omegaP = omegaPr + omegaPStar + omegaG + gainFunction * deltaBeta;

	return omegaP;
}

double ProjectMercury::AtlasTargetCutOffAzimuth(double simt, double ri, double longI, double latI, bool realData)
{
	// We target an orbit with perigee at current alt and target apogee set by user
	double planetRad = oapiGetSize(GetSurfaceRef());
	double planetMu = oapiGetMass(GetSurfaceRef()) * GGRAV;
	double earthAngularVel = 360.0 / oapiGetPlanetPeriod(GetSurfaceRef());

	double targetElevationAngle = 0.0;
	double rCutoff = ri; // or maybe this should be 160.9 km + planetRad
	double vc = sqrt(planetMu / rCutoff);
	double cutoffVel = sqrt(planetMu * (2.0 / rCutoff - 2.0 / (missionApogee * 1000.0 + planetRad + rCutoff)));
	if (realData) // debug
	{
		VECTOR3 currentSpaceVelocity;
		GetRelativeVel(GetSurfaceRef(), currentSpaceVelocity);
		VECTOR3 currentSpaceLocation;
		GetRelativePos(GetSurfaceRef(), currentSpaceLocation);

		targetElevationAngle = -acos(dotp(currentSpaceLocation, currentSpaceVelocity) / length(currentSpaceLocation) / length(currentSpaceVelocity)) * DEG + 90.0;
		cutoffVel = length(currentSpaceVelocity);
	}
	double targetSMa = 1.0 / (2.0 / rCutoff - cutoffVel * cutoffVel / planetMu);

	double pOverr1 = pow(cutoffVel / vc * cos(targetElevationAngle * RAD), 2.0);
	double TrACutoff = atan(tan(targetElevationAngle * RAD) * pOverr1 / (pOverr1 - 1.0));
	double targetEcc = (pOverr1 - 1) / cos(TrACutoff);
	double targetPeriod = PI2 * sqrt(pow(planetRad, 3.0) / planetMu) * pow(targetSMa / planetRad, 1.5);
	double targetSemilatusRectum = pOverr1 * rCutoff;

	double targetLongEquivalent = missionLandLong + missionOrbitNumber * earthAngularVel * targetPeriod;

	// iterative values
	double dOmegaSmall = 0.0;
	double dPhi2 = 0.0;
	double dOmegaLarge = 0.0;
	double dLambda2 = 0.0;
	double tOfTheta2eMinusTTheta1 = targetPeriod / 360.0 * (targetLongEquivalent - longI);
	double dLambda1minus2e = 0.0;
	double cosTrA2eMinusTrAI = 0.0;
	double tTrA2e = 0.0;
	double TrA2e = 0.0;
	double sinAzimuth1 = 0.0;
	double cosInclination = 0.0;

	int iterationNumber = 1;

	for (int i = 1; i <= 6; i++)
	{
		targetLongEquivalent = missionLandLong - dLambda2 + missionOrbitNumber * earthAngularVel * targetPeriod;


		if (i > 1)
		{
			tOfTheta2eMinusTTheta1 = TimeFromPerigee(targetPeriod, targetEcc, TrA2e * RAD) - TimeFromPerigee(targetPeriod, targetEcc, TrACutoff);
			dLambda1minus2e = targetLongEquivalent - longI + earthAngularVel * tOfTheta2eMinusTTheta1;
		}
		else
		{
			dLambda1minus2e = missionLandLong + missionOrbitNumber * earthAngularVel * targetPeriod + earthAngularVel * tOfTheta2eMinusTTheta1 - longI;
		}

		cosTrA2eMinusTrAI = sin(missionLandLat * RAD - dPhi2 * RAD) * sin(latI * RAD) + cos(missionLandLat * RAD - dPhi2 * RAD) * cos(latI * RAD) * cos(dLambda1minus2e * RAD);

		TrA2e = fmod(acos(cosTrA2eMinusTrAI) * DEG + TrACutoff * DEG, 360.0);
		tTrA2e = TimeFromPerigee(targetPeriod, targetEcc, TrA2e * RAD);

		sinAzimuth1 = sin(dLambda1minus2e * RAD) * cos(missionLandLat * RAD - dPhi2 * RAD) / sin(TrA2e * RAD - TrACutoff);

		cosInclination = sinAzimuth1 * cos(latI * RAD);

		if (i == iterationNumber && NonsphericalGravityEnabled())
		{
			dOmegaSmall = 3.4722e-3 / 60.0 * pow(planetRad / targetSemilatusRectum, 2.0) * pow(planetRad / targetSMa, 1.5) * (5.0 * cosInclination * cosInclination - 1.0) * (missionOrbitNumber * targetPeriod + TimeFromPerigee(targetPeriod, targetEcc, TrA2e * RAD) - 0.0);

			double omegaPlusTrA2e = asin(sin(latI * RAD) / sin(acos(cosInclination))) - TrACutoff + TrA2e * RAD;
			dPhi2 = sin(acos(cosInclination)) * cos(omegaPlusTrA2e) / cos(missionLandLat * RAD) * dOmegaSmall;

			dOmegaLarge = -6.9444e-3 / 60.0 * pow(planetRad / targetSemilatusRectum, 2.0) * pow(planetRad / targetSMa, 1.5) * cosInclination * (missionOrbitNumber * targetPeriod + TimeFromPerigee(targetPeriod, targetEcc, TrA2e * RAD) - 0.0);

			dLambda2 = cosInclination / pow(cos(omegaPlusTrA2e), 2.0) / (1.0 + cosInclination * cosInclination * pow(tan(omegaPlusTrA2e), 2.0)) * dOmegaSmall + dOmegaLarge;
		}
	}

	return asin(sinAzimuth1) * DEG;
}

// TrA in radians
inline double ProjectMercury::EccentricAnomaly(double ecc, double TrA)
{
	return 2.0 * atan(sqrt((1.0 - ecc) / (1.0 + ecc)) * tan(TrA / 2.0));
}

// TrA and Eanomaly in radians
inline double ProjectMercury::TimeFromPerigee(double period, double ecc, double TrA)
{
	return period / PI2 * (EccentricAnomaly(ecc, TrA) - ecc * sin(EccentricAnomaly(ecc, TrA)));
}

// MnA in radians
inline double ProjectMercury::MnA2TrA(double MnA, double Ecc)
{
	double TrA = MnA + (2.0 * Ecc - pow(Ecc, 3.0) / 4.0) * sin(MnA) + 5.0 / 4.0 * pow(Ecc, 2.0) * sin(2.0 * MnA) + 13.0 / 12.0 * pow(Ecc, 3.0) * sin(3.0 * MnA);

	return TrA;
}

inline double ProjectMercury::TrA2MnA(double TrA, double Ecc)
{
	double MnA = TrA - 2.0 * Ecc * sin(TrA) + (3.0 / 4.0 * pow(Ecc, 2.0) + pow(Ecc, 4.0) / 8.0) * sin(2.0 * TrA) - pow(Ecc, 3.0) / 3.0 * sin(3.0 * TrA) + 5.0 / 32.0 * pow(Ecc, 4.0) * sin(4.0 * TrA);

	return MnA;
}

inline VECTOR3 ProjectMercury::Ecl2Equ(VECTOR3 Ecl)
{
	MATRIX3 rot;
	oapiGetPlanetObliquityMatrix(GetSurfaceRef(), &rot);
	VECTOR3 Equ = tmul(rot, Ecl);
	return Equ;
}

inline VECTOR3 ProjectMercury::Equ2Ecl(VECTOR3 Equ)
{
	// This method is inverse of Ecl2Equ, but no easy way to get back-rotation matrix, so must do explicitly what oapiGetPlanetObliquityMatrix does
	MATRIX3 R1, R2, R;
	double theta = -oapiGetPlanetTheta(GetSurfaceRef());
	double phi = -oapiGetPlanetObliquity(GetSurfaceRef());
	R1 = _M(cos(theta), 0.0, -sin(theta),
		0.0, 1.0, 0.0,
		sin(theta), 0.0, cos(theta));
	R2 = _M(1.0, 0.0, 0.0,
		0.0, cos(phi), -sin(phi),
		0.0, sin(phi), cos(phi));

	R = mul(R1, R2);
	VECTOR3 Ecl = tmul(R, Equ);
	return Ecl;
}

void ProjectMercury::GetEquPosInTime(double t, double SMa, double Ecc, double Inc, double Per, double LPe, double LAN, double M, double longAtNow, double* longitude, double* latitude)
{
	// This method is partly from NTRS document 20160000809

	double planetRad = oapiGetSize(GetSurfaceRef());
	double planetMu = oapiGetMass(GetSurfaceRef()) * GGRAV;

	double M0 = M;
	// TrA in x seconds
	M = fmod(M + PI2 * t / Per, PI2);
	double TrA = M + (2.0 * Ecc - pow(Ecc, 3.0) / 4.0) * sin(M) + 5.0 / 4.0 * pow(Ecc, 2.0) * sin(2.0 * M) + 13.0 / 12.0 * pow(Ecc, 3.0) * sin(3.0 * M);
	double TrA0 = M0 + (2.0 * Ecc - pow(Ecc, 3.0) / 4.0) * sin(M0) + 5.0 / 4.0 * pow(Ecc, 2.0) * sin(2.0 * M0) + 13.0 / 12.0 * pow(Ecc, 3.0) * sin(3.0 * M0);

	double u = LPe - LAN + TrA;
	double u0 = LPe - LAN + TrA0;
	double alpha = atan2(cos(u) * sin(LAN) + sin(u) * cos(LAN) * cos(Inc), cos(u) * cos(LAN) - sin(u) * sin(LAN) * cos(Inc));
	double alpha0 = atan2(cos(u0) * sin(LAN) + sin(u0) * cos(LAN) * cos(Inc), cos(u0) * cos(LAN) - sin(u0) * sin(LAN) * cos(Inc));
	alpha -= alpha0;

	double longi = alpha + longAtNow - PI2 / oapiGetPlanetPeriod(GetSurfaceRef()) * t;
	if (longi > 0.0)
	{
		while (longi > PI)
			longi -= PI2;
	}
	else
	{
		while (longi < -PI)
			longi += PI2;
	}

	double lati = asin(sin(u) * sin(Inc));

	*longitude = longi;
	*latitude = lati;
}

void ProjectMercury::GetLandingPointIfRetroInXSeconds(double t, ELEMENTS el, ORBITPARAM prm, double longAtNow, double* longitude, double* latitude)
{
	// Constants
	double planetMu = oapiGetMass(GetSurfaceRef()) * GGRAV;
	double planetRad = oapiGetSize(GetSurfaceRef());

	// First calculate state vector at retroburn
	// Propagate TrA to the set time
	double MnA = prm.MnA;
	double TrA = MnA2TrA(fmod(MnA + PI2 * t / prm.T, PI2), el.e);
	// Find state vectors
	double E = EccentricAnomaly(el.e, TrA); // Find eccentric anomaly at the set time
	double r = el.a * (1.0 - el.e * cos(E)); // Find alt at the set time
	VECTOR3 orbitalFramePos = _V(cos(TrA), sin(TrA), 0.0) * r;
	VECTOR3 orbitalFrameVel = _V(-sin(E), sqrt(1.0 - el.e * el.e) * cos(E), 0.0) * sqrt(planetMu * el.a) / r;
	double LAN = el.theta;
	double APe = el.omegab - LAN;
	if (NonsphericalGravityEnabled()) // Take J2 effects into consideration. Perturbs LAN and APe
	{
		// J2 coeffs from historical accurate value in 19980227091 paper (published in 1959)
		APe += 3.4722e-3 * RAD / 60.0 * pow(planetRad / el.a, 3.0) / pow(1.0 - el.e * el.e, 2.0) * (5.0 * cos(el.i) * cos(el.i) - 1.0) * t;
		LAN += -6.9444e-3 * RAD / 60.0 * pow(planetRad / el.a, 3.0) / pow(1.0 - el.e * el.e, 2.0) * cos(el.i) * t;
	}
	VECTOR3 statePos, stateVel; // Thank you to https://downloads.rene-schwarz.com/download/M001-Keplerian_Orbit_Elements_to_Cartesian_State_Vectors.pdf
	statePos.x = orbitalFramePos.x * (cos(APe) * cos(LAN) - sin(APe) * cos(el.i) * sin(LAN)) - orbitalFramePos.y * (sin(APe) * cos(LAN) + cos(APe) * cos(el.i) * sin(LAN));
	statePos.y = orbitalFramePos.x * (cos(APe) * sin(LAN) + sin(APe) * cos(el.i) * cos(LAN)) + orbitalFramePos.y * (cos(APe) * cos(el.i) * cos(LAN) - sin(APe) * sin(LAN));
	statePos.z = orbitalFramePos.x * sin(APe) * sin(el.i) + orbitalFramePos.y * cos(APe) * sin(el.i);
	stateVel.x = orbitalFrameVel.x * (cos(APe) * cos(LAN) - sin(APe) * cos(el.i) * sin(LAN)) - orbitalFrameVel.y * (sin(APe) * cos(LAN) + cos(APe) * cos(el.i) * sin(LAN));
	stateVel.y = orbitalFrameVel.x * (cos(APe) * sin(LAN) + sin(APe) * cos(el.i) * cos(LAN)) + orbitalFrameVel.y * (cos(APe) * cos(el.i) * cos(LAN) - sin(APe) * sin(LAN));
	stateVel.z = orbitalFrameVel.x * sin(APe) * sin(el.i) + orbitalFrameVel.y * cos(APe) * sin(el.i);

	// Then calculate equatorial position at that time
	double longAtRetro, latAtRetro;
	GetEquPosInTime(t, el.a, el.e, el.i, prm.T, APe + LAN, LAN, MnA, longAtNow, &longAtRetro, &latAtRetro); // the parameters must be in equatorial frame

	// First calculate current position
	VECTOR3 currPos, currVel, postBurnPos, postBurnVel;

	currPos = statePos;
	currVel = stateVel;

	postBurnPos = currPos;
	double dV = 132.5;
	postBurnVel = currVel - unit(currVel) * dV * cos(34.0 * RAD) - unit(currPos) * dV * sin(34.0 * RAD); // assumning currVel is horizontal

	double postBurnR = length(postBurnPos);
	double postBurnV = length(postBurnVel);
	double postBurnRadVel = dotp(postBurnPos, postBurnVel) / postBurnR;
	VECTOR3 postBurnH = crossp(postBurnPos, postBurnVel);
	double postBurnInc = acos(postBurnH.z / length(postBurnH));
	VECTOR3 postBurnN = crossp(_V(0.0, 0.0, 1.0), postBurnH);
	double postBurnLAN = acos(postBurnN.x / length(postBurnN));
	if (postBurnN.y < 0.0)
		postBurnLAN = PI2 - postBurnLAN;
	VECTOR3 postBurnEccV = (postBurnPos * (postBurnV * postBurnV - planetMu / postBurnR) - postBurnVel * postBurnR * postBurnRadVel) / planetMu;
	double postBurnEcc = length(postBurnEccV);
	double postBurnAPe = acos(dotp(unit(postBurnN), unit(postBurnEccV)));
	if (postBurnEccV.z < 0.0)
		postBurnAPe = PI2 - postBurnAPe;
	double postBurnTrA = acos(dotp(postBurnEccV, postBurnPos) / postBurnEcc / postBurnR);
	if (postBurnRadVel < 0.0)
		postBurnTrA = PI2 - postBurnTrA;

	double postBurnEnergy = length2(postBurnVel) / 2.0 - planetMu / length(postBurnPos);
	double postBurnSMa = -planetMu / (2.0 * postBurnEnergy);
	double postBurnPer = PI2 * sqrt(pow(postBurnSMa, 3.0) / planetMu);
	double postBurnMnA = TrA2MnA(postBurnTrA, postBurnEcc);

	// Entry interface at altitude 87 550 m
	double entryRadius = 87550.0 + oapiGetSize(GetSurfaceRef());
	double entryTrA = acos((postBurnSMa / entryRadius * (1.0 - postBurnEcc * postBurnEcc) - 1.0) / postBurnEcc);

	if (entryTrA < PI) // entry is on trajectory towards perigee, which is at TrA = 0.0
		entryTrA = PI2 - entryTrA;
	double timeToEntry = TimeFromPerigee(postBurnPer, postBurnEcc, entryTrA) - TimeFromPerigee(postBurnPer, postBurnEcc, postBurnTrA);
	if (NonsphericalGravityEnabled()) // Take J2 effects into consideration. Perturbs LAN and APe
	{
		// J2 coeffs from historical accurate value in 19980227091 paper (published in 1959)
		 postBurnAPe += 3.4722e-3 * RAD / 60.0 * pow(planetRad / postBurnSMa, 3.0) / pow(1.0 - postBurnEcc * postBurnEcc, 2.0) * (5.0 * cos(postBurnInc) * cos(postBurnInc) - 1.0) * timeToEntry;
		 postBurnLAN += -6.9444e-3 * RAD / 60.0 * pow(planetRad / postBurnSMa, 3.0) / pow(1.0 - postBurnEcc * postBurnEcc, 2.0) * cos(postBurnInc) * timeToEntry;
		 // We are not sooo extreme that we consider perturbions also after entry interface.
		 // But for a typical 500 seconds from retroburn to entry interface, there is a ~0.05 deg change, which accounts for 7.5 km cross-range. So it has some use here
	}
	double postBurnLPe = fmod(postBurnLAN + postBurnAPe, PI2);
	double entryLong, entryLat;
	GetEquPosInTime(timeToEntry, postBurnSMa, postBurnEcc, postBurnInc, postBurnPer, postBurnLPe, postBurnLAN, postBurnMnA, longAtRetro, &entryLong, &entryLat);

	// Entry angle
	double entryAngle = -abs(acos((1.0 + postBurnEcc * cos(entryTrA)) / sqrt(1.0 + postBurnEcc * postBurnEcc + 2.0 * postBurnEcc * cos(entryTrA))));
	double entryAngleDeg = entryAngle * DEG;
	double angleCoveredDuringReentry = (2.1545 * entryAngleDeg * entryAngleDeg + 11.1236 * entryAngleDeg + 23.9225) * RAD; // empirical formula from dataset of reentries. Second order polynomial
	double landingLat = asin(sin(postBurnLPe - postBurnLAN + entryTrA + angleCoveredDuringReentry) * sin(postBurnInc));
	double landingLong = entryLong + acos((cos(angleCoveredDuringReentry) - sin(entryLat) * sin(landingLat)) / cos(entryLat) / cos(landingLat));

	*longitude = landingLong;
	*latitude = landingLat;
}

void ProjectMercury::AtlasEngineDir(void)
{
	double TotalPitch = GetControlSurfaceLevel(AIRCTRL_ELEVATOR);
	double TotalYaw = GetControlSurfaceLevel(AIRCTRL_RUDDER);
	double TotalRoll = GetControlSurfaceLevel(AIRCTRL_AILERON);

	VECTOR3 vernier1Direction, vernier2Direction;
	if (TotalRoll != 0.0)
	{
		double deflRo = -TotalRoll * 70.0 * RAD; // negative because it was inverted
		VECTOR3 t0 = _V(VERNIER_EXHAUST_DIR.x, VERNIER_EXHAUST_DIR.y, VERNIER_EXHAUST_DIR.z);
		vernier1Direction = _V(t0.x * cos(deflRo) + t0.z * sin(deflRo), t0.y, -t0.x * sin(deflRo) + t0.z * cos(deflRo));
		SetThrusterDir(th_vernier[0], vernier1Direction);
		t0 = FlipY(t0);
		vernier2Direction = _V(t0.x * cos(-deflRo) + t0.z * sin(-deflRo), t0.y, -t0.x * sin(-deflRo) + t0.z * cos(-deflRo));
		SetThrusterDir(th_vernier[1], vernier2Direction);

		if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP) // get help from the two booster engines
		{
			// Thrust gimbal of booster thrusters. Swiveled max 5 deg in pitch and yaw (19630012071 page 97(
			VECTOR3 thrustDirection;
			thrustDirection.z = cos(TotalRoll * 5.0 * RAD);
			thrustDirection.y = sin(-TotalRoll * 5.0 * RAD); // negative because it was inverted
			thrustDirection.x = 0.0;
			SetThrusterDir(th_booster[0], thrustDirection);
			SetThrusterDir(th_booster[1], FlipY(thrustDirection));
		}
	}
	else
	{
		double deflPi = TotalPitch * 25.0 * RAD;
		double deflYa = -TotalYaw * 70.0 * RAD; // negative because it was inverted
		VECTOR3 t0 = VERNIER_EXHAUST_DIR;
		vernier1Direction = _V(t0.x * cos(deflYa) + (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) - t0.x * sin(deflYa));
		SetThrusterDir(th_vernier[0], vernier1Direction);
		t0 = FlipY(t0);
		vernier2Direction = _V(t0.x * cos(deflYa) + (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) - t0.x * sin(deflYa));
		SetThrusterDir(th_vernier[1], vernier2Direction);

		if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP) // get help from the two booster engines
		{
			// Thrust gimbal of booster thrusters. Swiveled max 5 deg in pitch and yaw (19630012071 page 97)
			t0 = BOOSTER_EXHAUST_DIR;
			deflPi = TotalPitch * 5.0 * RAD;
			deflYa = -TotalYaw * 5.0 * RAD; // negative because it was inverted
			VECTOR3 booster1Direction = _V(t0.x * cos(deflYa) + (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) - t0.x * sin(deflYa));
			SetThrusterDir(th_booster[1], booster1Direction);
			SetThrusterDir(th_booster[0], booster1Direction);
		}

		// Thrust gimbal of core thruster. Swiveled max 3 deg in pitch and yaw (19630012071 page 97)
		t0 = CORE_EXHAUST_DIR;
		deflPi = TotalPitch * 3.0 * RAD;
		deflYa = -TotalYaw * 3.0 * RAD; // negative because it was inverted
		VECTOR3 coreDirection = _V(t0.x * cos(deflYa) + (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) - t0.x * sin(deflYa));
		SetThrusterDir(th_main, coreDirection);
	}
}

void ProjectMercury::DefineVernierAnimations(void)
{
	// Max deflection from 19630012071 page 97 (chamber can be moved through an arc of ~140 deg in pitch and ~50 deg in yaw)
	// -y vernier yaw
	static UINT vernier1[1] = { 3 };
	static MGROUP_ROTATE vern1x(
		Atlas,
		vernier1, 1,
		VERNIER1_OFFSET,
		_V(1, 0, 0),
		50 * RAD
	);
	Vernier1AnimX = CreateAnimation(0.5);
	AddAnimationComponent(Vernier1AnimX, 0.0, 1.0, &vern1x);

	// -y vernier pitch
	static MGROUP_ROTATE vern1y(
		Atlas,
		vernier1, 1,
		VERNIER1_OFFSET,
		_V(0, 1, 0),
		140 * RAD
	);
	Vernier1AnimY = CreateAnimation(0.5);
	AddAnimationComponent(Vernier1AnimY, 0.0, 1.0, &vern1y);

	// +y vernier yaw
	static UINT vernier2[1] = { 4 };
	static MGROUP_ROTATE vern2x(
		Atlas,
		vernier2, 1,
		VERNIER2_OFFSET,
		_V(1, 0, 0),
		50 * RAD
	);
	Vernier2AnimX = CreateAnimation(0.5);
	AddAnimationComponent(Vernier2AnimX, 0.0, 1.0, &vern2x);

	// +y vernier pitch
	static MGROUP_ROTATE vern2y(
		Atlas,
		vernier2, 1,
		VERNIER2_OFFSET,
		_V(0, 1, 0),
		140 * RAD
	);
	Vernier2AnimY = CreateAnimation(0.5);
	AddAnimationComponent(Vernier2AnimY, 0.0, 1.0, &vern2y);
}

void ProjectMercury::CreateAirfoilsAtlas(void)
{
	ClearAirfoilDefinitions(); // delete previous airfoils
	SetRotDrag(_V(0.25, 0.25, 0.1)); // from BrianJ's Falcon 9
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 0.1), vliftAtlas, NULL, 3.0, 3.0 * 3.0 * PI / 4.0, 1.0); // Width from Wikipedia (Atlas LV-3B page. 3.0 m diameter, but that does not consider booster extra bulk
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 0.1), hliftAtlas, NULL, 3.0, 3.0 * 3.0 * PI / 4.0, 1.0); // spherical symmetric
}

double ProjectMercury::EmptyMass(void)
{
	double mass;

	if (VesselStatus == TOWERSEP)
		mass = MERCURY_MASS + CORE_DRY_MASS + BOOSTER_DRY_MASS;
	else if (VesselStatus == LAUNCHCORE)
		mass = MERCURY_MASS + ABORT_MASS + CORE_DRY_MASS;
	else if (VesselStatus == LAUNCHCORETOWERSEP)
		mass = MERCURY_MASS + CORE_DRY_MASS;
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
	else
		// LAUNCH
		mass = MERCURY_MASS + CORE_DRY_MASS + BOOSTER_DRY_MASS + ABORT_MASS;

	return mass;
}

void ProjectMercury::TowerSeparation(void)
{
	if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == ABORT) // only occasions with tower
	{
		SeparateTower(true);

		if (VesselStatus == LAUNCH)
		{
			VesselStatus = TOWERSEP;
			CGshifted = false;
		}
		else if (VesselStatus == LAUNCHCORE)
		{
			VesselStatus = LAUNCHCORETOWERSEP;
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

	// Angular velocity
	VECTOR3 angVel;
	GetAngularVel(angVel);

	// Treshholds from MA-9 postlaunch report part 1 page 155
	if (abs(angVel.x * DEG) > 3.0)
	{
		abortConditionsMet = true;
		char cbuf[256];
		sprintf(cbuf, "Abort due to error in pitch rate at T+%.1f. It was %.1f deg/s", met, angVel.x * DEG);
		oapiWriteLog(cbuf);
	}
	if (abs(angVel.y * DEG) > 3.0)
	{
		abortConditionsMet = true;
		char cbuf[256];
		sprintf(cbuf, "Abort due to error in yaw rate at T+%.1f. It was %.1f deg/s", met, angVel.y * DEG);
		oapiWriteLog(cbuf);
	}
	if (abs(angVel.z * DEG) > rollLimit) // Wiki for MA-8 says it encountered a roll of 7.83 deg/s, which was "20 % from abort condition". Indicating limit 10 deg. Don't know what limit was before MA-9. MA-9 report also reports oscillations of 6 deg/s, but are probably so small that they are not considered
	{
		abortConditionsMet = true;
		char cbuf[256];
		sprintf(cbuf, "Abort due to error in roll rate at T+%.1f. It was %.1f deg/s", met, angVel.z * DEG);
		oapiWriteLog(cbuf);
	}

	if (met < 0.1) // don't accumulate angular velocity when landed
		return;

	// Attitude error
	integratedPitch += angVel.x * DEG * simdt;
	integratedYaw += angVel.y * DEG * simdt;
	integratedRoll += angVel.z * DEG * simdt;

	// These are currently dependent on autopilot, to let the user launch in any direction if needed
	/*if (autoPilot && abs(GetPitch() * DEG - currentPitchAim) > 5.0)
	{
		abortConditionsMet = true;
		char cbuf[256];
		sprintf(cbuf, "Abort due to error in pitch at T+%.1f. It was %.1f deg. Target was %.2f deg", met, GetPitch() * DEG, currentPitchAim);
		oapiWriteLog(cbuf);
	}
	if (autoPilot && abs(integratedYaw - currentYawAim) > 5.0)
	{
		abortConditionsMet = true;
		char cbuf[256];
		sprintf(cbuf, "Abort due to error in yaw at T+%.1f. It was %.1f deg. Target was %.2f deg", met, integratedYaw, currentYawAim);
		oapiWriteLog(cbuf);
	}
	if (autoPilot && !rollProgram && abs(integratedRoll - currentRollAim) > 10.0)
	{
		abortConditionsMet = true;
		char cbuf[256];
		sprintf(cbuf, "Abort due to error in roll at T+%.1f. It was %.1f deg. Target was %.2f deg", met, integratedRoll, currentRollAim);
		oapiWriteLog(cbuf);
	}*/
}

void ProjectMercury::LaunchAbort(void)
{
	abort = true;
	suborbitalMission = true; // an abort is always ballistic

	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE)) // Abort after liftoff 
	{
		SeparateAtlasBooster(false);
		SeparateAtlasCore();
		SeparateRingsAndAdapters(); // Adapters and rings
		
		SeparateRetroPack(false);
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
	if (VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP)
	{
		SeparateAtlasCore();
		SeparateRingsAndAdapters(); // Adapters and rings

		CreatePosigradeRockets();
		SetPropellantMass(posigrade_propellant[0], POSIGRADE_MASS_FUEL);
		SetPropellantMass(posigrade_propellant[1], POSIGRADE_MASS_FUEL);
		SetPropellantMass(posigrade_propellant[2], POSIGRADE_MASS_FUEL);
		CreateRetroRockets();
		SetPropellantMass(retro_propellant[0], RETRO_MASS_FUEL);
		SetPropellantMass(retro_propellant[1], RETRO_MASS_FUEL);
		SetPropellantMass(retro_propellant[2], RETRO_MASS_FUEL);
		SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);
		SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);

		autoPilot = true;
		AutopilotStatus = POSIGRADEDAMP; // Automatically aux-damp 
		VesselStatus = FLIGHT;
		CGshifted = false;

		// Check if in the go-no-go plot. Constrains from looking at image in 19630012071 page 132
		double minVel = historyCutOffAngl * historyCutOffAngl * 55.661 + 7802.9;
		double maxVel = -historyCutOffAngl * historyCutOffAngl * 55.661 + 7894.3;
		if (abs(historyCutOffAngl) < 0.9063 && historyCutOffVel < maxVel && historyCutOffVel > minVel)
		{
			oapiWriteLog("GO for orbit!");
		}
		else if (abs(historyCutOffAngl) < 0.9063 && historyCutOffVel < maxVel) // not larger than minVel
		{
			oapiWriteLog("NO GO for orbit! Orbit lifetime less than one pass!");
		}
		else if (abs(historyCutOffAngl) < 0.9063) // not smaller than maxVel
		{
			oapiWriteLog("NO GO for orbit! Safe reentry not possible from all points!");
		}
		else if (historyCutOffVel != 0.0) // sanity check, if manual launch, there is no recorded value
		{
			oapiWriteLog("NO GO for orbit!");
		}
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

	if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE)
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
	{
		createdAbove50km[stuffCreated] = true;
	}

	stuffCreated += 1;
	DelMesh(Tower);
	DelPropellantResource(escape_tank);
	DelThruster(escape_engine);
	towerJetTime = oapiGetSimTime();
}

void ProjectMercury::SeparateAtlasBooster(bool noAbortSep)
{
	if (noAbortSep)
	{
		VESSELSTATUS2 vs;
		vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
		vs.version = 2;

		GetStatusEx(&vs);
		char name[256];
		VECTOR3 redstoneOffset = ATLAS_BOOSTER_OFFSET;
		VECTOR3 redstoneDirection = { 0,-1, 0 };
		double redstoneVelocity = 0.0;

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
			vs.vrot.x = ATLAS_CORE_LENGTH / 2.0 + heightOverGround;
		}
		else
		{
			VECTOR3 relativeOffset;
			Local2Rel(redstoneOffset, vs.rpos);
			GlobalRot(redstoneDirection, relativeOffset);
			vs.rvel += relativeOffset * redstoneVelocity;
			inFlightAbort = true;
		}

		strcpy(name, GetName());
		strcat(name, " Atlas booster");
		int k = 0;
		while (oapiGetVesselByName(name) != NULL)
		{
			k += 1;
			strcat(name, "1");
		}

		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_atlas_booster", &vs);
		if (GetAltitude() > 5e4)
			createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;

		if (VesselStatus == LAUNCH)
		{
			VesselStatus = LAUNCHCORE;
		}
		else if (VesselStatus == TOWERSEP)
		{
			VesselStatus = LAUNCHCORETOWERSEP;
		}
	}

	// Delete stuff either way if abort or not
	DelThruster(th_booster[0]);
	DelThruster(th_booster[1]);
	DelExhaustStream(turbineExhaustContrail);
	DelExhaustStream(boosterExhaustContrail[0]);
	DelExhaustStream(boosterExhaustContrail[1]);
	ClearLightEmitters();
	DelExhaust(exBooster[0]);
	DelExhaust(exBooster[1]);
	thCluster[0] = th_main;
	thCluster[1] = th_vernier[0];
	thCluster[2] = th_vernier[1];
	thCluster[3] = NULL;
	thCluster[4] = NULL;
	CreateThrusterGroup(thCluster, 3, THGROUP_MAIN);
	DelMesh(AtlasBooster);
}

void ProjectMercury::SeparateAtlasCore(void)
{
	VESSELSTATUS2 vs;
	VESSELSTATUS2::FUELSPEC fuel;
	VESSELSTATUS2::THRUSTSPEC thrust;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	GetStatusEx(&vs);
	char name[256];
	VECTOR3 redstoneOffset = ATLAS_CORE_OFFSET;
	VECTOR3 redstoneDirection = { 0,-1, 0 };
	double redstoneVelocity = 0.0;

	vs.flag = VS_FUELLIST;
	vs.fuel = &fuel;
	vs.nfuel = 1;
	vs.fuel->idx = 0;
	vs.fuel->level = GetFuelMass() / GetMaxFuelMass(); // this does not seem to work, so doing the same further down this function

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
		vs.vrot.x = ATLAS_CORE_LENGTH / 2.0 + heightOverGround;
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
		vs.nthruster = 5;
		vs.thruster->idx = 0;
		vs.thruster->level = 1.0;
		vs.thruster->idx = 1;
		vs.thruster->level = 1.0;
		vs.thruster->idx = 2;
		vs.thruster->level = 1.0;
		vs.thruster->idx = 3;
		vs.thruster->level = 1.0;
		vs.thruster->idx = 4;
		vs.thruster->level = 1.0;
		oapiWriteLog("Keep Atlas burning due to T < +30");
	}

	strcpy(name, GetName());
	strcat(name, " Atlas core");
	int k = 0;
	while (oapiGetVesselByName(name) != NULL)
	{
		k += 1;
		strcat(name, "1");
	}

	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "Vessels\\ProjectMercury\\Mercury_AtlasCore", &vs);
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;

	// The thrusterlist spec doesnt seem to work, sadly
	VESSEL* v = oapiGetVesselInterface(createdVessel[stuffCreated - 1]);
	double levl = GetPropellantMass(atlas_propellant);
	v->SetPropellantMass(v->GetPropellantHandleByIndex(0), levl);
	if (GetThrusterLevel(th_main) != 0.0 && oapiGetSimTime() - launchTime < 30.0)
	{
		v->SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
	}

	DelThruster(th_main);
	DelThruster(th_vernier[0]);
	DelThruster(th_vernier[1]);
	DelThruster(th_booster[0]);
	DelThruster(th_booster[1]);
	DelPropellantResource(atlas_propellant);
	DelThruster(th_rcsDummyBooster[0]);
	DelThruster(th_rcsDummyBooster[1]);
	DelThruster(th_rcsDummyBooster[2]);
	DelThruster(th_rcsDummyBooster[3]);
	DelThruster(th_rcsDummyBooster[4]);
	DelThruster(th_rcsDummyBooster[5]);
	DelControlSurface(Verniers[0]);
	DelControlSurface(Verniers[1]);
	DelControlSurface(Verniers[2]);
	DelMesh(Atlas);
	DelMesh(AtlasAdapter);
	DelMesh(AtlasBooster);
}

void ProjectMercury::SeparateRingsAndAdapters()
{
	VESSELSTATUS2 vs;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	char name[256];
	VECTOR3 vel = _V(0, .95, 0);
	VECTOR3 relativeOffset1;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER1, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset1);
	VECTOR3 vel2;
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 1");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover1", &vs);
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Adaptcover1);

	vel = _V(-0.823, -0.475, 0);
	VECTOR3 relativeOffset2;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER2, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset2);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 2");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover2", &vs);
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Adaptcover2);

	vel = _V(0.823, -0.475, 0);
	VECTOR3 relativeOffset3;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER3, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset3);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 3");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover3", &vs);
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Adaptcover3);

	vel = _V(-0.823, 0.475, 0);
	VECTOR3 relativeOffset4;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING1, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset4);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 1");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing1", &vs);
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Adaptring1);

	vel = _V(0.0, -0.95, 0);
	VECTOR3 relativeOffset5;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING2, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset5);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 2");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing2", &vs);
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Adaptring2);

	vel = _V(0.823, 0.475, 0);
	VECTOR3 relativeOffset6;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING3, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset6);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 3");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing3", &vs);
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Adaptring3);
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
	oapiCloseFile(pitchDataLogFile, FILE_OUT);

	if (vessel) delete (ProjectMercury*)vessel;
}

void ProjectMercury::vliftAtlas(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
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

	static const double mach[12] = {
		0.0, 0.50, 0.7, 0.90, 1.00, 1.15, 1.5, 2.0, 3.0, 5.0, 7.0, 9.6
	};
	static const double cdp[12] = { // drag coeff at 0 AoA (tip first) for different mach numbers.
		0.63, 0.64, 0.64, 0.72, 0.92, 0.9, 0.78, 0.66, 0.46, 0.3, 0.23, 0.18
	};

	double aoastep = 30.0 * RAD;
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

void ProjectMercury::hliftAtlas(VESSEL* v, double beta, double M, double Re, void* context, double* cl, double* cm, double* cd)
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