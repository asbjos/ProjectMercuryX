// ==============================================================
//				Source file for Mercury Atlas.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2019-2020
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
#define VESSELVER VESSEL4

#include "orbitersdk.h"

#include "MercuryAtlas.h"
#include "..\..\FunctionsForOrbiter2016.h"
#include "..\..\MercuryCapsule.h"
#include <fstream> // debug, for appending entry data to file


ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel)
	: VESSELVER(hVessel, flightmodel)
{
	// Load exterior meshes
	// capsule is defined in loadstate
	atlasAdapter = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_adapt");
	atlas = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main");
	atlasIce = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main_ice");
	atlasIce2 = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main_ice2");
	atlasIce3 = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_main_ice3");
	atlasBooster = oapiLoadMeshGlobal("ProjectMercury\\merc_atl_boost");
	
	MercuryGenericConstructor();
}

ProjectMercury::~ProjectMercury()
{
	WriteFlightParameters();

	//oapiCloseFile(PIDdebug, FILE_OUT);
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

	oapiReadItem_bool(cfg, "CAPSULEONLY", capsuleOnly); // spawning a single capsule
	oapiReadItem_bool(cfg, "CAPSULETOWERRETROONLY", capsuleTowerRetroOnly); // spawning a capsule with retro and abort tower
	oapiReadItem_bool(cfg, "CAPSULETOWERONLY", capsuleTowerOnly); // spawning a capsule with abort tower (a la beach abort)
	if (capsuleOnly || capsuleTowerRetroOnly || capsuleTowerOnly)
	{
		if (capsuleOnly)
		{
			VesselStatus = FLIGHT;
			oapiWriteLog("Starting from capsule mode!");
		}
		else if (capsuleTowerOnly)
		{
			VesselStatus = ABORTNORETRO;
			oapiWriteLog("Starting from capsule + tower mode!");
		}
		else // capsuleTowerRetroOnly
		{
			VesselStatus = ABORT;
			oapiWriteLog("Starting from capsule + tower + retro mode!");
		}

		// Change vectors, so that spacecraft loads at centre, and no shift takes place
		MERCURY_OFS_CAPSULE.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		ABORT_OFFSET.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_SHIELD.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_ANTHOUSE.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_ABORT.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_DROGUECOVER.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_LANDBAG.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_RETRO.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_RETROCOVER1.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_RETROCOVER2.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_RETROCOVER3.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_RETROSTRAP1.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_RETROSTRAP2.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_RETROSTRAP3.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		MERCURY_OFS_EXPLOSIVEBOLT.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		OFS_ADAPTRING1.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		OFS_ADAPTRING2.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		OFS_ADAPTRING3.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		OFS_ADAPTCOVER1.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		OFS_ADAPTCOVER2.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
		OFS_ADAPTCOVER3.z += -(MERCURY_LENGTH_CAPSULE) / 2.0 - ATLAS_CORE_LENGTH / 2.0;
	}

	oapiReadItem_bool(cfg, "MANOUVERCONCEPT", conceptManouverUnit);
	if (conceptManouverUnit)
	{
		atlasAdapter = oapiLoadMeshGlobal("ProjectMercury\\ManouverConcept\\posigrade");
		conceptRetrogradeThrusters = oapiLoadMeshGlobal("ProjectMercury\\ManouverConcept\\retrograde");
		conceptCover1 = oapiLoadMeshGlobal("ProjectMercury\\ManouverConcept\\concept_retrograde_caps1_trans");
		conceptCover2 = oapiLoadMeshGlobal("ProjectMercury\\ManouverConcept\\concept_retrograde_caps2_trans");
		POSIGRADE_MASS_FUEL = 0.0; // remove regular posigrades

		// Extend vectors, as new adapter is 0.238 m longer
		MERCURY_OFS_CAPSULE.z += CONCEPT_EXTRA_LENGTH;
		ATLAS_ADAPTER_OFFSET.z += CONCEPT_EXTRA_LENGTH / 2.0; // grows in both ends
		ABORT_OFFSET.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_SHIELD.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_ANTHOUSE.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_ABORT.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_DROGUECOVER.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_LANDBAG.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_RETRO.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_RETROCOVER1.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_RETROCOVER2.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_RETROCOVER3.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_RETROSTRAP1.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_RETROSTRAP2.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_RETROSTRAP3.z += CONCEPT_EXTRA_LENGTH;
		MERCURY_OFS_EXPLOSIVEBOLT.z += CONCEPT_EXTRA_LENGTH;
		OFS_ADAPTRING1.z += CONCEPT_EXTRA_LENGTH;
		OFS_ADAPTRING2.z += CONCEPT_EXTRA_LENGTH;
		OFS_ADAPTRING3.z += CONCEPT_EXTRA_LENGTH;
		OFS_ADAPTCOVER1.z += CONCEPT_EXTRA_LENGTH;
		OFS_ADAPTCOVER2.z += CONCEPT_EXTRA_LENGTH;
		OFS_ADAPTCOVER3.z += CONCEPT_EXTRA_LENGTH;

		oapiWriteLog("Using conceptual manouver unit!");
	}

	static const DWORD tchdwnLaunchPadNum = 4;
	const VECTOR3 TOUCHDOWN_LAUNCH0 = _V(0.0, -1.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH1 = _V(-0.7, 0.7, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH2 = _V(0.7, 0.7, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH3 = _V(0.0, 0.0, MERCURY_OFS_ABORT.z + MERCURY_LENGTH_ABORT / 2.0);
	VersionDependentTouchdown(TOUCHDOWN_LAUNCH0, TOUCHDOWN_LAUNCH1, TOUCHDOWN_LAUNCH2, TOUCHDOWN_LAUNCH3, 1e7, 1e5, 10.0);
	//SetTouchdownPoints(TOUCHDOWN_LAUNCH0, TOUCHDOWN_LAUNCH1, TOUCHDOWN_LAUNCH2);

	SetCW(1.0, 0.1, 0.3, 0.3);
	SetCrossSections(_V(80.15, 85.27, 10.89));
	SetRotDrag(_V(0.7, 0.7, 1.2));
	//SetPMI(_V(33.0, 33.0, 20.51)); // actually much smaller z-component, but makes atmospheric flight unstable at >1x
	SetPMI(_V(39.7, 39.7, 1.45)); // From legacy Atlas.h

	CreateCapsuleFuelTanks();

	if (conceptManouverUnit) // Create tanks and thrusters
	{
		conceptPropellant = CreatePropellantResource(CONCEPT_POSIGRADE_FUEL_MASS);

		COLOUR4 col_d = { 0.5f, 0.4f, 0.6f, 0.0f };
		COLOUR4 col_s = { 0.6f, 0.4f, 0.6f, 0.0f };
		COLOUR4 col_a = { 0.0f, 0.0f, 0.0f, 0.0f };
		double a0 = 1;
		double a1 = 1;
		double a2 = 1;

		conceptPosigrade[0] = CreateThruster(CONCEPT_POSIGRADE_THRUSTER_LEFT, CONCEPT_POSIGRADE_THRUSTER_LEFT_DIR, CONCEPT_POSIGRADE_THRUST, conceptPropellant, CONCEPT_POSIGRADE_ISP);
		conceptPosigrade[1] = CreateThruster(CONCEPT_POSIGRADE_THRUSTER_RIGHT, CONCEPT_POSIGRADE_THRUSTER_RIGHT_DIR, CONCEPT_POSIGRADE_THRUST, conceptPropellant, CONCEPT_POSIGRADE_ISP);
		LightEmitter* concept0Light = AddPointLight(CONCEPT_POSIGRADE_THRUSTER_LEFT, 10, a0, a1, a2, col_d, col_s, col_a);
		concept0Light->SetIntensityRef(&conceptThrusterLevel[0]);
		LightEmitter* concept1Light = AddPointLight(CONCEPT_POSIGRADE_THRUSTER_RIGHT, 10, a0, a1, a2, col_d, col_s, col_a);
		concept1Light->SetIntensityRef(&conceptThrusterLevel[1]);

		conceptRetrograde[0] = CreateThruster(CONCEPT_RETROGRADE_THRUSTER_UP, CONCEPT_RETROGRADE_THRUSTER_UP_DIR, CONCEPT_POSIGRADE_THRUST, conceptPropellant, CONCEPT_POSIGRADE_ISP);
		conceptRetrograde[1] = CreateThruster(CONCEPT_RETROGRADE_THRUSTER_DOWN, CONCEPT_RETROGRADE_THRUSTER_DOWN_DIR, CONCEPT_POSIGRADE_THRUST, conceptPropellant, CONCEPT_POSIGRADE_ISP);
		LightEmitter* concept2Light = AddPointLight(CONCEPT_RETROGRADE_THRUSTER_UP, 10, a0, a1, a2, col_d, col_s, col_a);
		concept2Light->SetIntensityRef(&conceptThrusterLevel[2]);
		LightEmitter* concept3Light = AddPointLight(CONCEPT_RETROGRADE_THRUSTER_DOWN, 10, a0, a1, a2, col_d, col_s, col_a);
		concept3Light->SetIntensityRef(&conceptThrusterLevel[3]);

		AddExhaust(conceptPosigrade[0], 0.8, 0.1);
		AddExhaust(conceptPosigrade[1], 0.8, 0.1);
		AddExhaust(conceptRetrograde[0], 0.8, 0.1);
		AddExhaust(conceptRetrograde[1], 0.8, 0.1);

		conceptLinear[0] = CreateThruster(MERCURY_OFS_CAPSULE, _V(0, 1, 0), CONCEPT_LINEAR_THRUST, conceptPropellant, CONCEPT_POSIGRADE_ISP);
		conceptLinear[1] = CreateThruster(MERCURY_OFS_CAPSULE, _V(0, -1, 0), CONCEPT_LINEAR_THRUST, conceptPropellant, CONCEPT_POSIGRADE_ISP);
		conceptLinear[2] = CreateThruster(MERCURY_OFS_CAPSULE, _V(-1, 0, 0), CONCEPT_LINEAR_THRUST, conceptPropellant, CONCEPT_POSIGRADE_ISP);
		conceptLinear[3] = CreateThruster(MERCURY_OFS_CAPSULE, _V(1, 0, 0), CONCEPT_LINEAR_THRUST, conceptPropellant, CONCEPT_POSIGRADE_ISP);

		AddExhaust(conceptLinear[0], 0.2, 0.05, CONCEPT_LINEAR_UP - MERCURY_OFS_CAPSULE + _V(0,0, CONCEPT_EXTRA_LENGTH/2.0), _V(0, -1, 0));
		AddExhaust(conceptLinear[1], 0.2, 0.05, CONCEPT_LINEAR_DOWN - MERCURY_OFS_CAPSULE + _V(0, 0, CONCEPT_EXTRA_LENGTH/2.0), _V(0, 1, 0));
		AddExhaust(conceptLinear[2], 0.2, 0.05, CONCEPT_LINEAR_RIGHT - MERCURY_OFS_CAPSULE + _V(0, 0, CONCEPT_EXTRA_LENGTH/2.0), _V(1, 0, 0));
		AddExhaust(conceptLinear[3], 0.2, 0.05, CONCEPT_LINEAR_LEFT - MERCURY_OFS_CAPSULE + _V(0, 0, CONCEPT_EXTRA_LENGTH/2.0), _V(-1, 0, 0));
	}

	atlas_propellant = CreatePropellantResource(CORE_FUEL_MASS);

	escape_tank = CreatePropellantResource(ABORT_MASS_FUEL); // need to create escape tank afterwards, as it's removed before Atlas prop
	//SetPropellantMass(redstone_propellant, STAGE1_FUEL_MASS);

	// This must be last, as to not make Orbiter spawn an old scenario with this index taking another, and thus make e.g. 1 retro without fuel
	dummyControllerFuel = CreatePropellantResource(1e-3); // create a 1 gram fuel supply for control stick, to always have ability to use dummy thrusters on control stick.

	SetDefaultPropellantResource(atlas_propellant);

	// main engine
	th_main = CreateThruster(CORE_EXHAUST_POS, CORE_EXHAUST_DIR, CORE_THRUST, atlas_propellant, CORE_ISP_VAC, CORE_ISP_SL);
	th_vernier[0] = CreateThruster(VERNIER_EXHAUST_POS, VERNIER_EXHAUST_DIR, VERNIER_THRUST_VAC, atlas_propellant, VERNIER_ISP_VAC, VERNIER_ISP_SL); // -y
	th_vernier[1] = CreateThruster(FlipX(VERNIER_EXHAUST_POS), FlipX(VERNIER_EXHAUST_DIR), VERNIER_THRUST_VAC, atlas_propellant, VERNIER_ISP_VAC, VERNIER_ISP_SL); // +y
	th_booster[0] = CreateThruster(BOOSTER_EXHAUST_POS, BOOSTER_EXHAUST_DIR, BOOSTER_THRUST, atlas_propellant, BOOSTER_ISP_VAC, BOOSTER_ISP_SL);
	th_booster[1] = CreateThruster(FlipY(BOOSTER_EXHAUST_POS), FlipY(BOOSTER_EXHAUST_DIR), BOOSTER_THRUST, atlas_propellant, BOOSTER_ISP_VAC, BOOSTER_ISP_SL);
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

	PARTICLESTREAMSPEC iceVent = {
		0, 1.0, 10, 1.0, 10, 5.0, 0.2, 10.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_PLOG, 0.7, 0.9,
	};

	turbineExhaustContrail = AddParticleStream(&turbineExhaust, TURBINE_EXHAUST_POS + _V(0.0, 0.0, -2.0), -TURBINE_EXHAUST_DIR, &turbineContrailLevel);
	boosterExhaustContrail[0] = AddParticleStream(&boosterExhaust, BOOSTER_EXHAUST_POS, -BOOSTER_EXHAUST_DIR, &turbineContrailLevel);
	boosterExhaustContrail[1] = AddParticleStream(&boosterExhaust, FlipY(BOOSTER_EXHAUST_POS), -BOOSTER_EXHAUST_DIR, &turbineContrailLevel);

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
	Atlas = AddMesh(atlasIce, &ATLAS_CORE_OFFSET);
	AddDefaultMeshes();
	AtlasIceStatus = ICE3;
	AtlasBooster = AddMesh(atlasBooster, &ATLAS_BOOSTER_OFFSET);
	AtlasAdapter = AddMesh(atlasAdapter, &ATLAS_ADAPTER_OFFSET);
	if (conceptManouverUnit)
	{
		ConceptRetrogradeThrusters = AddMesh(conceptRetrogradeThrusters, &CONCEPT_RETROGRADE_MESH_OFFSET);
		ConceptCover1 = AddMesh(conceptCover1, &CONCEPT_RETROGRADE_COVER1_OFFSET);
		ConceptCover2 = AddMesh(conceptCover2, &CONCEPT_RETROGRADE_COVER2_OFFSET);
	}
	// capsule defined in LoadState

	DefineVernierAnimations();

	if (capsuleTowerOnly) padAttach = CreateAttachment(true, _V(0.0, 0.0, MERCURY_OFS_CAPSULE.z - MERCURY_LENGTH_CAPSULE / 2.0), _V(0, -1, 0), _V(0, 0, 1), "PAD", false); // for Little Joe ramp, used to set specific launch angle
	else padAttach = CreateAttachment(true, _V(0.0, 0.0, ATLAS_CORE_OFFSET.z - ATLAS_CORE_LENGTH / 2.0), _V(0, 0, 1), _V(0, 0, 1), "PAD", true);
}

void ProjectMercury::clbkPostCreation()
{
	CapsuleGenericPostCreation();

	if (VesselStatus == TOWERSEP)
	{
		DelMesh(Tower);
		towerJettisoned = true;
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
		boosterSeparated = true;
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
		boosterSeparated = true;
		towerJettisoned = true;
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

		if (conceptManouverUnit)
		{
			DelThruster(conceptPosigrade[0]);
			DelThruster(conceptPosigrade[1]);
			DelThruster(conceptRetrograde[0]);
			DelThruster(conceptRetrograde[1]);
			DelThruster(conceptLinear[0]);
			DelThruster(conceptLinear[1]);
			DelThruster(conceptLinear[2]);
			DelThruster(conceptLinear[3]);

			DelMesh(AtlasAdapter);
			conceptManouverUnitAttached = false;
		}

		CreateAbortThrusters();
		boosterSeparated = true;
		coreSeparated = true;
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

		if (conceptManouverUnit)
		{
			DelThruster(conceptPosigrade[0]);
			DelThruster(conceptPosigrade[1]);
			DelThruster(conceptRetrograde[0]);
			DelThruster(conceptRetrograde[1]);
			DelThruster(conceptLinear[0]);
			DelThruster(conceptLinear[1]);
			DelThruster(conceptLinear[2]);
			DelThruster(conceptLinear[3]);

			DelMesh(AtlasAdapter);
			conceptManouverUnitAttached = false;
		}

		CreateAbortThrusters();
		boosterSeparated = true;
		coreSeparated = true;
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
		DelMesh(AtlasBooster);

		if (!(conceptManouverUnit && conceptManouverUnitAttached))
		{
			DelMesh(AtlasAdapter);
			DelMesh(Adaptcover1);
			DelMesh(Adaptcover2);
			DelMesh(Adaptcover3);
			DelMesh(Adaptring1);
			DelMesh(Adaptring2);
			DelMesh(Adaptring3);
		}

		if (conceptManouverUnit && !conceptCoverAttached) // covers have been separated in previous scenario
		{
			DelMesh(ConceptCover1);
			DelMesh(ConceptCover2);
		}

		// CreatePosigradeRockets(); // Probably already fired. No scenario saved in the second they fire.
		CreateRetroRockets();
		//CreateRCS();

		if (conceptManouverUnit && !conceptManouverUnitAttached)
		{
			DelThruster(conceptPosigrade[0]);
			DelThruster(conceptPosigrade[1]);
			DelThruster(conceptRetrograde[0]);
			DelThruster(conceptRetrograde[1]);
			DelThruster(conceptLinear[0]);
			DelThruster(conceptLinear[1]);
			DelThruster(conceptLinear[2]);
			DelThruster(conceptLinear[3]);

			DelMesh(AtlasAdapter);
		}

		DestabiliserStatus = P_OPENING;
		boosterSeparated = true;
		coreSeparated = true;
		towerJettisoned = true;
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

		if (conceptManouverUnit)
		{
			DelThruster(conceptPosigrade[0]);
			DelThruster(conceptPosigrade[1]);
			DelThruster(conceptRetrograde[0]);
			DelThruster(conceptRetrograde[1]);
			DelThruster(conceptLinear[0]);
			DelThruster(conceptLinear[1]);
			DelThruster(conceptLinear[2]);
			DelThruster(conceptLinear[3]);

			DelMesh(AtlasAdapter);
			conceptManouverUnitAttached = false;

			if (!conceptCoverAttached) // covers have been separated in previous scenario
			{
				DelMesh(ConceptCover1);
				DelMesh(ConceptCover2);
			}
		}

		DestabiliserStatus = P_OPENING;
		boosterSeparated = true;
		coreSeparated = true;
		towerJettisoned = true;
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

		if (conceptManouverUnit)
		{
			DelThruster(conceptPosigrade[0]);
			DelThruster(conceptPosigrade[1]);
			DelThruster(conceptRetrograde[0]);
			DelThruster(conceptRetrograde[1]);
			DelThruster(conceptLinear[0]);
			DelThruster(conceptLinear[1]);
			DelThruster(conceptLinear[2]);
			DelThruster(conceptLinear[3]);

			DelMesh(AtlasAdapter);
			conceptManouverUnitAttached = false;

			if (!conceptCoverAttached) // covers have been separated in previous scenario
			{
				DelMesh(ConceptCover1);
				DelMesh(ConceptCover2);
			}
		}

		drogueSeparated = true;
		boosterSeparated = true;
		coreSeparated = true;
		towerJettisoned = true;
	}
}

void ProjectMercury::clbkPreStep(double simt, double simdt, double mjd)
{
	if (simt - launchTime > 2.0 && simdt > timeStepLimit && !rollProgram && autoPilot && (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP))
	{
		AimEulerAngle(eulerPitch, eulerYaw); // SetGlobalOrientation must be in prestep ?
		//SetAngularVel(_V(0, 0, 0));
	}

	// Automatic abort
	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE) && abortConditionsMet)
	{
		LaunchAbort();
	}
	else if ((VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP) && abortConditionsMet)
	{
		CapsuleSeparate();
		abort = true; // Debug! Check this! Does this interfere with anything else?
	}

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
	else if (separateConceptAdapterAction)
	{
		SeparateConceptAdapter();
		separateConceptAdapterAction = false;
	}
	else if (separateConceptCoverAction)
	{
		SeparateConceptCovers();
		separateConceptCoverAction = false;
	}

	SetEmptyMass(EmptyMass()); // calculate drymass of current setting (launch, abort, flight ...)

	switch (VesselStatus)
	{
	case LAUNCH:
		// Allow to be launched from LC14. The trigger is if engine is lit from an attached vessel
		OBJHANDLE attachedParent;
		attachedParent = GetAttachmentStatus(padAttach);
		if (!launchFromLC14 && attachedParent != NULL && oapiGetFocusObject() == attachedParent && GetThrusterGroupLevel(THGROUP_MAIN) != 0.0) // attached, pad in focus, engines on, and has been uninitialised. Launch is initiated from pad
		{
			if (oapiGetTimeAcceleration() > 1.0) oapiSetTimeAcceleration(1.0);
			autoPilot = true;
			launchFromLC14 = true; // don't repeat this process, so that the launchTime stays put
			launchTime = simt + holdDownTime;
			integratedSpeed = 0.0;
		}
		else if (attachedParent != NULL && simt - launchTime > 0.0)
		{
			launchTime = simt; // so that the clock does not increase when standing attached
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
	case TOWERSEP: // used by Big Joe
		OBJHANDLE attachedParent2;
		attachedParent2 = GetAttachmentStatus(padAttach);
		if (!launchFromLC14 && attachedParent2 != NULL && oapiGetFocusObject() == attachedParent2 && GetThrusterGroupLevel(THGROUP_MAIN) != 0.0) // attached, pad in focus, engines on, and has been uninitialised. Launch is initiated from pad
		{
			if (oapiGetTimeAcceleration() > 1.0) oapiSetTimeAcceleration(1.0);
			autoPilot = true;
			launchFromLC14 = true; // don't repeat this process, so that the launchTime stays put
			launchTime = simt + holdDownTime;
			integratedSpeed = 0.0;
		}
		else if (attachedParent2 != NULL && simt - launchTime > 0.0)
		{
			launchTime = simt; // so that the clock does not increase when standing attached
		}

		if (GetThrusterGroupLevel(THGROUP_MAIN) != 0.0)
		{
			turbineContrailLevel = 1.0;
		}
		else
		{
			turbineContrailLevel = 0.0;
		}

		if (!CGshifted && !GroundContact())
		{
			/*SetSize(ATLAS_CORE_LENGTH + MERCURY_LENGTH_CAPSULE);
			VersionDependentTouchdown(TOUCHDOWN_TOWSEP0, TOUCHDOWN_TOWSEP1, TOUCHDOWN_TOWSEP2, TOUCHDOWN_TOWSEP3, 1e7, 1e5, 10);*/
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
			VersionDependentTouchdown(TOUCHDOWN_TOWSEP0, TOUCHDOWN_TOWSEP1, TOUCHDOWN_TOWSEP2, TOUCHDOWN_TOWSEP3, 1e7, 1e5, 10);
			//SetTouchdownPoints(TOUCHDOWN_TOWSEP0, TOUCHDOWN_TOWSEP1, TOUCHDOWN_TOWSEP2);
			CGshifted = true; // Yeah, not bothering with CG here

			PreviousVesselStatus = LAUNCHCORETOWERSEP;
		}
		break;
	case FLIGHT:
		if (!CGshifted && !GroundContact())
		{
			CreateAirfoils();

			SetSize(MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_RETRO);
			//SetTouchdownPoints(tchdwnFlight, tchdwnFlightNum);
			VersionDependentTouchdown(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2, TOUCHDOWN_FLIGHT3, stiffness, damping, 10.0);
			//SetTouchdownPoints(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2);
			//ShiftCG(MERCURY_OFS_CAPSULE);
			if (PreviousVesselStatus == ABORT) // if in capsuleTowerRetroOnly state
			{
				ShiftCG(-_V(0.0, 0.0, (77.8 - 19.3) * 0.0254));  // 19670022650 page 36 (CG of escape setup at 77.8 in, compared to 19.3 in for spacecraft)

				CreateRetroRockets();
				SetPropellantMass(retro_propellant[0], RETRO_MASS_FUEL);
				SetPropellantMass(retro_propellant[1], RETRO_MASS_FUEL);
				SetPropellantMass(retro_propellant[2], RETRO_MASS_FUEL);

			}
			else // only from TOWERSEP or scenario
			{
				ShiftCG(MERCURY_OFS_CAPSULE);
			}
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
			VersionDependentTouchdown(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2, TOUCHDOWN_FLIGHT3, stiffness, damping, 10.0);
			//SetTouchdownPoints(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2);

			if (PreviousVesselStatus == ABORT || PreviousVesselStatus == ABORTNORETRO)
			{
				ShiftCG(-_V(0.0, 0.0, (77.8 - 19.3) * 0.0254));  // 19670022650 page 36 (CG of escape setup at 77.8 in, compared to 19.3 in for spacecraft)
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			}
			else if (PreviousVesselStatus == LAUNCH || PreviousVesselStatus == TOWERSEP || PreviousVesselStatus == LAUNCHCORETOWERSEP) // spawned in REENTRY from scenario, or from a no-retropack launch config (boilerplate)
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

			SetSize(MERCURY_LENGTH_CAPSULE);
			VersionDependentTouchdown(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2, TOUCHDOWN_FLIGHT3, stiffness, damping, 3.0);
			//SetTouchdownPoints(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2);

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
			VersionDependentTouchdown(TOUCHDOWN_ABORT0, TOUCHDOWN_ABORT1, TOUCHDOWN_ABORT2, TOUCHDOWN_ABORT3, 1e7, 1e5, 10);
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

		if ((vesselAcceleration < 0.25 * G && (simt - abortTime) > 2.0 && GetPropellantMass(escape_tank) == 0.0) || (autoPilot && GetAltitude() > 1e5)) // When aborting from inflight and acc<0.25g and empty propellant, or autopilot and above 100 km
		{
			SeparateTower(true);
			CGshifted = false;
			VesselStatus = FLIGHT; // we have retro attached
		}
		break;
	case ABORTNORETRO:
		// Allow to be launched from ramp, reuse LC14 code from LAUNCH and TOWERSEP. The trigger is if engine is lit from an attached vessel
		attachedParent = GetAttachmentStatus(padAttach);

		if (!launchFromLC14 && attachedParent != NULL && oapiGetFocusObject() == attachedParent && GetThrusterGroupLevel(THGROUP_MAIN) != 0.0) // attached, pad in focus, engines on, and has been uninitialised. Launch is initiated from pad
		{
			if (oapiGetTimeAcceleration() > 1.0) oapiSetTimeAcceleration(1.0);

			launchFromLC14 = true; // don't repeat this process, so that the launchTime stays put
			abort = true;
			suborbitalMission = true; // an abort is always ballistic

			launchTime = simt; // appropriate, so that we can see the time from firing 

			SetPropellantMass(escape_tank, ABORT_MASS_FUEL); // just in case
			SetDefaultPropellantResource(escape_tank);
			SetThrusterGroupLevel(THGROUP_MAIN, 1.0);

			abortTime = simt;

		}
		else if (attachedParent != NULL && simt - launchTime > 0.0)
		{
			launchTime = simt; // so that the clock does not increase when standing attached
		}

		if (!CGshifted && !GroundContact())
		{
			CreateAirfoilsEscape();
			SetSize(MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT);
			VersionDependentTouchdown(TOUCHDOWN_ABORT0, TOUCHDOWN_ABORT1, TOUCHDOWN_ABORT2, TOUCHDOWN_ABORT3, 1e7, 1e5, 10);
			//SetTouchdownPoints(TOUCHDOWN_ABORT0, TOUCHDOWN_ABORT1, TOUCHDOWN_ABORT2);
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

		// Equations for tower separate time from http://aresinstitute.org/spacepdf/19730065749_1973065749.pdf page 72
		double abortMet = abortTime - launchTime;
		if (abortMet < 0.0) abortMet = 0.0;
		double towerSepTime;
		if (abortMet < 62.0)
			towerSepTime = 0.1855 * abortMet + 7.0;
		else if (abortMet < 81.62)
			towerSepTime = 1.6139 * abortMet - 81.5630;
		else
			towerSepTime = 50.165;

		if (vesselAcceleration < 0.25 * G && simt - abortTime > towerSepTime && GetPropellantMass(escape_tank) == 0.0) // When aborting from inflight, and acc<0.25g and "maximum altitude sensor runs out" (http://aresinstitute.org/spacepdf/19740076641_1974076641.pdf page 10), and empty propellant (has fired, used in Beach Abort scenarios and similar)
		{
			OffPadAbortSeparate();
		}
		break;
	}

	DeleteRogueVessels();

	if (periscope && PeriscopeStatus != P_DEPLOYED)
	{
		PeriscopeStatus = P_OPENING;
	}
	else if (!periscope && PeriscopeStatus != P_CLOSED)
	{
		PeriscopeStatus = P_CLOSING;
	}
	AnimatePeriscope(simt, simdt);
	AnimateAntennaDestabiliser(simt, simdt);
	if (fireflyBangTime + 0.25 < simt || simt < fireflyBangTime) fireflyLevel = 0.0; // reset fireflies. Second condition if we jump back in time immediately after a bang.

	//OBJHANDLE earthHandle = oapiGetGbodyByName("Earth");
	//if (GetSurfaceRef() == earthHandle)
	//{
	//	VECTOR3 earthPos, sunPos, sunVel;
	//	GetRelativePos(earthHandle, earthPos);
	//	GetRelativePos(oapiGetGbodyByIndex(0), sunPos); // Sun must be index 0.
	//	double sunAngle = acos(dotp(earthPos, sunPos) / length(earthPos) / length(sunPos)) - PI05;
	//	double horizonAngle = acos(oapiGetSize(earthHandle) / length(earthPos));

	//	GetRelativeVel(oapiGetGbodyByIndex(0), sunVel);

	//	double sunVelAngle = acos(dotp(sunPos, sunVel) / length(sunPos) / length(sunVel)); // due to flipped directions of vectors, the angle is 0 if going away, and PI if towards.

	//	if (sunVelAngle > PI05 && sunAngle + horizonAngle > 0.0 && sunAngle + horizonAngle < 5.0 * RAD && oapiGetTimeAcceleration() <= 1.0)
	//	{
	//		fireflyLevel = 1.0;
	//	}
	//}
}

void ProjectMercury::clbkPostStep(double simt, double simdt, double mjd)
{
	if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP)
	{
		if (!GroundContact())
		{
			// Attitude
			VECTOR3 angVel;
			GetAngularVel(angVel);
			integratedPitch += angVel.x * DEG * simdt;
			integratedYaw += angVel.y * DEG * simdt;
			integratedRoll += angVel.z * DEG * simdt;
		}

		// Animate verniers
		double vernPi = GetControlSurfaceLevel(AIRCTRL_ELEVATOR);
		double vernYa = GetControlSurfaceLevel(AIRCTRL_RUDDER);
		double vernRo = GetControlSurfaceLevel(AIRCTRL_AILERON);

		if (vernRo != 0.0)
		{
			SetAnimation(Vernier1AnimY, 0.5 - 0.5 * vernRo);
			SetAnimation(Vernier2AnimY, 0.5 + 0.5 * vernRo);
		}
		else // Not within T+2 s to T+15 s, or possibly when manual
		{
			SetAnimation(Vernier1AnimX, 0.5 + 0.5 * vernYa); // calibrate these
			SetAnimation(Vernier1AnimY, 0.5 - 0.5 * vernPi);
			SetAnimation(Vernier2AnimX, 0.5 + 0.5 * vernYa);
			SetAnimation(Vernier2AnimY, 0.5 - 0.5 * vernPi);
		}

		// Abort due to failure mode
		if (FailureMode == BOOSTERPROBLEM && simt - launchTime > timeOfError)
		{
			abortConditionsMet = true;
			oapiWriteLog("Abort due to random failure!");
		}

		double currentSustainerLevel = GetThrusterLevel(th_main);
		if (currentSustainerLevel == 0.0 && previousSustainerLevel != 0.0)
		{
			// Ran dry, or general cutoff
			char cbuf[256];
			sprintf(cbuf, "Sustainer engine turned off T+%.1f", simt - launchTime); // debug
			oapiWriteLog(cbuf);

			historyCutOffAlt = GetAltitude();
			VECTOR3 currentSpaceVelocity;
			GetRelativeVel(GetSurfaceRef(), currentSpaceVelocity);
			historyCutOffVel = length(currentSpaceVelocity);
			VECTOR3 currentSpaceLocation;
			GetRelativePos(GetSurfaceRef(), currentSpaceLocation);

			historyCutOffAngl = -acos(dotp(currentSpaceLocation, currentSpaceVelocity) / length(currentSpaceLocation) / length(currentSpaceVelocity)) * DEG + 90.0;

			double radiusNoCare;
			GetEquPos(historyCutOffLong, historyCutOffLat, radiusNoCare);
		}
		previousSustainerLevel = currentSustainerLevel;
	}

	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP) && enableAbortConditions && GetDamageModel() != 0)
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

	// De-icing of mesh
	if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP)
	{
		if (Atlas == NULL && GroundContact()) // no idea why I have to do this! It all worked yesterday! >:(
		{
			DelMesh(Atlas);
			Atlas = AddMesh(atlasIce, &ATLAS_CORE_OFFSET);
			if (rocketCam) SetMeshVisibilityMode(Atlas, MESHVIS_ALWAYS);
			else SetMeshVisibilityMode(Atlas, MESHVIS_EXTERNAL);
		}

		if (GetAtmDensity() < 1e-6 && AtlasIceStatus == ICE1)
		{
			DelMesh(Atlas);
			Atlas = AddMesh(atlas, &ATLAS_CORE_OFFSET);
			if (rocketCam) SetMeshVisibilityMode(Atlas, MESHVIS_ALWAYS);
			else SetMeshVisibilityMode(Atlas, MESHVIS_EXTERNAL);
			AtlasIceStatus = ICE0;
		}
		else if (GetAtmDensity() < 1e-4 && AtlasIceStatus == ICE2)
		{
			DelMesh(Atlas);
			Atlas = AddMesh(atlasIce3, &ATLAS_CORE_OFFSET);
			if (rocketCam) SetMeshVisibilityMode(Atlas, MESHVIS_ALWAYS);
			else SetMeshVisibilityMode(Atlas, MESHVIS_EXTERNAL);
			AtlasIceStatus = ICE1;
		}
		else if (GetAtmDensity() < 1e-2 && AtlasIceStatus == ICE3)
		{
			DelMesh(Atlas);
			Atlas = AddMesh(atlasIce2, &ATLAS_CORE_OFFSET);
			if (rocketCam) SetMeshVisibilityMode(Atlas, MESHVIS_ALWAYS);
			else SetMeshVisibilityMode(Atlas, MESHVIS_EXTERNAL);
			AtlasIceStatus = ICE2;
		}
	}

	if (FailureMode == ATTSTUCKOFF && (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) && simt - launchTime > timeOfError)
	{
		DisableAttitudeThruster(attitudeThrusterErrorNum);
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
			if (boosterShutdownTime != 0.0 && simt > boosterShutdownTime + 1.0 && vesselAcceleration < 0.25 * G) // capsule sep at 1 sec after cutoff (19620004691 page 34)
			{
				oapiWriteLog("Automatic capsule separate +1 second after shutdown");
				separateCapsuleAction = true;
			}
		}
	}
	else
	{
		CapsuleAttitudeControl(simt, simdt);
	}

	if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP) && !autoPilot) // not autopilot
	{
		SetADCtrlMode(7); // enable adc

		AtlasEngineDir();
	}

	double longit, latit, radiusDontCare;
	GetEquPos(longit, latit, radiusDontCare);
	double getAlt = GetAltitude();

	MercuryCapsuleGenericTimestep(simt, simdt, latit, longit, getAlt); // Various functions and checks for controlling all kinds of things during entire flight

	if ((VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP || VesselStatus == ABORT || VesselStatus == ABORTNORETRO) && vesselAcceleration > historyMaxLaunchAcc)
		historyMaxLaunchAcc = vesselAcceleration;

	if (historyInclination < latit * DEG)
	{
		historyInclination = latit * DEG;
	}

	if (VesselStatus == FLIGHT && getAlt < historyPerigee && historyBottomPrev < getAlt && historyBottomPrevPrev > historyBottomPrev) // we have perigee, and it's the absolute lowest
	{
		historyPerigee = getAlt;
	}
	historyBottomPrevPrev = historyBottomPrev;
	historyBottomPrev = getAlt;

	if (VesselStatus == FLIGHT)
	{
		if (latit > historyCutOffLat && historyCutOffLat > previousFrameLatitude)
		{
			orbitCount += 1;
		}

		previousFrameLatitude = latit;
	}

	// Separate concept retrograde thruster covers
	if (VesselStatus == FLIGHT && conceptManouverUnit && conceptManouverUnitAttached && conceptRetrograde != NULL)
	{
		if (conceptCoverAttached && GetThrusterLevel(conceptRetrograde[0]) != 0.0)
		{
			// If with tank, firing thruster, and has covers attached
			separateConceptCoverAction = true;
		}

		conceptThrusterLevel[0] = GetThrusterLevel(conceptPosigrade[0]);
		conceptThrusterLevel[1] = GetThrusterLevel(conceptPosigrade[1]);
		conceptThrusterLevel[2] = GetThrusterLevel(conceptRetrograde[0]);
		conceptThrusterLevel[3] = GetThrusterLevel(conceptRetrograde[1]);
	}
	else
	{
		conceptThrusterLevel[0] = 0.0;
		conceptThrusterLevel[1] = 0.0;
		conceptThrusterLevel[2] = 0.0;
		conceptThrusterLevel[3] = 0.0;
	}
}

