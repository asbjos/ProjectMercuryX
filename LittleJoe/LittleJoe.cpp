// ==============================================================
//				Source file for Mercury Little Joe.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2020
// 
// Based on Little Joe addon by "4th rock"
// This code is my own work.
// 
// Thank you to Ricardo Nunes.
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================

#define STRICT
#define ORBITER_MODULE
#define VESSELVER VESSEL4

#include "orbitersdk.h"
#include "LittleJoe.h"
#include "..\FunctionsForOrbiter2016.h"
#include "..\MercuryCapsule.h"


ProjectMercury::ProjectMercury(OBJHANDLE hVessel, int flightmodel)
	: VESSELVER(hVessel, flightmodel)
{
	// Load exterior meshes
	// capsule is defined in loadstate
	booster = oapiLoadMeshGlobal("ProjectMercury\\LJ");

	MercuryGenericConstructor();
}

ProjectMercury::~ProjectMercury()
{
	//oapiCloseFile(pitchLog, FILE_OUT);
	WriteFlightParameters();
}

// --------------------------------------------------------------
// Set the capabilities of the vessel class
// --------------------------------------------------------------
void ProjectMercury::clbkSetClassCaps(FILEHANDLE cfg)
{
	// physical vessel parameters
	SetSize(LITTLEJOE_LENGTH + MERCURY_LENGTH_CAPSULE + MERCURY_LENGTH_ABORT);
	SetEmptyMass(MERCURY_MASS + LITTLEJOE_TOT_DRY_MASS + ABORT_MASS);

	ReadConfigSettings(cfg);

	static const DWORD tchdwnLaunchPadNum = 4;
	const VECTOR3 TOUCHDOWN_LAUNCH0 = _V(0.0, -1.0, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH1 = _V(-0.7, 0.7, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH2 = _V(0.7, 0.7, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0 - heightOverGround);
	const VECTOR3 TOUCHDOWN_LAUNCH3 = _V(0.0, 0.0, MERCURY_OFS_ABORT.z + MERCURY_LENGTH_ABORT / 2.0);
	VersionDependentTouchdown(TOUCHDOWN_LAUNCH0, TOUCHDOWN_LAUNCH1, TOUCHDOWN_LAUNCH2, TOUCHDOWN_LAUNCH3, 1e7, 1e5, 10.0);

	SetCW(1.0, 0.1, 0.3, 0.3);
	SetCrossSections(_V(38.4, 38.4, 4.04));
	SetRotDrag(_V(0.7, 0.7, 1.2));
	SetPMI(_V(33.0, 33.0, 20.51)); // actually much smaller z-component, but makes atmospheric flight unstable at >1x

	CreateCapsuleFuelTanks();

	for (int i = 0; i < 4; i++)
	{
		castor_propellant[i] = CreatePropellantResource(SINGLECASTOR_FUEL_MASS);
		recruit_propellant[i] = CreatePropellantResource(SINGLERECRUIT_FUEL_MASS);
	}

	escape_tank = CreatePropellantResource(ABORT_MASS_FUEL); // Need to define after Redstone, as this is deleted first
	//SetPropellantMass(redstone_propellant, STAGE1_FUEL_MASS);

	SetDefaultPropellantResource(castor_propellant[0]);

	// main engine
	th_castor[0] = CreateThruster(mul(ROTMATRIX, LITTLEJOE_CASTOR_EXHAUST_POS),					mul(ROTMATRIX, LITTLEJOE_CASTOR_EXHAUST_DIR),				SINGLECASTOR_THRUST_VAC, castor_propellant[0], SINGLECASTOR_ISP_VAC, SINGLECASTOR_ISP_SL);
	th_castor[1] = CreateThruster(mul(ROTMATRIX, FlipX(LITTLEJOE_CASTOR_EXHAUST_POS)),			mul(ROTMATRIX, FlipX(LITTLEJOE_CASTOR_EXHAUST_DIR)),		SINGLECASTOR_THRUST_VAC, castor_propellant[1], SINGLECASTOR_ISP_VAC, SINGLECASTOR_ISP_SL);
	th_castor[2] = CreateThruster(mul(ROTMATRIX, FlipY(FlipX(LITTLEJOE_CASTOR_EXHAUST_POS))),	mul(ROTMATRIX, FlipY(FlipX(LITTLEJOE_CASTOR_EXHAUST_DIR))),	SINGLECASTOR_THRUST_VAC, castor_propellant[2], SINGLECASTOR_ISP_VAC, SINGLECASTOR_ISP_SL);
	th_castor[3] = CreateThruster(mul(ROTMATRIX, FlipY(LITTLEJOE_CASTOR_EXHAUST_POS)),			mul(ROTMATRIX, FlipY(LITTLEJOE_CASTOR_EXHAUST_DIR)),		SINGLECASTOR_THRUST_VAC, castor_propellant[3], SINGLECASTOR_ISP_VAC, SINGLECASTOR_ISP_SL);

	th_recruit[0] = CreateThruster(mul(ROTMATRIX, LITTLEJOE_RECRUIT_EXHAUST_POS),				mul(ROTMATRIX, LITTLEJOE_RECRUIT_EXHAUST_DIR),					SINGLERECRUIT_THRUST_VAC, recruit_propellant[0], SINGLERECRUIT_ISP_VAC, SINGLERECRUIT_ISP_SL);
	th_recruit[1] = CreateThruster(mul(ROTMATRIX, FlipX(LITTLEJOE_RECRUIT_EXHAUST_POS)),		mul(ROTMATRIX, FlipX(LITTLEJOE_RECRUIT_EXHAUST_DIR)),			SINGLERECRUIT_THRUST_VAC, recruit_propellant[1], SINGLERECRUIT_ISP_VAC, SINGLERECRUIT_ISP_SL);
	th_recruit[2] = CreateThruster(mul(ROTMATRIX, FlipY(SwapXY(LITTLEJOE_RECRUIT_EXHAUST_POS))),mul(ROTMATRIX, FlipY(SwapXY(LITTLEJOE_RECRUIT_EXHAUST_DIR))),	SINGLERECRUIT_THRUST_VAC, recruit_propellant[2], SINGLERECRUIT_ISP_VAC, SINGLERECRUIT_ISP_SL);
	th_recruit[3] = CreateThruster(mul(ROTMATRIX, SwapXY(LITTLEJOE_RECRUIT_EXHAUST_POS)),		mul(ROTMATRIX, SwapXY(LITTLEJOE_RECRUIT_EXHAUST_DIR)),			SINGLERECRUIT_THRUST_VAC, recruit_propellant[3], SINGLERECRUIT_ISP_VAC, SINGLERECRUIT_ISP_SL);

	CreateThrusterGroup(th_recruit, 4, THGROUP_MAIN);
	SURFHANDLE exhaustTexture = oapiRegisterExhaustTexture("Exhaust2");
	AddExhaust(th_castor[0], 8, 1, mul(ROTMATRIX, LITTLEJOE_CASTOR_EXHAUST_POS),				mul(ROTMATRIX, -LITTLEJOE_CASTOR_EXHAUST_DIR), exhaustTexture);
	AddExhaust(th_castor[1], 8, 1, mul(ROTMATRIX, FlipX(LITTLEJOE_CASTOR_EXHAUST_POS)),			mul(ROTMATRIX, -FlipX(LITTLEJOE_CASTOR_EXHAUST_DIR)), exhaustTexture);
	AddExhaust(th_castor[2], 8, 1, mul(ROTMATRIX, FlipY(FlipX(LITTLEJOE_CASTOR_EXHAUST_POS))),	mul(ROTMATRIX, -FlipY(FlipX(LITTLEJOE_CASTOR_EXHAUST_DIR))), exhaustTexture);
	AddExhaust(th_castor[3], 8, 1, mul(ROTMATRIX, FlipY(LITTLEJOE_CASTOR_EXHAUST_POS)),			mul(ROTMATRIX, -FlipY(LITTLEJOE_CASTOR_EXHAUST_DIR)), exhaustTexture);

	AddExhaust(th_recruit[0], 4, 1, mul(ROTMATRIX, LITTLEJOE_RECRUIT_EXHAUST_POS), mul(ROTMATRIX, -LITTLEJOE_RECRUIT_EXHAUST_DIR), exhaustTexture);
	AddExhaust(th_recruit[1], 4, 1, mul(ROTMATRIX, FlipX(LITTLEJOE_RECRUIT_EXHAUST_POS)), mul(ROTMATRIX, -FlipX(LITTLEJOE_RECRUIT_EXHAUST_DIR)), exhaustTexture);
	AddExhaust(th_recruit[2], 4, 1, mul(ROTMATRIX, FlipY(FlipX(LITTLEJOE_RECRUIT_EXHAUST_POS))), mul(ROTMATRIX, -FlipY(FlipX(LITTLEJOE_RECRUIT_EXHAUST_DIR))), exhaustTexture);
	AddExhaust(th_recruit[3], 4, 1, mul(ROTMATRIX, FlipY(LITTLEJOE_RECRUIT_EXHAUST_POS)), mul(ROTMATRIX, -FlipY(LITTLEJOE_RECRUIT_EXHAUST_DIR)), exhaustTexture);

	// Control surfaces for air rudders. dCl and delay values from DeltaGlider and/or Falcon 9 by BrianJ
	CreateAirfoilsLittleJoe(); // try this
	/*CreateControlSurface3(AIRCTRL_AILERON, 2.36, 1.7, _V(0.0, -2.0, -2.3), NULL, 1e5);
	CreateControlSurface3(AIRCTRL_AILERON, 2.36, 1.7, _V(0.0, 2.0, -2.3), NULL, 1e5);
	CreateControlSurface3(AIRCTRL_AILERON, 2.36, 1.7, _V(-2.0, 0.0, -2.3), NULL, 1e5);
	CreateControlSurface3(AIRCTRL_AILERON, 2.36, 1.7, _V(2.0, 0.0, -2.3), NULL, 1e5);*/
	// Rudders[2] = CreateControlSurface3(AIRCTRL_ELEVATOR, 0.515, 3, _V(0.0, 1.8, -8.6), AIRCTRL_AXIS_YPOS, 1.0);
	//Rudders[2] = CreateControlSurface3(AIRCTRL_AILERON, 0.515, 1.7, _V(0.0, 1.8, -8.6), AIRCTRL_AXIS_YPOS, 0.5);
	//Rudders[3] = CreateControlSurface3(AIRCTRL_FLAP, 0.515, 1.7, _V(1.8, 0.0, -8.6), AIRCTRL_AXIS_XPOS, 0.5);
	//DefineRudderAnimations();

	// Contrail stuff
	contrail_main_castor = {
		0, 3.0, 100, 200, 0.1, 20.0, 2.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin, contrailEnd
	};
	PARTICLESTREAMSPEC exhCastor = {
		0, 2.0, 5.0, 50, 0.5, 5.0, 10.0, 3.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, 0.0, 0.3
	};

	contrail_castor[0] = AddExhaustStream(th_castor[0], _V(0, 0.0, -20), &contrail_main_castor);
	exhaustCastor[0] = AddExhaustStream(th_castor[0], _V(0, 0.0, -5), &exhCastor);
	exhaustCastor[1] = AddExhaustStream(th_castor[1], _V(0, 0.0, -5), &exhCastor);

	contrail_main_recruit = {
		0, 3.0, 100, 200, 0.1, 20.0, 2.0, 50.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_PSQRT, 0, 2,
		PARTICLESTREAMSPEC::ATM_PLOG, contrailBegin, contrailEnd
	};
	contrail_recruit[0] = AddExhaustStream(th_recruit[0], _V(0, 0.0, -15), &contrail_main_recruit);

	// capsule defined in LoadState
	AddDefaultMeshes();
	// associate a mesh for the visual
	Booster = AddMesh(booster, &LITTLEJOE_OFFSET);

	//padAttach = CreateAttachment(true, _V(0, 0, -10), _V(0, 0, 1), _V(0, 1, 0), "PAD", false);
	padAttach = CreateAttachment(true, _V(0.0, 0.0, LITTLEJOE_OFFSET.z - LITTLEJOE_LENGTH / 2.0), _V(0, -1, 0), _V(0, 0, 1), "PAD", false);
}

void ProjectMercury::clbkPostCreation()
{
	CapsuleGenericPostCreation();

	if (VesselStatus == TOWERSEP)
	{
		DelMesh(Tower);
		towerJettisoned = true;
	}
	else if (VesselStatus == ABORT)
	{
		// Airfoil-stuff
		CreateAirfoilsEscape();

		DelThruster(th_castor[0]);
		DelThruster(th_castor[1]);
		DelThruster(th_castor[2]);
		DelThruster(th_castor[3]);
		DelThruster(th_recruit[0]);
		DelThruster(th_recruit[1]);
		DelThruster(th_recruit[2]);
		DelThruster(th_recruit[3]);
		DelPropellantResource(castor_propellant[0]);
		DelPropellantResource(castor_propellant[1]);
		DelPropellantResource(castor_propellant[2]);
		DelPropellantResource(castor_propellant[3]);
		DelPropellantResource(recruit_propellant[0]);
		DelPropellantResource(recruit_propellant[1]);
		DelPropellantResource(recruit_propellant[2]);
		DelPropellantResource(recruit_propellant[3]);
		DelMesh(Booster);
		DelMesh(Adaptcover1);
		DelMesh(Adaptcover2);
		DelMesh(Adaptcover3);
		DelMesh(Adaptring1);
		DelMesh(Adaptring2);
		DelMesh(Adaptring3);

		CreateAbortThrusters();
		ClearControlSurfaceDefinitions();
		boosterSeparated = true;
		// SetThrusterGroupLevel(THGROUP_MAIN, 1.0); // Probably already fired. No scenario saved in the second it fires.

		//oapiWriteLog("PostCreation ABORT");
	}
	else if (VesselStatus == ABORTNORETRO)
	{
		// Airfoil-stuff
		CreateAirfoilsEscape();

		DelThruster(th_castor[0]);
		DelThruster(th_castor[1]);
		DelThruster(th_castor[2]);
		DelThruster(th_castor[3]);
		DelThruster(th_recruit[0]);
		DelThruster(th_recruit[1]);
		DelThruster(th_recruit[2]);
		DelThruster(th_recruit[3]);
		DelPropellantResource(castor_propellant[0]);
		DelPropellantResource(castor_propellant[1]);
		DelPropellantResource(castor_propellant[2]);
		DelPropellantResource(castor_propellant[3]);
		DelPropellantResource(recruit_propellant[0]);
		DelPropellantResource(recruit_propellant[1]);
		DelPropellantResource(recruit_propellant[2]);
		DelPropellantResource(recruit_propellant[3]);
		DelMesh(Booster);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);
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
		ClearControlSurfaceDefinitions();

		boosterSeparated = true;
		//oapiWriteLog("PostCreation ABORTNORETRO");
	}
	else if (VesselStatus == FLIGHT)
	{
		// Airfoil-stuff
		CreateAirfoils();

		DelThruster(th_castor[0]);
		DelThruster(th_castor[1]);
		DelThruster(th_castor[2]);
		DelThruster(th_castor[3]);
		DelThruster(th_recruit[0]);
		DelThruster(th_recruit[1]);
		DelThruster(th_recruit[2]);
		DelThruster(th_recruit[3]);
		DelPropellantResource(castor_propellant[0]);
		DelPropellantResource(castor_propellant[1]);
		DelPropellantResource(castor_propellant[2]);
		DelPropellantResource(castor_propellant[3]);
		DelPropellantResource(recruit_propellant[0]);
		DelPropellantResource(recruit_propellant[1]);
		DelPropellantResource(recruit_propellant[2]);
		DelPropellantResource(recruit_propellant[3]);
		DelMesh(Booster);
		DelPropellantResource(escape_tank);
		DelMesh(Tower);
		DelMesh(Adaptcover1);
		DelMesh(Adaptcover2);
		DelMesh(Adaptcover3);
		DelMesh(Adaptring1);
		DelMesh(Adaptring2);
		DelMesh(Adaptring3);

		// CreatePosigradeRockets(); // Probably already fired. No scenario saved in the second they fire.
		CreateRetroRockets();
		ClearControlSurfaceDefinitions();
		boosterSeparated = true;
		towerJettisoned = true;
		//CreateRCS();
	}
	else if (VesselStatus == REENTRY)
	{
		CreateAirfoils();

		DelThruster(th_castor[0]);
		DelThruster(th_castor[1]);
		DelThruster(th_castor[2]);
		DelThruster(th_castor[3]);
		DelThruster(th_recruit[0]);
		DelThruster(th_recruit[1]);
		DelThruster(th_recruit[2]);
		DelThruster(th_recruit[3]);
		DelPropellantResource(castor_propellant[0]);
		DelPropellantResource(castor_propellant[1]);
		DelPropellantResource(castor_propellant[2]);
		DelPropellantResource(castor_propellant[3]);
		DelPropellantResource(recruit_propellant[0]);
		DelPropellantResource(recruit_propellant[1]);
		DelPropellantResource(recruit_propellant[2]);
		DelPropellantResource(recruit_propellant[3]);
		DelMesh(Booster);
		DelPropellantResource(escape_tank);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);

		//CreateRCS();

		DelMesh(Tower);
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
		ClearControlSurfaceDefinitions();
		boosterSeparated = true;
		towerJettisoned = true;
	}
	else if (VesselStatus == REENTRYNODROGUE)
	{
		CreateAirfoils();

		DelThruster(th_castor[0]);
		DelThruster(th_castor[1]);
		DelThruster(th_castor[2]);
		DelThruster(th_castor[3]);
		DelThruster(th_recruit[0]);
		DelThruster(th_recruit[1]);
		DelThruster(th_recruit[2]);
		DelThruster(th_recruit[3]);
		DelPropellantResource(castor_propellant[0]);
		DelPropellantResource(castor_propellant[1]);
		DelPropellantResource(castor_propellant[2]);
		DelPropellantResource(castor_propellant[3]);
		DelPropellantResource(recruit_propellant[0]);
		DelPropellantResource(recruit_propellant[1]);
		DelPropellantResource(recruit_propellant[2]);
		DelPropellantResource(recruit_propellant[3]);
		DelMesh(Booster);
		DelPropellantResource(escape_tank);
		DelPropellantResource(posigrade_propellant[0]);
		DelPropellantResource(posigrade_propellant[1]);
		DelPropellantResource(posigrade_propellant[2]);
		DelPropellantResource(retro_propellant[0]);
		DelPropellantResource(retro_propellant[1]);
		DelPropellantResource(retro_propellant[2]);

		//CreateRCS();

		DelMesh(Tower);
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
		ClearControlSurfaceDefinitions();
		// Drogue-mesh has not been created, and thus needs no deletion
		boosterSeparated = true;
		towerJettisoned = true;
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
		}

		if (!launchPerformed && GetThrusterGroupLevel(THGROUP_MAIN) != 0.0)
			launchPerformed = true;

		if (launchPerformed)
		{
			SetThrusterLevel(th_recruit[0], 1.0);
			SetThrusterLevel(th_recruit[1], 1.0);
			SetThrusterLevel(th_recruit[2], 1.0);
			SetThrusterLevel(th_recruit[3], 1.0);

			SetThrusterLevel(th_castor[0], 1.0);
			SetThrusterLevel(th_castor[2], 1.0);

			if (simt - launchTime > castorFireTime)
			{
				SetThrusterLevel(th_castor[1], 1.0);
				SetThrusterLevel(th_castor[3], 1.0);
			}
		}

		break;
	case TOWERSEP:
		if (!CGshifted && !GroundContact())
		{
			SetSize(LITTLEJOE_LENGTH + MERCURY_LENGTH_CAPSULE);
			VersionDependentTouchdown(TOUCHDOWN_TOWSEP0, TOUCHDOWN_TOWSEP1, TOUCHDOWN_TOWSEP2, TOUCHDOWN_TOWSEP3, 1e7, 1e5, 10.0);
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
			VersionDependentTouchdown(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2, TOUCHDOWN_FLIGHT3, stiffness, damping, 10.0);
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
			VersionDependentTouchdown(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2, TOUCHDOWN_FLIGHT3, stiffness, damping, 10.0);

			if (PreviousVesselStatus == ABORT || PreviousVesselStatus == ABORTNORETRO)
			{
				ShiftCG(-_V(0.0, 0.0, (77.8 - 19.3) * 0.0254) + MERCURY_CAPSULE_OFFSET);  // 19670022650 page 36 (CG of escape setup at 77.8 in, compared to 19.3 in for spacecraft)
				SetCameraOffset(_V(0.0, 0.0, 0.0));
			}
			else if (PreviousVesselStatus == LAUNCH || PreviousVesselStatus == TOWERSEP) // spawned in REENTRY from scenario
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
			VersionDependentTouchdown(TOUCHDOWN_FLIGHT0, TOUCHDOWN_FLIGHT1, TOUCHDOWN_FLIGHT2, TOUCHDOWN_FLIGHT3, stiffness, damping, 10.0);

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
			VersionDependentTouchdown(TOUCHDOWN_ABORT0, TOUCHDOWN_ABORT1, TOUCHDOWN_ABORT2, TOUCHDOWN_ABORT3, 1e7, 1e5, 10.0);
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

		if (inFlightAbort && vesselAcceleration < 0.25 * G && (simt - abortTime) > 2.0) // When aborting from inflight, and acc<0.25g and "maximum altitude sensor runs out" (http://aresinstitute.org/spacepdf/19740076641_1974076641.pdf page 10)
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
			VersionDependentTouchdown(TOUCHDOWN_ABORT0, TOUCHDOWN_ABORT1, TOUCHDOWN_ABORT2, TOUCHDOWN_ABORT3, 1e7, 1e5, 10.0);
			if (PreviousVesselStatus == LAUNCH) // either abort or from scenario
			{
				ShiftCG(MERCURY_OFS_CAPSULE + _V(0.0, 0.0, (77.8 - 19.3) * 0.0254));
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

		// Equations for tower separate time from http://aresinstitute.org/spacepdf/19740075935_1974075935.pdf page 54
		double abortMet = abortTime - launchTime;
		if (abortMet < 0.0) abortMet = 0.0;
		double towerSepTime = 0.1855 * abortMet + 7.0;

		if (vesselAcceleration < 0.25 * G && simt - abortTime > towerSepTime) // When aborting from inflight, and acc<0.25g and "maximum altitude sensor runs out" (http://aresinstitute.org/spacepdf/19740076641_1974076641.pdf page 10)
		{
			OffPadAbortSeparate();
		}
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

	//// DEBUG
	//if (simt - launchTime > 0.0 && launchTime != 0.0)
	//{
	//	if (pitchLog == NULL)
	//		pitchLog = oapiOpenFile("LJ-pitchLog.txt", FILE_OUT, ROOT); // debug

	//	//// Debug
	//	char pitchLogData[256];
	//	double lo, la, ra;
	//	GetEquPos(lo, la, ra);
	//	double range = oapiOrthodome(historyLaunchLong, historyLaunchLat, lo, la) * oapiGetSize(historyReference) / 1000.0;
	//	sprintf(pitchLogData, "%.3f\t%.3f\t%.3f\t%.3f\t%.3f", simt - launchTime, GetPitch() * DEG, GetAltitude(), range, GetGroundspeed());
	//	oapiWriteLine(pitchLog, pitchLogData);
	//}
}

void ProjectMercury::clbkPostStep(double simt, double simdt, double mjd)
{
	if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP)
	{
		//if (Redstone == NULL && GroundContact()) // no idea why I have to do this! It all worked yesterday! >:(
		//{
		//	Redstone = AddMesh(redstone, &LITTLEJOE_OFFSET);
		//	char stringToWrite[256];
		//	sprintf(stringToWrite, "AddMesh because NULL %.3f", simt);
		//	oapiWriteLog(stringToWrite);
		//}

		// Abort due to failure mode
		if (FailureMode == BOOSTERPROBLEM && simt - launchTime > timeOfError)
		{
			abortConditionsMet = true;
			oapiWriteLog("Abort due to random failure!");
		}
	}

	if (VesselStatus == LAUNCH && enableAbortConditions && GetDamageModel() != 0)
		CheckAbortConditions(simt, simdt);

	// Redstone contrail
	//if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP) && GetAtmDensity() < contrailEnd)
	//{
	//	DelExhaustStream(contrail_vapour);

	//	contrailActive = false;
	//}
	//else if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP) && !contrailActive) // If in dense atmosphere and contrail has not been activated
	//{
	//	contrail_vapour = AddExhaustStream(th_castor[0], _V(0, 0.3, -20), &contrail_main);
	//	contrailActive = true;
	//}

	if (FailureMode == ATTSTUCKOFF && (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) && simt - launchTime > timeOfError)
	{
		DisableAttitudeThruster(attitudeThrusterErrorNum);
	}

	// Autopilot-stuff
	if (autoPilot && VesselStatus == LAUNCH)
	{
		if (AutopilotStatus == AUTOLAUNCH)
		{
			//RedstoneAutopilot(simt, simdt);
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
		//SetADCtrlMode(7); // enable adc
		//double PitchUp = GetThrusterGroupLevel(THGROUP_ATT_PITCHUP);
		//double PitchDown = GetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN);
		//double TotalPitch = PitchUp - PitchDown;

		//double YawRight = GetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT);
		//double YawLeft = GetThrusterGroupLevel(THGROUP_ATT_YAWLEFT);
		//double TotalYaw = YawRight - YawLeft;

		//double flameArea = 0.46 * 0.46 * PI;
		//double pitchVaneArea = 2 * 0.025 * sin(27.5 * RAD * GetControlSurfaceLevel(AIRCTRL_RUDDER));
		//double yawVaneArea = 2 * 0.025 * sin(27.5 * RAD * GetControlSurfaceLevel(AIRCTRL_ELEVATOR));

		//VECTOR3 thrustDirection;
		//thrustDirection.z = 1 - ((abs(pitchVaneArea) + abs(yawVaneArea)) / flameArea);
		//thrustDirection.x = -pitchVaneArea / flameArea;
		//thrustDirection.y = -yawVaneArea / flameArea;
		//SetThrusterDir(th_main, thrustDirection);
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
			oapiWriteLogV("1Bankleft: %.2f %.2f %.2f", DEG * angVel.x, DEG * angVel.y, DEG * angVel.z);
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
			oapiWriteLogV("2Yawleft: %.2f %.2f %.2f", DEG * angVel.x, DEG * angVel.y, DEG * angVel.z);
			PMIn++;
			SetAngularVel(_V(0.0, 0.0, 0.0));
			SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 1.0);
			PMItime = simt;
		}
		else if (simt >= PMItime + 1.0 && PMIn == 3)
		{
			SetThrusterGroupLevel(THGROUP_ATT_PITCHUP, 0.0);
			VECTOR3 angVel;
			GetAngularVel(angVel);
			oapiWriteLogV("3Pitchup: %.2f %.2f %.2f", DEG * angVel.x, DEG * angVel.y, DEG * angVel.z);
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
		case OAPI_KEY_R:
			if (VesselStatus == REENTRY)
			{
				DelMesh(Drogue);
				Drogue = AddMesh(drogue, &MERCURY_OFS_DROGUE);
				SetMeshVisibilityMode(Drogue, MESHVIS_ALWAYS);
			}
			else if (VesselStatus == REENTRYNODROGUE)
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
				launchPerformed = true;
				spaceLaunch = true;
				autoPilot = false;

				if (spaceLaunch)
					SetThrusterGroupLevel(THGROUP_MAIN, 1.0);

				if (GroundContact())
				{
					launchTime = oapiGetSimTime();
				}
			}
			else if (VesselStatus == FLIGHT)
			{
				engageRetro = true;
				retroStartTime = oapiGetSimTime() + 30.0; // retrosequence starts 30 sec before firing
				char cbuf[256];
				sprintf(cbuf, "Retrosequence T+%.0f", retroStartTime - 30.0 - launchTime);
				oapiWriteLog(cbuf);

				AutopilotStatus = PITCHHOLD;
				autoPilot = true;
				attitudeHold14deg = false;
			}

			return 1;
		case OAPI_KEY_P:
			if (GroundContact() && VesselStatus == LAUNCH)
			{
				launchPerformed = true;
				autoPilot = true;
				SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
				launchTime = oapiGetSimTime();
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
			if (VesselStatus == LAUNCH && GetThrusterLevel(th_castor[0]) == 0.0 && GetThrusterLevel(th_castor[1]) == 0.0) // C++ should exit if first condition is false, omiting a crash for when th_main is deleted
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
				//PMIn = 1;

				AutopilotStatus = TURNAROUND;
				autoPilot = true;
				attitudeHold14deg = !attitudeHold14deg;
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
			if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP) && GroundContact() && GetAttachmentStatus(padAttach) == NULL)
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
		case OAPI_KEY_M: // turn on manual mode
			if (enableAbortConditions)
				oapiWriteLog("Automatic abort turned off by key");
			enableAbortConditions = false;
			DisableAutopilot(true);
			abortDamping = false;
			engageFuelDump = false;

			if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP) // If not for this, the control surfaces get stuck in the last position from the autopilot
			{
				SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0);
				SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0);
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
		}
	}
	return 0;
}