int ProjectMercury::clbkConsumeBufferedKey(DWORD key, bool down, char* kstate)
{
	bool TargetBaseInput(void* id, char* str, void* data);
	bool TargetBaseInputLaunch(void* id, char* str, void* data);
	bool NumOrbitsInput(void* id, char* str, void* data);

	if (!down) return 0; // only process keydown events

	if (KEYMOD_CONTROL(kstate)) // Ctrl+button
	{
		switch (key)
		{
		case OAPI_KEY_T: // hide on-screen 
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
		case OAPI_KEY_R: // refresh dynamic meshes. A pity that this has to be done
			if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP) // Set Atlas mesh
			{
				DelMesh(Atlas);
				if (GetAtmDensity() < 1e-6 && AtlasIceStatus == ICE1)
				{
					Atlas = AddMesh(atlas, &ATLAS_CORE_OFFSET);
					if (rocketCam) SetMeshVisibilityMode(Atlas, MESHVIS_ALWAYS);
					else SetMeshVisibilityMode(Atlas, MESHVIS_EXTERNAL);
					AtlasIceStatus = ICE0;
				}
				else if (GetAtmDensity() < 1e-4 && AtlasIceStatus == ICE2)
				{
					Atlas = AddMesh(atlasIce3, &ATLAS_CORE_OFFSET);
					if (rocketCam) SetMeshVisibilityMode(Atlas, MESHVIS_ALWAYS);
					else SetMeshVisibilityMode(Atlas, MESHVIS_EXTERNAL);
					AtlasIceStatus = ICE1;
				}
				else if (GetAtmDensity() < 1e-2 && AtlasIceStatus == ICE3)
				{
					Atlas = AddMesh(atlasIce2, &ATLAS_CORE_OFFSET);
					if (rocketCam) SetMeshVisibilityMode(Atlas, MESHVIS_ALWAYS);
					else SetMeshVisibilityMode(Atlas, MESHVIS_EXTERNAL);
					AtlasIceStatus = ICE2;
				}
				else
				{
					Atlas = AddMesh(atlasIce, &ATLAS_CORE_OFFSET);
					if (rocketCam) SetMeshVisibilityMode(Atlas, MESHVIS_ALWAYS);
					else SetMeshVisibilityMode(Atlas, MESHVIS_EXTERNAL);
					AtlasIceStatus = ICE3;
				}
			}
			else if (VesselStatus == REENTRY)
			{
				DelMesh(Drogue);
				Drogue = AddMesh(drogue, &MERCURY_OFS_DROGUE);
				SetMeshVisibilityMode(Drogue, MESHVIS_ALWAYS);
			}
			else if (VesselStatus == REENTRYNODROGUE && !GroundContact())
			{
				DelMesh(Mainchute);
				if (landingBagDeployed) DelMesh(Landingbag);

				Mainchute = AddMesh(mainChute, &MERCURY_OFS_MAINCHUTE);
				SetMeshVisibilityMode(Mainchute, MESHVIS_ALWAYS);
				if (landingBagDeployed) Landingbag = AddMesh(landingbag, &(MERCURY_OFS_LANDBAG - MERCURY_OFS_CAPSULE));
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
			else if (VesselStatus == TOWERSEP && GetThrusterGroupLevel(THGROUP_MAIN) != 0.0) // some engine must be running. This is basically a catch for Big Joe
			{
				separateBoosterAction = true;
				autoPilot = false;
			}
			else if (VesselStatus == LAUNCHCORETOWERSEP || (VesselStatus == TOWERSEP && GetThrusterGroupLevel(THGROUP_MAIN) == 0.0)) // or a catch for Big Joe
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
			else if (VesselStatus == FLIGHT && conceptManouverUnit && conceptManouverUnitAttached)
			{
				// Separate concept adapter
				separateConceptAdapterAction = true;
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
				reserveChuteDeployed = true;
				DeployMainChute();
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
					launchTime = oapiGetSimTime() + holdDownTime;
				}
				else if (GroundContact())
				{
					launchTime = oapiGetSimTime();
				}
			}
			else if (VesselStatus == FLIGHT && conceptManouverUnit && conceptManouverUnitAttached)
			{
				if (GetThrusterGroupLevel(THGROUP_MAIN) == 0.0) SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
				else SetThrusterGroupLevel(THGROUP_MAIN, 0.0);
			}
			else if (VesselStatus == FLIGHT)
			{
				InitiateRetroSequence();
			}
			else if (VesselStatus == ABORT) // abort
			{
				if (oapiGetTimeAcceleration() > 1.0) oapiSetTimeAcceleration(1.0); // something extreme is happening, let's give Orbiter time to cope

				abort = true;
				suborbitalMission = true; // an abort is always ballistic

				if (GroundContact()) launchTime = oapiGetSimTime(); // appropriate, so that we can see the time from firing 

				SetPropellantMass(escape_tank, ABORT_MASS_FUEL); // just in case
				SetDefaultPropellantResource(escape_tank);
				SetThrusterGroupLevel(THGROUP_MAIN, 1.0);

				abortTime = oapiGetSimTime();

				// Separate potentially attached pad
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

			return 1;
		case OAPI_KEY_P:
			if (GroundContact() && (VesselStatus == LAUNCH || VesselStatus == TOWERSEP)) // TOWERSEP is for Big Joe
			{
				if (oapiGetTimeAcceleration() > 1.0) oapiSetTimeAcceleration(1.0);
				autoPilot = true;
				SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
				launchTime = oapiGetSimTime() + holdDownTime;
				integratedSpeed = 0.0;
			}
			else if (VesselStatus == LAUNCHCORETOWERSEP) // allow autopilot from a manual launch
			{
				autoPilot = true;
				AutopilotStatus = AUTOLAUNCH;
			}
			//else if (VesselStatus == FLIGHT || VesselStatus == REENTRY) // switch fuel tank for attitude control
			//{
			//	SwitchPropellantSource();
			//}
			else if (VesselStatus == REENTRYNODROGUE)
			{
				engageFuelDump = true;
			}
			else if (VesselStatus == ABORTNORETRO) // launch beach abort
			{
				if (oapiGetTimeAcceleration() > 1.0) oapiSetTimeAcceleration(1.0); // something extreme is happening, let's give Orbiter time to cope

				abort = true;
				suborbitalMission = true; // an abort is always ballistic

				if (GroundContact()) launchTime = oapiGetSimTime(); // appropriate, so that we can see the time from firing 

				SetPropellantMass(escape_tank, ABORT_MASS_FUEL); // just in case
				SetDefaultPropellantResource(escape_tank);
				SetThrusterGroupLevel(THGROUP_MAIN, 1.0);

				abortTime = oapiGetSimTime();

				// Separate potentially attached pad
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
			/*else if (VesselStatus == FLIGHT) // engage retro attitude
			{
				//PMIcheck = true;
				//// Test of PMI
				//SetAngularVel(_V(0, 0, 0));
				//SetThrusterGroupLevel(THGROUP_ATT_BANKLEFT, 1.0);
				//PMItime = oapiGetSimTime();
				//PMIn++;

				AutopilotStatus = TURNAROUND;
				autoPilot = true;
				attitudeHold14deg = !attitudeHold14deg;
			}
			else if (VesselStatus == REENTRY)
			{
				AutopilotStatus = REENTRYATT;
				autoPilot = true;
			}*/
			else if (VesselStatus == ABORT)
			{
				separateTowerAction = true;
			}

			return 1;
		//case OAPI_KEY_G:
		//	if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE)
		//	{
		//		if (RcsStatus == AUTOLOW)
		//		{
		//			RcsStatus = MANUAL;
		//			SwitchPropellantSource();
		//		}
		//		else if (RcsStatus == MANUAL)
		//		{
		//			RcsStatus = AUTOHIGH;
		//			SwitchPropellantSource();
		//		}
		//		else
		//			RcsStatus = AUTOLOW; // don't switch propellant, as we're from Autohigh->Autolow

		//		SwitchAttitudeMode();

		//		if (conceptManouverUnit) SetAttitudeMode(RCS_ROT);
		//	}
		//	return 1;
		case OAPI_KEY_M: // turn on manual mode
			if (enableAbortConditions)
				oapiWriteLog("Automatic abort turned off by key");
			enableAbortConditions = false;
			DisableAutopilot(true);
			//abortDamping = false;
			engageFuelDump = false;

			if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP)
			{
				SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);
				SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
				SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
			}

			return 1;
		case OAPI_KEY_C: // switch to launch complex
			if (VesselStatus == LAUNCH && GroundContact() && GetAttachmentStatus(padAttach) == NULL)
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
			else if (periscope && oapiCameraInternal())
			{
				narrowField = !narrowField;

				if (narrowField)
				{
					oapiCameraSetAperture(PERISCOPE_NARROW);
				}
				else
				{
					oapiCameraSetAperture(PERISCOPE_WIDE);
				}
			}
			else if (rocketCam && oapiCameraInternal())
			{
				rocketCamMode = (rocketCamMode + 1) % numRocketCamModes;
				SetCameraOffset(ROCKET_CAMERA_OFFSET[rocketCamMode]);
				SetCameraDefaultDirection(ROCKET_CAMERA_DIRECTION[rocketCamMode]);
				oapiCameraSetCockpitDir(0, 0);
			}

			return 1;
		case OAPI_KEY_B: // set base index when in FLIGHT
			if (periscope)
			{
				if (periscopeAltitude < 465.0)
				{
					periscopeAltitude += 5.0; // 50 to 250 nautical miles, which I convert to 95 to 465 km

					SetPeriscopeAltitude(periscopeAltitude);
				}
			}
			else if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP)
			{
				oapiOpenInputBox("Set target base / coordinate / inclination", TargetBaseInputLaunch, 0, 20, (void*)this);
			}
			else
			{
				oapiOpenInputBox("Set target base / coordinate", TargetBaseInput, 0, 20, (void*)this);
			}

			return 1;
		case OAPI_KEY_V:
			if (periscope)
			{
				if (periscopeAltitude > 95.0)
				{
					periscopeAltitude -= 5.0; // 50 to 250 nautical miles, which I convert to 95 to 465 km

					SetPeriscopeAltitude(periscopeAltitude);
				}
			}

			return 1;
		case OAPI_KEY_N: // set number of orbits for launch target
			if (VesselStatus == LAUNCH)
			{
				oapiOpenInputBox("Set number of orbits for mission", NumOrbitsInput, 0, 20, (void*)this);
			}

			return 1;
		}
	}
	return 0;
}
bool TargetBaseInput(void* id, char* str, void* data)
{
	return ((ProjectMercury*)data)->SetTargetBaseIdx(str, false);
}