bool ProjectMercury::clbkDrawHUD(int mode, const HUDPAINTSPEC* hps, oapi::Sketchpad* skp)
{
	char cbuf[256];
	int yIndex = 0;

	if (oapiCockpitMode() == COCKPIT_PANELS && panelView)
	{
		AnimateDials();

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
		if (VesselStatus == LAUNCH)
		{
			sprintf(cbuf, "J:     ABORT");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == TOWERSEP)
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
		if (VesselStatus == LAUNCH || VesselStatus == TOWERSEP)
		{
			if (!spaceLaunch)
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
		else if (VesselStatus == FLIGHT || VesselStatus == REENTRY) // switch fuel tank for attitude control
		{
			if (attitudeFuelAuto)
			{
				sprintf(cbuf, "P:     Set propellant source MANUAL");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "P:     Set propellant source AUTO");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
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
		if (VesselStatus == LAUNCH && GetThrusterLevel(th_castor[0]) == 0.0 && GetThrusterLevel(th_castor[1]) == 0.0) // C++ should exit if first condition is false, omiting a crash for when th_main is deleted
		{
			sprintf(cbuf, "K:     Separate escape tower");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == FLIGHT || VesselStatus == REENTRY)
		{
			sprintf(cbuf, "K:     Engage automatic attitude");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Key G
		if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE)
		{
			if (RcsStatus == AUTOLOW)
			{
				sprintf(cbuf, "G:     Switch to attitude MANUAL");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else if (RcsStatus == MANUAL)
			{
				sprintf(cbuf, "G:     Switch to attitude AUTOHIGH");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "G:     Switch to attitude AUTOLOW");
				skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}

		// Key M
		sprintf(cbuf, "M:     Disengage autopilot");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;


		// Key C
		if (VesselStatus == LAUNCH && GroundContact() && GetAttachmentStatus(padAttach) == NULL)
		{
			sprintf(cbuf, "C:     Set focus on launch ramp");
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

		// Key V and B
		if (periscope)
		{
			sprintf(cbuf, "B/V:   Set periscope altitude +/-");
			skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
	}

	yIndex = -4;
	double simt = oapiGetSimTime();

	if (showInfoOnHud < 2)
	{
		// This sets the x so that the text is centered
		int stringWidth = skp->GetTextWidth(contactBase, strlen(contactBase));
		skp->Text((int)floor(ScreenWidth / 2 - stringWidth / 2), TextY0 - 5 * LineSpacing, contactBase, strlen(contactBase));
		yIndex += 1;

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
		int metM = (int)floor(metAbs / 60.0);
		int metS = (int)floor((metAbs - metM * 60.0));

		sprintf(cbuf, "T%s%02i %02i", signChar, metM, metS);
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
			sprintf(cbuf, "Retrosequence time:");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  %02i %02i", 4, 44);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		if (attitudeFuelAuto && (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) && fuel_manual != NULL) // there's a pesky crash if one checks for propellant level of a non-created propellant source
		{
			sprintf(cbuf, "Manual fuel: %.1f %%", GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_MAN * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Auto fuel: %.1f %%", GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_AUTO * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if ((VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) && fuel_manual != NULL)
		{
			sprintf(cbuf, "Manual fuel: %.1f %%", GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_MAN * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Auto fuel: %.1f %%", GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_AUTO * 100.0);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		// Sensor dials

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

		// Descent rate
		VECTOR3 groundSpeedVec;
		GetAirspeedVector(FRAME_HORIZON, groundSpeedVec);
		double descRate = groundSpeedVec.y;
		if (descRate > 0.0) descRate = 0.0;
		if (descRate < -43.0) descRate = -43.0;
		if (GetAltitude() > 5e4) descRate = 0.0;
		DWORD previousTextColour; // red
		previousTextColour = skp->SetTextColor(0x0000FF); // red
		if (descRate > -9.754) skp->SetTextColor(0x00FF00); // green
		sprintf(cbuf, "Descent: %.1f m/s", -descRate);
		skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;
		skp->SetTextColor(previousTextColour);

		// Altitude
		double airAltitude = GetAltitude();
		if (airAltitude > 30480.0) airAltitude = 30480.0;
		previousTextColour = skp->SetTextColor(0x00FF00); // green
		if (airAltitude < 3048.0) skp->SetTextColor(0x0000FF); // red
		else if (airAltitude < 6401.0) skp->SetTextColor(0x00FFFF); // yellow
		sprintf(cbuf, "Altitude: %.0f m", floor(airAltitude / 10.0) * 10.0);
		skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;
		skp->SetTextColor(previousTextColour);


		// Next event
		if (autoPilot && VesselStatus == LAUNCH)
		{
			if (AutopilotStatus == AUTOLAUNCH)
			{
				sprintf(cbuf, "Awaiting cutoff");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "Separate escape tower in %.2f s", 0.5 + boosterShutdownTime - simt);
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
		else if (autoPilot && VesselStatus == TOWERSEP)
		{
			sprintf(cbuf, "Separate capsule in %.2f s", 9.5 + boosterShutdownTime - simt);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else
		{
			WriteHUDAutoFlightReentry(skp, simt, &yIndex, cbuf);
		}

		// Launch target pitch and yaw
		if (autoPilot && VesselStatus == LAUNCH)
		{
			sprintf(cbuf, "Current pitch:");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  %.2f\u00B0", GetPitch() * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (VesselStatus == LAUNCH)
		{
			VECTOR3 angVel;
			GetAngularVel(angVel);

			sprintf(cbuf, "Pitch rate: %.2f\u00B0/s", angVel.x * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Yaw rate: %.2f\u00B0/s", angVel.y * DEG);
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

	yIndex = 0;

	if (showInfoOnHud == 1) // Show Mercury dashboard left row of status lights. This is in the end to not affect right side font colour
	{
		// Remember BBGGRR, not RRGGBB
		const DWORD Gray = 0xB0B0B0;
		const DWORD Red = 0x0000FF;
		const DWORD Green = 0x00FF00;

		// ABORT
		if (abort) skp->SetTextColor(Red);
		else skp->SetTextColor(Gray);
		sprintf(cbuf, "ABORT");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// JETT TOWER
		if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == TOWERSEP) skp->SetTextColor(Green); // I don't implement any failures of tower sep, so don't implement any failure light. This may change when fuses are added
		else skp->SetTextColor(Gray);
		sprintf(cbuf, "JETT TOWER");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// SEP CAPSULE
		if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORT || VesselStatus == ABORTNORETRO) skp->SetTextColor(Green);
		else if (boosterShutdownTime == 0.0) skp->SetTextColor(Gray); // time between shutdown and sep is red, so gray if not shutdown yet
		else skp->SetTextColor(Red);
		sprintf(cbuf, "SEP CAPSULE");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// RETRO SEQ
		if (engageRetro && VesselStatus == FLIGHT) skp->SetTextColor(Green); // Debug. Not sure if this is correct. Check with old addon. Does light turn off after retrosep, or stay green?
		else if (retroStartTime == 0.0) skp->SetTextColor(Gray);
		else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Green);
		else skp->SetTextColor(Red);
		sprintf(cbuf, "RETRO SEQ");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// RETRO ATT
		double currP = GetPitch() + pitchOffset;
		double currY = GetSlipAngle() + yawOffset;
		if (engageRetro && abs(currP + 34.0 * RAD) < 15.0 * RAD && abs(normangle(currY + PI)) < 15.0 * RAD) skp->SetTextColor(Green); // within limits
		else if (retroStartTime == 0.0) skp->SetTextColor(Gray); // haven't engaged retro
		else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Green);
		else skp->SetTextColor(Red);
		sprintf(cbuf, "RETRO ATT");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// FIRE RETRO
		if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Gray);
		else if ((retroStartTime != 0.0 && retroStartTime - 10.0 < simt && (!retroCoverSeparated[2] || FailureMode == RETROSTUCKOFF)) || (!engageRetro && (retroCoverSeparated[0] || retroCoverSeparated[1] || retroCoverSeparated[2]))) skp->SetTextColor(Red);
		else if (retroStartTime != 0.0 && retroStartTime - 10.0 > simt) skp->SetTextColor(Green);
		else if (retroStartTime == 0.0) skp->SetTextColor(Gray);
		else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Green);
		else skp->SetTextColor(Red);
		sprintf(cbuf, "FIRE RETRO");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// JETT RETRO
		if (retroStartTime != 0.0 && simt - retroStartTime > 58.0 && VesselStatus == FLIGHT) skp->SetTextColor(Red);
		else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Green);
		else if (retroStartTime == 0.0) skp->SetTextColor(Gray);
		else skp->SetTextColor(Red);
		sprintf(cbuf, "JETT RETRO");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// RETRACT SCOPE
		if (PeriscopeStatus != P_CLOSED && (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) && simt - retroStartTime > (60.0 + 40.0)) skp->SetTextColor(Red);
		else if (PeriscopeStatus != P_CLOSED && (VesselStatus == FLIGHT || VesselStatus == REENTRYNODROGUE || (VesselStatus == LAUNCH && GroundContact()))) skp->SetTextColor(Green);
		else if (PeriscopeStatus == P_CLOSED) skp->SetTextColor(Gray);
		else skp->SetTextColor(Red);
		sprintf(cbuf, "RETRACT SCOPE");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// .05 G
		if (FailureMode != LOWGDEACTIVE && VesselStatus == REENTRY && vesselAcceleration > 0.05 * G) skp->SetTextColor(Green);
		else if (AutopilotStatus == LOWG) skp->SetTextColor(Green);
		else skp->SetTextColor(Gray);
		sprintf(cbuf, ".05 G");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// MAIN
		if (mainChuteDeployed && simt > mainChuteDeployTime + 2.0) skp->SetTextColor(Green);
		else if (!mainChuteDeployed) skp->SetTextColor(Gray);
		else skp->SetTextColor(Red);
		sprintf(cbuf, "MAIN");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;

		// LANDING BAG
		if (landingBagDeployed) skp->SetTextColor(Green);
		else if (mainChuteDeployed && simt - mainChuteDeployTime > 10.0) skp->SetTextColor(Red);
		else if (!landingBagDeployed) skp->SetTextColor(Gray);
		else skp->SetTextColor(Red);
		sprintf(cbuf, "LANDING BAG");
		skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
		yIndex += 1;
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
			LoadCapsule(cbuf + 8); // don't send space after "CAPSULE "
		}
		else if (!_strnicmp(cbuf, "CASTORFIRETIME", 14))
		{
			sscanf_s(cbuf + 14, "%lf", &castorFireTime);
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
		else if (!_strnicmp(cbuf, "LOGERROR", 8))
		{
			logError = true;
		}
		else if (!_strnicmp(cbuf, "REPLACETEXTURE", 14))
		{
			scenarioTextureUserEnable = true; // Load textures
			ReadCapsuleTextureReplacement(cbuf + 15); // don't send space char after REPLACETEXTURE
		}
		else ParseScenarioLineEx(cbuf, status);
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

	oapiWriteScenario_int(scn, "FAILURE", FailureMode);

	if (difficultyHard)
	{
		oapiWriteScenario_string(scn, "HARDMODE", "");
	}

	int i = 0;
	while (scenarioTextureUserEnable && i < numTextures)
	{
		oapiWriteScenario_string(scn, "REPLACETEXTURE", textureString[i]);
		i++;
	}
}


inline bool ProjectMercury::ReadRocketTextureReplacement(const char* flag, const char* cbuf, int len)
{
	// Little Joe
	if (strcmp(flag, "LITTLEJ") == 0)
	{
		if (ReadTextureString(cbuf, len, texLittlej.file, &texLittlej.w, &texLittlej.h))
			oapiWriteLogV("New Littlej texture: >%s<, w: %i, h: %i", texLittlej.file, texLittlej.w, texLittlej.h);
		else oapiWriteLog("Failed to load Littlej texture");
	}
	else if (strcmp(flag, "BAIXXO") == 0)
	{
		if (ReadTextureString(cbuf, len, texBaixxo.file, &texBaixxo.w, &texBaixxo.h))
			oapiWriteLogV("New Baixxo texture: >%s<, w: %i, h: %i", texBaixxo.file, texBaixxo.w, texBaixxo.h);
		else oapiWriteLog("Failed to load Baixxo texture");
	}
	else if (strcmp(flag, "JOEFINS") == 0)
	{
		if (ReadTextureString(cbuf, len, texJoefins.file, &texJoefins.w, &texJoefins.h))
			oapiWriteLogV("New JoeFins texture: >%s<, w: %i, h: %i", texJoefins.file, texJoefins.w, texJoefins.h);
		else oapiWriteLog("Failed to load JoeFins texture");
	}
	else if (strcmp(flag, "FINFRON") == 0)
	{
		if (ReadTextureString(cbuf, len, texFinfron.file, &texFinfron.w, &texFinfron.h))
			oapiWriteLogV("New FinFron texture: >%s<, w: %i, h: %i", texFinfron.file, texFinfron.w, texFinfron.h);
		else oapiWriteLog("Failed to load FinFron texture");
	}
	else if (strcmp(flag, "BODYNUM") == 0)
	{
		if (ReadTextureString(cbuf, len, texBodynum.file, &texBodynum.w, &texBodynum.h))
			oapiWriteLogV("New Bodynum texture: >%s<, w: %i, h: %i", texBodynum.file, texBodynum.w, texBodynum.h);
		else oapiWriteLog("Failed to load Bodynum texture");
	}
	else return false;

	return true; // did not read else, and therefore success
}

void ProjectMercury::LoadRocketTextureReplacement(void)
{
	int grpLittlej = 1, grpBaixxo = 2, grpJoefins = 3, grpFinfron = 4, grpBodynum = 5;

	if (strlen(texLittlej.file) != 0)
	{
		LoadTextureFile(texLittlej, "Littlej", booster, grpLittlej);
	}
	if (strlen(texBaixxo.file) != 0)
	{
		LoadTextureFile(texBaixxo, "Baixxo", booster, grpBaixxo);
	}
	if (strlen(texJoefins.file) != 0)
	{
		LoadTextureFile(texJoefins, "JoeFins", booster, grpJoefins);
	}
	if (strlen(texFinfron.file) != 0)
	{
		LoadTextureFile(texFinfron, "FinFron", booster, grpFinfron);
	}
	if (strlen(texBodynum.file) != 0)
	{
		LoadTextureFile(texBodynum, "BodyNum", booster, grpBodynum);
	}
}

// ==============================================================
// Custom Vessel Functions
// ==============================================================

void ProjectMercury::CreateAirfoilsLittleJoe(void)
{
	ClearAirfoilDefinitions(); // delete previous airfoils
	SetRotDrag(_V(0.25, 0.25, 0.1)); // from BrianJ's Falcon 9
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 2.0), vliftLittleJoe, NULL, 1.893, 2.81, 1.0); // 19670022650 page 4. The centre of pressure is estimated, bases on Little Joe II (Apollo)
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 2.0), hliftLittleJoe, NULL, 1.893, 2.81, 1.0); // spherical symmetric
}

double ProjectMercury::EmptyMass(void)
{
	double mass;

	if (VesselStatus == TOWERSEP)
		mass = MERCURY_MASS + LITTLEJOE_TOT_DRY_MASS;
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
		mass = MERCURY_MASS + LITTLEJOE_TOT_DRY_MASS + ABORT_MASS;

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

	// Angular velocity
	if (abs(angVel.x * DEG) > 10.0)
	{
		abortConditionsMet = true;
		oapiWriteLogV("Abort due to error in pitch rate at T+%.1f. It was %.1f deg/s", met, angVel.x * DEG);
	}
	if (abs(angVel.y * DEG) > 10.0)
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
		SeparateLittleJoe();
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
		if (boilerplateMission)
		{
			SeparateLittleJoe();
			SeparateRingsAndAdapters(_V(0.0, -1.0, 0.0)); // Adapters and rings
			SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);
			SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);
			VesselStatus = REENTRY;
			CGshifted = false;
		}
		else
		{
			SeparateLittleJoe();
			SeparateRingsAndAdapters(_V(0.0, -1.0, 0.0)); // Adapters and rings

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
		abortDamping = true; // abort. So that we can enable abort damping
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
	DestabiliserStatus = P_OPENING; // is held down by escape tower
	towerJettisoned = true;
}

void ProjectMercury::SeparateLittleJoe(void)
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
		vs.vrot.x = LITTLEJOE_LENGTH / 2.0 + heightOverGround;
	}
	else
	{
		VECTOR3 relativeOffset;
		Local2Rel(redstoneOffset, vs.rpos);
		GlobalRot(redstoneDirection, relativeOffset);
		vs.rvel += relativeOffset * redstoneVelocity;
		inFlightAbort = true;
	}

	if (GetThrusterLevel(th_castor[0]) != 0.0 || GetThrusterLevel(th_castor[1]) != 0.0) // 19670028606 page 87, keep engine running if less than T+30
	{
		vs.flag = VS_THRUSTLIST;
		vs.thruster = &thrust;
		vs.nthruster = 1;
		vs.thruster->idx = 0;
		vs.thruster->level = 1.0;
		oapiWriteLog("Keep Little Joe burning");
	}

	strcpy(name, GetName());
	strcat(name, " booster");
	int k = 0;
	while (oapiGetVesselByName(name) != NULL)
	{
		k += 1;
		strcat(name, "1");
	}

	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "Vessels\\ProjectMercury\\LittleJoeBooster", &vs);;
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;

	// The thrusterlist spec doesnt seem to work, sadly
	VESSEL* v = oapiGetVesselInterface(createdVessel[stuffCreated - 1]);
	double levl = GetPropellantMass(castor_propellant[1]);
	v->SetPropellantMass(v->GetPropellantHandleByIndex(0), levl);
	if (GetThrusterLevel(th_castor[0]) != 0.0 || GetThrusterLevel(th_castor[1]) != 0.0)
	{
		v->SetThrusterGroupLevel(THGROUP_MAIN, 1.0);
	}

	DelThruster(th_castor[0]);
	DelThruster(th_castor[1]);
	DelThruster(th_castor[2]);
	DelThruster(th_castor[3]);
	DelThruster(th_recruit[0]);
	DelThruster(th_recruit[1]);
	DelThruster(th_recruit[2]);
	DelThruster(th_recruit[3]);
	DelPropellantResource(castor_propellant[0]);
	DelPropellantResource(castor_propellant[1]);
	DelPropellantResource(castor_propellant[2]);
	DelPropellantResource(castor_propellant[3]);
	DelPropellantResource(recruit_propellant[0]);
	DelPropellantResource(recruit_propellant[1]);
	DelPropellantResource(recruit_propellant[2]);
	DelPropellantResource(recruit_propellant[3]);
	DelMesh(Booster);
	ClearControlSurfaceDefinitions();

	boosterSeparated = true;
}

void ProjectMercury::SeparateRingsAndAdapters(VECTOR3 redstoneDirection)
{
	VESSELSTATUS2 vs;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	char name[256];
	VECTOR3 vel = _V(0, .95, 0);
	VECTOR3 relativeOffset1;
	VECTOR3 addOfs = _V(0.0, 0.0, -0.65);
	GetStatusEx(&vs);
	Local2Rel(OFS_ADAPTCOVER1 + addOfs, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset1);
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
	Local2Rel(OFS_ADAPTCOVER2 + addOfs, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset2);
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
	Local2Rel(OFS_ADAPTCOVER3 + addOfs, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset3);
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
	Local2Rel(OFS_ADAPTRING1 + addOfs, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset4);
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
	Local2Rel(OFS_ADAPTRING2 + addOfs, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset5);
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
	Local2Rel(OFS_ADAPTRING3 + addOfs, vs.rpos);
	GlobalRot(redstoneDirection, relativeOffset6);
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

	if (!boosterSeparated && (VesselStatus == LAUNCH || VesselStatus == TOWERSEP))
	{
		if (Booster != NULL && GetMeshVisibilityMode(Booster) != mode) SetMeshVisibilityMode(Booster, mode);

		if (Adaptcover1 != NULL && GetMeshVisibilityMode(Adaptcover1) != mode) SetMeshVisibilityMode(Adaptcover1, mode);

		if (Adaptcover2 != NULL && GetMeshVisibilityMode(Adaptcover2) != mode) SetMeshVisibilityMode(Adaptcover2, mode);

		if (Adaptcover3 != NULL && GetMeshVisibilityMode(Adaptcover3) != mode) SetMeshVisibilityMode(Adaptcover3, mode);

		if (Adaptring1 != NULL && GetMeshVisibilityMode(Adaptring1) != mode) SetMeshVisibilityMode(Adaptring1, mode);

		if (Adaptring2 != NULL && GetMeshVisibilityMode(Adaptring2) != mode) SetMeshVisibilityMode(Adaptring2, mode);

		if (Adaptring3 != NULL && GetMeshVisibilityMode(Adaptring3) != mode) SetMeshVisibilityMode(Adaptring3, mode);
	}

	if (!towerJettisoned && (VesselStatus == LAUNCH || VesselStatus == ABORT || VesselStatus == ABORTNORETRO))
	{
		if (Tower != NULL && GetMeshVisibilityMode(Tower) != mode) SetMeshVisibilityMode(Tower, mode);
	}

	if (Capsule != NULL && GetMeshVisibilityMode(Capsule) != mode) SetMeshVisibilityMode(Capsule, mode);

	if (VesselStatus != REENTRYNODROGUE && !drogueSeparated && Antennahouse != NULL && GetMeshVisibilityMode(Antennahouse) != mode) SetMeshVisibilityMode(Antennahouse, mode);

	oapiWriteLog("End scene visibility");
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
	//oapiCloseFile(jetVaneLogFile, FILE_OUT);

	if (vessel) delete (ProjectMercury*)vessel;
}

void ProjectMercury::vliftLittleJoe(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	// The papers on Little Joe aerodynamics are strangely classified. The Little Joe II (Apollo) aerodynamics are luckily not, so we use them here.

	// 19680013484 page 73, the pitching moment is approximately linear, with slope 0.16 * aoa(deg).
	// As the Little Joe was very stable, following the wind, we only care about this small AoA regime anyway.
	*cm = -0.16 * aoa * DEG;

	// Similarly, page 76, we see that the lift coefficient is approximately linear, with slope 0.26 * aoa(deg).
	*cl = -0.26 * aoa * DEG;
	//sprintf(oapiDebugString(), "aoa: %.1f, cm: %.2f, cl: %.2f", aoa, cm, cl);

	// Finally, the drag coefficient varies greatly both with AoA and mach, but as with the Mercury capsule, we assume AoA zero, and create only a mach dependence.
	if (M > 6.8) M = 6.8; // max of this interpolation
	if (M < 0.0) M = 0.0; // min of this interpolation
	*cd = -2.6154644599e-06 * pow(M, 15) + 1.3925595645e-04 * pow(M, 14) + -3.3479687526e-03 * pow(M, 13) + 4.8015136575e-02 * pow(M, 12) + -4.5705296374e-01 * pow(M, 11) + 3.0391668768e+00 * pow(M, 10) + -1.4462570476e+01 * pow(M, 9) + 4.9598487277e+01 * pow(M, 8) + -1.2169523577e+02 * pow(M, 7) + 2.0891394075e+02 * pow(M, 6) + -2.4064585332e+02 * pow(M, 5) + 1.7341363369e+02 * pow(M, 4) + -7.0416769088e+01 * pow(M, 3) + 1.5178741115e+01 * pow(M, 2) + -2.3548590922e+00 * pow(M, 1) + 8.8797126179e-01 * pow(M, 0);
	//static const int numMachEntries = 15;
	//static const  double mach[numMachEntries] = {
	//	0.056,	0.1,	0.2,	0.3,	0.5,	0.7,	0.8,	0.9,	0.95,	1.0,	1.2,	1.5,	1.57,	1.8,	2.16
	//};
	//static const double cdp[numMachEntries] = { // drag coeff at 0 AoA (tip first) for different mach numbers. Note that there is a possible error for the drag data for lowest mach, as it is roughly half of the later data. So I multiplied by two
	//	0.96,	0.96,	0.82,	0.60,	0.62,	0.72,	0.77,	0.96,	1.20,	1.42,	1.40,	1.22,	1.17,	1.07,	0.96
	//};

	//int i = 0;
	//while (i < numMachEntries && M > mach[i])
	//{
	//	i++;
	//}

	//if (i == numMachEntries)
	//{
	//	*cd = cdp[numMachEntries - 1];
	//}
	//else if (i == 0)
	//{
	//	*cd = cdp[0];
	//}
	//else
	//{
	//	*cd = cdp[i - 1] + (cdp[i] - cdp[i - 1]) * (M - mach[i - 1]) / (mach[i] - mach[i - 1]);
	//}

	*cd *= 0.5;
}

void ProjectMercury::hliftLittleJoe(VESSEL* v, double beta, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	// The papers on Little Joe aerodynamics are strangely classified. The Little Joe II (Apollo) aerodynamics are luckily not, so we use them here.

	// 19680013484 page 73, the pitching moment is approximately linear, with slope 0.16 * aoa(deg).
	// As the Little Joe was very stable, following the wind, we only care about this small AoA regime anyway.
	*cm = 0.0; // we're in hlift

	// Similarly, page 76, we see that the lift coefficient is approximately linear, with slope 0.26 * aoa(deg).
	*cl = -0.26 * beta * DEG;

	// Finally, the drag coefficient varies greatly both with AoA and mach, but as with the Mercury capsule, we assume AoA zero, and create only a mach dependence.
	if (M > 6.8) M = 6.8; // max of this interpolation
	if (M < 0.0) M = 0.0; // min of this interpolation
	*cd = -2.6154644599e-06 * pow(M, 15) + 1.3925595645e-04 * pow(M, 14) + -3.3479687526e-03 * pow(M, 13) + 4.8015136575e-02 * pow(M, 12) + -4.5705296374e-01 * pow(M, 11) + 3.0391668768e+00 * pow(M, 10) + -1.4462570476e+01 * pow(M, 9) + 4.9598487277e+01 * pow(M, 8) + -1.2169523577e+02 * pow(M, 7) + 2.0891394075e+02 * pow(M, 6) + -2.4064585332e+02 * pow(M, 5) + 1.7341363369e+02 * pow(M, 4) + -7.0416769088e+01 * pow(M, 3) + 1.5178741115e+01 * pow(M, 2) + -2.3548590922e+00 * pow(M, 1) + 8.8797126179e-01 * pow(M, 0);
	//static const int numMachEntries = 15;
	//static const  double mach[numMachEntries] = {
	//	0.056,	0.1,	0.2,	0.3,	0.5,	0.7,	0.8,	0.9,	0.95,	1.0,	1.2,	1.5,	1.57,	1.8,	2.16
	//};
	//static const double cdp[numMachEntries] = { // drag coeff at 0 AoA (tip first) for different mach numbers. Note that there is a possible error for the drag data for lowest mach, as it is roughly half of the later data. So I multiplied by two
	//	0.96,	0.96,	0.82,	0.60,	0.62,	0.72,	0.77,	0.96,	1.20,	1.42,	1.40,	1.22,	1.17,	1.07,	0.96
	//};

	//int i = 0;
	//while (i < numMachEntries && M > mach[i])
	//{
	//	i++;
	//}

	//if (i == numMachEntries)
	//{
	//	*cd = cdp[numMachEntries - 1];
	//}
	//else if (i == 0)
	//{
	//	*cd = cdp[0];
	//}
	//else
	//{
	//	*cd = cdp[i - 1] + (cdp[i] - cdp[i - 1]) * (M - mach[i - 1]) / (mach[i] - mach[i - 1]);
	//}

	*cd *= 0.5;
}

inline void ProjectMercury::GetPanelRetroTimes(double met, int* rH, int* rM, int* rS, int* dH, int* dM, int* dS)
{
	// No retrosequence on Little Joe
	*rH = 0;
	*rM = 0;
	*rS = 0;
	*dH = 0;
	*dM = 0;
	*dS = 0;
}