bool TargetBaseInputLaunch(void* id, char* str, void* data)
{
	return ((ProjectMercury*)data)->SetTargetBaseIdx(str, true);
}

bool NumOrbitsInput(void* id, char* str, void* data)
{
	return ((ProjectMercury*)data)->SetNumberOfOrbits(str);
}

bool ProjectMercury::clbkDrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp)
{
	char cbuf[256];
	int yIndex = 0;
	double simt = oapiGetSimTime();


	// Show indicator for nearest object within x km (like a docking radar)
	if (conceptManouverUnit && conceptManouverUnitAttached && (oapiGetHUDMode() == HUD_DOCKING || panelView)) // borrowed/stolen from igel's amazing First in Space Polyot
	{
		double vesselDistance = 1e4; // 50 km, or any other limit
		double vesselVel;
		VECTOR3 vesselPos, bufferPos;
		// Get nearest object
		for (int i = 0; i < int(oapiGetVesselCount()); i++)
		{
			OBJHANDLE ves = oapiGetVesselByIndex(i);
			GetRelativePos(ves, bufferPos);
			if (length(bufferPos) < vesselDistance && ves != GetHandle() && oapiGetSize(ves) > 1.0)
			{
				vesselPos = bufferPos;
				vesselDistance = length(vesselPos);
				VECTOR3 relVel;
				GetRelativeVel(oapiGetVesselByIndex(i), relVel);
				vesselVel = relVel.z;
				//if (relVel.z < 0.0) vesselVel *= 1.0;
			}
		}

		if (vesselDistance < 1e4)
		{
			double verticalFoV = oapiCameraAperture();
			double horizontalFoV = verticalFoV * double(ScreenWidth) / double(ScreenHeight);

			MATRIX3 m;
			GetRotationMatrix(m);
			vesselPos = tmul(m, vesselPos);

			double z = -vesselPos.z;			// skip if behind
			double y = -vesselPos.y;			// skip if high-low
			double x = -vesselPos.x;			// skip if far on side
			double Yr = atan2(y, -z);
			double Xr = atan2(-x, -z);

			if (z > 0 && abs(Yr) > verticalFoV&& abs(Xr) > horizontalFoV)
			{
				double YY = (-z) * tan(verticalFoV);
				double yy = y / YY; // -1 to 1
				yy += 1; // 0 to 2
				yy /= 2; // 0 to 1
				yy *= int(ScreenHeight);

				double XX = YY * double(ScreenWidth) / double(ScreenHeight);
				double xx = -x / XX; // -1 to 1
				xx += 1; // 0 to 2
				xx /= 2; // 0 to 1
				xx *= int(ScreenWidth);

				int X = (int)xx;
				int Y = (int)yy;

				skp->Rectangle(X - 5, Y - 5, X + 5, Y + 5);
				char label[30];
				sprintf(label, "%.0f m | %.0f m/s", vesselDistance, vesselVel);
				skp->Text(X + 8, Y - 5, label, strlen(label));
			}
		}
	}

	if (oapiCockpitMode() == COCKPIT_PANELS && panelView)
	{
		if (showInfoOnHud < 2)
		{
			// This sets the x so that the text is centered
			int stringWidth = skp->GetTextWidth(contactBase, strlen(contactBase));
			skp->Text((int)floor(ScreenWidth / 2 - stringWidth / 2), TextY0 - 5 * LineSpacing, contactBase, strlen(contactBase));
		}

		return false;
	}

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

		// Button Ctrl+R
		sprintf(cbuf, "Ctrl+R:Reload missing meshes");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;


		// Key J
		if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE)
		{
			sprintf(cbuf, "J:     ABORT");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP)
		{
			sprintf(cbuf, "J:     Separate capsule");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == ABORT)
		{
			sprintf(cbuf, "J:     Separate escape tower");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == FLIGHT)
		{
			sprintf(cbuf, "J:     Separate retropack");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == ABORTNORETRO)
		{
			sprintf(cbuf, "J:     Separate escape tower");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRY && !drogueDeployed)
		{
			sprintf(cbuf, "J:     Deploy drogue chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRY && drogueDeployed)
		{
			sprintf(cbuf, "J:     Separate drogue chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE && !mainChuteDeployed && !mainChuteSeparated)
		{
			sprintf(cbuf, "J:     Deploy main chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE && !landingBagDeployed)
		{
			sprintf(cbuf, "J:     Deploy landing bag");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE && mainChuteDeployed && !mainChuteSeparated)
		{
			sprintf(cbuf, "J:     Separate main chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == REENTRYNODROGUE && mainChuteSeparated && !reserveChuteDeployed)
		{
			sprintf(cbuf, "J:     Deploy reserve chute");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key SPACE
		if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP)
		{
			if (spaceLaunch)
			{
				sprintf(cbuf, "SPACE: Engine off");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "SPACE: Engine on");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
		else if (VesselStatus == FLIGHT)
		{
			sprintf(cbuf, "SPACE: Fire retros");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key P
		if (GroundContact() && VesselStatus == LAUNCH)
		{
			sprintf(cbuf, "P:     Automatic launch");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		//else if (VesselStatus == FLIGHT || VesselStatus == REENTRY) // switch fuel tank for attitude control
		//{
		//	if (attitudeFuelAuto)
		//	{
		//		sprintf(cbuf, "P:     Set propellant source MANUAL");
		//		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		//		yIndex += 1;
		//	}
		//	else
		//	{
		//		sprintf(cbuf, "P:     Set propellant source AUTO");
		//		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		//		yIndex += 1;
		//	}
		//}
		else if (VesselStatus == REENTRYNODROGUE)
		{
			if (!engageFuelDump)
			{
				sprintf(cbuf, "P:     Fuel dump");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}

		// Key K
		if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP))
		{
			sprintf(cbuf, "K:     Separate booster stage");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == LAUNCHCORE)
		{
			sprintf(cbuf, "K:     Separate escape tower");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == FLIGHT || VesselStatus == REENTRY)
		{
			if (attitudeHold14deg) sprintf(cbuf, "K:     Engage automatic attitude (34\u00B0)");
			else sprintf(cbuf, "K:     Engage automatic attitude (14.5\u00B0)");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		//// Key G
		//if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE)
		//{
		//	if (RcsStatus == AUTOLOW)
		//	{
		//		sprintf(cbuf, "G:     Switch to attitude MANUAL");
		//		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		//		yIndex += 1;
		//	}
		//	else if (RcsStatus == MANUAL)
		//	{
		//		sprintf(cbuf, "G:     Switch to attitude AUTOHIGH");
		//		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		//		yIndex += 1;
		//	}
		//	else
		//	{
		//		sprintf(cbuf, "G:     Switch to attitude AUTOLOW");
		//		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		//		yIndex += 1;
		//	}
		//}

		// Key M
		sprintf(cbuf, "M:     Disengage autopilot");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// Key C
		if (VesselStatus == LAUNCH && GroundContact() && GetAttachmentStatus(padAttach) == NULL)
		{
			sprintf(cbuf, "C:     Set focus on launch pad");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (periscope)
		{
			if (narrowField) sprintf(cbuf, "C:     Set periscope WIDE aperture");
			else  sprintf(cbuf, "C:     Set periscope NARROW aperture");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (rocketCam)
		{
			sprintf(cbuf, "C:     Switch rocket camera view");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		
		// Key B
		if (periscope)
		{
			// Do nothing
		}
		else if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORETOWERSEP)
		{
			sprintf(cbuf, "B:     Set launch target");
			skp->Text(TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else
		{
			sprintf(cbuf, "B:     Set landing target");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key V and B
		if (periscope)
		{
			sprintf(cbuf, "B/V:   Set periscope altitude +/-");
			skp->Text(TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key N
		if (VesselStatus == LAUNCH)
		{
			sprintf(cbuf, "N:     Set mission number of orbits");
			skp->Text(TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

	}

	yIndex = -4;

	if (showInfoOnHud < 2)
	{
		// This sets the x so that the text is centered
		int stringWidth = skp->GetTextWidth(contactBase, strlen(contactBase));
		skp->Text((int)floor(ScreenWidth / 2 - stringWidth / 2), TextY0 - 5 * LineSpacing, contactBase, strlen(contactBase));

		if (periscope)
		{
			// This sets the x so that the text is centered
			char PeriscopeAlt[50];
			sprintf(PeriscopeAlt, "Periscope altitude setting: %i km", int(periscopeAltitude));
			stringWidth = skp->GetTextWidth(PeriscopeAlt, strlen(PeriscopeAlt));
			skp->Text((int)floor(ScreenWidth / 2 - stringWidth / 2), TextY0 - 6 * LineSpacing, PeriscopeAlt, strlen(PeriscopeAlt));
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
					bool entryInterface = GetLandingPointIfRetroInXSeconds(time, el, prm, longAtNow, &landingLong, &landingLat);

					if (entryInterface)
					{
						if (noMissionLandLat)
						{
							missionLandLat = landingLat * DEG; // only target the landing longitude
						}

						if (oapiOrthodome(landingLong, landingLat, missionLandLong * RAD, missionLandLat * RAD) < 5.0 * RAD)
						{
							closeSolution = true;
							time -= deltaT; // correct for later addition
						}
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

				if (FailureMode == RETROCALCOFF && simt - launchTime > timeOfError)
				{
					sprintf(cbuf, "  ERROR ERROR ERROR");
					skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
					yIndex += 1;
				}
				else if (minAngDistTime != 0.0)
				{
					double metRetroTime = minAngDistTime - 30.0 + metAbs - 6.6; // -30 to account for retroseq to retroburn. -6.6 is empirical, mostly from retroburn not being instantanious
					if (switchRetroDelay == 1) metRetroTime += 30.0; // instentanious retro, so add the 30 seconds back
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
			for (int i = 0; i < (STORED_RETROSEQUENCE_TIMES - 1); i++)
			{
				if (metAbs > retroTimes[i])
				{
					retroseqTime = retroTimes[i + 1];
					sprintf(retroseqName, "%s", retroNames[i + 1]);
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

		if ((VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) && fuel_manual != NULL) // there's a pesky crash if one checks for propellant level of a non-created propellant source
		{
			sprintf(cbuf, "Manual fuel: %.1f %%", GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_MAN * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Auto fuel: %.1f %%", GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_AUTO * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		/*else if ((VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) && fuel_manual != NULL)
		{
			sprintf(cbuf, "Manual fuel: %.1f %%", GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_MAN * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Auto fuel: %.1f %%", GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_AUTO * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}*/

		// Sensor dials
		// Remove, as it's now displayed on the 2D panel
		//// Longitudinal acc:
		//double m = GetMass();
		//VECTOR3 F, W;
		//GetForceVector(F);
		//GetWeightVector(W);
		//double longAcc = (F - W).z / m;
		//if (longAcc > 21.0 * G) longAcc = 21.0 * G;
		//if (longAcc < -9.0 * G) longAcc = -9.0 * G;
		//sprintf(cbuf, "Longitudinal acceleration: %.1f g", longAcc / G);
		//skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		//yIndex += 1;

		//// Descent rate
		//VECTOR3 groundSpeedVec;
		//GetAirspeedVector(FRAME_HORIZON, groundSpeedVec);
		//double descRate = groundSpeedVec.y;
		//if (descRate > 0.0) descRate = 0.0;
		//if (descRate < -43.0) descRate = -43.0;
		//if (GetAltitude() > 5e4) descRate = 0.0;
		//DWORD previousTextColour; // red
		//previousTextColour = skp->SetTextColor(0x0000FF); // red
		//if (descRate > -9.754) skp->SetTextColor(0x00FF00); // green
		//sprintf(cbuf, "Descent: %.1f m/s", -descRate);
		//skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		//yIndex += 1;
		//skp->SetTextColor(previousTextColour);

		//// Altitude
		//double airAltitude = GetAltitude();
		//if (airAltitude > 30480.0) airAltitude = 30480.0;
		//previousTextColour = skp->SetTextColor(0x00FF00); // green
		//if (airAltitude < 3048.0) skp->SetTextColor(0x0000FF); // red
		//else if (airAltitude < 6401.0) skp->SetTextColor(0x00FFFF); // yellow
		//sprintf(cbuf, "Altitude: %.0f m", floor(airAltitude / 10.0) * 10.0);
		//skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		//yIndex += 1;
		//skp->SetTextColor(previousTextColour);

		// Next event
		if (autoPilot && (VesselStatus == LAUNCH || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP) && boosterShutdownTime == 0.0)
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
		else if (autoPilot && VesselStatus == LAUNCHCORETOWERSEP)
		{
			sprintf(cbuf, "Separate capsule in %.2f s", 1.0 + boosterShutdownTime - simt);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else
		{
			WriteHUDAutoFlightReentry(skp, simt, &yIndex, cbuf);
		}

		if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP || VesselStatus == TOWERSEP)
		{
			VECTOR3 angVel;
			GetAngularVel(angVel);

			double planetRad = oapiGetSize(GetSurfaceRef());
			double targetOrbitalVelocity = sqrt(GGRAV * oapiGetMass(GetSurfaceRef()) * (2.0 / (missionPerigee * 1000.0 + planetRad) - 2.0 / (missionApogee * 1000.0 + planetRad + missionPerigee * 1000.0 + planetRad))); // switch from currentRadius to missionPerigee
			double cutoffAlt = missionPerigee * 1000.0;
			if (missileMission)
			{
				targetOrbitalVelocity = missileCutoffVelocity;
				cutoffAlt = missileCutoffAltitude * 1000.0;
			}
			double targetPitchRate = AtlasPitchControl(cutoffAlt, targetOrbitalVelocity);
			if (VesselStatus == LAUNCHCORETOWERSEP)
				sprintf(cbuf, "Pitch rate: %.2f\u00B0/s (targ.: %.2f\u00B0/s)", angVel.x * DEG, targetPitchRate * DEG);
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

			double longi, lati, radi;
			GetEquPos(longi, lati, radi);
			radi = oapiGetSize(GetSurfaceRef());
			if (launchTargetPosition)
			{
				sprintf(cbuf, "Lat: %.2f\u00B0, long: %.2f\u00B0 (%i)", missionLandLat, missionLandLong, missionOrbitNumber);
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				sprintf(cbuf, "Target heading: %.1f\u00B0", AtlasTargetCutOffAzimuth(simt, radi + missionPerigee * 1e3, longi * DEG, lati * DEG, false));
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "Inc: %.2f\u00B0", targetInclination);
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;

				double targetAzimuth = asin(cos(targetInclination * RAD) / cos(lati)) * DEG;
				if (targetInclination < 0.0)
					targetAzimuth = 180.0 - targetAzimuth;
				sprintf(cbuf, "Target heading: %.1f\u00B0", targetAzimuth);
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
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
			/*if (attitudeFuelAuto)
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
			}*/

			if (switchASCSMode == -1 && switchControlMode == -1)
			{
				sprintf(cbuf, "ASCS auto attitude");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (switchASCSMode == 0 && switchControlMode == -1)
			{
				sprintf(cbuf, "AUX damp");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (switchASCSMode == 1)
			{
				sprintf(cbuf, "Fly By Wire");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (!tHandlePitchPushed)
			{
				sprintf(cbuf, "  Auto pitch thrusters OFF");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			if (!tHandleYawPushed)
			{
				sprintf(cbuf, "  Auto yaw thrusters OFF");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (!tHandleRollPushed)
			{
				sprintf(cbuf, "  Auto roll thrusters OFF");
				skp->Text(secondColumnHUDx* TextX0, yIndex* LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (!tHandleManualPushed)
			{
				sprintf(cbuf, "Manual direct");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (tHandleManualPushed && switchControlMode == 1)
			{
				sprintf(cbuf, "Rate command (RSCS)");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}



			//if (RcsStatus == MANUAL)
			//{
			//	sprintf(cbuf, "Attitude mode MANUAL");
			//	skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			//	yIndex += 1;
			//}
			//else if (RcsStatus == AUTOHIGH)
			//{
			//	sprintf(cbuf, "Attitude mode AUTOHIGH");
			//	skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			//	yIndex += 1;
			//}
			//else
			//{
			//	sprintf(cbuf, "Attitude mode AUTOLOW");
			//	skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			//	yIndex += 1;
			//}

			if (engageFuelDump)
			{
				sprintf(cbuf, "Dumping attitude fuel");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
	}

	yIndex = 0;

	if (showInfoOnHud == 1) // Show Mercury dashboard left row of status lights. This is in the end to not affect right side font colour
	{
		WriteHUDIndicators(skp, simt, &yIndex, cbuf);
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
			sscanf_s(cbuf + 5, "%i", &VesselStatus);
		}
		else if (!_strnicmp(cbuf, "CAPSULE", 7))
		{
			LoadCapsule(cbuf + 8); // don't send space after "CAPSULE "
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
		else if (!_strnicmp(cbuf, "HOLDTIME", 8))
		{
			sscanf_s(cbuf + 8, "%lf", &holdDownTime);
			char oLog[50];
			sprintf(oLog, "Atlas hold-down time set to %.1f seconds.", holdDownTime);
			oapiWriteLog(oLog);
		}
		else if (!_strnicmp(cbuf, "BECOTIME", 8))
		{
			sscanf_s(cbuf + 8, "%lf", &becoTime);
			char oLog[50];
			sprintf(oLog, "BECO time set to T+%.1f seconds.", becoTime);
			oapiWriteLog(oLog);
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
			sscanf_s(cbuf + 6, "%lf", &missionApogee);
			limitApogee = true;
		}
		else if (!_strnicmp(cbuf, "PERIGEE", 7))
		{
			sscanf_s(cbuf + 7, "%lf", &missionPerigee);
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
		else if (!_strnicmp(cbuf, "MISSILESPEED", 12))
		{
			missileMission = true;
			sscanf_s(cbuf + 12, "%lf", &missileCutoffVelocity);
		}
		else if (!_strnicmp(cbuf, "MISSILEANGLE", 12))
		{
			sscanf_s(cbuf + 12, "%lf", &missileCutoffAngle);
		}
		else if (!_strnicmp(cbuf, "MISSILEALTITUDE", 15))
		{
			sscanf_s(cbuf + 15, "%lf", &missileCutoffAltitude);
		}
		else if (!_strnicmp(cbuf, "REPLACETEXTURE", 14))
		{
			scenarioTextureUserEnable = true; // Load textures
			ReadCapsuleTextureReplacement(cbuf + 15); // don't send space char after REPLACETEXTURE
		}
		else if (!_strnicmp(cbuf, "CONCEPTATTACHED", 15))
		{
			conceptManouverUnitAttached = bool(atoi(cbuf + 15)); // 0 is false, anything else true
		}
		else if (!_strnicmp(cbuf, "CONCEPTCOVER", 12))
		{
			conceptCoverAttached = bool(atoi(cbuf + 12)); // 0 is false, anything else true
		}
		else if (!_strnicmp(cbuf, "SWITCHAUTRETJET", 15))
		{
			switchAutoRetroJet = atoi(cbuf + 15);
		}
		else if (!_strnicmp(cbuf, "SWITCHRETRODELA", 15))
		{
			switchRetroDelay = atoi(cbuf + 15);
		}
		else if (!_strnicmp(cbuf, "SWITCHRETROATTI", 15))
		{
			switchRetroAttitude = atoi(cbuf + 15);
		}
		else if (!_strnicmp(cbuf, "SWITCHASCSMODE", 14))
		{
			switchASCSMode = atoi(cbuf + 14);
		}
		else if (!_strnicmp(cbuf, "SWITCHCONTRMODE", 15))
		{
			switchControlMode = atoi(cbuf + 15);
		}
		else if (!_strnicmp(cbuf, "THANDLEMANUAL", 13))
		{
			tHandleManualPushed = bool(atoi(cbuf + 13));
		}
		else if (!_strnicmp(cbuf, "THANDLEPITCH", 12))
		{
			tHandlePitchPushed = bool(atoi(cbuf + 12));
		}
		else if (!_strnicmp(cbuf, "THANDLEYAW", 10))
		{
			tHandleYawPushed = bool(atoi(cbuf + 10));
		}
		else if (!_strnicmp(cbuf, "THANDLEROLL", 11))
		{
			tHandleRollPushed = bool(atoi(cbuf + 11));
		}
		else ParseScenarioLineEx(cbuf, status);
	}
}

inline bool ProjectMercury::ReadRocketTextureReplacement(const char *flag, const char* cbuf, int len)
{
	// Atlas main
	if (strcmp(flag, "ATLAS3") == 0)
	{
		if (ReadTextureString(cbuf, len, texAtlas3.file, &texAtlas3.w, &texAtlas3.h))
			oapiWriteLogV("New Atlas3 texture: >%s<, w: %i, h: %i", texAtlas3.file, texAtlas3.w, texAtlas3.h);
		else oapiWriteLog("Failed to load Atlas3 texture");
	}
	else if (strcmp(flag, "ATLAS4") == 0)
	{
		if (ReadTextureString(cbuf, len, texAtlas4.file, &texAtlas4.w, &texAtlas4.h))
			oapiWriteLogV("New Atlas4 texture: >%s<, w: %i, h: %i", texAtlas4.file, texAtlas4.w, texAtlas4.h);
		else oapiWriteLog("Failed to load Atlas4 texture");
	}
	else if (strcmp(flag, "ATLAS5") == 0)
	{
		if (ReadTextureString(cbuf, len, texAtlas5.file, &texAtlas5.w, &texAtlas5.h))
			oapiWriteLogV("New Atlas5 texture: >%s<, w: %i, h: %i", texAtlas5.file, texAtlas5.w, texAtlas5.h);
		else oapiWriteLog("Failed to load Atlas5 texture");
	} // Atlas main ice
	else if (strcmp(flag, "ATLAS3ICE") == 0)
	{
		if (ReadTextureString(cbuf, len, texAtlas3ice.file, &texAtlas3ice.w, &texAtlas3ice.h))
			oapiWriteLogV("New Atlas3ice texture: >%s<, w: %i, h: %i", texAtlas3ice.file, texAtlas3ice.w, texAtlas3ice.h);
		else oapiWriteLog("Failed to load Atlas3ice texture");
	} // Atlas main ice2
	else if (strcmp(flag, "ATLAS3ICE2") == 0)
	{
		if (ReadTextureString(cbuf, len, texAtlas3ice2.file, &texAtlas3ice2.w, &texAtlas3ice2.h))
			oapiWriteLogV("New Atlas3ice2 texture: >%s<, w: %i, h: %i", texAtlas3ice2.file, texAtlas3ice2.w, texAtlas3ice2.h);
		else oapiWriteLog("Failed to load Atlas3ice2 texture");
	} // Atlas main ice3
	else if (strcmp(flag, "ATLAS3ICE3") == 0)
	{
		if (ReadTextureString(cbuf, len, texAtlas3ice3.file, &texAtlas3ice3.w, &texAtlas3ice3.h))
			oapiWriteLogV("New Atlas3ice3 texture: >%s<, w: %i, h: %i", texAtlas3ice3.file, texAtlas3ice3.w, texAtlas3ice3.h);
		else oapiWriteLog("Failed to load Atlas3ice3 texture");
	}// Adapter
	else if (strcmp(flag, "ADAPTERRIDGE") == 0)
	{
		if (ReadTextureString(cbuf, len, texAdapterridge.file, &texAdapterridge.w, &texAdapterridge.h))
			oapiWriteLogV("New AdapterRidge texture: >%s<, w: %i, h: %i", texAdapterridge.file, texAdapterridge.w, texAdapterridge.h);
		else oapiWriteLog("Failed to load AdapterRidge texture");
	}
	else if (strcmp(flag, "METALADAPT") == 0)
	{
		if (ReadTextureString(cbuf, len, texMetaladapt.file, &texMetaladapt.w, &texMetaladapt.h))
			oapiWriteLogV("New MetalAdapt texture: >%s<, w: %i, h: %i", texMetaladapt.file, texMetaladapt.w, texMetaladapt.h);
		else oapiWriteLog("Failed to load MetalAdapt texture");
	} // Booster
	else if (strcmp(flag, "ATLAS5BOOST") == 0)
	{
		if (ReadTextureString(cbuf, len, texAtlas5boost.file, &texAtlas5boost.w, &texAtlas5boost.h))
			oapiWriteLogV("New Atlas5Booster texture: >%s<, w: %i, h: %i", texAtlas5boost.file, texAtlas5boost.w, texAtlas5boost.h);
		else oapiWriteLog("Failed to load Atlas5Booster texture");
	}
	else if (strcmp(flag, "ATLAS3BOOST") == 0)
	{
		if (ReadTextureString(cbuf, len, texAtlas3boost.file, &texAtlas3boost.w, &texAtlas3boost.h))
			oapiWriteLogV("New Atlas3Booster texture: >%s<, w: %i, h: %i", texAtlas3boost.file, texAtlas3boost.w, texAtlas3boost.h);
		else oapiWriteLog("Failed to load Atlas3Booster texture");
	}
	else return false;

	return true; // did not read else, and therefore success
}

void ProjectMercury::LoadRocketTextureReplacement(void)
{
	int grpAtlas3 = 1, grpAtlas4 = 2, grpAtlas5 = 3, grpAtlas3ice = 1, grpAtlas3ice2 = 1, grpAtlas3ice3 = 1, grpAdapterridge = 1, grpMetaladapt = 2, grpAtlas5boost = 1, grpAtlas3boost = 2;

	if (strlen(texAtlas3.file) != 0 && grpAtlas3 != NULL)
	{
		LoadTextureFile(texAtlas3, "Atlas3", atlas, grpAtlas3);
	}
	if (strlen(texAtlas4.file) != 0 && grpAtlas4 != NULL)
	{
		LoadTextureFile(texAtlas4, "Atlas4", atlas, grpAtlas4);
		LoadTextureFile(texAtlas4, "Atlas4Ice", atlasIce, grpAtlas4);
		LoadTextureFile(texAtlas4, "Atlas4Ice2", atlasIce2, grpAtlas4);
		LoadTextureFile(texAtlas4, "Atlas4Ice3", atlasIce3, grpAtlas4);
	}
	if (strlen(texAtlas5.file) != 0 && grpAtlas5 != NULL)
	{
		LoadTextureFile(texAtlas5, "Atlas5", atlas, grpAtlas5);
		LoadTextureFile(texAtlas5, "Atlas5Ice", atlasIce, grpAtlas5);
		LoadTextureFile(texAtlas5, "Atlas5Ice2", atlasIce2, grpAtlas5);
		LoadTextureFile(texAtlas5, "Atlas5Ice3", atlasIce3, grpAtlas5);
	}
	if (strlen(texAtlas3ice.file) != 0 && grpAtlas3ice != NULL)
	{
		LoadTextureFile(texAtlas3ice, "Atlas3ice", atlasIce, grpAtlas3ice);
	}
	if (strlen(texAtlas3ice2.file) != 0 && grpAtlas3ice2 != NULL)
	{
		LoadTextureFile(texAtlas3ice2, "Atlas3ice2", atlasIce2, grpAtlas3ice2);
	}
	if (strlen(texAtlas3ice3.file) != 0 && grpAtlas3ice3 != NULL)
	{
		LoadTextureFile(texAtlas3ice3, "Atlas3ice3", atlasIce3, grpAtlas3ice3);
	} // Adapter
	if (strlen(texAdapterridge.file) != 0 && grpAdapterridge != NULL)
	{
		LoadTextureFile(texAdapterridge, "Adapterridge", atlasAdapter, grpAdapterridge);
	}
	if (strlen(texMetaladapt.file) != 0 && grpMetaladapt != NULL)
	{
		LoadTextureFile(texMetaladapt, "Metaladapt", atlasAdapter, grpMetaladapt);
	}
	if (strlen(texAtlas5boost.file) != 0 && grpAtlas5boost != NULL)
	{
		LoadTextureFile(texAtlas5boost, "Atlas5boost", atlasBooster, grpAtlas5boost);
	}
	if (strlen(texAtlas3boost.file) != 0 && grpAtlas3boost != NULL)
	{
		LoadTextureFile(texAtlas3boost, "Atlas3boost", atlasBooster, grpAtlas3boost);
	}
}

void ProjectMercury::clbkSaveState(FILEHANDLE scn)
{
	VESSELVER::clbkSaveState(scn); // write default parameters (orbital elements etc.)

	oapiWriteScenario_int(scn, "STATE", VesselStatus);

	if (configTextureUserEnable) oapiWriteScenario_string(scn, "CAPSULE", configTextureUserName[configTextureUserNum]);
	else oapiWriteScenario_int(scn, "CAPSULE", CapsuleVersion);

	oapiWriteScenario_int(scn, "AUTOMODE", AutopilotStatus);

	oapiWriteScenario_int(scn, "SUBORBITAL", suborbitalMission);

	oapiWriteScenario_int(scn, "ABORTBUS", enableAbortConditions);

	if (launchTime != 0.0) // if we have launched using autopilot
	{
		oapiWriteScenario_int(scn, "MET", int(oapiGetSimTime() - launchTime));
	}

	if (VesselStatus == FLIGHT)
	{
		oapiWriteScenario_int(scn, "ORBITNR", orbitCount);
	}

	oapiWriteScenario_int(scn, "FAILURE", FailureMode);

	if (difficultyHard)
	{
		oapiWriteScenario_string(scn, "HARDMODE", "");
	}

	oapiWriteScenario_int(scn, "SWITCHAUTRETJET", switchAutoRetroJet);
	oapiWriteScenario_int(scn, "SWITCHRETRODELA", switchRetroDelay);
	oapiWriteScenario_int(scn, "SWITCHRETROATTI", switchRetroAttitude);
	oapiWriteScenario_int(scn, "SWITCHASCSMODE", switchASCSMode);
	oapiWriteScenario_int(scn, "SWITCHCONTRMODE", switchControlMode);

	oapiWriteScenario_int(scn, "THANDLEMANUAL", (int)tHandleManualPushed);
	oapiWriteScenario_int(scn, "THANDLEPITCH", (int)tHandlePitchPushed);
	oapiWriteScenario_int(scn, "THANDLEYAW", (int)tHandleYawPushed);
	oapiWriteScenario_int(scn, "THANDLEROLL", (int)tHandleRollPushed);

	int i = 0;
	while (scenarioTextureUserEnable && i < numTextures)
	{
		oapiWriteScenario_string(scn, "REPLACETEXTURE", textureString[i]);
		i++;
	}

	if (conceptManouverUnit) oapiWriteScenario_int(scn, "CONCEPTATTACHED", conceptManouverUnitAttached);
	if (conceptManouverUnit && !conceptCoverAttached) oapiWriteScenario_int(scn, "CONCEPTCOVER", conceptCoverAttached);
}

bool ProjectMercury::SetTargetBaseIdx(char* rstr, bool launch)
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
		launchTargetPosition = true;
		manualInputRetroTime = NULL;
		return true;
	}
	else // typed in a coordinate
	{
		// Look for lat and long, separated by space
		char* strPos;
		strPos = strchr(rstr, ' ');
		oapiWriteLogV("No base with that name. StrPos: %d", strPos - rstr + 1);

		if (strPos != NULL) // found two or more values
		{
			// Search for new space. If so, we're writing in a specific HH MM SS timestamp for retroclock
			char* newStrPos;
			newStrPos = strchr(strPos + 1, ' ');
			if (newStrPos != NULL) // found another space, and thus time stamp
			{
				oapiWriteLogV("You wrote time stamp.");
				char hText[5];
				char mText[5];
				char sText[5];
				strncpy(hText, rstr, strPos - rstr + 1);
				oapiWriteLogV("Part one: %s", hText);
				int timeHour = atoi(hText);
				oapiWriteLogV("Hour: %02i", timeHour);
				strncpy(mText, rstr + int(strPos - rstr + 1), int(newStrPos - rstr + 1));
				oapiWriteLogV("Part two: %s", mText);
				int timeMinute = atoi(mText);
				oapiWriteLogV("Minute: %02i", timeMinute);
				strncpy(sText, rstr + int(newStrPos - rstr + 1), strlen(rstr) - int(newStrPos - rstr + 1));
				oapiWriteLogV("Part three: %s", sText);
				int timeSecond = atoi(sText);
				oapiWriteLogV("Second: %02i", timeSecond);
				landingComputing = false;
				manualInputRetroTime = timeHour * 3600 + timeMinute * 60 + timeSecond;
				return true;
			}
			else // typed in only two numbers, and thus lat long coordinates
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
				launchTargetPosition = true;
				manualInputRetroTime = NULL;
				return true;
			}
		}
		else// only longitude is input (no space detected)
		{
			char* strPosU, * strPosL;
			strPosU = strchr(rstr, 'N');
			strPosL = strchr(rstr, 'n');

			if (strPosU != NULL || strPosL != NULL) // disable landing computer. Saves framerate etc.
			{
				landingComputing = false;
				manualInputRetroTime = NULL;
				oapiWriteLog("Disabled landing computing");
				return true;
			}
			else
			{
				if (launch)
				{
					targetInclination = atof(rstr);
					launchTargetPosition = false;
					oapiWriteLogV("You only wrote inc: %.2f", targetInclination);
					return true;
				}
				else
				{
					missionLandLong = atof(rstr); // atof returns 0.0 if no valid number, which is a good backup solution for erranous input
					noMissionLandLat = true;
					oapiWriteLogV("You only wrote long: %.2f", missionLandLong);
					landingComputing = true;
					launchTargetPosition = true;
					manualInputRetroTime = NULL;
					return true;
				}
			}
			
		}
	}

	return false;
}

bool ProjectMercury::SetNumberOfOrbits(char* rstr)
{
	int num;

	sscanf(rstr, "%i", &num);

	if (num == NULL)
	{
		missionOrbitNumber = 1;
		return true;
	}
	else if (num >= 0)
	{
		missionOrbitNumber = num;
		return true;
	}
	return false;
}


// ==============================================================
// Custom Vessel Functions
// ==============================================================

void ProjectMercury::AtlasAutopilot(double simt, double simdt)
{
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
	eulerPitch = 0.0;
	eulerYaw = 0.0;
	VECTOR3 currentAngAcc;
	GetAngularAcc(currentAngAcc);

	for (int i = 0; i < 12; i++)
	{
		if (met > MET[i])
		{
			pitchRate = aimPitchover[i] * (-RAD);
		}
	}

	if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP) // abort check, but TOWERSEP to catch Big Joe, as this uses the pitchAim
	{
		currentPitchAim = PitchProgramAim(met);
	}

	double BECO = becoTime; // T+130.1 from 19620004691 page 34. T+128.6 from 19630002114 page 21. An earlier BECO time results in longer burntime.

	// Different actions during launch
	if (met > BECO + 26.0 && !GroundContact()) // time from 19930074071 page 54
	{
		controlAttitude = true;
		pitch = GetPitch() * DEG; // integrated pitch becomes off by up to five degrees
	}

	if (met > BECO + 4.0 && VesselStatus == LAUNCH && !GroundContact()) // time from 19930074071 page 54. Big Joe failed to separate booster stage
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
	double cutoffAlt = missionPerigee * 1000.0;
	if (missileMission)
	{
		targetOrbitalVelocity = missileCutoffVelocity;
		cutoffAlt = missileCutoffAltitude * 1000.0;
	}

	double omegaP = AtlasPitchControl(cutoffAlt, targetOrbitalVelocity);
	// This solution is excellent for having cutoff at set height, but not so good for achieving zero flight angle. Therefore use the solution down to a few seconds before cutoff, where we turn to the basic zero y-vel solution

	if (controlAttitude) // final pitch and yaw program to enter target orbit
	{
		double pitchDiff;

		double verticalSpeed = velocity.y;
		if (missileMission)
		{
			verticalSpeed = velocity.y - targetOrbitalVelocity * sin(missileCutoffAngle * RAD);
		}

		VECTOR3 currentVelocity, currentPosition;
		GetRelativePos(GetSurfaceRef(), currentPosition);
		GetRelativeVel(GetSurfaceRef(), currentVelocity);
		double currentRadius = length(currentPosition);
		double currentSpeed = length(currentVelocity);
		double deltaV = targetOrbitalVelocity - currentSpeed;
		double timeToCutoff = GetMass() * CORE_ISP_VAC / CORE_THRUST * (1.0 - exp(-deltaV / CORE_ISP_VAC));

		if (timeToCutoff < 15.0) // if close to orbit insertion
		{
			if (abs(verticalSpeed) < deltaV) // if within valid range of asin (i.e. vertical speed deviation is smaller than remaining orbit insertion speed)
				bottomPitch = -asin(verticalSpeed / deltaV) * DEG;
			else if (verticalSpeed < 0.0)
				bottomPitch = 90.0;
			else
				bottomPitch = -90.0;

			pitchDiff = bottomPitch - pitch;

			if (pitchDiff > 0.1) // we're below aim, pitch up
				pitchRate = 0.5 * RAD; // guesstimate
			else if (pitchDiff < -0.1) // we're above aim, pitch down
				pitchRate = -0.5 * RAD;
			else
				pitchRate = 0.0;
		}
		else // normal PEG algorithm: use the PEG pitch rate
		{
			pitchRate = omegaP;
		}

		if (abs(pitchRate) > 2.0 * RAD) // if target rate above 2 deg/s, then truncate to 2 deg/s (pitch rate above 3.0 deg/s results in abort)
		{
			pitchRate = pitchRate / abs(pitchRate) * 2.0 * RAD;
		}

		if (simdt > timeStepLimit) // force attitude, as we are time accelerating, and rocket computer can't keep up  
		{
			if (eulerPitch == 0.0) eulerPitch = pitchRate * simdt;
		}
		else // not time acc, so use organic rocket mechanics
		{

			const double proportionalGainConstant = 3.0; // Kp
			const double integralGainConstant = 0.0; // Ki
			const double derivativeGainConstant = 0.0; // Kd
			double PIDerror = pitchRate - currentAngRate.x; // setpoint - measured_value
			PIDintegral += PIDerror * simdt; // integral + error * dt
			double PIDderivative = (PIDerror - PIDpreviousError) / simdt;
			double PIDoutput = proportionalGainConstant * PIDerror + integralGainConstant * PIDintegral + derivativeGainConstant * PIDderivative;
			PIDpreviousError = PIDerror;

			SetControlSurfaceLevel(AIRCTRL_ELEVATOR, PIDoutput);

			//if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x > pitchRate + 0.0005)
			//{
			//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, -(currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
			//}
			//else if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x < pitchRate - 0.0005) // hello Kuddel! Thanks a lot! https://www.orbiter-forum.com/showthread.php?p=606147&postcount=100
			//{
			//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, (currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
			//}
			//else
			//{
			//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
			//}
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
			targetInclination = acos(sin(targetAzimuth * RAD) * cos(lat1 * RAD)) * DEG;
		}
		else
		{
			targetAzimuth = asin(cos(targetInclination * RAD) / cos(lat1 * RAD)) * DEG;
			if (targetInclination < 0.0)
				targetAzimuth = 180.0 - targetAzimuth;
		}

		double currentAzimuth;
		oapiGetHeading(GetHandle(), &currentAzimuth);
		currentAzimuth -= OrbitalFrameSlipAngle2(currentPosition, currentVelocity);
		currentAzimuth *= DEG;

		// nynyny, just like bottomPitch
		double horizontalSpeed = 2.0 * currentSpeed * sin((currentAzimuth * RAD - targetAzimuth * RAD) / 2.0);
		double bottomYaw;
		if (abs(horizontalSpeed) < deltaV)
			bottomYaw = asin(horizontalSpeed / deltaV) * DEG;
		else if (verticalSpeed < 0.0)
			bottomYaw = -90.0;
		else
			bottomYaw = 90.0;

		double yawDiff = bottomYaw + GetSlipAngle() * DEG; // slip angle is flipped
		double yawRate;

		if (yawDiff > 0.1)
			yawRate = 0.5 * RAD; // guesstimate
		else if (yawDiff < -0.1)
			yawRate = -0.5 * RAD;
		else
			yawRate = 0.0;

		if (simdt > timeStepLimit) // force attitude
		{
			if (eulerYaw == 0.0) eulerYaw = yawRate * simdt;
		}
		else // not time acc, so use organic rocket mechanics
		{

			const double proportionalGainConstant = 3.0; // Kp
			const double integralGainConstant = 0.0; // Ki
			const double derivativeGainConstant = 0.0; // Kd
			double PIDerror = yawRate - currentAngRate.y; // setpoint - measured_value
			PIDintegral += PIDerror * simdt; // integral + error * dt
			double PIDderivative = (PIDerror - PIDpreviousError) / simdt;
			double PIDoutput = proportionalGainConstant * PIDerror + integralGainConstant * PIDintegral + derivativeGainConstant * PIDderivative;
			PIDpreviousError = PIDerror;

			SetControlSurfaceLevel(AIRCTRL_RUDDER, -PIDoutput);

			//if (currentAngRate.y > yawRate + 0.0005)
			//{
			//	SetControlSurfaceLevel(AIRCTRL_RUDDER, (currentAngRate.y * DEG * currentAngRate.y * DEG * ampFactor + ampAdder));
			//}
			//else if (currentAngRate.y < yawRate - 0.0005)
			//{
			//	SetControlSurfaceLevel(AIRCTRL_RUDDER, -(currentAngRate.y * DEG * currentAngRate.y * DEG * ampFactor + ampAdder));
			//}
			//else
			//{
			//	SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
			//}
		}

		//currentYawAim = -targetSlipAngle; // the reading is flipped, nut sure why
	}
	else if (met > MET[1] && pitchProgram) // after T+15 s and finalised roll
	{
		if (rollProgram)
		{
			currentRollAim = integratedRoll;
			oapiWriteLogV("Roll program ended at T+%.1f. Rolled %.1f degrees.", met, currentRollAim);
		}
		rollProgram = false;
		SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);

		if (simdt > timeStepLimit) // force attitude
		{
			if (eulerPitch == 0.0) eulerPitch = currentPitchAim * RAD - GetPitch();
		}
		else // not time acc, so use organic rocket mechanics
		{

			const double proportionalGainConstant = 3.0; // Kp
			const double integralGainConstant = 1.0; // Ki
			const double derivativeGainConstant = 0.0; // Kd
			double PIDerror = pitchRate - currentAngRate.x; // setpoint - measured_value
			PIDintegral += PIDerror * simdt; // integral + error * dt
			double PIDderivative = (PIDerror - PIDpreviousError) / simdt;
			double PIDoutput = proportionalGainConstant * PIDerror + integralGainConstant * PIDintegral + derivativeGainConstant * PIDderivative;
			PIDpreviousError = PIDerror;

			//if (PIDdebug == NULL) PIDdebug = oapiOpenFile("PIDdebug.txt", FILE_OUT);

			//char PIDline[200];
			//sprintf(PIDline, "%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f", met, pitchRate * DEG, currentAngRate.x * DEG, PIDoutput, currentPitchAim, GetPitch() * DEG);
			//if (!GroundContact()) oapiWriteLine(PIDdebug, PIDline);

			SetControlSurfaceLevel(AIRCTRL_ELEVATOR, PIDoutput);

			//if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x > pitchRate + 0.0005)
			//{
			//	//thrusterAngle = 0.00075;
			//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, -(currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
			//}
			//else if (abs(currentAngAcc.x) < 0.75 * RAD && currentAngRate.x < pitchRate - 0.0005)
			//{
			//	//thrusterAngle = -0.00075;
			//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, (currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
			//}
			//else
			//{
			//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
			//}
		}
	}
	else if (met > 2.0) // roll program, time from 19930074071 page 54
	{
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);

		double initialHeading = historyLaunchHeading;
		double targetHeading = 72.55;

		if (launchTargetPosition)
		{
			targetHeading = AtlasTargetCutOffAzimuth(simt, missionPerigee * 1000.0 + planetRad, historyLaunchLong * DEG, historyLaunchLat * DEG, false);
		}
		else
		{
			targetHeading = asin(cos(targetInclination * RAD) / cos(historyLaunchLat)) * DEG;
			if (targetInclination < 0.0)
				targetHeading = 180.0 - targetHeading;
		}

		// Include Earth's rotation
		double orbitVel = sqrt(planetMu / (planetRad + missionPerigee * 1000.0));
		targetHeading = atan2((orbitVel * sin(targetHeading * RAD) - PI2 * planetRad / oapiGetPlanetPeriod(GetSurfaceRef()) * cos(historyLaunchLat)), (orbitVel * cos(targetHeading * RAD))) * DEG;

		double targetRoll = initialHeading - targetHeading;
		currentRollAim = targetRoll;

		if (!rollProgram)
		{
			PIDintegral = 0.0; // integral + error * dt
			PIDpreviousError = 0.0;
			rollProgram = true; // is this doing anything?
		}

		double rollRate;

		double rollDiff = targetRoll - integratedRoll;
		if (rollDiff > 180.0) rollDiff -= 360.0;
		else if (rollDiff < -180.0) rollDiff += 360.0;
		double highLimit = 10.0;
		double medLimit = 2.5;
		double lowLimit = 0.1;
		double lowerLimit = 0.01;
		double highRate = 4.0 * RAD;
		double medRate = 4.0 * RAD;
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

		// PID control
		const double proportionalGainConstant = 3.0; // Kp
		const double integralGainConstant = 0.0; // Ki
		const double derivativeGainConstant = 0.0; // Kd
		double PIDerror = rollRate - currentAngRate.z; // setpoint − measured_value
		PIDintegral += PIDerror * simdt; // integral + error * dt
		double PIDderivative = (PIDerror - PIDpreviousError) / simdt;
		double PIDoutput = proportionalGainConstant * PIDerror + integralGainConstant * PIDintegral + derivativeGainConstant * PIDderivative;
		PIDpreviousError = PIDerror;

		SetControlSurfaceLevel(AIRCTRL_AILERON, PIDoutput);

		/*if (abs(currentAngAcc.z) < 2.0 * RAD && currentAngRate.z > rollRate + 0.0005)
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
		}*/

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
	else
	{
		// First two seconds

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

		const double proportionalGainConstant = 3.0; // Kp
		const double integralGainConstant = 0.0; // Ki
		const double derivativeGainConstant = 0.0; // Kd
		double PIDerror = pitchRate - currentAngRate.x; // setpoint - measured_value
		PIDintegral += PIDerror * simdt; // integral + error * dt
		double PIDderivative = (PIDerror - PIDpreviousError) / simdt;
		double PIDoutput = proportionalGainConstant * PIDerror + integralGainConstant * PIDintegral + derivativeGainConstant * PIDderivative;
		PIDpreviousError = PIDerror;

		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, PIDoutput);

		//if (currentAngRate.x > pitchRate + 0.0005)
		//{
		//	//thrusterAngle = 0.00075;
		//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, -(currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
		//}
		//else if (currentAngRate.x < pitchRate - 0.0005)
		//{
		//	//thrusterAngle = -0.00075;
		//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, (currentAngRate.x * DEG * currentAngRate.x * DEG * ampFactor + ampAdder));
		//}
		//else
		//{
		//	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
		//}
	}

	// Automatically null yaw
	if (!controlAttitude)
	{
		double yawDiff = 0.0 - yaw;
		if (simdt > timeStepLimit) yawDiff = -GetSlipAngle() * DEG;
		double yawRate;
		if (yawDiff > 0.1)
			yawRate = 0.67 * RAD;
		else if (yawDiff < -0.1)
			yawRate = -0.67 * RAD;
		else
			yawRate = 0.0;

		if (simdt > timeStepLimit && !rollProgram) // force attitude
		{
			if (eulerYaw == 0.0) eulerYaw = -yawDiff * RAD;
		}
		else // not time acc, so use organic rocket mechanics
		{

			const double proportionalGainConstant = 3.0; // Kp
			const double integralGainConstant = 0.0; // Ki
			const double derivativeGainConstant = 0.0; // Kd
			double PIDerror = yawRate - currentAngRate.y; // setpoint - measured_value
			PIDintegral += PIDerror * simdt; // integral + error * dt
			double PIDderivative = (PIDerror - PIDpreviousError) / simdt;
			double PIDoutput = proportionalGainConstant * PIDerror + integralGainConstant * PIDintegral + derivativeGainConstant * PIDderivative;
			PIDpreviousError = PIDerror;

			SetControlSurfaceLevel(AIRCTRL_RUDDER, -PIDoutput);

			//if (currentAngRate.y > yawRate + 0.0005)
			//{
			//	//thrusterAngle = 0.00075;
			//	SetControlSurfaceLevel(AIRCTRL_RUDDER, (currentAngRate.y * DEG * currentAngRate.y * DEG * ampFactor + ampAdder));
			//}
			//else if (currentAngRate.y < yawRate - 0.0005)
			//{
			//	//thrusterAngle = -0.00075;
			//	SetControlSurfaceLevel(AIRCTRL_RUDDER, -(currentAngRate.y * DEG * currentAngRate.y * DEG * ampFactor + ampAdder));
			//}
			//else
			//{
			//	SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
			//}
		}
	}

	if (met > BECO && met < BECO + 5.0 && (VesselStatus == LAUNCH || VesselStatus == TOWERSEP)) // in time between BECO and booster sep + 1 sec
	{
		SetThrusterLevel(th_booster[0], 0.0);
		SetThrusterLevel(th_booster[1], 0.0);

		// Ensure good staging by stopping engine gimbal
		SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
		SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);
	}

	if (simdt > timeStepLimit && !rollProgram)
	{
		SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
		SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
		SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0);
	}

	AtlasEngineDir();

	VECTOR3 Force;
	GetForceVector(Force);
	integratedSpeed += simdt * length(Force) / GetMass();

	if (targetOrbitalVelocity + speedError < currentOrbitalVelocity + 3.4) // posigrades give additional 3.4 m/s dV
	{
		char cbuf[256];
		SetThrusterGroupLevel(THGROUP_MAIN, 0.0);
		AutopilotStatus = POSIGRADEDAMP;
		boosterShutdownTime = simt;
		sprintf(cbuf, "SECO T+%.1f", met);
		oapiWriteLog(cbuf);
	}

	//// Debug
	//char pitchLog[256];
	//sprintf(pitchLog, "%.3f\t%.3f", simt - launchTime, GetPitch() * DEG);
	//oapiWriteLine(pitchDataLogFile, pitchLog);
}

double ProjectMercury::PitchProgramAim(double met)
{
	// Give a continous pitch aim, both for abort check and autopilot if time acceleration

	// Solution is for several linear curves
	int i = 1;
	double pitch = 90.0; // initial pitch
	while (met > MET[i])
	{
		pitch -= aimPitchover[i - 1] * (MET[i] - MET[i - 1]);
		i += 1;
	}
	pitch -= aimPitchover[i - 1] * (met - MET[i - 1]);
	return pitch;
}

double ProjectMercury::AtlasPitchControl(double cutoffAlt, double cutoffVel)
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
	double targetOrbitalVelocity = cutoffVel;
	double deltaV = targetOrbitalVelocity - currentSpeed;
	double timeToCutoff = GetMass() * CORE_ISP_VAC / CORE_THRUST * (1.0 - exp(-deltaV / CORE_ISP_VAC));
	double currentFlightPathAngle = -acos(dotp(currentPosition, currentVelocity) / currentRadius / currentSpeed) + PI05;
	double omegaPStar = -currentSpeed * cos(currentFlightPathAngle) / currentRadius;
	double gEp = G * pow(planetRad / currentRadius, 2.0) - pow(currentSpeed * cos(currentFlightPathAngle), 2.0) / currentRadius;
	double DVe = deltaV; // assuming no loss. Actual defined as (fig 2.3) Vf - V + VL - VLf
	double betaPAvg = (0.0 - verticalSpeed) / DVe;
	double accAtCutoff = CORE_THRUST / (GetMass() - CORE_THRUST / CORE_ISP_VAC * timeToCutoff); // DOUBLECHECK THIS!
	double Q6 = CORE_ISP_VAC / DVe - CORE_ISP_VAC / (accAtCutoff * timeToCutoff); // CORE_ISP_VAC is here used as exhaust velocity. Note that I neglect contribution from verniers.
	double omegaPr = (planetRad + cutoffAlt - currentRadius - timeToCutoff * (Q6 * (0.0 - verticalSpeed) + verticalSpeed)) / (Q6 - 0.5) / pow(timeToCutoff, 2.0) / CORE_ISP_VAC; // DOUBLECHECK THIS!

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
	double rCutoff = ri;
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

// TrA and Eanomaly in radians
inline double ProjectMercury::TimeFromPerigee(double period, double ecc, double TrA)
{
	return period / PI2 * (EccentricAnomaly(ecc, TrA) - ecc * sin(EccentricAnomaly(ecc, TrA)));
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

bool ProjectMercury::GetLandingPointIfRetroInXSeconds(double t, ELEMENTS el, ORBITPARAM prm, double longAtNow, double* longitude, double* latitude)
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
	if (abs((postBurnSMa / entryRadius * (1.0 - postBurnEcc * postBurnEcc) - 1.0) / postBurnEcc) > 1.0)
	{
		return false; // no entry, because the perigee is above entry interface
	}
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
	double coeff1 = 1.5578, coeff2 = 9.3007, coeff3 = 22.6108;
	double angleCoveredDuringReentry = (coeff1 * entryAngleDeg * entryAngleDeg + coeff2 * entryAngleDeg + coeff3) * RAD; // empirical formula from dataset of reentries. Second order polynomial
	double landingLat = asin(sin(postBurnLPe - postBurnLAN + entryTrA + angleCoveredDuringReentry) * sin(postBurnInc));
	double landingLong = entryLong + acos((cos(angleCoveredDuringReentry) - sin(entryLat) * sin(landingLat)) / cos(entryLat) / cos(landingLat));

	*longitude = landingLong;
	*latitude = landingLat;
	return true;
}

void ProjectMercury::AtlasEngineDir(void)
{
	double TotalPitch = GetControlSurfaceLevel(AIRCTRL_ELEVATOR);
	double TotalYaw = GetControlSurfaceLevel(AIRCTRL_RUDDER);
	double TotalRoll = GetControlSurfaceLevel(AIRCTRL_AILERON); // negative because it was inverted

	VECTOR3 vernier1Direction, vernier2Direction;
	if (TotalRoll != 0.0)
	{
		double deflRo = TotalRoll * 70.0 * RAD;
		VECTOR3 t0 = VERNIER_EXHAUST_DIR;
		vernier1Direction = _V(t0.x, t0.y * cos(deflRo) - t0.z * sin(deflRo), t0.y * sin(deflRo) + t0.z * cos(deflRo));
		SetThrusterDir(th_vernier[0], vernier1Direction);
		t0 = FlipX(t0);
		vernier2Direction = _V(t0.x, t0.y * cos(deflRo) + t0.z * sin(deflRo), -t0.y * sin(deflRo) + t0.z * cos(deflRo));
		SetThrusterDir(th_vernier[1], vernier2Direction);

		if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP) // get help from the two booster engines
		{
			// Thrust gimbal of booster thrusters. Swiveled max 5 deg in pitch and yaw (19630012071 page 97(
			VECTOR3 thrustDirection;
			thrustDirection.z = cos(TotalRoll * 5.0 * RAD);
			thrustDirection.x = sin(TotalRoll * 5.0 * RAD); // negative because it was inverted
			thrustDirection.y = 0.0;
			SetThrusterDir(th_booster[0], thrustDirection);
			SetThrusterDir(th_booster[1], FlipX(thrustDirection));
		}
	}
	else
	{
		double deflPi = TotalPitch * 70.0 * RAD;
		double deflYa = TotalYaw * 25.0 * RAD; // negative because it was inverted
		VECTOR3 t0 = VERNIER_EXHAUST_DIR;
		vernier1Direction = _V(t0.x * cos(deflYa) + (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) - t0.x * sin(deflYa));
		SetThrusterDir(th_vernier[0], vernier1Direction);
		t0 = FlipX(t0);
		vernier2Direction = _V(t0.x * cos(deflYa) + (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) - t0.x * sin(deflYa));
		SetThrusterDir(th_vernier[1], vernier2Direction);

		if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP) // get help from the two booster engines
		{
			// Thrust gimbal of booster thrusters. Swiveled max 5 deg in pitch and yaw (19630012071 page 97)
			t0 = BOOSTER_EXHAUST_DIR;
			deflPi = TotalPitch * 5.0 * RAD;
			deflYa = TotalYaw * 5.0 * RAD; // negative because it was inverted
			VECTOR3 booster1Direction = _V(t0.x * cos(deflYa) - (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) + t0.x * sin(deflYa));
			SetThrusterDir(th_booster[0], booster1Direction);
			SetThrusterDir(th_booster[1], booster1Direction);
		}

		// Thrust gimbal of core thruster. Swiveled max 3 deg in pitch and yaw (19630012071 page 97)
		t0 = CORE_EXHAUST_DIR;
		deflPi = TotalPitch * 3.0 * RAD;
		deflYa = TotalYaw * 3.0 * RAD; // negative because it was inverted
		VECTOR3 coreDirection = _V(t0.x * cos(deflYa) - (t0.z * cos(deflPi) + t0.y * sin(deflPi)) * sin(deflYa), t0.y * cos(deflPi) - t0.z * sin(deflPi), cos(deflYa) * (t0.y * sin(deflPi) + t0.z * cos(deflPi)) + t0.x * sin(deflYa));
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
		_V(0, 1, 0),
		float(50.0 * RAD)
	);
	Vernier1AnimX = CreateAnimation(0.5);
	AddAnimationComponent(Vernier1AnimX, 0.0, 1.0, &vern1x);

	// -y vernier pitch
	static MGROUP_ROTATE vern1y(
		Atlas,
		vernier1, 1,
		VERNIER1_OFFSET,
		_V(-1, 0, 0),
		float(140.0 * RAD)
	);
	Vernier1AnimY = CreateAnimation(0.5);
	AddAnimationComponent(Vernier1AnimY, 0.0, 1.0, &vern1y);

	// +y vernier yaw
	static UINT vernier2[1] = { 4 };
	static MGROUP_ROTATE vern2x(
		Atlas,
		vernier2, 1,
		VERNIER2_OFFSET,
		_V(0, 1, 0),
		float(50.0 * RAD)
	);
	Vernier2AnimX = CreateAnimation(0.5);
	AddAnimationComponent(Vernier2AnimX, 0.0, 1.0, &vern2x);

	// +y vernier pitch
	static MGROUP_ROTATE vern2y(
		Atlas,
		vernier2, 1,
		VERNIER2_OFFSET,
		_V(-1, 0, 0),
		float(140.0 * RAD)
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

	if (conceptManouverUnit && conceptManouverUnitAttached)
		mass += CONCEPT_POSIGRADE_EMPTY_MASS;

	return mass;
}

void ProjectMercury::TowerSeparation(void)
{
	if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE || VesselStatus == ABORT || VesselStatus == ABORTNORETRO) // only occasions with tower
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
		else if (VesselStatus == ABORT)
		{
			autoPilot = true;
			AutopilotStatus = POSIGRADEDAMP; // Automatically aux-damp 
			VesselStatus = FLIGHT;
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

	// These are currently dependent on autopilot, to let the user launch in any direction if needed
	if (!GroundContact() && GetAttachmentStatus(padAttach) == NULL && autoPilot && VesselStatus == LAUNCH && abs(GetPitch() * DEG - currentPitchAim) > 5.0)
	{
		abortConditionsMet = true;
		char cbuf[256];
		sprintf(cbuf, "Abort due to error in pitch at T+%.1f. It was %.1f deg. Target was %.2f deg", met, GetPitch() * DEG, currentPitchAim);
		oapiWriteLog(cbuf);
	}/*
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
	if (oapiGetTimeAcceleration() > 1.0) oapiSetTimeAcceleration(1.0); // something extreme is happening, let's give Orbiter time to cope

	abort = true;
	suborbitalMission = true; // an abort is always ballistic

	if (VesselStatus == LAUNCH || VesselStatus == LAUNCHCORE) // Abort after liftoff, and has tower
	{
		SeparateAtlasBooster(false);
		SeparateAtlasCore();
		SeparateRingsAndAdapters(-0.65); // Adapters and rings
		
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

	// Separate potentially attached pad
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
		if (VesselStatus == TOWERSEP) SeparateAtlasBooster(false); // don't separate booster, just delete thrusters
		SeparateAtlasCore(); // this automatically tells core to let booster attached or not
		if (!conceptManouverUnit) SeparateRingsAndAdapters(-0.65); // Adapters and rings

		if (boilerplateMission) // no retropack
		{
			SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);
			SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);
			CGshifted = false;
			VesselStatus = REENTRY;
		}
		else
		{
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

	strcpy(name, GetName());
	strcat(name, " abort tower");

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

		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AbortNominal", &vs);
	}
	else
	{
		vs.fuel->idx = 1;
		vs.fuel->level = 0.0;
		AutopilotStatus = REENTRYATTITUDE;
		//abortDamping = true; // we have abort, so dampen any movement

		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Abort", &vs);
	}

	stuffCreated += 1;
	DelMesh(Tower);
	DelPropellantResource(escape_tank);
	DelThruster(escape_engine);
	towerJetTime = oapiGetSimTime();
	towerJettisoned = true;
	DestabiliserStatus = P_OPENING; // is held down by escape tower
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
			//inFlightAbort = true;
		}

		strcpy(name, GetName());
		strcat(name, " Atlas booster");

		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_atlas_booster", &vs);
		stuffCreated += 1;

		char buff[256];
		sprintf(buff, "Booster stage separated at T+%.1f", oapiGetSimTime() - launchTime);
		oapiWriteLog(buff);

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
	DelExhaustStream(iceVenting[0]);
	DelExhaustStream(iceVenting[1]);
	DelExhaustStream(iceVenting[2]);
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
	boosterSeparated = true;
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
	VECTOR3 redstoneOffset = _V(0, 0, -MERCURY_LENGTH_CAPSULE / 2.0 + 0.45);
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
		//inFlightAbort = true;
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

	if (VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP)
	{
		if (conceptManouverUnit) createdVessel[stuffCreated] = oapiCreateVesselEx(name, "Vessels\\ProjectMercury\\Mercury_AtlasCoreConcept", &vs); // no adapter, as we keep it attached to capsule
		else createdVessel[stuffCreated] = oapiCreateVesselEx(name, "Vessels\\ProjectMercury\\Mercury_AtlasCore", &vs); // regular SECO with adapter and without booster
	}
	else
	{
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "Vessels\\ProjectMercury\\Mercury_AtlasCoreB", &vs); // with booster attached
		if (conceptManouverUnit)
		{
			DelPropellantResource(conceptPropellant);
			SetDefaultPropellantResource(fuel_auto);
			conceptManouverUnitAttached = false;
			DelMesh(AtlasAdapter); // remove adapter mesh as we're in LAUNCH or TOWERSEP, meaning abort
		}

		DelMesh(AtlasBooster); // Remove booster mesh as we're in LAUNCH or TOWERSEP, and therefore had it attached
	}

	stuffCreated += 1;

	char buff[256];
	sprintf(buff, "Core stage separated at T+%.1f", oapiGetSimTime() - launchTime);
	oapiWriteLog(buff);

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
	if (!conceptManouverUnit) DelMesh(AtlasAdapter);

	coreSeparated = true;

	if (oapiCockpitMode() == COCKPIT_PANELS)
	{
		clbkLoadGenericCockpit();
		oapiWriteLogV("Cockpit mode: %i", oapiCockpitMode());
	}
}

void ProjectMercury::SeparateRingsAndAdapters(double offZ)
{
	VESSELSTATUS2 vs;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	char name[256];
	VECTOR3 vel = _V(0, .95, 0);
	VECTOR3 relativeOffset1;
	VECTOR3 addOfs = _V(0.0, 0.0, offZ);
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER1 + addOfs, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset1);
	VECTOR3 vel2;
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 1");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover1", &vs);
	stuffCreated += 1;
	DelMesh(Adaptcover1);

	vel = _V(-0.823, -0.475, 0);
	VECTOR3 relativeOffset2;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER2 + addOfs, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset2);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 2");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover2", &vs);
	stuffCreated += 1;
	DelMesh(Adaptcover2);

	vel = _V(0.823, -0.475, 0);
	VECTOR3 relativeOffset3;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER3 + addOfs, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset3);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Cover 3");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptCover3", &vs);
	stuffCreated += 1;
	DelMesh(Adaptcover3);

	vel = _V(-0.823, 0.475, 0);
	VECTOR3 relativeOffset4;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING1 + addOfs, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset4);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 1");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing1", &vs);
	stuffCreated += 1;
	DelMesh(Adaptring1);

	vel = _V(0.0, -0.95, 0);
	VECTOR3 relativeOffset5;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING2 + addOfs, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset5);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 2");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing2", &vs);
	stuffCreated += 1;
	DelMesh(Adaptring2);

	vel = _V(0.823, 0.475, 0);
	VECTOR3 relativeOffset6;
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTRING3 + addOfs, vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffset6);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Adapter Ring 3");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AdaptRing3", &vs);
	stuffCreated += 1;
	DelMesh(Adaptring3);

	//capsuleSepTime = oapiGetSimTime();
}

void ProjectMercury::SeparateConceptAdapter(void)
{
	if (conceptManouverUnit && conceptManouverUnitAttached)
	{
		// Separate rings and adapters
		SeparateRingsAndAdapters(-0.5 - MERCURY_OFS_CAPSULE.z + CONCEPT_EXTRA_LENGTH); // we have new centre of mass, as we are in FLIGHT mode

		// Create adapter vessel
		VESSELSTATUS2 vs;
		VESSELSTATUS2::FUELSPEC fuel;
		vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
		vs.version = 2;
		GetStatusEx(&vs);
		Local2Rel(ATLAS_ADAPTER_OFFSET - MERCURY_OFS_CAPSULE, vs.rpos);
		VECTOR3 vel;
		GlobalRot(_V(0,0,-0.2), vel);
		vs.rvel += vel;

		vs.flag = VS_FUELLIST;
		vs.fuel = &fuel;
		vs.nfuel = 1;
		vs.fuel->idx = 0;
		vs.fuel->level = GetPropellantMass(conceptPropellant) / CONCEPT_POSIGRADE_FUEL_MASS;

		char name[100];
		strcpy(name, GetName());
		strcat(name, " Concept Adapter");
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\ConceptAdapter", &vs);
		stuffCreated += 1;

		// Delete thrusters
		DelThruster(conceptPosigrade[0]);
		DelThruster(conceptPosigrade[1]);
		DelThruster(conceptRetrograde[0]);
		DelThruster(conceptRetrograde[1]);

		DelThruster(conceptLinear[0]);
		DelThruster(conceptLinear[1]);
		DelThruster(conceptLinear[2]);
		DelThruster(conceptLinear[3]);

		// Delete propellant
		DelPropellantResource(conceptPropellant);

		// Update HUD tank indicator
		SetDefaultPropellantResource(fuel_auto);

		// Update bool
		conceptManouverUnitAttached = false;

		// Delete mesh
		DelMesh(AtlasAdapter);

		oapiWriteLogV("Separated concept maouverable adapter at T+%.1f", oapiGetSimTime() - launchTime);
	}
	else
	{
		oapiWriteLog("Could not separate concept adapter. Possibly already done.");
	}
}

void ProjectMercury::SeparateConceptCovers(void)
{
	oapiWriteLog("Separating concept retrograde covers");
	// Create cover vessel1
	VESSELSTATUS2 vs;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;
	GetStatusEx(&vs);
	Local2Rel(CONCEPT_RETROGRADE_COVER1_OFFSET - MERCURY_OFS_CAPSULE, vs.rpos);
	VECTOR3 vel;
	GlobalRot(-CONCEPT_RETROGRADE_THRUSTER_UP_DIR, vel);
	vs.rvel += vel;

	char name[100];
	strcpy(name, GetName());
	strcat(name, " Concept Cover 1");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\ConceptCover1", &vs);
	stuffCreated += 1;

	// Create cover vessel2
	Local2Rel(CONCEPT_RETROGRADE_COVER2_OFFSET - MERCURY_OFS_CAPSULE, vs.rpos);
	vs.rvel -= vel;
	GlobalRot(-CONCEPT_RETROGRADE_THRUSTER_DOWN_DIR, vel);
	vs.rvel += vel;

	strcpy(name, GetName());
	strcat(name, " Concept Cover 2");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\ConceptCover2", &vs);
	stuffCreated += 1;

	// Update bool
	conceptCoverAttached = false;

	// Delete mesh
	DelMesh(ConceptCover1);
	DelMesh(ConceptCover2);

	oapiWriteLog("Finished separating concept retrograde covers");
}

// Created here in rocket code file, as the meshes are different for each rocket
inline void ProjectMercury::SetCameraSceneVisibility(WORD mode)
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

	if (!boosterSeparated && (VesselStatus == LAUNCH || VesselStatus == TOWERSEP))
	{
		if (AtlasBooster != NULL && GetMeshVisibilityMode(AtlasBooster) != mode) SetMeshVisibilityMode(AtlasBooster, mode);
	}

	if (!coreSeparated && (VesselStatus == LAUNCH || VesselStatus == TOWERSEP || VesselStatus == LAUNCHCORE || VesselStatus == LAUNCHCORETOWERSEP))
	{
		if (Atlas != NULL && GetMeshVisibilityMode(Atlas) != mode)
		{
			SetMeshVisibilityMode(Atlas, mode);
		}

		if (AtlasAdapter != NULL && GetMeshVisibilityMode(AtlasAdapter) != mode) SetMeshVisibilityMode(AtlasAdapter, mode);

		if (Adaptcover1 != NULL && GetMeshVisibilityMode(Adaptcover1) != mode) SetMeshVisibilityMode(Adaptcover1, mode);
		if (Adaptcover2 != NULL && GetMeshVisibilityMode(Adaptcover2) != mode) SetMeshVisibilityMode(Adaptcover2, mode);
		if (Adaptcover3 != NULL && GetMeshVisibilityMode(Adaptcover3) != mode) SetMeshVisibilityMode(Adaptcover3, mode);
		if (Adaptring1 != NULL && GetMeshVisibilityMode(Adaptring1) != mode) SetMeshVisibilityMode(Adaptring1, mode);
		if (Adaptring2 != NULL && GetMeshVisibilityMode(Adaptring2) != mode) SetMeshVisibilityMode(Adaptring2, mode);
		if (Adaptring3 != NULL && GetMeshVisibilityMode(Adaptring3) != mode) SetMeshVisibilityMode(Adaptring3, mode);
	}

	if (!towerJettisoned && (VesselStatus != FLIGHT && VesselStatus != REENTRY && VesselStatus != REENTRYNODROGUE && VesselStatus != TOWERSEP && VesselStatus != LAUNCHCORETOWERSEP))
	{
		if (Tower != NULL && GetMeshVisibilityMode(Tower) != mode) SetMeshVisibilityMode(Tower, mode);
	}

	if (Capsule != NULL && GetMeshVisibilityMode(Capsule) != mode) SetMeshVisibilityMode(Capsule, mode);

	if (VesselStatus != REENTRYNODROGUE && !drogueSeparated && Antennahouse != NULL && GetMeshVisibilityMode(Antennahouse) != mode) SetMeshVisibilityMode(Antennahouse, mode);
	oapiWriteLog("End CameraSceneVisibility");
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

// Give panel retro times. This function has a sibling in Redstone, giving the basic 4:44 retro-time
inline void ProjectMercury::GetPanelRetroTimes(double met, int* rH, int* rM, int* rS, int* dH, int* dM, int* dS)
{
	int ret3H = 0, ret3M = 0, ret3S = 0, dRetH = 0, dRetM = 0, dRetS = 0;
	double simt = oapiGetSimTime();

	if (retroStartTime != 0.0)
	{
		double metRetroTime = retroStartTime - 30.0 - launchTime; // -30 to account for retroseq to retroburn. -6.6 is empirical
		if (switchRetroDelay == 1) metRetroTime += 30.0; // instentanious retro burn, so add 30 seconds back
		double timeToRetro = metRetroTime - met;

		if (metRetroTime > (100.0 * 3600.0 - 1.0)) metRetroTime = fmod(metRetroTime, 100.0 * 3600.0); // overflow
		ret3H = (int)floor(metRetroTime / 3600.0);
		ret3M = (int)floor((metRetroTime - ret3H * 3600.0) / 60.0);
		ret3S = (int)floor((metRetroTime - ret3H * 3600.0 - ret3M * 60.0));

		if (timeToRetro < 0.0) timeToRetro = 0.0; // don't need to count negative numbers
		if (timeToRetro > (100.0 * 3600.0 - 1.0)) timeToRetro = fmod(timeToRetro, 100.0 * 3600.0); // overflow
		if (timeToRetro > 30.0) retroWarnLight = false; // warning light, actuated if less than 30 seconds until retrosequnce
		dRetH = (int)floor(timeToRetro / 3600.0);
		dRetM = (int)floor((timeToRetro - dRetH * 3600.0) / 60.0);
		dRetS = (int)floor((timeToRetro - dRetH * 3600.0 - dRetM * 60.0));
	}
	else if (landingComputing && VesselStatus == FLIGHT)
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
			bool entryInterface = GetLandingPointIfRetroInXSeconds(time, el, prm, longAtNow, &landingLong, &landingLat);

			if (entryInterface)
			{
				if (noMissionLandLat)
				{
					missionLandLat = landingLat * DEG; // only target the landing longitude
				}

				if (oapiOrthodome(landingLong, landingLat, missionLandLong * RAD, missionLandLat * RAD) < 5.0 * RAD)
				{
					closeSolution = true;
					time -= deltaT; // correct for later addition
				}
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

		double metRetroTime = minAngDistTime - 30.0 + met - 6.6; // -30 to account for retroseq to retroburn. -6.6 is empirical, mostly from retroburn not being instantanious
		if (switchRetroDelay == 1) metRetroTime += 30.0; // instentanious retro burn, so add 30 seconds back
		double timeToRetro = metRetroTime - met;

		if (metRetroTime > (100.0 * 3600.0 - 1.0)) metRetroTime = fmod(metRetroTime, 100.0 * 3600.0); // overflow
		ret3H = (int)floor(metRetroTime / 3600.0);
		ret3M = (int)floor((metRetroTime - ret3H * 3600.0) / 60.0);
		ret3S = (int)floor((metRetroTime - ret3H * 3600.0 - ret3M * 60.0));

		if (timeToRetro < 0.0) timeToRetro = 0.0; // don't need to count negative numbers
		if (timeToRetro > (100.0 * 3600.0 - 1.0)) timeToRetro = fmod(timeToRetro, 100.0 * 3600.0); // overflow
		if (timeToRetro > 30.0) retroWarnLight = false; // warning light, actuated if less than 30 seconds until retrosequnce
		dRetH = (int)floor(timeToRetro / 3600.0);
		dRetM = (int)floor((timeToRetro - dRetH * 3600.0) / 60.0);
		dRetS = (int)floor((timeToRetro - dRetH * 3600.0 - dRetM * 60.0));

		if (FailureMode == RETROCALCOFF && simt - launchTime > timeOfError)
		{
			// error numbers
			ret3H = 99;
			ret3M = 99;
			ret3S = 99;

			dRetH = 99;
			dRetM = 99;
			dRetS = 99;
		}
		else if (minAngDistTime == 0.0) // not found
		{
			// Blank numbers
			ret3H = 0;
			ret3M = 0;
			ret3S = 0;

			dRetH = 0;
			dRetM = 0;
			dRetS = 0;
		}
		else if (timeToRetro < 30.0)
		{
			retroWarnLight = true; // warning light, actuated if less than 30 seconds until retrosequnce
		}
	}
	else if (!landingComputing && manualInputRetroTime != NULL)
	{
		double metRetroTime = manualInputRetroTime;
		double timeToRetro = metRetroTime - met;

		if (metRetroTime > (100.0 * 3600.0 - 1.0)) metRetroTime = fmod(metRetroTime, 100.0 * 3600.0); // overflow
		ret3H = (int)floor(metRetroTime / 3600.0);
		ret3M = (int)floor((metRetroTime - ret3H * 3600.0) / 60.0);
		ret3S = (int)floor((metRetroTime - ret3H * 3600.0 - ret3M * 60.0));

		if (timeToRetro < 0.0) timeToRetro = 0.0; // don't need to count negative numbers
		if (timeToRetro > (100.0 * 3600.0 - 1.0)) timeToRetro = fmod(timeToRetro, 100.0 * 3600.0); // overflow
		if (timeToRetro > 30.0) retroWarnLight = false; // warning light, actuated if less than 30 seconds until retrosequnce
		dRetH = (int)floor(timeToRetro / 3600.0);
		dRetM = (int)floor((timeToRetro - dRetH * 3600.0) / 60.0);
		dRetS = (int)floor((timeToRetro - dRetH * 3600.0 - dRetM * 60.0));

		if (timeToRetro < 30.0)
		{
			retroWarnLight = true; // warning light, actuated if less than 30 seconds until retrosequnce
		}
	}

	*rH = ret3H;
	*rM = ret3M;
	*rS = ret3S;
	*dH = dRetH;
	*dM = dRetM;
	*dS = dRetS;
}