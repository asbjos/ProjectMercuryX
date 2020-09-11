#pragma once

#include <time.h> // for seed in random function
#include "VirtualCockpit.h"
#include "ProjectMercuryGeneric.h"

// ==============================================================
//				Header file for Mercury Capsule.
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

bool ProjectMercury::clbkLoadGenericCockpit(void)
{
	periscope = false;
	rocketCam = false;
	SetCameraDefaultDirection(_V(0, 0, 1));
	//SetCameraOffset(_V(0.0, 0.0, 0.0));
	SetCameraOffset(_V(0,0,0));
	oapiCameraSetCockpitDir(0, 0); // Rotate camera to desired direction
	SetCameraRotationRange(0.8 * PI, 0.8 * PI, 0.4 * PI, 0.4 * PI); // SetCameraRotationRange-values are the default ones (page 524 in Orbitersdk\doc\API_Reference.pdf
	oapiCameraSetAperture(oldFOV);
	SetCameraSceneVisibility(MESHVIS_EXTERNAL);

	DelMesh(PeriscopeFilter); // So that it doesn't appear in generic cockpit

	return true;
}

void ProjectMercury::clbkRenderHUD(int mode, const HUDPAINTSPEC* hps, SURFHANDLE hDefaultTex)
{
	OBJHANDLE planetRef = oapiGetGbodyByName("Earth");
	if (planetRef == NULL) planetRef = GetSurfaceRef();
	bool radioContact = InRadioContact(planetRef);

	if (oapiCockpitMode() == COCKPIT_PANELS)
	{
		// It seems like I have to do something here, or else Orbiter crashes when returning to generic HUD. So call a stupid function or something.
		double uselessValue = GetPitch();
		uselessValue *= GGRAV;
		return; // i.e. supress HUD. Else let HUD show
	}
	else
	{
		if (!radioContact)
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

inline void ProjectMercury::DisableAutopilot(bool turnOff)
{
	if (turnOff)
	{
		autoPilot = false;
		//abortDamping = false;
	}

	if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) // RCS exists
	{
		if (!rcsExists) return; // there are no RCS to disable

		SetThrusterLevel(thruster_auto_py[0], 0.0);
		SetThrusterLevel(thruster_auto_py[1], 0.0);
		SetThrusterLevel(thruster_auto_py[2], 0.0);
		SetThrusterLevel(thruster_auto_py[3], 0.0);
		SetThrusterLevel(thruster_auto_roll[0], 0.0);
		SetThrusterLevel(thruster_auto_roll[1], 0.0);

		SetThrusterLevel(thruster_man_py[0], 0.0);
		SetThrusterLevel(thruster_man_py[1], 0.0);
		SetThrusterLevel(thruster_man_py[2], 0.0);
		SetThrusterLevel(thruster_man_py[3], 0.0);
		SetThrusterLevel(thruster_man_roll[0], 0.0);
		SetThrusterLevel(thruster_man_roll[1], 0.0);

		SetThrusterLevel(thruster_auto_py_1lb[0], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[1], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[2], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[3], 0.0);
		SetThrusterLevel(thruster_auto_roll_1lb[0], 0.0);
		SetThrusterLevel(thruster_auto_roll_1lb[1], 0.0);
	}
}

// For documentation of ASCS, see twombly1962.pdf (doi:10.1002/j.2161-4296.1962.tb02524.x , "The Mercury Capsule Attitude Control System").
// In summary (see phase diagram page 7 in pdf), limits are (dev = deviation):
// dev < 1			-> rate beneath 0.5 deg/s gives no applied torque (stable)
// 1 < dev < 3		-> rate 0.5 deg/s
// 3 < dev < 5.5	-> rate 2 deg/s
// 5.5 < dev		-> rate 10 deg/s

void ProjectMercury::AuxDampingAuto(bool highThrust) // return number of active engines
{
	THRUSTER_HANDLE py0, py1, py2, py3, roll0, roll1;

	if (!rcsExists) return; // no RCS, so not damping

	if (highThrust)
	{
		py0 = thruster_auto_py[0];
		py1 = thruster_auto_py[1];
		py2 = thruster_auto_py[2];
		py3 = thruster_auto_py[3];
		roll0 = thruster_auto_roll[0];
		roll1 = thruster_auto_roll[1];
	}
	else
	{
		py0 = thruster_auto_py_1lb[0];
		py1 = thruster_auto_py_1lb[1];
		py2 = thruster_auto_py_1lb[2];
		py3 = thruster_auto_py_1lb[3];
		roll0 = thruster_auto_roll_1lb[0];
		roll1 = thruster_auto_roll_1lb[1];
	}

	VECTOR3 angVel;
	GetAngularVel(angVel);

	const double rateLimit = 0.5 * RAD;

	if (tHandlePitchPushed)
	{
		if (angVel.x > rateLimit)
			SetThrusterLevel(py1, 1.0);
		else
			SetThrusterLevel(py1, 0.0);

		if (angVel.x < -rateLimit)
			SetThrusterLevel(py0, 1.0);
		else
			SetThrusterLevel(py0, 0.0);
	}
	else
	{
		SetThrusterLevel(thruster_auto_py[0], 0.0);
		SetThrusterLevel(thruster_auto_py[1], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[0], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[1], 0.0);
	}
	
	if (tHandleYawPushed)
	{
		if (angVel.y > rateLimit)
			SetThrusterLevel(py2, 1.0);
		else
			SetThrusterLevel(py2, 0.0);

		if (angVel.y < -rateLimit)
			SetThrusterLevel(py3, 1.0);
		else
			SetThrusterLevel(py3, 0.0);
	}
	else
	{
		SetThrusterLevel(thruster_auto_py[2], 0.0);
		SetThrusterLevel(thruster_auto_py[3], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[2], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[3], 0.0);
	}
	
	if (tHandleRollPushed)
	{
		if (angVel.z > rateLimit)
			SetThrusterLevel(roll1, 1.0);
		else
			SetThrusterLevel(roll1, 0.0);

		if (angVel.z < -rateLimit)
			SetThrusterLevel(roll0, 1.0);
		else
			SetThrusterLevel(roll0, 0.0);
	}
	else
	{
		SetThrusterLevel(thruster_auto_roll[0], 0.0);
		SetThrusterLevel(thruster_auto_roll[1], 0.0);
		SetThrusterLevel(thruster_auto_roll_1lb[0], 0.0);
		SetThrusterLevel(thruster_auto_roll_1lb[1], 0.0);
	}
}

void ProjectMercury::RetroAttitudeAuto(double simt, double simdt, bool highTorque) // finished
{
	//bool highTorque = false;

	if (oapiGetTimeAcceleration() < 15.0) // for high time acc things get unstable
	{
		bool result[3] = { false, false, false }; // If you want to log which attitudes are at target

		if (highTorque)
		{
			result[0] = SetPitchAuto(-34.0, highTorque);
		}
		else
		{
			// Remember that highTorque here always will be false.
			// But no. On MA-6, with lowTorque thruster failed, ASCS went into highTorque when passing more than 20 deg off-course
			if (attitudeHold14deg) result[0] = SetPitchAuto(-14.5, highTorque); // for periscope centering
			else result[0] = SetPitchAuto(-34.0, highTorque);
		}

		result[1] = SetYawAuto(highTorque);
		result[2] = SetRollAuto(highTorque);

		if (result[0] && result[1] && result[2] && AutopilotStatus == TURNAROUND) AutopilotStatus = ORBITATTITUDE;
	}
	else // disable autopilot if time acc > 15x. This is especially needed for when flying two capsules, and forgetting that one has active attitude control.
	{
		DisableAutopilot(true); // eliminate stuck thrusters
	}
}

bool ProjectMercury::SetPitchAuto(double targetPitch, bool highTorque)
{
	VECTOR3 angVel;
	GetAngularVel(angVel);
	double pitch = normangle(GetPitch() + pitchOffset);
	pitch -= targetPitch * RAD; // target pitch is -34 degrees, so that would give 0 deg if correct pitch
	double autoVel = 0.0;
	int sign = (pitch > 0.0) - (pitch < 0.0);
	double lvlH, lvlL;
	if (pitch > 0.0)
	{
		lvlH = 1.0;
		lvlL = 0.0;
	}
	else // inverted
	{
		lvlH = 0.0;
		lvlL = 1.0;
	}

	bool returnValue = false;

	// If hightorque, the regular ASCS functions, reducing attitude to +- 1 deg and +-0.5 deg/s.
	// If lowTorque, the ASCS should be within +- 5.5 deg, and then moves at +- 0.05 deg/s

	// If in lowTorque, but difference is more than 20 deg, highTorque is initiated (source: error on MA-6, 
	// where lowTorque yaw thruster failed, and Glenn commented that it thus drifted to "about 20 deg", before
	// highTorque pit him back on track)
	// See ma-6-results.pdf page 169 (voice transcript at 01 33 52), and page 79 of same document .
	if (!highTorque && abs(pitch) > 20.0 * RAD) AutopilotStatus = TURNAROUND;

	if (highTorque)
	{
		if (abs(pitch) > 1.0 * RAD)
		{
			if (abs(pitch) > 5.5 * RAD)
				autoVel = 10.0 * RAD;
			else if (abs(pitch) > 3.0 * RAD)
				autoVel = 2.0 * RAD;
			else
				autoVel = 0.5 * RAD;

			if (sign * angVel.x > 0.0) // pitching away from target
			{
				SetThrusterLevel(thruster_auto_py[0], lvlL);
				SetThrusterLevel(thruster_auto_py[1], lvlH);
			}
			else if (abs(angVel.x) < autoVel) // pitching towards target, and too slow
			{
				SetThrusterLevel(thruster_auto_py[0], lvlL);
				SetThrusterLevel(thruster_auto_py[1], lvlH);
			}
			else // coast
			{
				SetThrusterLevel(thruster_auto_py[0], 0.0);
				SetThrusterLevel(thruster_auto_py[1], 0.0);
			}
		}
		else // Damp
		{
			if (angVel.x > 0.5 * RAD)
			{
				SetThrusterLevel(thruster_auto_py[0], lvlL);
				SetThrusterLevel(thruster_auto_py[1], lvlH);
			}
			else if (angVel.x < -0.5 * RAD)
			{
				SetThrusterLevel(thruster_auto_py[0], lvlH);
				SetThrusterLevel(thruster_auto_py[1], lvlL);
			}
			else
			{
				SetThrusterLevel(thruster_auto_py[0], 0.0);
				SetThrusterLevel(thruster_auto_py[1], 0.0);
				returnValue = true; // successfully attained desired attitude, and within rate limits
			}
		}
	}
	else // low torque
	{
		if (abs(pitch) > 5.5 * RAD)
		{
			if (sign * angVel.x > 0.0) // pitching away from target
			{
				SetThrusterLevel(thruster_auto_py_1lb[0], lvlL);
				SetThrusterLevel(thruster_auto_py_1lb[1], lvlH);
			}
			else if (abs(angVel.x) < 0.05 * RAD) // pitching towards target, and too slow
			{
				SetThrusterLevel(thruster_auto_py_1lb[0], lvlL);
				SetThrusterLevel(thruster_auto_py_1lb[1], lvlH);
			}
			else // coast
			{
				SetThrusterLevel(thruster_auto_py_1lb[0], 0.0);
				SetThrusterLevel(thruster_auto_py_1lb[1], 0.0);
			}
		}
		else // Damp
		{
			SetThrusterLevel(thruster_auto_py_1lb[0], 0.0);
			SetThrusterLevel(thruster_auto_py_1lb[1], 0.0);
			returnValue = true; // successfully attained desired attitude, and within rate limits
		}
	}
	

	if (!tHandlePitchPushed) // must be activated to function
	{
		SetThrusterLevel(thruster_auto_py[0], 0.0);
		SetThrusterLevel(thruster_auto_py[1], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[0], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[1], 0.0);
	}
	
	return returnValue; // default false
}

bool ProjectMercury::SetYawAuto(bool highTorque)
{
	VECTOR3 angVel;
	GetAngularVel(angVel);
	double yaw = normangle(GetSlipAngle() + yawOffset);
	double autoVel = 0.0;
	int sign = (yaw > 0.0) - (yaw < 0.0);
	double lvlH, lvlL;
	if (yaw > 0.0)
	{
		lvlH = 1.0;
		lvlL = 0.0;
	}
	else // inverted
	{
		lvlH = 0.0;
		lvlL = 1.0;
	}

	bool returnValue = false;

	// If in lowTorque, but difference is more than 20 deg, highTorque is initiated (source: error on MA-6, 
	// where lowTorque yaw thruster failed, and Glenn commented that it thus drifted to "about 20 deg", before
	// highTorque pit him back on track)
	// See ma-6-results.pdf page 169 (voice transcript at 01 33 52), and page 79 of same document .
	if (!highTorque && abs(yaw) < 160.0 * RAD) AutopilotStatus = TURNAROUND;

	if (highTorque)
	{
		if (abs(yaw) < 179.0 * RAD)
		{
			if (abs(yaw) < 174.5 * RAD)
				autoVel = 10.0 * RAD;
			else if (abs(yaw) < 177.0 * RAD)
				autoVel = 2.0 * RAD;
			else
				autoVel = 0.5 * RAD;

			if (sign * angVel.y > 0.0) // yawing away from target
			{
				SetThrusterLevel(thruster_auto_py[2], lvlH);
				SetThrusterLevel(thruster_auto_py[3], lvlL);
			}
			else if (abs(angVel.y) < autoVel) // yawing towards target, and too slow
			{
				SetThrusterLevel(thruster_auto_py[2], lvlH);
				SetThrusterLevel(thruster_auto_py[3], lvlL);
			}
			else // coast
			{
				SetThrusterLevel(thruster_auto_py[2], 0.0);
				SetThrusterLevel(thruster_auto_py[3], 0.0);
			}
		}
		else
		{
			if (angVel.y > 0.5 * RAD)
			{
				SetThrusterLevel(thruster_auto_py[2], lvlH); // RIGHT
				SetThrusterLevel(thruster_auto_py[3], lvlL); // LEFT
			}
			else if (angVel.y < -0.5 * RAD)
			{
				SetThrusterLevel(thruster_auto_py[2], lvlL); // RIGHT
				SetThrusterLevel(thruster_auto_py[3], lvlH); // LEFT
			}
			else
			{
				SetThrusterLevel(thruster_auto_py[2], 0.0); // RIGHT
				SetThrusterLevel(thruster_auto_py[3], 0.0); // LEFT
				returnValue = true; // success
			}
		}
	}
	else // Low torque.
	{
		if (abs(yaw) < 174.5 * RAD)
		{
			if (sign * angVel.y > 0.0) // yawing away from target
			{
				SetThrusterLevel(thruster_auto_py_1lb[2], lvlH);
				SetThrusterLevel(thruster_auto_py_1lb[3], lvlL);
			}
			else if (abs(angVel.y) < 0.05 * RAD) // yawing towards target, and too slow
			{
				SetThrusterLevel(thruster_auto_py_1lb[2], lvlH);
				SetThrusterLevel(thruster_auto_py_1lb[3], lvlL);
			}
			else // coast
			{
				SetThrusterLevel(thruster_auto_py_1lb[2], 0.0);
				SetThrusterLevel(thruster_auto_py_1lb[3], 0.0);
			}
		}
		else // Damp
		{
			SetThrusterLevel(thruster_auto_py_1lb[2], 0.0); // RIGHT
			SetThrusterLevel(thruster_auto_py_1lb[3], 0.0); // LEFT
			returnValue = true; // success
		}
	}
	

	if (!tHandleYawPushed) // must be activated to function
	{
		SetThrusterLevel(thruster_auto_py[2], 0.0); // RIGHT
		SetThrusterLevel(thruster_auto_py[3], 0.0); // LEFT
		SetThrusterLevel(thruster_auto_py_1lb[2], 0.0); // RIGHT
		SetThrusterLevel(thruster_auto_py_1lb[3], 0.0); // LEFT
	}

	return returnValue; // default false;
}

bool ProjectMercury::SetRollAuto(bool highTorque)
{
	VECTOR3 angVel;
	GetAngularVel(angVel);
	angVel.z *= -1.0; // WARNING: ORBITER IS LEFT-HANDED, SO WE HAVE TO FLIP THE SIGN FOR ANGVELZ
	double roll = normangle(GetBank() + rollOffset);
	double autoVel = 0.0;
	int sign = (roll > 0.0) - (roll < 0.0);
	double lvlH, lvlL;
	if (roll > 0.0)
	{
		lvlH = 1.0;
		lvlL = 0.0;
	}
	else // inverted
	{
		lvlH = 0.0;
		lvlL = 1.0;
	}

	bool returnValue = false;

	// If in lowTorque, but difference is more than 20 deg, highTorque is initiated (source: error on MA-6, 
	// where lowTorque yaw thruster failed, and Glenn commented that it thus drifted to "about 20 deg", before
	// highTorque pit him back on track)
	// See ma-6-results.pdf page 169 (voice transcript at 01 33 52), and page 79 of same document .
	if (!highTorque && abs(roll) > 20.0 * RAD) AutopilotStatus = TURNAROUND;

	if (highTorque)
	{
		if (abs(roll) > 0.5 * RAD)
		{
			if (abs(roll) > 5.5 * RAD)
				autoVel = 10.0 / 4.0 * RAD; // divided by 4 as roll is a fourth of the power of yaw/pitch
			else if (abs(roll) > 3.0 * RAD)
				autoVel = 2.0 / 4.0 * RAD; // divided by 4 as roll is a fourth of the power of yaw/pitch
			else
				autoVel = 0.5 * RAD;

			if (sign * angVel.z > 0.0) // rolling away from target
			{
				SetThrusterLevel(thruster_auto_roll[0], lvlH);
				SetThrusterLevel(thruster_auto_roll[1], lvlL);
			}
			else if (abs(angVel.z) < autoVel) // rolling towards target, and too slow
			{
				SetThrusterLevel(thruster_auto_roll[0], lvlH);
				SetThrusterLevel(thruster_auto_roll[1], lvlL);
			}
			else // coast
			{
				SetThrusterLevel(thruster_auto_roll[0], 0.0);
				SetThrusterLevel(thruster_auto_roll[1], 0.0);
			}
		}
		else
		{
			if (angVel.z > 0.5 * RAD)
			{
				SetThrusterLevel(thruster_auto_roll[0], lvlH);
				SetThrusterLevel(thruster_auto_roll[1], lvlL);
			}
			else if (angVel.z < -0.5 * RAD)
			{
				SetThrusterLevel(thruster_auto_roll[0], lvlL);
				SetThrusterLevel(thruster_auto_roll[1], lvlH);
			}
			else
			{
				SetThrusterLevel(thruster_auto_roll[0], 0.0);
				SetThrusterLevel(thruster_auto_roll[1], 0.0);
				returnValue = true; // success
			}
		}
	}
	else // Low torque
	{
		if (abs(roll) > 5.5 * RAD)
		{
			if (sign * angVel.z > 0.0) // rolling away from target
			{
				SetThrusterLevel(thruster_auto_roll_1lb[0], lvlH);
				SetThrusterLevel(thruster_auto_roll_1lb[1], lvlL);
			}
			else if (abs(angVel.z) < 0.05 * RAD) // rolling towards target, and too slow
			{
				SetThrusterLevel(thruster_auto_roll_1lb[0], lvlH);
				SetThrusterLevel(thruster_auto_roll_1lb[1], lvlL);
			}
			else // coast
			{
				SetThrusterLevel(thruster_auto_roll_1lb[0], 0.0);
				SetThrusterLevel(thruster_auto_roll_1lb[1], 0.0);
			}
		}
		else
		{
			SetThrusterLevel(thruster_auto_roll_1lb[0], 0.0);
			SetThrusterLevel(thruster_auto_roll_1lb[1], 0.0);
			returnValue = true; // success
		}
	}

	if (!tHandleRollPushed) // must be activated to function
	{
		SetThrusterLevel(thruster_auto_roll[0], 0.0);
		SetThrusterLevel(thruster_auto_roll[1], 0.0);
		SetThrusterLevel(thruster_auto_roll_1lb[0], 0.0);
		SetThrusterLevel(thruster_auto_roll_1lb[1], 0.0);
	}

	return returnValue; // default false
}

void ProjectMercury::ReentryAttitudeAuto(double simt, double simdt)
{
	bool highTorque = true; // always high torque ...

	if (oapiGetTimeAcceleration() >= 2.0) // ... unless in TimeAcc
	{
		highTorque = false;
	}

	if (oapiGetTimeAcceleration() < 15.0) // for high time acc things get unstable
	{
		if (suborbitalMission) // Do suborbital reentry attitude (40 deg pitch)
		{
			SetPitchAuto(40.0, highTorque);
			SetYawAuto(highTorque);
			SetRollAuto(highTorque);
		}
		else // Do orbital reentry attitude (1.5 degree pitch)
		{
			SetPitchAuto(1.5, highTorque);
			SetYawAuto(highTorque);
			SetRollAuto(highTorque);
		}
	}
	else
	{
		DisableAutopilot(false); // eliminate stuck thrusters
	}
}

void ProjectMercury::GRollAuto(double simt, double simdt)
{
	VECTOR3 angVel;
	GetAngularVel(angVel);

	// Roll between 10 and 12 deg/s
	if (angVel.z > 12.0 * RAD && tHandleRollPushed)
	{
		SetThrusterLevel(thruster_auto_roll[1], 1.0);
		SetThrusterLevel(thruster_auto_roll[0], 0.0); // ensure not stuck
	}
	else if (angVel.z < 10.0 * RAD && tHandleRollPushed)
	{
		SetThrusterLevel(thruster_auto_roll[0], 1.0);
		SetThrusterLevel(thruster_auto_roll[1], 0.0); // ensure not stuck
	}
	else
	{
		SetThrusterLevel(thruster_auto_roll[0], 0.0);
		SetThrusterLevel(thruster_auto_roll[1], 0.0);
	}

	// Dampen pitch and yaw to 2 deg/s each
	if (angVel.x > 2.0 * RAD && tHandlePitchPushed)
		SetThrusterLevel(thruster_auto_py[1], 1.0);
	else
		SetThrusterLevel(thruster_auto_py[1], 0.0);

	if (angVel.x < -2.0 * RAD && tHandlePitchPushed)
		SetThrusterLevel(thruster_auto_py[0], 1.0);
	else
		SetThrusterLevel(thruster_auto_py[0], 0.0);

	if (angVel.y > 2.0 * RAD && tHandleYawPushed)
		SetThrusterLevel(thruster_auto_py[2], 1.0);
	else
		SetThrusterLevel(thruster_auto_py[2], 0.0);

	if (angVel.y < -2.0 * RAD && tHandleYawPushed)
		SetThrusterLevel(thruster_auto_py[3], 1.0);
	else
		SetThrusterLevel(thruster_auto_py[3], 0.0);
}

inline void ProjectMercury::InitiateRetroSequence(void)
{
	engageRetro = true;
	if (switchRetroDelay == -1) retroStartTime = oapiGetSimTime() + 30.0; // retrosequence starts 30 sec before firing
	else if (switchRetroDelay == 1) retroStartTime = oapiGetSimTime(); // start retrosequence immediately
	//char cbuf[256];
	//sprintf(cbuf, "Retrosequence initiated at T+%.0f, simt: %.0f", retroStartTime - launchTime - 30.0, retroStartTime - 30.0);
	//oapiWriteLog(cbuf);

	AutopilotStatus = RETROATTITUDE;
	//autoPilot = true;
	attitudeHold14deg = false;
}

inline void ProjectMercury::FireRetroRocket(int rckNum)
{
	double currP = GetPitch() + pitchOffset;
	double currY = GetSlipAngle() + yawOffset;
	double currR = GetBank() + rollOffset;

	if (FailureMode == RETROSTUCKOFF && retroErrorNum == rckNum)
	{
		// Do nothing, as the rocket failed
	}
	else if ((abs(currP + 34.0 * RAD) < 12.5 * RAD && abs(normangle(currY + PI)) < 30.0 * RAD && abs(currR) < 30.0 * RAD) || switchRetroAttitude == 1) // (1) for RETRO ATT switch auto, must be green to fire. Same check as for indicator. (2) Or be in RETRO ATT man
	{
		SetThrusterLevel(thruster_retro[rckNum], 1.0);
	}

	RETRO_THRUST_LEVEL[rckNum] = GetThrusterLevel(thruster_retro[rckNum]); // For local lights

	if (!retroCoverSeparated[rckNum] && RETRO_THRUST_LEVEL[rckNum] > 0.0) // only blow away cover if actually fired
	{
		separateRetroCoverAction[rckNum] = true;
	}
}

void ProjectMercury::CreateAbortThrusters(void)
{
	VECTOR3 esc_exhaust_pos[3];
	VECTOR3 esc_exhaust_ref[3];
	THGROUP_HANDLE escape_engine_group;
	PSTREAM_HANDLE contrail_escape;
	PARTICLESTREAMSPEC exhaust_escape;
	escape_engine = CreateThruster(_V(0.0, 0.0198, -0.056106), _V(0.0, 0.0, 1.0), ABORT_THRUST, escape_tank, ABORT_ISP, ABORT_ISP); // adjust position to offset 0.78 inch (0.0198 m). Was 0.04 cm
	esc_exhaust_pos[0] = _V(0.0, -0.300, -.018702);
	esc_exhaust_ref[0] = _V(0, 0.300, .946);
	esc_exhaust_pos[1] = _V(0.25980762, 0.150, -.018702);
	esc_exhaust_ref[1] = _V(0.25980762, .150, .946);
	esc_exhaust_pos[2] = _V(0.25980762, -.150, -.018702);
	esc_exhaust_ref[2] = _V(-0.25980762, .150, .946);
	normalise(esc_exhaust_ref[0]);
	normalise(esc_exhaust_ref[1]);
	normalise(esc_exhaust_ref[2]);

	escape_engine_group = CreateThrusterGroup(&escape_engine, 1, THGROUP_MAIN);
	//SURFHANDLE tex = oapiRegisterExhaustTexture("Exhaust2");
	//AddExhaust(escape_engine, 1.0, 0.05, esc_exhaust_pos[0] + _V(0.0, 0.0, MERCURY_LENGTH_ABORT / 2.0 - (77.8 - 19.3) * 0.0254), -esc_exhaust_ref[0], tex);
	//AddExhaust(escape_engine, 1.0, 0.05, esc_exhaust_pos[1] + _V(0.0, 0.0, MERCURY_LENGTH_ABORT / 2.0 - (77.8 - 19.3) * 0.0254), -esc_exhaust_ref[1], tex);
	//AddExhaust(escape_engine, 1.0, 0.05, esc_exhaust_pos[2] + _V(0.0, 0.0, MERCURY_LENGTH_ABORT / 2.0 - (77.8 - 19.3) * 0.0254), -esc_exhaust_ref[2], tex);

	exhaust_escape = {
		0, 0.5, 120, 100, 0.5, 2, 10, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1
	};
	contrail_escape = AddExhaustStream(escape_engine, _V(0.0, 0.04, MERCURY_LENGTH_ABORT / 2.0 - 0.056106), &exhaust_escape);
}

void ProjectMercury::InflightAbortSeparate(void)
{
	SeparateRetroPack(false);
	for (int i = 1; i <= 3; i++)
	{
		SeparateRetroCoverN(i);
	}
	SeparateTower(false);
	//SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);
	//SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);
	CGshifted = false;
	VesselStatus = REENTRY;
}

void ProjectMercury::OffPadAbortSeparate(void)
{
	SeparateTower(false);
	//SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);
	//SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);
	CGshifted = false;
	VesselStatus = REENTRY;
}

void ProjectMercury::CreatePosigradeRockets(void)
{
	VECTOR3 m_exhaust_pos;
	VECTOR3 m_exhaust_ref;

	SURFHANDLE tex = oapiRegisterExhaustTexture("Exhaust2"); // See When We Left Earth first part at 2:50 for reference

	m_exhaust_pos = _V(-0.305, MERCURY_OFS_RETRO.y - 0.11, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(0.329, 0.119, 0.95);
	thruster_posigrade[0] = CreateThruster(m_exhaust_pos, m_exhaust_ref, POSIGRADE_THRUST, posigrade_propellant[0], POSIGRADE_ISP);
	AddExhaust(thruster_posigrade[0], 0.5, 0.02, tex);

	m_exhaust_pos = _V(0.065, MERCURY_OFS_RETRO.y + 0.285, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(-0.075, -0.337, 0.95);
	thruster_posigrade[1] = CreateThruster(m_exhaust_pos, m_exhaust_ref, POSIGRADE_THRUST, posigrade_propellant[1], POSIGRADE_ISP);
	AddExhaust(thruster_posigrade[1], 0.5, 0.02, tex);

	m_exhaust_pos = _V(.214, MERCURY_OFS_RETRO.y - 0.23, MERCURY_OFS_RETRO.z - .2);
	m_exhaust_ref = _V(-0.235, 0.253, 0.95);
	thruster_posigrade[2] = CreateThruster(m_exhaust_pos, m_exhaust_ref, POSIGRADE_THRUST, posigrade_propellant[2], POSIGRADE_ISP);
	AddExhaust(thruster_posigrade[2], 0.5, 0.02, tex);

	SetThrusterLevel(thruster_posigrade[0], 1.0);
	SetThrusterLevel(thruster_posigrade[1], 1.0);
	SetThrusterLevel(thruster_posigrade[2], 1.0);
}

void ProjectMercury::CreateRetroRockets(void)
{
	VECTOR3 m_exhaust_pos;
	VECTOR3 m_exhaust_ref;

	COLOUR4 col_d = { (float)(0.9),(float)(0.8),1,0 };
	COLOUR4 col_s = { (float)(1.9),(float)(0.8),1,0 };
	COLOUR4 col_a = { 0,0,0,0 };

	SURFHANDLE tex = oapiRegisterExhaustTexture("Exhaust2");

	m_exhaust_pos = _V(-0.196, 0.095, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(0.9, -0.436, 6.7);
	thruster_retro[0] = CreateThruster(m_exhaust_pos, m_exhaust_ref, RETRO_THRUST, retro_propellant[0], RETRO_ISP);
	AddExhaust(thruster_retro[0], 1.8, 0.05, tex);
	LightEmitter* retro1_light = AddPointLight(m_exhaust_pos, 20, 0.1, -0.01, 0.2, col_d, col_s, col_a);
	retro1_light->SetIntensityRef(&RETRO_THRUST_LEVEL[0]);

	m_exhaust_pos = _V(-0.005, -0.19, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(0.026, 1, 7.7);
	thruster_retro[1] = CreateThruster(m_exhaust_pos, m_exhaust_ref, RETRO_THRUST, retro_propellant[1], RETRO_ISP);
	AddExhaust(thruster_retro[1], 1.8, 0.05, tex);
	LightEmitter* retro2_light = AddPointLight(m_exhaust_pos, 20, 0.1, -0.01, 0.2, col_d, col_s, col_a);
	retro2_light->SetIntensityRef(&RETRO_THRUST_LEVEL[1]);

	m_exhaust_pos = _V(0.175, 0.107, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(-0.853, -0.522, 7.1);
	thruster_retro[2] = CreateThruster(m_exhaust_pos, m_exhaust_ref, RETRO_THRUST, retro_propellant[2], RETRO_ISP);
	AddExhaust(thruster_retro[2], 1.8, 0.05, tex);
	LightEmitter* retro3_light = AddPointLight(m_exhaust_pos, 20, 0.1, -0.01, 0.2, col_d, col_s, col_a);
	retro3_light->SetIntensityRef(&RETRO_THRUST_LEVEL[2]);
}

void ProjectMercury::PrepareReentry()
{
	double reentryStresses = 0.5 * GetAtmDensity() * pow(GetAirspeed(), 3);
	if (FailureMode == RETRONOSEP && reentryStresses < 1e6) // things could be interesting if this failure mode is on, and aborting. Check that in debugging process
	{
		//oapiWriteLog("Failed to separate retropack due to failure"); // not writing this, as we would get it every frame for automatic retropack sep
	}
	else if (VesselStatus == FLIGHT)
	{
		DisableAutopilot(false);
		SeparateRetroPack(true);
		for (int i = 1; i <= 3; i++)
		{
			SeparateRetroCoverN(i);
		}
		VesselStatus = REENTRY;
		CGshifted = false;
	}
}

void ProjectMercury::CreateRCS(void)
{
	if (rcsExists) return;

	rcsExists = true;
	VECTOR3 att_ref;
	VECTOR3 att_dir;

	PARTICLESTREAMSPEC RCSHigh = {
		0, 0.05, 20, 1, 0.01, 0.1, 1, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1
	};
	PARTICLESTREAMSPEC RCSLow = {
		0, 0.01, 20, 2, 0.01, 0.1, 1, 1.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1
	};

	att_ref = _V(0.0, -0.41, 1.05);
	att_dir = _V(0, 1.0, 0);
	thruster_man_py[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_manual, MERCURY_ISP_ATT);
	att_ref = _V(-0.06, -.41, 1.05);
	att_dir = _V(0, -1, 0);
	//AddExhaust(thruster_man_py[0], .2, .02, att_ref, att_dir);
	rcsStream[0] = AddExhaustStream(thruster_man_py[0], att_ref, &RCSHigh);

	att_ref = _V(0.0, .41, 1.05);
	att_dir = _V(0, -1, 0);
	thruster_man_py[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_manual, MERCURY_ISP_ATT);
	att_ref = _V(0.06, .41, 1.05);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_man_py[1], .2, .02, att_ref, att_dir);
	rcsStream[1] = AddExhaustStream(thruster_man_py[1], att_ref, &RCSHigh);

	att_ref = _V(-.41, 0.0, 1.05);
	att_dir = _V(1, 0, 0);
	thruster_man_py[2] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_manual, MERCURY_ISP_ATT);
	att_ref = _V(-.41, 0.06, 1.05);
	att_dir = _V(-1, 0, 0);
	// AddExhaust(thruster_man_py[2], .2, .02, att_ref, att_dir);
	rcsStream[2] = AddExhaustStream(thruster_man_py[2], att_ref, &RCSHigh);

	att_ref = _V(.41, 0.0, 1.05);
	att_dir = _V(-1, 0, 0);
	thruster_man_py[3] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_manual, MERCURY_ISP_ATT);
	att_ref = _V(.41, -0.06, 1.05);
	att_dir = _V(1, 0, 0);
	// AddExhaust(thruster_man_py[3], .2, .02, att_ref, att_dir);
	rcsStream[3] = AddExhaustStream(thruster_man_py[3], att_ref, &RCSHigh);

	att_ref = _V(0.80, .05, 0.0);
	att_dir = _V(0, -1, 0);
	thruster_man_roll[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ROLL_ATT, fuel_manual, MERCURY_ISP_ATT);
	att_ref = _V(0.80, .05, -.6);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_man_roll[0], .2, .02, att_ref, att_dir);
	rcsStream[4] = AddExhaustStream(thruster_man_roll[0], att_ref, &RCSHigh);

	att_ref = _V(0.80, -.05, 0.0);
	att_dir = _V(0, 1, 0);
	thruster_man_roll[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ROLL_ATT, fuel_manual, MERCURY_ISP_ATT);
	att_ref = _V(0.80, -.05, -.6);
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_man_roll[1], .2, .02, att_ref, att_dir);
	rcsStream[5] = AddExhaustStream(thruster_man_roll[1], att_ref, &RCSHigh);

	//// Manual groups
	//CreateThrusterGroup(&thruster_man_py[0], 1, THGROUP_ATT_PITCHUP);
	//CreateThrusterGroup(&thruster_man_py[1], 1, THGROUP_ATT_PITCHDOWN);
	//CreateThrusterGroup(&thruster_man_py[2], 1, THGROUP_ATT_YAWRIGHT);
	//CreateThrusterGroup(&thruster_man_py[3], 1, THGROUP_ATT_YAWLEFT);
	//CreateThrusterGroup(&thruster_man_roll[0], 1, THGROUP_ATT_BANKRIGHT);
	//CreateThrusterGroup(&thruster_man_roll[1], 1, THGROUP_ATT_BANKLEFT);

	PROPELLANT_HANDLE fuel = fuel_auto;
	if (FailureMode == ATTMODEOFF)
	{
		fuel = NULL; // disable thruster
	}

	att_ref = _V(0.0, -.41, 1.05);
	att_dir = _V(0, 1, 0);
	thruster_auto_py[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel, MERCURY_ISP_ATT); // fuel auto
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_auto_py[0], .2, .02, att_ref, att_dir);
	rcsStream[6] = AddExhaustStream(thruster_auto_py[0], att_ref, &RCSHigh);

	att_ref = _V(0.0, -.41, 1.05);
	att_dir = _V(0, 1, 0);
	thruster_auto_py_1lb[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(0.07, -.41, 1.05);
	//att_ref = _V(0.07,-.41,1.05);
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_auto_py_1lb[0], .07, .02, att_ref, att_dir);
	rcsStream[7] = AddExhaustStream(thruster_auto_py_1lb[0], att_ref, &RCSLow);

	att_ref = _V(0.0, .41, 1.05);
	att_dir = _V(0, -1, 0);
	thruster_auto_py[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel, MERCURY_ISP_ATT); // fuel auto
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_auto_py[1], .2, .02, att_ref, att_dir);
	rcsStream[8] = AddExhaustStream(thruster_auto_py[1], att_ref, &RCSHigh);

	att_ref = _V(0.0, .41, 1.05);
	att_dir = _V(0, -1, 0);
	thruster_auto_py_1lb[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.07, .41, 1.05);
	//att_ref = _V(-0.12,.41,1.05);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_auto_py_1lb[1], .07, .02, att_ref, att_dir);
	rcsStream[9] = AddExhaustStream(thruster_auto_py_1lb[1], att_ref, &RCSLow);

	att_ref = _V(-.41, 0.0, 1.05);
	att_dir = _V(1, 0, 0);
	thruster_auto_py[2] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel, MERCURY_ISP_ATT); // fuel auto
	att_dir = _V(-1, 0, 0);
	// AddExhaust(thruster_auto_py[2], .2, .02, att_ref, att_dir);
	rcsStream[10] = AddExhaustStream(thruster_auto_py[2], att_ref, &RCSHigh);

	att_ref = _V(-.41, 0.0, 1.05);
	att_dir = _V(1, 0, 0);
	thruster_auto_py_1lb[2] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-.41, -0.07, 1.05);
	att_dir = _V(-1, 0, 0);
	// AddExhaust(thruster_auto_py_1lb[2], .07, .02, att_ref, att_dir);
	rcsStream[11] = AddExhaustStream(thruster_auto_py_1lb[2], att_ref, &RCSLow);

	att_ref = _V(.41, 0.0, 1.05);
	att_dir = _V(-1, 0, 0);
	thruster_auto_py[3] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel, MERCURY_ISP_ATT); // fuel auto
	att_dir = _V(1, 0, 0);
	// AddExhaust(thruster_auto_py[3], .2, .02, att_ref, att_dir);
	rcsStream[12] = AddExhaustStream(thruster_auto_py[3], att_ref, &RCSHigh);

	att_ref = _V(.41, 0.0, 1.05);
	att_dir = _V(-1, 0, 0);
	thruster_auto_py_1lb[3] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(.41, 0.07, 1.05);
	att_dir = _V(1, 0, 0);
	// AddExhaust(thruster_auto_py_1lb[3], .07, .02, att_ref, att_dir);
	rcsStream[13] = AddExhaustStream(thruster_auto_py_1lb[3], att_ref, &RCSLow);

	att_ref = _V(-0.80, -.05, 0.0);
	att_dir = _V(0, 1, 0);
	thruster_auto_roll[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ROLL_ATT, fuel, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.80, -.05, -.6);
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_auto_roll[0], .2, .02, att_ref, att_dir);
	rcsStream[14] = AddExhaustStream(thruster_auto_roll[0], att_ref, &RCSHigh);

	att_ref = _V(-0.80, -.05, 0.0);
	att_dir = _V(0, 1, 0);
	thruster_auto_roll_1lb[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.80, -.05, -.63);
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_auto_roll_1lb[0], .07, .02, att_ref, att_dir);
	rcsStream[15] = AddExhaustStream(thruster_auto_roll_1lb[0], att_ref, &RCSLow);

	att_ref = _V(-0.80, .05, 0.0);
	att_dir = _V(0, -1, 0);
	thruster_auto_roll[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ROLL_ATT, fuel, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.80, .05, -.6);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_auto_roll[1], .2, .02, att_ref, att_dir);
	rcsStream[16] = AddExhaustStream(thruster_auto_roll[1], att_ref, &RCSHigh);

	att_ref = _V(-0.80, .05, 0.0);
	att_dir = _V(0, -1, 0);
	thruster_auto_roll_1lb[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.80, .05, -.63);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_auto_roll_1lb[1], .07, .02, att_ref, att_dir);
	rcsStream[17] = AddExhaustStream(thruster_auto_roll_1lb[1], att_ref, &RCSLow);

	//// Auto groups
	//pitchup = CreateThrusterGroup(&thruster_auto_py[0], 1, THGROUP_ATT_PITCHUP);
	//pitchdown = CreateThrusterGroup(&thruster_auto_py[1], 1, THGROUP_ATT_PITCHDOWN);
	//yawright = CreateThrusterGroup(&thruster_auto_py[2], 1, THGROUP_ATT_YAWRIGHT);
	//yawleft = CreateThrusterGroup(&thruster_auto_py[3], 1, THGROUP_ATT_YAWLEFT);
	//bankright = CreateThrusterGroup(&thruster_auto_roll[0], 1, THGROUP_ATT_BANKRIGHT);
	//bankleft = CreateThrusterGroup(&thruster_auto_roll[1], 1, THGROUP_ATT_BANKLEFT);

	//RcsStatus = AUTOHIGH;
	//SetDefaultPropellantResource(fuel_auto);

	// Create the thrusters for capsule control. Dummy thrusters, and we map the levels onto the actual thrusters, everything dependent on which mode we are in (ASCS, rate cmd, etc.)
	controllerPitchup = CreateThruster(_V(0, 0, 0), _V(0, 0, 1), 0, dummyControllerFuel, 0, 0);
	controllerPitchdown = CreateThruster(_V(0, 0, 0), _V(0, 0, 1), 0, dummyControllerFuel, 0, 0);
	controllerYawleft = CreateThruster(_V(0, 0, 0), _V(0, 0, 1), 0, dummyControllerFuel, 0, 0);
	controllerYawright = CreateThruster(_V(0, 0, 0), _V(0, 0, 1), 0, dummyControllerFuel, 0, 0);
	controllerBankright = CreateThruster(_V(0, 0, 0), _V(0, 0, 1), 0, dummyControllerFuel, 0, 0);
	controllerBankleft = CreateThruster(_V(0, 0, 0), _V(0, 0, 1), 0, dummyControllerFuel, 0, 0);
	CreateThrusterGroup(&controllerPitchup, 1, THGROUP_ATT_PITCHUP);
	CreateThrusterGroup(&controllerPitchdown, 1, THGROUP_ATT_PITCHDOWN);
	CreateThrusterGroup(&controllerYawleft, 1, THGROUP_ATT_YAWLEFT);
	CreateThrusterGroup(&controllerYawright, 1, THGROUP_ATT_YAWRIGHT);
	CreateThrusterGroup(&controllerBankright, 1, THGROUP_ATT_BANKRIGHT);
	CreateThrusterGroup(&controllerBankleft, 1, THGROUP_ATT_BANKLEFT);

	// And if we have the concept unit, then create the linear groups:
	if (conceptManouverUnit && conceptManouverUnitAttached)
	{
		SetDefaultPropellantResource(conceptPropellant);
		CreateThrusterGroup(conceptPosigrade, 2, THGROUP_MAIN);
		CreateThrusterGroup(conceptRetrograde, 2, THGROUP_RETRO);
		CreateThrusterGroup(&conceptLinear[0], 1, THGROUP_ATT_UP);
		CreateThrusterGroup(&conceptLinear[1], 1, THGROUP_ATT_DOWN);
		CreateThrusterGroup(&conceptLinear[2], 1, THGROUP_ATT_LEFT);
		CreateThrusterGroup(&conceptLinear[3], 1, THGROUP_ATT_RIGHT);
		CreateThrusterGroup(conceptPosigrade, 2, THGROUP_ATT_FORWARD); // doppelt gemoppelt from th-main and th-retro, but why not?
		CreateThrusterGroup(conceptRetrograde, 2, THGROUP_ATT_BACK); // doppelt gemoppelt
	}

	// Fireflies
	PARTICLESTREAMSPEC fireflies = {
		0, 0.01, 100, 0.3, 0.75, 10.0, 0.0, 0.0, PARTICLESTREAMSPEC::DIFFUSE,
		PARTICLESTREAMSPEC::LVL_SQRT, 0, 1,
		PARTICLESTREAMSPEC::ATM_FLAT, 1, 1,
		oapiRegisterParticleTexture("ProjectMercury\\fireflies")
	};

	AddParticleStream(&fireflies, _V(0, 0.5, 0), _V(0, 0, 1), &fireflyLevel);
}

void ProjectMercury::CreateAirfoils(void)
{
	ClearAirfoilDefinitions(); // delete previous airfoils
	SetRotDrag(MERCURY_ROT_DRAG);
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, 0.7239), vlift, NULL, 1.89, 1.89 * 1.89 * PI / 4.0, 1.0); // 0.7239 m from CG to CP (19670022650 page 36)
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, 0.7239), hlift, NULL, 1.89, 1.89 * 1.89 * PI / 4.0, 1.0); // spherical symmetric
}

void ProjectMercury::CreateAirfoilsEscape(void)
{
	ClearAirfoilDefinitions(); // delete previous airfoils
	SetRotDrag(MERCURY_ROT_DRAG);
	CreateAirfoil3(LIFT_VERTICAL, _V(0, 0, -4.91), vliftEscape, NULL, 1.89, 1.89 * 1.89 * PI / 4.0, 1.0); // 1.923 m from CG to CP post escape burn (19670022650 page 36)
	CreateAirfoil3(LIFT_HORIZONTAL, _V(0, 0, -4.91), hliftEscape, NULL, 1.89, 1.89 * 1.89 * PI / 4.0, 1.0); // spherical symmetric
}

//void ProjectMercury::SwitchAttitudeMode(void)
//{
//	if (RcsStatus == AUTOHIGH)
//	{
//		DelThrusterGroup(pitchup);
//		DelThrusterGroup(pitchdown);
//		DelThrusterGroup(yawright);
//		DelThrusterGroup(yawleft);
//		DelThrusterGroup(bankleft);
//		DelThrusterGroup(bankright);
//
//		pitchup = CreateThrusterGroup(&thruster_auto_py[0], 1, THGROUP_ATT_PITCHUP);
//		pitchdown = CreateThrusterGroup(&thruster_auto_py[1], 1, THGROUP_ATT_PITCHDOWN);
//		yawright = CreateThrusterGroup(&thruster_auto_py[2], 1, THGROUP_ATT_YAWRIGHT);
//		yawleft = CreateThrusterGroup(&thruster_auto_py[3], 1, THGROUP_ATT_YAWLEFT);
//		bankright = CreateThrusterGroup(&thruster_auto_roll[0], 1, THGROUP_ATT_BANKRIGHT);
//		bankleft = CreateThrusterGroup(&thruster_auto_roll[1], 1, THGROUP_ATT_BANKLEFT);
//	}
//	else if (RcsStatus == MANUAL)
//	{
//		DelThrusterGroup(pitchup);
//		DelThrusterGroup(pitchdown);
//		DelThrusterGroup(yawright);
//		DelThrusterGroup(yawleft);
//		DelThrusterGroup(bankleft);
//		DelThrusterGroup(bankright);
//
//		pitchup = CreateThrusterGroup(&thruster_man_py[0], 1, THGROUP_ATT_PITCHUP);
//		pitchdown = CreateThrusterGroup(&thruster_man_py[1], 1, THGROUP_ATT_PITCHDOWN);
//		yawright = CreateThrusterGroup(&thruster_man_py[2], 1, THGROUP_ATT_YAWRIGHT);
//		yawleft = CreateThrusterGroup(&thruster_man_py[3], 1, THGROUP_ATT_YAWLEFT);
//		bankright = CreateThrusterGroup(&thruster_man_roll[0], 1, THGROUP_ATT_BANKRIGHT);
//		bankleft = CreateThrusterGroup(&thruster_man_roll[1], 1, THGROUP_ATT_BANKLEFT);
//	}
//	else
//	{
//		DelThrusterGroup(pitchup);
//		DelThrusterGroup(pitchdown);
//		DelThrusterGroup(yawright);
//		DelThrusterGroup(yawleft);
//		DelThrusterGroup(bankleft);
//		DelThrusterGroup(bankright);
//
//		pitchup = CreateThrusterGroup(&thruster_auto_py_1lb[0], 1, THGROUP_ATT_PITCHUP);
//		pitchdown = CreateThrusterGroup(&thruster_auto_py_1lb[1], 1, THGROUP_ATT_PITCHDOWN);
//		yawright = CreateThrusterGroup(&thruster_auto_py_1lb[2], 1, THGROUP_ATT_YAWRIGHT);
//		yawleft = CreateThrusterGroup(&thruster_auto_py_1lb[3], 1, THGROUP_ATT_YAWLEFT);
//		bankright = CreateThrusterGroup(&thruster_auto_roll_1lb[0], 1, THGROUP_ATT_BANKRIGHT);
//		bankleft = CreateThrusterGroup(&thruster_auto_roll_1lb[1], 1, THGROUP_ATT_BANKLEFT);
//	}
//}

//void ProjectMercury::SwitchPropellantSource(void)
//{
//	double currentAuto = GetPropellantMass(fuel_auto);
//	double currentManual = GetPropellantMass(fuel_manual);
//	double currentAutoMax = GetPropellantMaxMass(fuel_auto);
//	double currentManualMax = GetPropellantMaxMass(fuel_manual);
//
//	SetPropellantMaxMass(fuel_manual, currentAutoMax);
//	SetPropellantMaxMass(fuel_auto, currentManualMax);
//	SetPropellantMass(fuel_manual, currentAuto);
//	SetPropellantMass(fuel_auto, currentManual);
//
//	attitudeFuelAuto = !attitudeFuelAuto;
//}

void ProjectMercury::DumpFuelRCS(void)
{
	SetThrusterLevel(thruster_auto_py[0], 1.0);
	SetThrusterLevel(thruster_auto_py[1], 1.0);
	SetThrusterLevel(thruster_auto_py[2], 1.0);
	SetThrusterLevel(thruster_auto_py[3], 1.0);
	SetThrusterLevel(thruster_auto_roll[0], 1.0);
	SetThrusterLevel(thruster_auto_roll[1], 1.0);

	SetThrusterLevel(thruster_auto_py_1lb[0], 1.0);
	SetThrusterLevel(thruster_auto_py_1lb[1], 1.0);
	SetThrusterLevel(thruster_auto_py_1lb[2], 1.0);
	SetThrusterLevel(thruster_auto_py_1lb[3], 1.0);
	SetThrusterLevel(thruster_auto_roll_1lb[0], 1.0);
	SetThrusterLevel(thruster_auto_roll_1lb[1], 1.0);

	SetThrusterLevel(thruster_man_py[0], 1.0);
	SetThrusterLevel(thruster_man_py[1], 1.0);
	SetThrusterLevel(thruster_man_py[2], 1.0);
	SetThrusterLevel(thruster_man_py[3], 1.0);
	SetThrusterLevel(thruster_man_roll[0], 1.0);
	SetThrusterLevel(thruster_man_roll[1], 1.0);

	//if (GetFuelMass() == 0.0 && (GetPropellantMass(fuel_manual) != 0.0 || GetPropellantMass(fuel_auto) != 0.0))
	//{
	//	SwitchPropellantSource();
	//}

}

void ProjectMercury::SeparateRetroPack(bool deleteThrusters) // only false if no thrusters have been created (abort)
{
	// Set local lights for thrusters to zero
	RETRO_THRUST_LEVEL[0] = 0.0;
	RETRO_THRUST_LEVEL[1] = 0.0;
	RETRO_THRUST_LEVEL[2] = 0.0;

	VESSELSTATUS2 vs;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	char name[256];
	VECTOR3 vel = _V(0.0, 0.0, -1.0);
	VECTOR3 vel2;
	VECTOR3 relativeOffsetRetro;
	GetStatusEx(&vs);
	if (VesselStatus == FLIGHT)
		Local2Rel(MERCURY_OFS_RETRO - MERCURY_OFS_CAPSULE, vs.rpos);
	else // abort mode
		Local2Rel(MERCURY_OFS_RETRO - MERCURY_OFS_CAPSULE - _V(0.0, 0.0, (77.8 - 19.3) * 0.0254), vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffsetRetro); // Currently does nothing
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	strcpy(name, GetName());
	strcat(name, " Retro");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Retro", &vs);
	//if (GetAltitude() > 5e4)
	//	createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Retro);

	vel = _V(0.0, 0.75, -1.0);
	VECTOR3 relativeOffsetRetrostrap1;
	GetStatusEx(&vs);
	if (VesselStatus == FLIGHT)
		Local2Rel(MERCURY_OFS_RETROSTRAP1 - MERCURY_OFS_CAPSULE, vs.rpos);
	else // abort mode
		Local2Rel(MERCURY_OFS_RETROSTRAP1 - MERCURY_OFS_CAPSULE - _V(0.0, 0.0, (77.8 - 19.3) * 0.0254), vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffsetRetrostrap1);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	vs.vrot.x = -1.0;
	vs.vrot.y = 0.0;
	strcpy(name, GetName());
	strcat(name, " Retrostrap 1");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Retrostrap1", &vs);
	//if (GetAltitude() > 5e4)
	//	createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Retrostrap1);

	vel = _V(-0.65, -0.375, -1.0);
	VECTOR3 relativeOffsetRetrostrap2;
	GetStatusEx(&vs);
	if (VesselStatus == FLIGHT)
		Local2Rel(MERCURY_OFS_RETROSTRAP2 - MERCURY_OFS_CAPSULE, vs.rpos);
	else // abort mode
		Local2Rel(MERCURY_OFS_RETROSTRAP2 - MERCURY_OFS_CAPSULE - _V(0.0, 0.0, (77.8 - 19.3) * 0.0254), vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffsetRetrostrap2);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	vs.vrot.x = 0.33;
	vs.vrot.y = -0.66;
	strcpy(name, GetName());
	strcat(name, " Retrostrap 2");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Retrostrap2", &vs);
	//if (GetAltitude() > 5e4)
	//	createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Retrostrap2);

	vel = _V(0.65, -0.375, -1.0);
	VECTOR3 relativeOffsetRetrostrap3;
	GetStatusEx(&vs);
	if (VesselStatus == FLIGHT)
		Local2Rel(MERCURY_OFS_RETROSTRAP3 - MERCURY_OFS_CAPSULE, vs.rpos);
	else // abort mode
		Local2Rel(MERCURY_OFS_RETROSTRAP3 - MERCURY_OFS_CAPSULE - _V(0.0, 0.0, (77.8 - 19.3) * 0.0254), vs.rpos);
	GlobalRot(_V(0.0, -1.0, 0.0), relativeOffsetRetrostrap3);
	GlobalRot(vel, vel2);
	vs.rvel += vel2;
	vs.vrot.x = 0.33;
	vs.vrot.y = 0.66;
	strcpy(name, GetName());
	strcat(name, " Retrostrap 3");
	createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Retrostrap3", &vs);
	//if (GetAltitude() > 5e4)
	//	createdAbove50km[stuffCreated] = true;
	stuffCreated += 1;
	DelMesh(Retrostrap3);

	if (deleteThrusters)
	{
		for (int i = 0; i < 3; i++)
		{
			DelThruster(thruster_retro[i]);
			DelPropellantResource(retro_propellant[i]);
		}
	}

	DelMesh(Explosivebolt); // Have not utilised this, maybe do that later
	DelPropellantResource(posigrade_propellant[0]);
	DelPropellantResource(posigrade_propellant[1]);
	DelPropellantResource(posigrade_propellant[2]);
	DelThruster(thruster_posigrade[0]);
	DelThruster(thruster_posigrade[1]);
	DelThruster(thruster_posigrade[2]);
	DelPropellantResource(retro_propellant[0]);
	DelPropellantResource(retro_propellant[1]);
	DelPropellantResource(retro_propellant[2]);
	engageRetro = false;
}

void ProjectMercury::SeparateRetroCoverN(int i) // int 1, 2, 3
{
	if (!retroCoverSeparated[i - 1])
	{
		VESSELSTATUS2 vs;
		vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
		vs.version = 2;

		char name[256];
		char configName[256];
		UINT meshToDelete;
		VECTOR3 offset;
		VECTOR3 vel = _V(0.0, 0.0, -1.0);
		VECTOR3 dir = _V(0.0, -1.0, 0.0);
		strcpy(name, GetName());

		if (i == 1)
		{
			offset = MERCURY_OFS_RETROCOVER1 - MERCURY_OFS_CAPSULE;
			strcat(name, " Retrocover 1");
			meshToDelete = Retrocover1;
			retroCoverSeparated[i - 1] = true;
		}
		else if (i == 2)
		{
			offset = MERCURY_OFS_RETROCOVER2 - MERCURY_OFS_CAPSULE;
			strcat(name, " Retrocover 2");
			meshToDelete = Retrocover2;
			retroCoverSeparated[i - 1] = true;
		}
		else
		{
			offset = MERCURY_OFS_RETROCOVER3 - MERCURY_OFS_CAPSULE;
			strcat(name, " Retrocover 3");
			meshToDelete = Retrocover3;
			retroCoverSeparated[i - 1] = true;
		}

		VECTOR3 relativeOffsetRetrocover;
		GetStatusEx(&vs);
		Local2Rel(offset, vs.rpos);
		GlobalRot(dir, relativeOffsetRetrocover);
		VECTOR3 vel2;
		GlobalRot(vel, vel2);
		vs.rvel += vel2;
		sprintf(configName, "ProjectMercury\\Mercury_RetroCover%i", i);
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, configName, &vs);
		//if (GetAltitude() > 5e4)
		//	createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(meshToDelete);
	}
}

void ProjectMercury::SeparateDrogueCover(void)
{
	if (!drogueCoverSeparated)
	{
		VESSELSTATUS2 vs;
		vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
		vs.version = 2;

		char name[256];
		VECTOR3 offset;
		VECTOR3 vel = _V(0.0, 1.0, 0.0);
		VECTOR3 dir = _V(0.0, -1.0, 0.0);
		strcpy(name, GetName());
		offset = MERCURY_OFS_DROGUECOVER - MERCURY_OFS_CAPSULE;
		strcat(name, " Drogue Cover");

		VECTOR3 relativeOffsetRetrocover;
		GetStatusEx(&vs);
		Local2Rel(offset, vs.rpos);
		GlobalRot(dir, relativeOffsetRetrocover);
		VECTOR3 vel2;
		GlobalRot(vel, vel2);
		vs.rvel += vel2;
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_DrogueCover", &vs);
		//if (GetAltitude() > 5e4)
		//	createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(Droguecover);
		drogueCoverSeparated = true;
	}
}

void ProjectMercury::SeparateDrogue(void)
{
	if (!drogueSeparated)
	{
		VESSELSTATUS2 vs;
		vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
		vs.version = 2;

		char name[256];
		VECTOR3 offset;
		VECTOR3 vel = _V(0.0, 1.0, 15.0); // guesstimate from MR-1 video (maybe y: 0.3, but main has a tendency to "crash" into antenna housing, so I'm incrasing it)
		VECTOR3 dir = _V(0.0, -1.0, 0.0);
		strcpy(name, GetName());
		offset = MERCURY_OFS_ANTHOUSE - MERCURY_OFS_CAPSULE;
		strcat(name, " Antennahouse"); // I must create a DLL for this!

		VECTOR3 relativeOffsetRetrocover;
		GetStatusEx(&vs);
		Local2Rel(offset, vs.rpos);
		GlobalRot(dir, relativeOffsetRetrocover);
		VECTOR3 vel2;
		GlobalRot(vel, vel2);
		vs.rvel += vel2;

		if (CapsuleVersion == CAPSULEBIGJOE) createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AntennaBJ", &vs);
		else if (CapsuleVersion == CAPSULELITTLEJOE) createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AntennaLJ", &vs);
		else if (CapsuleVersion == CAPSULEBD) createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_AntennaBD", &vs);
		else createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Antenna", &vs);

		//if (GetAltitude() > 5e4)
		//	createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(Drogue);
		DelMesh(Antennahouse);
		drogueSeparated = true;

		drogueProgress = 1.0;
		ClearVariableDragElements(); // Clear drogue drag
		SetSize(MERCURY_LENGTH_CAPSULE);
		SetRotDrag(MERCURY_ROT_DRAG);

		VesselStatus = REENTRYNODROGUE;
		CGshifted = false;
	}
}

void ProjectMercury::DeployDrogue(void)
{
	separateDrogueCoverAction = true;

	if (!drogueDeployed)
	{
		drogueDeployed = true;
		DrogueStatus = OPENING;
		drogueMoving = true;
		drogueProgress = 1.0;

		//SetCrossSections(_V(2.8, 2.8, 3.0));
		SetCW(1.5, 1.5, 0.3, 0.3);
		SetSize(9.0);
		DelMesh(Drogue);
		Drogue = AddMesh(drogue, &MERCURY_OFS_DROGUE);
		SetMeshVisibilityMode(Drogue, MESHVIS_ALWAYS);
		//SetMeshVisibleInternal(Drogue, true);
		DefineDrogueAnimation();

		CreateVariableDragElement(&drogueProgress, 2.365, MERCURY_OFS_DROGUE); // used (factor from main)/(main area)*(drogue area) for factor
		SetRotDrag(MERCURY_ROT_DRAG_DROGUE);
		
		drogueDeployTime = oapiGetSimTime();

		DisableAutopilot(true); // Stop rotating
	}
}

void ProjectMercury::DeployMainChute(void)
{
	if (!mainChuteDeployed)
	{
		mainChuteDeployed = true;
		MainChuteStatus = OPENING;
		mainChuteMoving = true;
		mainChuteProgress = 0.0;
		mainChuteProgressArea = 0.0;

		SetCW(1.5, 1.5, 0.3, 0.3);
		SetSize(12.0);
		DelMesh(Mainchute); // First remove if it exists
		Mainchute = AddMesh(mainChute, &MERCURY_OFS_MAINCHUTE);
		SetMeshVisibilityMode(Mainchute, MESHVIS_ALWAYS);
		DefineMainChuteAnimation();

		if (mainChuteSeparated) // we are now deploying reserve chute. Let's keep that fully functional
		{
			CreateVariableDragElement(&mainChuteProgressArea, 218.0, MERCURY_OFS_MAINCHUTE); // calibrated factor (second input) to sink rate 9.144 m/s (30 ft/s) at sea level
		}
		else // potentially messed up
		{
			CreateVariableDragElement(&mainChuteProgressArea, 218.0 * (1.0 - chuteDestroyFactor), MERCURY_OFS_MAINCHUTE); // calibrated factor (second input) to sink rate 9.144 m/s (30 ft/s) at sea level
			mainChuteDeployTime = oapiGetSimTime();
		}
		SetRotDrag(MERCURY_ROT_DRAG_CHUTE);
	}
}

void ProjectMercury::DeployLandingBag(void)
{
	engageFuelDump = true; // fuel is actually jettisoned at main chute deploy, but I prefer doing it here. (19730064919_1973064919 page 46)

	if (FailureMode == NOLANDBAG || boilerplateMission)
	{
		// Landing bag failed to deploy
	}
	else if (!landingBagDeployed)
	{
		landingBagDeployed = true;
		LandingBagStatus = L_OPENING;
		landingBagMoving = true;
		landingBagProgress = 0.0;

		Landingbag = AddMesh(landingbag, &(MERCURY_OFS_LANDBAG - MERCURY_OFS_CAPSULE));
		DefineLandingBagAnimation();
	}
}

void ProjectMercury::SeparateMainChute(void)
{
	VESSELSTATUS2 vs;
	vs.flag = 0; // no idea what it does, but thanks to face https://www.orbiter-forum.com/showthread.php?p=296740&postcount=2
	vs.version = 2;

	char name[256];
	VECTOR3 offset;
	VECTOR3 vel = _V(0.0, 1.0, 0.0);
	VECTOR3 dir = _V(0.0, -1.0, 0.0);
	strcpy(name, GetName());
	offset = MERCURY_OFS_MAINCHUTE;

	VECTOR3 relativeOffsetMainChute;
	GetStatusEx(&vs);
	Local2Rel(offset, vs.rpos);
	GlobalRot(dir, relativeOffsetMainChute);
	VECTOR3 vel2;
	GlobalRot(vel, vel2);
	vs.rvel += vel2;

	if (mainChuteDeployed && !mainChuteSeparated)
	{
		strcat(name, " Main chute");

		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Chute", &vs);
		//if (GetAltitude() > 5e4)
		//	createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(Mainchute);
		mainChuteSeparated = true;
		oapiWriteLog("Separate main chute");
	}
	else if (reserveChuteDeployed && !reserveChuteSeparated)
	{
		strcat(name, " Reserve chute");

		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Chute", &vs);
		//if (GetAltitude() > 5e4)
		//	createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(Mainchute);
		reserveChuteSeparated = true;
		oapiWriteLog("Separate reserve chute");
	}

	mainChuteProgress = 0.0;
	mainChuteProgressArea = 0.0;
	ClearVariableDragElements(); // Clear main chute drag
	SetSize(MERCURY_LENGTH_CAPSULE);
	SetRotDrag(MERCURY_ROT_DRAG);
}

void ProjectMercury::AnimateDrogueChute(double simt, double simdt)
{
	if (drogueMoving)
	{
		double animationStep = simdt * DROGUE_OPERATING_SPEED;

		if (DrogueStatus == OPENING) // initial deployment
		{
			if (drogueEndAnimation)
			{
				DrogueStatus = REEFED;
				drogueReefedTime = simt;
				drogueEndAnimation = false;
			}
			//else if (drogueProgress > 1.0) // finished
			else if (drogueProgress < 0.0)
			{
				drogueProgress = 0.0;
				drogueEndAnimation = true;
				drogueMoving = false;
			}
			else // carry on deploying
			{
				drogueProgress -= animationStep;
			}
			SetAnimation(DrogueDeployAnim, drogueProgress);
		}
		else if (DrogueStatus == REEFED && (simt - drogueReefedTime) > 0.0) // Reefed state. But don't see the point in the time check
		{
			DrogueStatus = UNREEFING;

		}
	}
}

void ProjectMercury::AnimateMainChute(double simt, double simdt)
{
	double animationStep = simdt * MAIN_CHUTE_OPERATING_SPEED;

	if (MainChuteStatus == OPENING)
	{
		if (mainChuteProgress > 1.0)
		{
			mainChuteProgress = 1.0;
			MainChuteStatus = OPENED;
		}
		else if (mainChuteProgress > 0.5 && !reefingWait)
		{
			reefingWait = true;
			reefingWaitStart = simt;
		}
		else if (simt - reefingWaitStart < 4.0 && reefingWait)
		{
			// Waiting 4 seconds before unreefing main chute
		}
		else // carry on deploying
		{
			mainChuteProgress += animationStep;

			if (mainChuteProgress < 0.5)
				mainChuteProgressArea = mainChuteProgress * 0.0144 / 0.5; // Unreefed chute area is 12 % of full area
			else
				mainChuteProgressArea = 1.9712 * mainChuteProgress - 0.9712; // linear curve from (0.5, 0.0144) to (1.0, 1.0)
		}
		SetAnimation(MainChuteDeployAnim, mainChuteProgress);
	}




	//if (mainChuteMoving)
	//{
	//	double animationStep = simdt * MAIN_CHUTE_OPERATING_SPEED;

	//	if (MainChuteStatus == OPENING) // initial deployment
	//	{
	//		if (mainChuteEndAnimation && mainChuteProgress == 1.0) // if reserve chute, then progress is reset, to allow new deployment
	//		{
	//			MainChuteStatus = REEFED;
	//			mainChuteReefedTime = simt;
	//			mainChuteEndAnimation = false;
	//		}
	//		else if (mainChuteProgress > 1.0) // finished
	//		{
	//			mainChuteProgress = 1.0;
	//			mainChuteEndAnimation = true;
	//			mainChuteMoving = false;
	//		}
	//		else if (mainChuteProgress > 0.5 && !reefingWait)
	//		{
	//			reefingWait = true;
	//			reefingWaitStart = simt;
	//		}
	//		else if (simt - reefingWaitStart < 4.0 && reefingWait)
	//		{
	//			// Waiting 4 seconds before unreefing main chute
	//		}
	//		else // carry on deploying
	//		{
	//			mainChuteProgress += animationStep;
	//			if (mainChuteProgress < 0.5)
	//				mainChuteProgressArea = mainChuteProgress * 0.0144 / 0.5; // Unreefed chute area is 12 % of full area
	//			else
	//				mainChuteProgressArea = 1.9712 * mainChuteProgress - 0.9712; // linear curve from (0.5, 0.0144) to (1.0, 1.0)

	//		}
	//		SetAnimation(MainChuteDeployAnim, mainChuteProgress);
	//	}
	//	else if (MainChuteStatus == REEFED && (simt - mainChuteReefedTime) > 0.0) // Reefed state. But don't see the point in the time check
	//	{
	//		MainChuteStatus = UNREEFING;
	//	}
	//}
}

void ProjectMercury::AnimateLandingBag(double simt, double simdt)
{
	if (landingBagMoving)
	{
		double animationStep = simdt * LANDING_BAG_OPERATING_SPEED;

		if (LandingBagStatus == L_OPENING) // initial deployment
		{
			if (landingBagProgress > 1.0) // finished
			{
				landingBagProgress = 1.0;
				landingBagEndAnimation = true;
				landingBagMoving = false;
			}
			else // carry on deploying
			{
				landingBagProgress += animationStep;
			}
			SetAnimation(LandingBagDeployAnim, landingBagProgress);
		}
	}
}

inline void ProjectMercury::DisableAttitudeThruster(int num)
{
	THRUSTER_HANDLE thFail;
	if (num < 4)
		thFail = thruster_man_py[num];
	else if (num < 6)
		thFail = thruster_man_roll[num - 4];
	else if (num < 10)
		thFail = thruster_auto_py[num - 6];
	else if (num < 14)
		thFail = thruster_auto_py_1lb[num - 10];
	else if (num < 16)
		thFail = thruster_auto_roll[num - 14];
	else
		thFail = thruster_auto_roll_1lb[num - 16];

	SetThrusterResource(thFail, NULL);
}

inline void ProjectMercury::MercuryGenericConstructor(void)
{
	adaptcover1 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover1");
	adaptcover2 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover2");
	adaptcover3 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover3");
	adaptring1 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring1");
	adaptring2 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring2");
	adaptring3 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring3");
	tower = oapiLoadMeshGlobal("ProjectMercury\\merc_tower");
	droguecover = oapiLoadMeshGlobal("ProjectMercury\\merc_DrogueCover");
	retro = oapiLoadMeshGlobal("ProjectMercury\\merc_retro");
	retrocover1 = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover1");
	retrocover2 = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover2");
	retrocover3 = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover3");
	retrostrap1 = oapiLoadMeshGlobal("ProjectMercury\\merc_strap1");
	retrostrap2 = oapiLoadMeshGlobal("ProjectMercury\\merc_strap2");
	retrostrap3 = oapiLoadMeshGlobal("ProjectMercury\\merc_strap3");
	explosivebolt = oapiLoadMeshGlobal("ProjectMercury\\merc_explosivebolt");

	mainChute = oapiLoadMeshGlobal("ProjectMercury\\merc_chute");
	drogue = oapiLoadMeshGlobal("ProjectMercury\\merc_drogue");
	landingbag = oapiLoadMeshGlobal("ProjectMercury\\merc_LandingBag");

	periscopeMesh = oapiLoadMeshGlobal("ProjectMercury\\PeriscopeInternalPanel");
	periscopeFilterRed = oapiLoadMeshGlobal("ProjectMercury\\PeriscopeFilterRed");
	periscopeFilterYellow = oapiLoadMeshGlobal("ProjectMercury\\PeriscopeFilterYellow");
	periscopeFilterGray = oapiLoadMeshGlobal("ProjectMercury\\PeriscopeFilterGray");
	circularFrameMesh = oapiLoadMeshGlobal("ProjectMercury\\FullCircle"); // For camera cutout. Is a duplicate of the one included in periscopeMesh

	cockpitPanelMesh = oapiLoadMeshGlobal("ProjectMercury\\Panel\\panel");

	//vcFrame = oapiLoadMeshGlobal("ProjectMercury\\VC\\GenericFrame");

	VesselStatus = LAUNCH;
	PreviousVesselStatus = LAUNCH;
	DrogueStatus = CLOSED;
	MainChuteStatus = CLOSED;
	//RcsStatus = AUTOHIGH;
	AutopilotStatus = AUTOLAUNCH;
	CurrentFilter = CLEAR;

	CGshifted = false;

	// Create random seed. Multiply with handle int, as two vessels created at same time (two capsules spawned at scenario start) will create same seed, and thus same failure.
	int timeSeed = (int)time(NULL);
	int handleSeed = (int)GetHandle();
	UINT seed = UINT(timeSeed * handleSeed);
	srand(seed);
	oapiWriteLogV("Vessel random function seeds: time %i, handle %i, combined %i", timeSeed, handleSeed, seed);
	

	//panelDynamicTexture = NULL; // debug
}

inline void ProjectMercury::WriteFlightParameters(void)
{
	char cbuf[256];
	sprintf(cbuf, "%s Flight parameters:", GetName());
	oapiWriteLog(cbuf);

	if (historyReference == NULL || historyLandLat == NULL)
	{
		if (historyReference == NULL) // probably spawned after launch
		{
			historyReference = GetSurfaceRef();
		}
		if (historyLandLat == NULL) // probably not landed when scenario exits
		{
			double radiusDontCare;
			GetEquPos(historyLandLong, historyLandLat, radiusDontCare);
			//oapiWriteLogV("Fetched landing coordinates: %.2fN %.2fE", historyLandLat * DEG, historyLandLong * DEG);
		}

		oapiWriteLogV(" > FULL FLIGHT NOT RECORDED");
	}
	
	oapiWriteLogV(" > Cut-off altitude: %.0f m (%.0f ft)", historyCutOffAlt, historyCutOffAlt / 0.3048);
	oapiWriteLogV(" > Cut-off space-fixed velocity: %.0f m/s (%.0f ft/s)", historyCutOffVel, historyCutOffVel / 0.3048);
	oapiWriteLogV(" > Cut-off flight path angle: %.4f deg", historyCutOffAngl);
	double range = oapiOrthodome(historyLaunchLong, historyLaunchLat, historyLandLong, historyLandLat) * oapiGetSize(historyReference) / 1000.0;
	if (range < 1.0) oapiWriteLogV(" > Range: %.0f m (%.0f ft)", range * 1e3, range * 1e3 / 0.3048); // show metres if less than 1 km
	else oapiWriteLogV(" > Range: %.1f km (%.1f nm)", range, range / 1.852); // else show kilometre

	if (suborbitalMission) // Redstone mission or Atlas abort
	{
		if (historyMaxAltitude < 1e3) oapiWriteLogV(" > Maximum altitude: %.0f m (%.0f ft)", historyMaxAltitude, historyMaxAltitude / 0.3048); // show metres if less than 1 km
		else oapiWriteLogV(" > Maximum altitude: %.1f km (%.1f nm)", historyMaxAltitude / 1000.0, historyMaxAltitude / 1852.0); // else show kilometre
		int weightM = (int)floor(historyWeightlessTime / 60.0);
		int weightS = (int)floor((historyWeightlessTime - weightM * 60.0));
		oapiWriteLogV(" > Period of weightlessness: %01i:%02i", weightM, weightS);
	}
	else // Non-aborted Atlas mission
	{
		oapiWriteLogV(" > Apogee altitude: %.1f km (%.1f nm)", historyMaxAltitude / 1000.0, historyMaxAltitude / 1852.0);
		if (historyPerigee == 1e10) oapiWriteLogV(" > No perigee experienced", historyPerigee / 1000.0, historyPerigee / 1852.0);
		else oapiWriteLogV(" > Perigee altitude: %.1f km (%.1f nm)", historyPerigee / 1000.0, historyPerigee / 1852.0);
		oapiWriteLogV(" > Inclination angle: %.2f deg", historyInclination); // only true for full orbit
	}
	oapiWriteLogV(" > Maximum exit load factor: %.1f g", historyMaxLaunchAcc / G);
	oapiWriteLogV(" > Maximum reentry load factor: %.1f g", historyMaxReentryAcc / G);
	oapiWriteLogV(" > Earth-fixed velocity: %.0f m/s (%.0f ft/s)", historyMaxEarthSpeed, historyMaxEarthSpeed / 0.3048);
	oapiWriteLogV(" > Space-fixed velocity: %.0f m/s (%.0f ft/s)", historyMaxSpaceSpeed, historyMaxSpaceSpeed / 0.3048);

	// Spend the opportunity to destroy the panel mesh. If we don't do this, it will be buggy if we relaunch a scenario with panel.
	//if (cockpitPanelMesh) oapiDeleteMesh(cockpitPanelMesh);
	//if (periscopeMesh) oapiDeleteMesh(periscopeMesh);
}

inline void ProjectMercury::ReadConfigSettings(FILEHANDLE cfg)
{
	FILEHANDLE commonCfg = oapiOpenFile("Vessels\\ProjectMercury\\ProjectMercuryCommon.cfg", FILE_IN_ZEROONFAIL, CONFIG);
	bool foundCommonFile = true;

	if (commonCfg == 0)
	{
		oapiWriteLog("ProjectMercuryCommon config file missing!");
		commonCfg = cfg; // maybe there's some data we can read from there, so use it as backup
		foundCommonFile = false;
	}

	// Read height over ground from config
	if (!oapiReadItem_float(cfg, "HeightOverGround", heightOverGround))
	{
		heightOverGround = 5.8; // if not available in config file
		oapiWriteLog("Mercury could not read height over ground config.");
	}

	// Read cutoff velocity from config
	if (!oapiReadItem_float(cfg, "SpeedIntegratorCutoff", integratedSpeedLimit))
	{
		integratedSpeedLimit = 2130.0;
		oapiWriteLog("Mercury could not read speed integrator cutoff config.");
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
	if (!oapiReadItem_int(commonCfg, "HUD2ndColumnPos", secondColumnHUDx))
	{
		secondColumnHUDx = 28;
		oapiWriteLog("Mercury could not read HUD column position config.");
	}

	if (!oapiReadItem_float(commonCfg, "TimeStepLimit", timeStepLimit))
	{
		timeStepLimit = 0.1;
		oapiWriteLog("Mercury could not read time step limit config for authentic autopilot.");
	}

	if (!oapiReadItem_bool(commonCfg, "MercuryNetwork", MercuryNetwork))
	{
		MercuryNetwork = true;
		oapiWriteLog("Mercury could not read network contact config.");
	}

	if (!oapiReadItem_float(commonCfg, "JoysticThrustThresholdLow", joystickThresholdLow))
	{
		joystickThresholdLow = 0.33;
		oapiWriteLog("Mercury could not read joystick low threshold.");
	}

	if (!oapiReadItem_float(commonCfg, "JoysticThrustThresholdHigh", joystickThresholdHigh))
	{
		joystickThresholdHigh = 0.75;
		oapiWriteLog("Mercury could not read joystick high threshold.");
	}

	char caps[NUMBER_SUPPORTED_CONFIG_CAPSULES][1000];
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE0", caps[0])) // Is defined
	{
		int newIndex = 0;

		oapiWriteLogV("====BEGIN READ CONFIG CAPSULE %i=====", newIndex);
		oapiWriteLog(caps[newIndex]); // Write out what is stored in config file

		// Read capsule number base
		configTextureUserBasis[newIndex] = atoi(caps[newIndex]); // atoi disregards any charcters after integral number

		// Read name tag of capsule (to be called in scenario)
		int firstCommaPos = strchr(caps[newIndex], ',') - caps[newIndex];
		myStrncpy(configTextureUserName[newIndex], caps[newIndex] + 2, firstCommaPos - 2); // Read rest of string until first comma. +- 2 because we omit the first integer, and therefore have a shorter string

		// Read all texture replacements
		char* comma = strchr(caps[newIndex] + firstCommaPos, ',');
		while (comma != NULL)
		{
			int beginningComma = comma - caps[newIndex] + 1;

			// Texture string is ended by new comma. Find it
			comma = strchr(caps[newIndex] + beginningComma + 1, ','); // location of next comma
			int endingComma = strlen(caps[newIndex]); // default end of line
			if (comma != NULL) endingComma = comma - caps[newIndex]; // if there's a new comma, set to that pos instead

			// Now parse the string to the texture handler
			char TexString[100];
			myStrncpy(TexString, caps[newIndex] + beginningComma + 1, endingComma - beginningComma - 1); // get the string
			ReadCapsuleTextureReplacement(TexString); // work with it
		}

		oapiWriteLogV("====END READ CONFIG CAPSULE %i=====", newIndex);
	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE1", caps[1])) // Is defined, DEBUG
	{

	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE2", caps[2])) // Is defined
	{

	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE3", caps[3])) // Is defined
	{

	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE4", caps[4])) // Is defined
	{

	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE5", caps[5])) // Is defined
	{

	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE6", caps[6])) // Is defined
	{

	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE7", caps[7])) // Is defined
	{

	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE8", caps[8])) // Is defined
	{

	}
	if (oapiReadItem_string(commonCfg, "DEFINENEWCAPSULE9", caps[9])) // Is defined
	{

	}

	if (foundCommonFile) oapiCloseFile(commonCfg, FILE_IN_ZEROONFAIL); // don't close if the regular cfg file
}

inline void ProjectMercury::CreateCapsuleFuelTanks(void)
{
	// propellant resources, in order of last deleted to first
	fuel_manual = CreatePropellantResource(MERCURY_FUEL_MASS_MAN);
	//SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);

	fuel_auto = CreatePropellantResource(MERCURY_FUEL_MASS_AUTO);
	//SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);

	//attitudeFuelAuto = true;

	posigrade_propellant[0] = CreatePropellantResource(POSIGRADE_MASS_FUEL);
	//SetPropellantMass(posigrade_propellant[0], POSIGRADE_MASS_FUEL);

	posigrade_propellant[1] = CreatePropellantResource(POSIGRADE_MASS_FUEL);
	//SetPropellantMass(posigrade_propellant[1], POSIGRADE_MASS_FUEL);

	posigrade_propellant[2] = CreatePropellantResource(POSIGRADE_MASS_FUEL);
	//SetPropellantMass(posigrade_propellant[2], POSIGRADE_MASS_FUEL);

	retro_propellant[0] = CreatePropellantResource(RETRO_MASS_FUEL);
	//SetPropellantMass(retro_propellant[0], RETRO_MASS_FUEL);

	retro_propellant[1] = CreatePropellantResource(RETRO_MASS_FUEL);
	//SetPropellantMass(retro_propellant[1], RETRO_MASS_FUEL);

	retro_propellant[2] = CreatePropellantResource(RETRO_MASS_FUEL);
	//SetPropellantMass(retro_propellant[2], RETRO_MASS_FUEL);

	//SetPropellantMass(escape_tank, ABORT_MASS_FUEL);

}

inline void ProjectMercury::AddDefaultMeshes(void)
{
	Adaptcover1 = AddMesh(adaptcover1, &OFS_ADAPTCOVER1);
	Adaptcover2 = AddMesh(adaptcover2, &OFS_ADAPTCOVER2);
	Adaptcover3 = AddMesh(adaptcover3, &OFS_ADAPTCOVER3);
	Adaptring1 = AddMesh(adaptring1, &OFS_ADAPTRING1);
	Adaptring2 = AddMesh(adaptring2, &OFS_ADAPTRING2);
	Adaptring3 = AddMesh(adaptring3, &OFS_ADAPTRING3);
	Tower = AddMesh(tower, &ABORT_OFFSET);

	Droguecover = AddMesh(droguecover, &MERCURY_OFS_DROGUECOVER);

	//VcFrame = AddMesh(vcFrame, &VC_FRAME_OFS);
	//SetMeshVisibilityMode(VcFrame, MESHVIS_VC | MESHVIS_ALWAYS); // Debug
}

inline void ProjectMercury::CapsuleGenericPostCreation(void)
{
	// HUD constants
	oapiGetViewportSize(&ScreenWidth, &ScreenHeight, &ScreenColour);
	oapiWriteLogV("Project Mercury registered screen of width %i and height %i.", ScreenWidth, ScreenHeight);
	TextX0 = (int)(0.025 * ScreenWidth);
	TextY0 = (int)(0.225 * ScreenHeight);
	LineSpacing = (int)(0.025 * ScreenHeight);

	if (!capsuleDefined) // something went wrong in the scenario reading
	{
		oapiWriteLog("No Mercury capsule specified; using Freedom 7II");
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Freedom7II");
		Capsule = AddMesh(capsule, &MERCURY_OFS_CAPSULE);
		CapsuleVersion = FREEDOM7II;
		heatShieldGroup = 29;

		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
		Antennahouse = AddMesh(antennahouse, &MERCURY_OFS_ANTHOUSE);

		Retro = AddMesh(retro, &MERCURY_OFS_RETRO);
		Retrocover1 = AddMesh(retrocover1, &MERCURY_OFS_RETROCOVER1);
		Retrocover2 = AddMesh(retrocover2, &MERCURY_OFS_RETROCOVER2);
		Retrocover3 = AddMesh(retrocover3, &MERCURY_OFS_RETROCOVER3);
		Retrostrap1 = AddMesh(retrostrap1, &MERCURY_OFS_RETROSTRAP1);
		Retrostrap2 = AddMesh(retrostrap2, &MERCURY_OFS_RETROSTRAP2);
		Retrostrap3 = AddMesh(retrostrap3, &MERCURY_OFS_RETROSTRAP3);
		Explosivebolt = AddMesh(explosivebolt, &MERCURY_OFS_EXPLOSIVEBOLT);
	}

	DefinePeriscopeAnimation(); // must be defined after capsule creation
	DefineAntennaDestabiliser();
	if (configTextureUserEnable || scenarioTextureUserEnable) LoadCapsuleTextureReplacement();

	// Set basic start coordinate for history recording. It requires to be in launch state, so just get the current pos in case we are not in launch.
	double radNoCare;
	GetEquPos(historyLaunchLong, historyLaunchLat, radNoCare);

	if (GetDamageModel() == 0) // overwrite any scenario definition
	{
		enableAbortConditions = false;
		FailureMode = NONE;
	}

	// Failure implementation
	if (!difficultyHard && (FailureMode == LOWGACTIVATE || FailureMode == ATTSTUCKON || FailureMode == RETROSTUCKON))
		FailureMode = NONE; // difficultyHard must be true for the above modes to be function

	double absolutelyHappens = 1.0 * 86400.0; // Happens within a day
	timeOfError = GenerateRandom01() * absolutelyHappens; // the T+ time of an error occuring. Extra requirement must be that timeAcc is 1 (or below)

	char errorMsg[256];
	switch (FailureMode)
	{
	case NONE:
		sprintf(errorMsg, "No failures generated");
		break;
	case LOWGACTIVATE: // HARDMODE
		sprintf(errorMsg, "Failure generated with 0.05 G going on at random at T+%i s", (int)timeOfError);
		break;
	case LOWGDEACTIVE:
		sprintf(errorMsg, "Failure generated with 0.05 G flag not activating");
		break;
	case ATTSTUCKON: // HARDMODE
		attitudeThrusterErrorNum = rand() % 18;
		sprintf(errorMsg, "Failure generated with thruster no. %i going on at random at T+%i s", attitudeThrusterErrorNum, (int)timeOfError);
		break;
	case ATTSTUCKOFF:
		attitudeThrusterErrorNum = rand() % 18;
		sprintf(errorMsg, "Failure generated with thruster no. %i not functioning after T+%i s", attitudeThrusterErrorNum, (int)timeOfError);
		break; 
	case ATTMODEOFF:
		sprintf(errorMsg, "Failure generated with automatic attitude not functioning");
		break; 
	case RETROSTUCKOFF:
		retroErrorNum = rand() % 3;
		sprintf(errorMsg, "Failure generated with retrorocket no. %i not firing", retroErrorNum);
		break; 
	case RETROSTUCKON: // HARDMODE
		retroErrorNum = rand() % 3;
		sprintf(errorMsg, "Failure generated with retrorocket no. %i going on at random at T+%i s", retroErrorNum, (int)timeOfError);
		break; 
	case RETRONOSEP:
		sprintf(errorMsg, "Failure generated with inability to separate retropack");
		break; 
	case MAINCHUTETORN:
		chuteDestroyFactor = sqrt(GenerateRandom01()); // sqrt, beacuse chute are depends on square. A 50 % destroy factor does not double fall speed. And that is boring.
		sprintf(errorMsg, "Failure generated with main chute being %.1f %%%% broken", chuteDestroyFactor * 100.0); // aparrently, this sprintf is getting printf'd in oapiWriteLog, so %%%% generates %%, which generates % in Orbiter.log
		break; 
	case NOLANDBAG:
		sprintf(errorMsg, "Failure generated with failed landing bag deployment");
		break; 
	case RETROCALCOFF:
		sprintf(errorMsg, "Failure generated with error in retrosequence computing at T+%i s", (int)timeOfError);
		break; 
	case BOOSTERDEVIATIONSPEED:
		speedError = GenerateRandomNorm() * 30.0; // Feel free to play with this value (the standard deviation). The go-no-go plot gives roughly 45 m/s each way. With 30, that means failed orbit in 13 % of cases.
		sprintf(errorMsg, "Failure generated with final launch speed deviation of %.1f m/s", speedError);
		break;
	case BOOSTERPROBLEM:
		timeOfError = GenerateRandom01() * 400.0; // On Redstone, this is probability 35 % of happening, while for Atlas is 75 %. OK, because Redstone is simpler=safer than Atlas
		sprintf(errorMsg, "Failure generated during launch at T+%i s", (int)timeOfError);
		break; 
	case ATTITUDEOFFSET:
		// Pick a1 and a2 that seem funny or realistic
		double a1 = 0.3;
		double a2 = 0.5;
		pitchOffset = GenerateRandomAngleNorm(a1, a2);
		yawOffset = GenerateRandomAngleNorm(a1, a2);
		rollOffset = GenerateRandomAngleNorm(a1, a2);
		
		sprintf(errorMsg, "Failure generated in capsule attitude with offsets P: %.1f deg, Y: %.1f, R: %.1f", pitchOffset * DEG, yawOffset * DEG, rollOffset * DEG);
		break;
	}

	oapiWriteLog(errorMsg);
}

// Control every attitude setting, from ASCS norm, to aux damp, to fly-by-wire, to rate command, to fully manual
inline void ProjectMercury::CapsuleAttitudeControl(double simt, double simdt)
{
	if (!(VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) || !rcsExists) return; // we must be in a free capsule state, so that the RCS exists

	// Set all thrusters to zero first
	DisableAutopilot(false);

	// ASCS autopilot
	if (switchASCSMode == -1 && switchControlMode == -1)
	{
		if (VesselStatus == FLIGHT)
		{
			if (AutopilotStatus == POSIGRADEDAMP)
			{
				AuxDampingAuto(true);
				if (!posigradeDampingActivated)
				{
					posigradeDampingTime = simt;
					posigradeDampingActivated = true;
				}
				else if (posigradeDampingActivated && simt - posigradeDampingTime > 5.0) // Damping for 5 seconds (MR-4 flight profile graphic)
				{
					AutopilotStatus = TURNAROUND;
				}
			}
			else if (AutopilotStatus == TURNAROUND)
			{
				RetroAttitudeAuto(simt, simdt, true);
			}
			else if (AutopilotStatus == ORBITATTITUDE)
			{
				RetroAttitudeAuto(simt, simdt, false);
			}
			else if (AutopilotStatus == RETROATTITUDE)
			{
				RetroAttitudeAuto(simt, simdt, true);
			}

			//if (engageRetro && simt - retroStartTime > 60.0 && switchAutoRetroJet == -1) // both for suborbital and orbital missions (19640056774 page 12 and doi:10.1002/j.2161-4296.1962.tb02524.x page 2)
			//{
			//	prepareReentryAction = true;
			//	AutopilotStatus = REENTRYATT;
			//}
		}
		else if (VesselStatus == REENTRY)
		{
			if (AutopilotStatus == REENTRYATTITUDE)
			{
				ReentryAttitudeAuto(simt, simdt);

				if (vesselAcceleration > 0.05 * G && FailureMode != LOWGDEACTIVE)
				{
					GetRelativeVel(GetSurfaceRef(), entryVel);
					GetRelativePos(GetSurfaceRef(), entryLoc);

					entryAng = -acos(dotp(entryLoc, entryVel) / length(entryLoc) / length(entryVel)) * DEG + 90.0;

					double radius;
					GetEquPos(lowGLong, lowGLat, radius);

					entryAngleToBase = oapiOrthodome(lowGLong, lowGLat, missionLandLong * RAD, missionLandLat * RAD);

					double heading;
					oapiGetHeading(GetHandle(), &heading);
					double slip = OrbitalFrameSlipAngle2(entryLoc, entryVel);
					lowGHeading = heading - slip;
					if (lowGHeading < 0.0) lowGHeading += PI2;
					else if (lowGHeading > PI2) lowGHeading -= PI2;

					char cbuf[256];
					sprintf(cbuf, "0.05 G trigger at T+%.0f. Entry angle: %.3f deg. Entry velocity %.2f m/s. Altitude %.0f m. Angle to target base %.3f deg", simt - launchTime, entryAng, length(entryVel), length(entryLoc) - oapiGetSize(GetSurfaceRef()), entryAngleToBase * DEG);
					oapiWriteLog(cbuf);
					DisableAutopilot(false); // disable stuck thrusters
					AutopilotStatus = LOWG;
				}
			}
			else if (AutopilotStatus == LOWG)
			{
				GRollAuto(simt, simdt);
			}
		}
	}
	
	// ASCS .05G mode, debug, check MA-9 transcript and logic for low g mode. Potentially move this to ASCS autopilot directly above
	if (switchASCSMode == -1 && FailureMode == LOWGACTIVATE && simt - launchTime > timeOfError && oapiGetTimeAcceleration() <= 1.0)
	{
		DisableAutopilot(false); // disable stuck thrusters
		AutopilotStatus = LOWG;
	}

	SetPropellantMass(dummyControllerFuel, 1e-3); // must always have fuel in the dummy tank to have thruster groups to work.
	double thrustLevelPitchup = GetThrusterGroupLevel(THGROUP_ATT_PITCHUP);
	double thrustLevelPitchdown = GetThrusterGroupLevel(THGROUP_ATT_PITCHDOWN);
	double thrustLevelYawright = GetThrusterGroupLevel(THGROUP_ATT_YAWRIGHT);
	double thrustLevelYawleft = GetThrusterGroupLevel(THGROUP_ATT_YAWLEFT);
	double thrustLevelBankright = GetThrusterGroupLevel(THGROUP_ATT_BANKRIGHT);
	double thrustLevelBankleft = GetThrusterGroupLevel(THGROUP_ATT_BANKLEFT);
	double totalThrustLevel = thrustLevelPitchup + thrustLevelPitchdown + thrustLevelYawright + thrustLevelYawleft + thrustLevelBankright + thrustLevelBankleft;

	// Fly-by-wire
	if (switchASCSMode == 1) // as far as I understand, the FLY BY WIRE switch alone responsible for fly by wire. The manualHandle has no effect.
	{
		// Pitch
		THRUSTER_HANDLE high = thruster_auto_py[0], low = thruster_auto_py_1lb[0];
		FlyByWireControlSingleDirection(thrustLevelPitchup, high, low, tHandlePitchPushed);

		high = thruster_auto_py[1], low = thruster_auto_py_1lb[1];
		FlyByWireControlSingleDirection(thrustLevelPitchdown, high, low, tHandlePitchPushed);

		// Yaw
		high = thruster_auto_py[2], low = thruster_auto_py_1lb[2];
		FlyByWireControlSingleDirection(thrustLevelYawright, high, low, tHandleYawPushed);

		high = thruster_auto_py[3], low = thruster_auto_py_1lb[3];
		FlyByWireControlSingleDirection(thrustLevelYawleft, high, low, tHandleYawPushed);

		// Roll
		high = thruster_auto_roll[0], low = thruster_auto_roll_1lb[0];
		FlyByWireControlSingleDirection(thrustLevelBankright, high, low, tHandleRollPushed);

		high = thruster_auto_roll[1], low = thruster_auto_roll_1lb[1];
		FlyByWireControlSingleDirection(thrustLevelBankleft, high, low, tHandleRollPushed);
	}

	// Manual mode
	if (!tHandleManualPushed)
	{
		// Pitch
		SetThrusterLevel(thruster_man_py[0], thrustLevelPitchup);

		SetThrusterLevel(thruster_man_py[1], thrustLevelPitchdown);

		// Yaw
		SetThrusterLevel(thruster_man_py[2], thrustLevelYawright);

		SetThrusterLevel(thruster_man_py[3], thrustLevelYawleft);

		// Roll
		SetThrusterLevel(thruster_man_roll[0], thrustLevelBankright);

		SetThrusterLevel(thruster_man_roll[1], thrustLevelBankleft);

		// Optional aux damp is controlled in the end of the function
	}

	// Rate command (RSCS), where it will "provide angular rates proportional to controller deflection, and in the absence of commands provides rate damping".
	// "Like the ASCS, the RSCS is an on-off system, but it uses fuel from the manual system supply."
	if (tHandleManualPushed/* && switchASCSMode == -1*/ && switchControlMode == 1) // I believe we must be in ASCS NORM. Familiarization page 126: "AUTO / RATE COMD switch provides a method of energizing either the RSCS or ASCS systems".
	{																			// That was wrong. In MA6_FlightOps.pdf page 47, pilot is instructed to used both Rate command AND FlyByWire.
		// Familiarization manual page 147:
		// "The dead band adjustments which are provided are a 2 deg/s to 4 deg/s in pitch and yaw axis, and a 1.5 deg/s to 3 deg/s in the roll axis."
		// But in familiarization manual on page 145:
		// "The rate portion of the indicator is driven by the miniature rate transducers which also serve as sensing elements for the Rate Stabilization
		// Control System. The range of the rate indication is 0 to +- 6 deg/s for all three indicators."
		// I assume this means that the rate command range is equal to the rate indication range, i.e. +- 6 deg/s.
		// In the old Project Mercury addon at least, it was +-10 deg/s. But in my opinion, its entire attitude implementation isn't very sophisticated, so it doesn't have too much credibility.
		// But then, on the other hand, seeing that the resolution was 3.0 deg/s (see below), it doesn't make sense to be limited to 6 deg/s. So maybe we should try 10 deg/s afterall ...

		VECTOR3 angVel;
		GetAngularVel(angVel);

		//const double RSCSresolution = 0.1 * RAD; // the resolution of the damping, so that it doesn't try to dampen an e.g. 1e-10 rate. This is entirely my own value, no historic valuue.
		const double RSCSresolution = 3.0 * RAD; // the resolution of the damping, 3.0 deg/s according to MA6_FlightPlan2.pdf page 39. AUX damping (+- 0.5 deg/s) is thus more precise than RSCS damping.
		const double MaxPitchRate = 10.0 * RAD; // 6.0 * RAD; // 4.0 * RAD;
		const double MaxYawRate = 10.0 * RAD; // 6.0 * RAD; // 4.0 * RAD;
		const double MaxRollRate = 10.0 * RAD; // 6.0 * RAD; // 3.0 * RAD;

		// If no input, the target rate will be Max * (0 - 0) = 0, and automatically damp. Thumbs up for that.

		// Pitch
		double targetPitchRate = MaxPitchRate * (thrustLevelPitchup - thrustLevelPitchdown);
		if (angVel.x < targetPitchRate - RSCSresolution)
			SetThrusterLevel(thruster_man_py[0], 1.0);
		else
			SetThrusterLevel(thruster_man_py[0], 0.0);

		if (angVel.x > targetPitchRate + RSCSresolution)
			SetThrusterLevel(thruster_man_py[1], 1.0);
		else
			SetThrusterLevel(thruster_man_py[1], 0.0);

		// Yaw
		double targetYawRate = MaxYawRate * (thrustLevelYawright - thrustLevelYawleft);
		if (-angVel.y < targetYawRate - RSCSresolution)
			SetThrusterLevel(thruster_man_py[2], 1.0);
		else
			SetThrusterLevel(thruster_man_py[2], 0.0);

		if (-angVel.y > targetYawRate + RSCSresolution)
			SetThrusterLevel(thruster_man_py[3], 1.0);
		else
			SetThrusterLevel(thruster_man_py[3], 0.0);

		// Roll
		// During .05G, RSCS gives 7.7 degree/s roll if no joystick input.
		// Familiarization page 147: "with the control stick at zero deflection (Rate Stabilization Control System operational) an automatic three axis ratedamper is achieved, including an automatic 7.7+-2 deg/s constant roll rate when re-entry is sensed."
		if (thrustLevelBankright + thrustLevelBankleft == 0.0 && FailureMode != LOWGDEACTIVE && VesselStatus == REENTRY && (vesselAcceleration > 0.05 * G || AutopilotStatus == LOWG))
		{
			if (angVel.z > 9.7 * RAD)
			{
				SetThrusterLevel(thruster_man_roll[1], 1.0);
				SetThrusterLevel(thruster_man_roll[0], 0.0); // ensure not stuck
			}
			else if (angVel.z < 7.7 * RAD)
			{
				SetThrusterLevel(thruster_man_roll[0], 1.0);
				SetThrusterLevel(thruster_man_roll[1], 0.0); // ensure not stuck
			}
			else
			{
				SetThrusterLevel(thruster_man_roll[0], 0.0);
				SetThrusterLevel(thruster_man_roll[1], 0.0);
			}
		}
		else
		{
			double targetRollRate = MaxRollRate * (thrustLevelBankright - thrustLevelBankleft);
			if (angVel.z < targetRollRate - RSCSresolution)
				SetThrusterLevel(thruster_man_roll[0], 1.0);
			else
				SetThrusterLevel(thruster_man_roll[0], 0.0);

			if (angVel.z > targetRollRate + RSCSresolution)
				SetThrusterLevel(thruster_man_roll[1], 1.0);
			else
				SetThrusterLevel(thruster_man_roll[1], 0.0);
		}
	}

	// Aux damp mode, which requires RSCS switch in AUTO (switchControlMode == -1)
	//if (switchControlMode == -1 && totalThrustLevel == 0.0 && !(switchASCSMode == -1)) // NOT in ASCS normal mode, which controls this itself
	// On the other hand, apparently it is possible to yaw at a 2 deg/s rate using FBW (MA6_FlightPlan2.pdf page 40), so there should then be no rate damping.
	// So constraint should be ASCS switch in AUX damp and RSCS switch in Auto
	if (switchControlMode == -1 && totalThrustLevel == 0.0 && switchASCSMode == 0)
	{
		AuxDampingAuto(true);
	}

	// Hardmode spontanious attitude thruster firing
	if (FailureMode == ATTSTUCKON && simt - launchTime > timeOfError && oapiGetTimeAcceleration() <= 1.0) // we are not THAT mean that we do this during time acc
	{
		THRUSTER_HANDLE thFail;
		if (attitudeThrusterErrorNum < 4)
			thFail = thruster_man_py[attitudeThrusterErrorNum];
		else if (attitudeThrusterErrorNum < 6)
			thFail = thruster_man_roll[attitudeThrusterErrorNum - 4];
		else if (attitudeThrusterErrorNum < 10)
			thFail = thruster_auto_py[attitudeThrusterErrorNum - 6];
		else if (attitudeThrusterErrorNum < 14)
			thFail = thruster_auto_py_1lb[attitudeThrusterErrorNum - 10];
		else if (attitudeThrusterErrorNum < 16)
			thFail = thruster_auto_roll[attitudeThrusterErrorNum - 14];
		else
			thFail = thruster_auto_roll_1lb[attitudeThrusterErrorNum - 16];

		SetThrusterLevel(thFail, 1.0);
	}

	// Disable auto thrusters if handles are pulled.
	if (!tHandlePitchPushed)
	{
		SetThrusterLevel(thruster_auto_py[0], 0.0);
		SetThrusterLevel(thruster_auto_py[1], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[0], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[1], 0.0);
	}

	if (!tHandleYawPushed)
	{
		SetThrusterLevel(thruster_auto_py[2], 0.0);
		SetThrusterLevel(thruster_auto_py[3], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[2], 0.0);
		SetThrusterLevel(thruster_auto_py_1lb[3], 0.0);
	}

	if (!tHandleRollPushed)
	{
		SetThrusterLevel(thruster_auto_roll[0], 0.0);
		SetThrusterLevel(thruster_auto_roll[1], 0.0);
		SetThrusterLevel(thruster_auto_roll_1lb[0], 0.0);
		SetThrusterLevel(thruster_auto_roll_1lb[1], 0.0);
	}
}

inline void ProjectMercury::FlyByWireControlSingleDirection(double thrustLevel, THRUSTER_HANDLE high, THRUSTER_HANDLE low, bool tHandlePushed)
{
	// Limits from twombly1962.pdf page 3.
	// Note that this does not fit nicely with Orbiter's Ctrl+Num combination, which gives 0.1 thrust, and therefore will not give any thrust.
	double lowDeflectionLimit = joystickThresholdLow; // 0.33, now set in config file
	double highDeflectionLimit = joystickThresholdHigh; // 0.75, now set in config file

	if (thrustLevel >= highDeflectionLimit && tHandlePushed)
	{
		SetThrusterLevel(high, 1.0);
		SetThrusterLevel(low, 0.0);
	}
	else if (thrustLevel >= lowDeflectionLimit && tHandlePushed)
	{
		SetThrusterLevel(high, 0.0);
		SetThrusterLevel(low, 1.0);
	}
	else
	{
		SetThrusterLevel(high, 0.0);
		SetThrusterLevel(low, 0.0);
	}
}

// This function always runs every timestep, from clbkPostStep.
inline void ProjectMercury::MercuryCapsuleGenericTimestep(double simt, double simdt, double latit, double longit, double getAlt)
{
	// If we're in panel mode with 2D panel, then animate. Note that oapiCameraInternal only gives true if in focus object, so first condition is not necessary, but better safe than sorry, especially considering potential later Orbiter verion changing it.
	if (oapiGetFocusObject() == GetHandle() && oapiCameraInternal() && oapiCockpitMode() == COCKPIT_PANELS && panelView)
	{
		AnimateDials();
	}

	//// Rate damping if aborting (19670028606 page 97 and 98)
	//if (abort && VesselStatus == REENTRY)
	//{
	//	// Debug, this should only be activated for an escape tower abort!
	//	//autoPilot = true;
	//	AutopilotStatus = LOWG;
	//}

	// Calculate G force for activation of tower & retro separation at 0.25 G
	double m = GetMass();
	VECTOR3 F, W;
	GetForceVector(F);
	GetWeightVector(W);
	vesselAcceleration = length((F - W) / m);
	longitudinalAcc = (F.z - W.z) / m;

	if (GroundContact() && vesselAcceleration < 0.1)
	{
		vesselAcceleration = GGRAV * oapiGetMass(GetSurfaceRef()) / oapiGetSize(GetSurfaceRef()) / oapiGetSize(GetSurfaceRef()); // Orbiter bug gives zero acc on surface
		longitudinalAcc = vesselAcceleration;
	}

	if (longitudinalAcc > maxVesselAcceleration) maxVesselAcceleration = longitudinalAcc;
	if (longitudinalAcc < minVesselAcceleration) minVesselAcceleration = longitudinalAcc;

	// Retrosequence
	if (engageRetro && simt > retroStartTime && VesselStatus == FLIGHT) // firing starts 30 sec after retrosequence start
	{
		FireRetroRocket(0);

		if (simt > (retroStartTime + 5.0))
		{
			FireRetroRocket(1);

			if (simt > (retroStartTime + 10.0))
			{
				FireRetroRocket(2);
			}
		}

		if (simt > retroStartTime + 60.0 && switchAutoRetroJet == -1) // both for suborbital and orbital missions (19640056774 page 12 and doi:10.1002/j.2161-4296.1962.tb02524.x page 2)
		{
			prepareReentryAction = true;
			AutopilotStatus = REENTRYATTITUDE;
		}
	}

	// Hardmode sponatious retro firing
	if (FailureMode == RETROSTUCKON && simt - launchTime > timeOfError && oapiGetTimeAcceleration() <= 1.0) // we are not THAT mean that we do this during time acc
	{
		SetThrusterLevel(thruster_retro[retroErrorNum], 1.0);
		
		RETRO_THRUST_LEVEL[retroErrorNum] = GetThrusterLevel(thruster_retro[retroErrorNum]); // For local lights
		
		if (!retroCoverSeparated[retroErrorNum] && RETRO_THRUST_LEVEL[retroErrorNum] > 0.0)
		{
			separateRetroCoverAction[retroErrorNum] = true;
		}
	}

	// Jettison retro if aerodynamic stresses are too high (a la MA-6)
	double ReentryStresses = 0.5 * GetAtmDensity() * pow(GetAirspeed(), 3);
	if (VesselStatus == FLIGHT && ReentryStresses > 1e6) // roughly five minutes before drogue opening, as indicated by MA6 transscript (~85 km)
	{
		oapiWriteLog("High dynamic stress seperate retro");
		prepareReentryAction = true;
	}

	if (engageFuelDump)
	{
		DumpFuelRCS();
	}

	// Deploy drogue
	if (VesselStatus == REENTRY && GetAtmPressure() > 44700.0 && !drogueDeployed && simt - towerJetTime > 3.0) // below 2.1e4 feet. Chutes deployed no less than 3 sec after towerJett (19630012071 page 269)
	{
		char cbuf[256];

		double angleCovered = oapiOrthodome(longit, latit, lowGLong, lowGLat);

		sprintf(cbuf, "Drogue deployed at %.2f ft. Angle covered since 0.05 G is %.3f deg", getAlt / 0.3048, angleCovered * DEG);
		oapiWriteLog(cbuf);

		// Debug
		std::ofstream outfile;
		outfile.open("MercuryEntryLog.txt", std::ios_base::app); // app = append
		outfile << entryAng << " , " << length(entryVel) << " , " << (length(entryLoc) - oapiGetSize(GetSurfaceRef())) << " , " << (entryAngleToBase * DEG) << " , " << (angleCovered * DEG) << " , " << (lowGLat * DEG) << " , " << (lowGHeading * DEG) << "\n";
		// eAng, eVel, eAlt, ang to targ base, angle covered during reentry, eLat, eHead

		separateDrogueCoverAction = true;
		DeployDrogue();
	}
	else if (VesselStatus == REENTRY && GetAtmPressure() > 69700.0 && drogueDeployed && simt - drogueDeployTime > 2.0)
	{
		separateDrogueAction = true;
	}
	else if (VesselStatus == REENTRYNODROGUE && GetAtmPressure() > 69700.0 && simt - towerJetTime > 3.0 && !mainChuteDeployed && simt - drogueDeployTime > 2.0) // below 1e4 feet
	{
		char cbuf[256];
		sprintf(cbuf, "Main deployed at %.2f ft", getAlt / 0.3048);
		oapiWriteLog(cbuf);
		DeployMainChute();
	}
	else if (!landingBagDeployed && mainChuteDeployed && simt - mainChuteDeployTime > 12.0)
	{
		DeployLandingBag();
	}

	// Record mission data
	if (getAlt > historyMaxAltitude)
		historyMaxAltitude = getAlt;

	if (GetGroundspeed() > historyMaxEarthSpeed)
		historyMaxEarthSpeed = GetGroundspeed();

	VECTOR3 currentSpaceVelocity;
	GetRelativeVel(GetSurfaceRef(), currentSpaceVelocity);
	double currentSpaceSpeed = sqrt(pow(currentSpaceVelocity.x, 2) + pow(currentSpaceVelocity.y, 2) + pow(currentSpaceVelocity.z, 2));
	if (currentSpaceSpeed > historyMaxSpaceSpeed)
		historyMaxSpaceSpeed = currentSpaceSpeed;

	if ((VesselStatus == LAUNCH || VesselStatus == TOWERSEP || VesselStatus == ABORTNORETRO) && GroundContact() && GetAttachmentStatus(padAttach) == NULL && (GetFuelMass() / GetPropellantMaxMass(GetDefaultPropellantResource())) > 0.9) // TOWERSEP is to catch a Big Joe scenario, ABORTNORETRO is to cactch a Beach Abort scenario. Fuel mass check is to ensure that we are loaded, used to catch MR-BD, which lands in same state as it launches, and therefore would overwrite launchCoord
	{
		historyLaunchLong = longit;
		historyLaunchLat = latit;
		historyReference = GetSurfaceRef();
		oapiGetHeading(GetHandle(), &historyLaunchHeading);
		historyLaunchHeading *= DEG;
	}

	if (VesselStatus == REENTRYNODROGUE && GroundContact() && historyLandLong == 0.0)
	{
		historyLandLong = longit;
		historyLandLat = latit;
	}

	if (VesselStatus == REENTRY && vesselAcceleration > historyMaxReentryAcc)
		historyMaxReentryAcc = vesselAcceleration;

	if (!GroundContact() && vesselAcceleration < 0.05 * G) // Orbiter sometimes give 0 G when landed, so assure no ground contact
	{
		historyWeightlessTime += simdt;
	}
}

inline void ProjectMercury::WriteHUDAutoFlightReentry(oapi::Sketchpad* skp, double simt, int* yIndexUpdate, char* cbuf)
{
	int yIndex = *yIndexUpdate;

	if (VesselStatus == FLIGHT)
	{
		if (switchASCSMode == -1 && switchControlMode == -1 && AutopilotStatus == POSIGRADEDAMP)
		{
			sprintf(cbuf, "Capsule turnaround in %.2f s", 5.0 + posigradeDampingTime - simt);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (switchASCSMode == -1 && switchControlMode == -1 && AutopilotStatus == TURNAROUND)
		{
			sprintf(cbuf, "Hold retro attitude");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  P: %.1f\u00B0, Y: %.1f\u00B0, R: %.1f\u00B0", NormAngleDeg(GetPitch() * DEG + pitchOffset * DEG), NormAngleDeg(GetSlipAngle() * DEG + yawOffset * DEG), NormAngleDeg(GetBank() * DEG + rollOffset * DEG));
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (switchASCSMode == -1 && switchControlMode == -1 && AutopilotStatus == ORBITATTITUDE)
		{
			sprintf(cbuf, "Holding retro attitude");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  P: %.1f\u00B0, Y: %.1f\u00B0, R: %.1f\u00B0", NormAngleDeg(GetPitch() * DEG + pitchOffset * DEG), NormAngleDeg(GetSlipAngle() * DEG + yawOffset * DEG), NormAngleDeg(GetBank() * DEG + rollOffset * DEG));
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}

		if (engageRetro && simt - retroStartTime < 60.0)
		{
			if (GetThrusterLevel(thruster_retro[0]) != 0.0)
			{
				sprintf(cbuf, "  Retro 1 BURNING");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (GetThrusterLevel(thruster_retro[1]) != 0.0)
			{
				sprintf(cbuf, "  Retro 2 BURNING");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (GetThrusterLevel(thruster_retro[2]) != 0.0)
			{
				sprintf(cbuf, "  Retro 3 BURNING");
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}

			if (simt - retroStartTime < 0.0)
			{
				sprintf(cbuf, "Retrofire in %.1f s", retroStartTime - simt);
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
			else
			{
				sprintf(cbuf, "Awaiting retropack separation in %.1f s", 60.0 + retroStartTime - simt);
				skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
				yIndex += 1;
			}
		}
	}
	else if (switchASCSMode == -1 && switchControlMode == -1 && VesselStatus == REENTRY)
	{
		if (AutopilotStatus == REENTRYATTITUDE)
		{
			sprintf(cbuf, "Holding reentry pitch:");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  %.1f\u00B0, %.1f\u00B0, %.1f\u00B0", NormAngleDeg(GetPitch() * DEG + pitchOffset * DEG), NormAngleDeg(GetSlipAngle() * DEG + yawOffset * DEG), NormAngleDeg(GetBank() * DEG + rollOffset * DEG));
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "Awaiting 0.05 G sensor");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (AutopilotStatus == LOWG)
		{
			VECTOR3 angVel;
			GetAngularVel(angVel);
			sprintf(cbuf, "Holding reentry roll: %.2f\u00B0/s", angVel.z * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
	}

	*yIndexUpdate = yIndex;
}

inline void ProjectMercury::WriteHUDIndicators(oapi::Sketchpad* skp, double simt, int* yIndexUpdate, char* cbuf)
{
	// Remember BBGGRR, not RRGGBB
	const DWORD Gray = 0xB0B0B0;
	const DWORD Red = 0x0000FF;
	const DWORD Green = 0x00FF00;

	int yIndex = *yIndexUpdate;

	// ABORT
	if (abort) skp->SetTextColor(Red);
	else skp->SetTextColor(Gray);
	sprintf(cbuf, "ABORT");
	skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
	yIndex += 1;

	// JETT TOWER
	if (towerJettisoned) skp->SetTextColor(Green); // I don't implement any failures of tower sep, so don't implement any failure light. This may change when fuses are added
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
	if (engageRetro && VesselStatus == FLIGHT) skp->SetTextColor(Green);
	else if (retroStartTime == 0.0) skp->SetTextColor(Gray); // uninitialised
	else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Gray);
	else skp->SetTextColor(Red);
	sprintf(cbuf, "RETRO SEQ");
	skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
	yIndex += 1;

	// RETRO ATT
	double currP = GetPitch() + pitchOffset;
	double currY = GetSlipAngle() + yawOffset;
	double currR = GetBank() + rollOffset;
	if (engageRetro && abs(currP + 34.0 * RAD) < 12.5 * RAD && abs(normangle(currY + PI)) < 30.0 * RAD && abs(currR) < 30.0 * RAD) skp->SetTextColor(Green); // within limits, Familiarization page 229
	else if (retroStartTime == 0.0) skp->SetTextColor(Gray); // haven't engaged retro
	else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Gray);
	else skp->SetTextColor(Red);
	sprintf(cbuf, "RETRO ATT");
	skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
	yIndex += 1;

	// FIRE RETRO
	if ((FailureMode != LOWGDEACTIVE && VesselStatus == REENTRY && vesselAcceleration > 0.05 * G) || AutopilotStatus == LOWG) skp->SetTextColor(Gray); // Disable if LOWG light is on
	else if ((retroStartTime != 0.0 && retroStartTime < simt) || (VesselStatus == FLIGHT && (retroCoverSeparated[0] || retroCoverSeparated[1] || retroCoverSeparated[2]))) skp->SetTextColor(Green); // green if retro is fired
	else if (retroStartTime == 0.0) skp->SetTextColor(Gray);
	else if (retroStartTime != 0.0 && retroStartTime - 15.0 < simt) skp->SetTextColor(Red); // red 15 sec before retrofire
	else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Green); // green until LOWG
	else skp->SetTextColor(Gray);
	sprintf(cbuf, "FIRE RETRO");
	skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
	yIndex += 1;

	// JETT RETRO
	if ((FailureMode != LOWGDEACTIVE && VesselStatus == REENTRY && vesselAcceleration > 0.05 * G) || AutopilotStatus == LOWG) skp->SetTextColor(Gray); // Disable if LOWG light is on //else if (retroStartTime == 0.0) skp->SetTextColor(Gray); // uninitialised
	else if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) skp->SetTextColor(Green); // Green until LOWG
	else if (retroStartTime != 0.0 && retroStartTime - 28.0 < simt) skp->SetTextColor(Red);
	else skp->SetTextColor(Gray);
	sprintf(cbuf, "JETT RETRO");
	skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
	yIndex += 1;

	// RETRACT SCOPE
	if (PeriscopeStatus != P_CLOSED && (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE || VesselStatus == ABORTNORETRO) && simt - retroStartTime > (60.0 + 40.0) && retroStartTime != 0.0) skp->SetTextColor(Red);
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

	// FUEL QUAN
	double autoLevel = GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_AUTO;
	double manualLevel = GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_MAN;
	//if (!attitudeFuelAuto) // swap (just how I've defined it)
	//{
	//	autoLevel = GetPropellantMass(fuel_manual) / MERCURY_FUEL_MASS_AUTO;
	//	manualLevel = GetPropellantMass(fuel_auto) / MERCURY_FUEL_MASS_MAN;
	//}
	if (autoLevel < 0.65 || manualLevel < 0.65) skp->SetTextColor(Red);
	else skp->SetTextColor(Gray);
	sprintf(cbuf, "FUEL QUAN");
	skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
	yIndex += 1;

	// RETRO WARN
	if (VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) skp->SetTextColor(Gray);
	else if (retroWarnLight) skp->SetTextColor(Red);
	else skp->SetTextColor(Gray);
	sprintf(cbuf, "RETRO WARN");
	skp->Text(TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
	yIndex += 1;

	*yIndexUpdate = yIndex;
}

inline void ProjectMercury::LoadCapsule(const char *cbuf)
{
	if (isalpha(cbuf[0])) // User specified capsule from config, e.g. "Demo"
	{
		oapiWriteLog("Scenario load user capsule.");
		for (int i = 0; i < NUMBER_SUPPORTED_CONFIG_CAPSULES; i++)
		{
			if (strcmp(cbuf, configTextureUserName[i]) == 0)
			{
				CapsuleVersion = (capsulever)configTextureUserBasis[i];
				capsuleDefined = true;
				configTextureUserEnable = true;
				configTextureUserNum = i;
				oapiWriteLogV("User capsule %s loaded, definition nr. %i.", cbuf, configTextureUserNum);
				break;
			}
		}
	}
	else
	{
		CapsuleVersion = (capsulever)atoi(cbuf);
		capsuleDefined = true;
	}

	if (CapsuleVersion == FREEDOM7)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Freedom7");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
		heatShieldGroup = 32;
		oapiWriteLog("Capsule Freedom 7 loaded.");
	}
	else if (CapsuleVersion == LIBERTYBELL7)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_LibertyBell7");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
		heatShieldGroup = 35;
		oapiWriteLog("Capsule Liberty Bell 7 loaded.");
	}
	else if (CapsuleVersion == FRIENDSHIP7)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Friendship7");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
		heatShieldGroup = 25;
		oapiWriteLog("Capsule Friendship 7 loaded.");
	}
	else if (CapsuleVersion == AURORA7)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Aurora7");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
		heatShieldGroup = 25;
		oapiWriteLog("Capsule Aurora 7 loaded.");
	}
	else if (CapsuleVersion == SIGMA7)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Sigma7");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
		heatShieldGroup = 25;
		oapiWriteLog("Capsule Sigma 7 loaded.");
	}
	else if (CapsuleVersion == FAITH7)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Faith7");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
		heatShieldGroup = 29;
		oapiWriteLog("Capsule Faith 7 loaded.");
	}
	else if (CapsuleVersion == CAPSULEBIGJOE)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\big_joe_capsule");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\big_joe_antenna");
		boilerplateMission = true;
		oapiWriteLog("Capsule Big Joe boilerplate loaded.");
	}
	else if (CapsuleVersion == CAPSULELITTLEJOE)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\little_joe_capsule");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\little_joe_antenna");
		boilerplateMission = true;
		oapiWriteLog("Capsule Little Joe boilerplate loaded.");
	}
	else if (CapsuleVersion == CAPSULEBD)
	{
		capsule = oapiLoadMeshGlobal("ProjectMercury\\MR-BD_capsule");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\MR-BD");
		boilerplateMission = true;
		oapiWriteLog("Capsule Booster Development boilerplate loaded.");
	}
	else
	{
		oapiWriteLogV("No capsule nr. %i defined", CapsuleVersion);
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Freedom7II");
		antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
		heatShieldGroup = 29;
		oapiWriteLog("Capsule Freedom 7II loaded.");
	}

	Capsule = AddMesh(capsule, &MERCURY_OFS_CAPSULE);
	Antennahouse = AddMesh(antennahouse, &MERCURY_OFS_ANTHOUSE);

	if (!boilerplateMission)
	{
		Retro = AddMesh(retro, &MERCURY_OFS_RETRO);
		Retrocover1 = AddMesh(retrocover1, &MERCURY_OFS_RETROCOVER1);
		Retrocover2 = AddMesh(retrocover2, &MERCURY_OFS_RETROCOVER2);
		Retrocover3 = AddMesh(retrocover3, &MERCURY_OFS_RETROCOVER3);
		Retrostrap1 = AddMesh(retrostrap1, &MERCURY_OFS_RETROSTRAP1);
		Retrostrap2 = AddMesh(retrostrap2, &MERCURY_OFS_RETROSTRAP2);
		Retrostrap3 = AddMesh(retrostrap3, &MERCURY_OFS_RETROSTRAP3);
		Explosivebolt = AddMesh(explosivebolt, &MERCURY_OFS_EXPLOSIVEBOLT);
	}
}

inline void ProjectMercury::ReadCapsuleTextureReplacement(const char* cbuf)
{
	char buffer[100];

	myStrncpy(buffer, cbuf, sizeof(buffer) - 2);

	char* flagPos = strchr(buffer, ' '); // find location/length of flag
	int flagIdx = int(flagPos - buffer);
	char flag[30];
	myStrncpy(flag, cbuf, min(flagIdx, sizeof(flag) - 2));

	if (strcmp(flag, "PORTHOLE") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx,  texPorthole.file, &texPorthole.w, &texPorthole.h))
			oapiWriteLogV("New Porthole texture: >%s<, w: %i, h: %i", texPorthole.file, texPorthole.w, texPorthole.h);
		else oapiWriteLog("Failed to load Porthole texture");
	}
	else if (strcmp(flag, "BRAILMAP") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx,  texBrailmap.file, &texBrailmap.w, &texBrailmap.h))
			oapiWriteLogV("New BRailmap texture: >%s<, w: %i, h: %i", texBrailmap.file, texBrailmap.w, texBrailmap.h);
		else oapiWriteLog("Failed to load BRailmap texture");
	}
	else if (strcmp(flag, "MBRAIL1") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texMBrail1.file, &texMBrail1.w, &texMBrail1.h))
			oapiWriteLogV("New MBRail1 texture: >%s<, w: %i, h: %i", texMBrail1.file, texMBrail1.w, texMBrail1.h);
		else oapiWriteLog("Failed to load MBRail1 texture");
	}
	else if (strcmp(flag, "MBRAIL2") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texMBrail2.file, &texMBrail2.w, &texMBrail2.h))
			oapiWriteLogV("New MBRail2 texture: >%s<, w: %i, h: %i", texMBrail2.file, texMBrail2.w, texMBrail2.h);
		else oapiWriteLog("Failed to load MBRail2 texture");
	}
	else if (strcmp(flag, "RIDGES") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texRidges.file, &texRidges.w, &texRidges.h))
			oapiWriteLogV("New Ridges texture: >%s<, w: %i, h: %i", texRidges.file, texRidges.w, texRidges.h);
		else oapiWriteLog("Failed to load Ridges texture");
	}
	else if (strcmp(flag, "VERRAIL1") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texVerrail1.file, &texVerrail1.w, &texVerrail1.h))
			oapiWriteLogV("New VerRail1 texture: >%s<, w: %i, h: %i", texVerrail1.file, texVerrail1.w, texVerrail1.h);
		else oapiWriteLog("Failed to load VerRail1 texture");
	}
	else if (strcmp(flag, "METAL") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texMetal.file, &texMetal.w, &texMetal.h))
			oapiWriteLogV("New Metal texture: >%s<, w: %i, h: %i", texMetal.file, texMetal.w, texMetal.h);
		else oapiWriteLog("Failed to load Metal texture");
	}
	else if (strcmp(flag, "VERRAIL2") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texVerrail2.file, &texVerrail2.w, &texVerrail2.h))
			oapiWriteLogV("New VerRail2 texture: >%s<, w: %i, h: %i", texVerrail2.file, texVerrail2.w, texVerrail2.h);
		else oapiWriteLog("Failed to load VerRail2 texture");
	}
	else if (strcmp(flag, "ORANGE") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texOrange.file, &texOrange.w, &texOrange.h))
			oapiWriteLogV("New Orange texture: >%s<, w: %i, h: %i", texOrange.file, texOrange.w, texOrange.h);
		else oapiWriteLog("Failed to load Orange texture");
	}
	else if (strcmp(flag, "PYTHR") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texPythr.file, &texPythr.w, &texPythr.h))
			oapiWriteLogV("New PyThr texture: >%s<, w: %i, h: %i", texPythr.file, texPythr.w, texPythr.h);
		else oapiWriteLog("Failed to load PyThr texture");
	}
	else if (strcmp(flag, "ROLLTHST") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texRollthst.file, &texRollthst.w, &texRollthst.h))
			oapiWriteLogV("New RollThst texture: >%s<, w: %i, h: %i", texRollthst.file, texRollthst.w, texRollthst.h);
		else oapiWriteLog("Failed to load RollThst texture");
	}
	else if (strcmp(flag, "SCANNER") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texScanner.file, &texScanner.w, &texScanner.h))
			oapiWriteLogV("New Scanner texture: >%s<, w: %i, h: %i", texScanner.file, texScanner.w, texScanner.h);
		else oapiWriteLog("Failed to load Scanner texture");
	}
	else if (strcmp(flag, "SCOPE") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texScope.file, &texScope.w, &texScope.h))
			oapiWriteLogV("New Scope texture: >%s<, w: %i, h: %i", texScope.file, texScope.w, texScope.h);
		else oapiWriteLog("Failed to load Scope texture");
	}
	else if (strcmp(flag, "SNORKEL") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texSnorkel.file, &texSnorkel.w, &texSnorkel.h))
			oapiWriteLogV("New Snorkel texture: >%s<, w: %i, h: %i", texSnorkel.file, texSnorkel.w, texSnorkel.h);
		else oapiWriteLog("Failed to load Snorkel texture");
	}
	else if (strcmp(flag, "TRAILMAP") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texTrailmap.file, &texTrailmap.w, &texTrailmap.h))
			oapiWriteLogV("New Trailmap texture: >%s<, w: %i, h: %i", texTrailmap.file, texTrailmap.w, texTrailmap.h);
		else oapiWriteLog("Failed to load Trailmap texture");
	}
	else if (strcmp(flag, "TOPBOXES") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texTopboxes.file, &texTopboxes.w, &texTopboxes.h))
			oapiWriteLogV("New Topboxes texture: >%s<, w: %i, h: %i", texTopboxes.file, texTopboxes.w, texTopboxes.h);
		else oapiWriteLog("Failed to load Topboxes texture");
	}
	else if (strcmp(flag, "USA") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texUsa.file, &texUsa.w, &texUsa.h))
			oapiWriteLogV("New USA texture: >%s<, w: %i, h: %i", texUsa.file, texUsa.w, texUsa.h);
		else oapiWriteLog("Failed to load USA texture");
	}
	else if (strcmp(flag, "ARTWORK") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texArtwork.file, &texArtwork.w, &texArtwork.h))
			oapiWriteLogV("New Artwork texture: >%s<, w: %i, h: %i", texArtwork.file, texArtwork.w, texArtwork.h);
		else oapiWriteLog("Failed to load Artwork texture");
	}
	else if (strcmp(flag, "CRACK") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texCrack.file, &texCrack.w, &texCrack.h))
			oapiWriteLogV("New Crack texture: >%s<, w: %i, h: %i", texCrack.file, texCrack.w, texCrack.h);
		else oapiWriteLog("Failed to load Crack texture");
	}
	else if (strcmp(flag, "GLASS") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texGlass.file, &texGlass.w, &texGlass.h))
			oapiWriteLogV("New Glass texture: >%s<, w: %i, h: %i", texGlass.file, texGlass.w, texGlass.h);
		else oapiWriteLog("Failed to load Glass texture");
	}
	else if (strcmp(flag, "FOILWINDOW") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texFoilwindow.file, &texFoilwindow.w, &texFoilwindow.h))
			oapiWriteLogV("New Foilwindow texture: >%s<, w: %i, h: %i", texFoilwindow.file, texFoilwindow.w, texFoilwindow.h);
		else oapiWriteLog("Failed to load Foilwindow texture");
	}
	else if (strcmp(flag, "FOIL") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texFoil.file, &texFoil.w, &texFoil.h))
			oapiWriteLogV("New Foil texture: >%s<, w: %i, h: %i", texFoil.file, texFoil.w, texFoil.h);
		else oapiWriteLog("Failed to load Foil texture");
	}
	else if (strcmp(flag, "WINDOWFR") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texWindowfr.file, &texWindowfr.w, &texWindowfr.h))
			oapiWriteLogV("New Windowfr texture: >%s<, w: %i, h: %i", texWindowfr.file, texWindowfr.w, texWindowfr.h);
		else oapiWriteLog("Failed to load Windowfr texture");
	}
	else if (strcmp(flag, "WFRFRONT") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texWfrfront.file, &texWfrfront.w, &texWfrfront.h))
			oapiWriteLogV("New Wfrfront texture: >%s<, w: %i, h: %i", texWfrfront.file, texWfrfront.w, texWfrfront.h);
		else oapiWriteLog("Failed to load Wfrfront texture");
	}
	else if (strcmp(flag, "WINDOW") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texWindow.file, &texWindow.w, &texWindow.h))
			oapiWriteLogV("New Window texture: >%s<, w: %i, h: %i", texWindow.file, texWindow.w, texWindow.h);
		else oapiWriteLog("Failed to load Window texture");
	}
	else if (strcmp(flag, "FLAG") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texFlag.file, &texFlag.w, &texFlag.h))
			oapiWriteLogV("New Flag texture: >%s<, w: %i, h: %i", texFlag.file, texFlag.w, texFlag.h);
		else oapiWriteLog("Failed to load Flag texture");
	}
	else if (strcmp(flag, "BOILERPLATECAPSULE") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texBoilerplateCapsule.file, &texBoilerplateCapsule.w, &texBoilerplateCapsule.h))
			oapiWriteLogV("New BoilerplateCapsule texture: >%s<, w: %i, h: %i", texBoilerplateCapsule.file, texBoilerplateCapsule.w, texBoilerplateCapsule.h);
		else oapiWriteLog("Failed to load BoilerplateCapsule texture");
	} // BEGIN ANTHOUSE TEXTURES
	else if (strcmp(flag, "METALANT") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texMetalant.file, &texMetalant.w, &texMetalant.h))
			oapiWriteLogV("New MetalAnt texture: >%s<, w: %i, h: %i", texMetalant.file, texMetalant.w, texMetalant.h);
		else oapiWriteLog("Failed to load MetalAnt texture");
	}
	else if (strcmp(flag, "ANTRIDGE") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texAntridge.file, &texAntridge.w, &texAntridge.h))
			oapiWriteLogV("New Antridge texture: >%s<, w: %i, h: %i", texAntridge.file, texAntridge.w, texAntridge.h);
		else oapiWriteLog("Failed to load Antridge texture");
	}
	else if (strcmp(flag, "SCREW") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texScrew.file, &texScrew.w, &texScrew.h))
			oapiWriteLogV("New Screw texture: >%s<, w: %i, h: %i", texScrew.file, texScrew.w, texScrew.h);
		else oapiWriteLog("Failed to load Screw texture");
	}
	else if (strcmp(flag, "DIALEC") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texDialec.file, &texDialec.w, &texDialec.h))
			oapiWriteLogV("New Dialec texture: >%s<, w: %i, h: %i", texDialec.file, texDialec.w, texDialec.h);
		else oapiWriteLog("Failed to load Dialec texture");
	} // BEGIN RETRO TEXTURES
	else if (strcmp(flag, "METALRET") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texMetalret.file, &texMetalret.w, &texMetalret.h))
			oapiWriteLogV("New MetalRet texture: >%s<, w: %i, h: %i", texMetalret.file, texMetalret.w, texMetalret.h);
		else oapiWriteLog("Failed to load MetalRet texture");
	}
	else if (strcmp(flag, "DIALECRET") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texDialecret.file, &texDialecret.w, &texDialecret.h))
			oapiWriteLogV("New DialecRet texture: >%s<, w: %i, h: %i", texDialecret.file, texDialecret.w, texDialecret.h);
		else oapiWriteLog("Failed to load DialecRet texture");
	}
	else if (strcmp(flag, "BW") == 0)
	{
		if (ReadTextureString(cbuf, flagIdx, texBw.file, &texBw.w, &texBw.h))
			oapiWriteLogV("New Bw texture: >%s<, w: %i, h: %i", texBw.file, texBw.w, texBw.h);
		else oapiWriteLog("Failed to load Bw texture");
	}
	else
	{
		bool success = ReadRocketTextureReplacement(flag, cbuf, flagIdx);

		if (!success) oapiWriteLogV("Error, texture replacement could not read flag >%s<.", flag);
	}
}

inline bool ProjectMercury::ReadTextureString(const char* cbuf, const int len, char* texturePath, int* textureWidth, int* textureHeight)
{
	sprintf(textureString[numTextures], cbuf);
	numTextures += 1;

	char nonConstCbuf[100];
	strcpy(nonConstCbuf, cbuf + len + 1); // "nnn nnn TTTT\TTTT", +len+1 removes "FLAG ", where len is length of flag
	char* spaceLoc, * newSpaceLoc;
	spaceLoc = strchr(nonConstCbuf, ' ');

	char width[50], height[50];

	if (spaceLoc != NULL) // found a space
	{
		strncpy(width, nonConstCbuf, spaceLoc - nonConstCbuf + 1); // "___ nnn TTTTT\TTTT"
		*textureWidth = atoi(width);

		newSpaceLoc = strchr(spaceLoc + 1, ' '); // "nnn TTTT\TTTT"
		if (newSpaceLoc != NULL) // found a second space
		{
			strncpy(height, nonConstCbuf + int(spaceLoc - nonConstCbuf + 1), int(newSpaceLoc - nonConstCbuf + 1)); // "___ TTTT\TTTT"
			*textureHeight = atoi(height);

			sprintf(texturePath, "%s", nonConstCbuf + int(newSpaceLoc - nonConstCbuf + 1)); // length 9 + space = 10
			return true;
		}
	}

	return false;
}

inline void ProjectMercury::LoadCapsuleTextureReplacement(void)
{
	int grpBrailmap, grpMbrail1, grpMbrail2, grpPorthole, grpRidges, grpVerrail1, grpMetal, grpVerrail2, grpOrange, grpPythr, grpRollthst, grpScanner, grpScope, grpSnorkel, grpTrailmap, grpTopboxes, grpUSA,
		grpArtwork,
		grpCrack, grpGlass, grpFoilwindow, grpFoil, grpWindowfr,
		grpWfrfront, grpWindow, grpFlag,
		grpBoilerplateCapsule,
		grpMetalant, grpAntridge, grpScrew, grpDialec;

	switch (CapsuleVersion)
	{
	case FREEDOM7:
		grpBrailmap = 1;
		grpMbrail1 = 2;
		grpMbrail2 = 3;
		grpPorthole = 4;
		grpRidges = 5;
		grpVerrail1 = 6;
		grpMetal = 7;
		grpVerrail2 = 8;
		grpOrange = 9;
		grpArtwork = 10;
		grpPythr = 11;
		grpRollthst = 12;
		grpScanner = 13;
		grpScope = 14;
		grpSnorkel = 15;
		grpTrailmap = 16;
		grpTopboxes = 17;
		grpUSA = 18;

		grpMetalant = 1;
		grpAntridge = 2;
		grpScrew = 3;
		grpDialec = 4;
		break;
	case LIBERTYBELL7:
		grpBrailmap = 1;
		grpMbrail1 = 2;
		grpMbrail2 = 3;
		grpRidges = 4;
		grpVerrail1 = 5;
		grpMetal = 6;
		grpVerrail2 = 7;
		grpCrack = 8;
		grpGlass = 9;
		grpOrange = 10;
		grpArtwork = 11;
		grpPythr = 12;
		grpRollthst = 13;
		grpScanner = 14;
		grpScope = 15;
		grpSnorkel = 16;
		grpTrailmap = 17;
		grpTopboxes = 18;
		grpUSA = 19;
		grpFoilwindow = 20;
		grpFoil = 21;
		grpWindowfr = 22;

		grpMetalant = 1;
		grpAntridge = 2;
		grpScrew = 3;
		grpDialec = 4;
		break;
	case FRIENDSHIP7:
		grpPythr = 1;
		grpVerrail1 = 2;
		grpWindowfr = 3;
		grpWfrfront = 4;
		grpMetal = 5;
		grpWindow = 6;
		grpUSA = 7;
		grpVerrail2 = 8;
		grpRidges = 9;
		grpTopboxes = 10;
		grpTrailmap = 11;
		grpSnorkel = 12;
		grpScope = 13;
		grpScanner = 14;
		grpRollthst = 15;
		grpArtwork = 16;
		grpOrange = 17;
		grpGlass = 18;
		grpFlag = 19;
		grpMbrail2 = 20;
		grpMbrail1 = 21;
		grpBrailmap = 22;

		grpMetalant = 1;
		grpAntridge = 2;
		grpScrew = 3;
		grpDialec = 4;
		break;
	case AURORA7:
		grpPythr = 1;
		grpVerrail1 = 2;
		grpWindowfr = 3;
		grpWfrfront = 4;
		grpMetal = 5;
		grpWindow = 6;
		grpUSA = 7;
		grpVerrail2 = 8;
		grpRidges = 9;
		grpTopboxes = 10;
		grpTrailmap = 11;
		grpSnorkel = 12;
		grpScope = 13;
		grpScanner = 14;
		grpRollthst = 15;
		grpArtwork = 16;
		grpOrange = 17;
		grpGlass = 18;
		grpFlag = 19;
		grpMbrail2 = 20;
		grpMbrail1 = 21;
		grpBrailmap = 22;

		grpMetalant = 1;
		grpAntridge = 2;
		grpScrew = 3;
		grpDialec = 4;
		break;
	case SIGMA7:
		grpPythr = 1;
		grpVerrail1 = 2;
		grpWindowfr = 3;
		grpWfrfront = 4;
		grpMetal = 5;
		grpWindow = 6;
		grpUSA = 7;
		grpVerrail2 = 8;
		grpRidges = 9;
		grpTopboxes = 10;
		grpTrailmap = 11;
		grpSnorkel = 12;
		grpScope = 13;
		grpScanner = 14;
		grpRollthst = 15;
		grpArtwork = 16;
		grpOrange = 17;
		grpGlass = 18;
		grpFlag = 19;
		grpMbrail2 = 20;
		grpMbrail1 = 21;
		grpBrailmap = 22;

		grpMetalant = 1;
		grpAntridge = 2;
		grpScrew = 3;
		grpDialec = 4;
		break;
	case FAITH7:
		grpBrailmap = 1;
		grpMbrail1 = 2;
		grpMbrail2 = 3;
		grpRidges = 4;
		grpVerrail1 = 5;
		grpMetal = 6;
		grpVerrail2 = 7;
		grpFlag = 8;
		grpGlass = 9;
		grpOrange = 10;
		grpArtwork = 11;
		grpPythr = 12;
		grpRollthst = 13;
		grpScanner = 14;
		grpSnorkel = 15;
		grpTrailmap = 16;
		grpTopboxes = 17;
		grpUSA = 18;
		grpWindow = 19;
		grpWfrfront = 20;
		grpWindowfr = 21;

		grpMetalant = 1;
		grpAntridge = 2;
		grpScrew = 3;
		grpDialec = 4;
		break;
	case FREEDOM7II:
		grpBrailmap = 1;
		grpMbrail1 = 2;
		grpMbrail2 = 3;
		grpRidges = 4;
		grpVerrail1 = 5;
		grpMetal = 6;
		grpVerrail2 = 7;
		grpFlag = 8;
		grpGlass = 9;
		grpOrange = 10;
		grpArtwork = 11;
		grpPythr = 12;
		grpRollthst = 13;
		grpScanner = 14;
		grpSnorkel = 15;
		grpTrailmap = 16;
		grpTopboxes = 17;
		grpUSA = 18;
		grpWindow = 19;
		grpWfrfront = 20;
		grpWindowfr = 21;

		grpMetalant = 1;
		grpAntridge = 2;
		grpScrew = 3;
		grpDialec = 4;
		break;
	case CAPSULEBIGJOE:
		grpScope = 1;
		grpMetal = 2;
		grpOrange = 3;
		grpBoilerplateCapsule = 4;

		grpMetalant = 1;
		grpScrew = 2;
		break;
	case CAPSULELITTLEJOE:
		grpScope = 1;
		grpMetal = 2;
		grpOrange = 3;
		grpBoilerplateCapsule = 4;

		grpMetalant = 1;
		grpScrew = 2;
		break;
	case CAPSULEBD:
		grpScope = 1;
		grpMetal = 2;
		grpOrange = 3;
		grpBoilerplateCapsule = 4;

		grpMetalant = 1;
		grpScrew = 2;
		break;
	default:
		oapiWriteLog("LoadCapsuleTextureReplacement failed to read Capsule version! Please report this bug!");
		break;
	}

	// Capsule textures
	if (strlen(texPorthole.file) != 0 && grpPorthole != NULL)
	{
		LoadTextureFile(texPorthole, "Porthole", capsule, grpPorthole);
	}
	if (strlen(texBrailmap.file) != 0 && grpBrailmap != NULL)
	{
		LoadTextureFile(texBrailmap, "Brailmap", capsule, grpBrailmap);
	}
	if (strlen(texMBrail1.file) != 0 && grpMbrail1 != NULL)
	{
		LoadTextureFile(texMBrail1, "Mbrail1", capsule, grpMbrail1);
	}
	if (strlen(texMBrail2.file) != 0 && grpMbrail2 != NULL)
	{
		LoadTextureFile(texMBrail2, "Mbrail2", capsule, grpMbrail2);
	}
	if (strlen(texRidges.file) != 0 && grpRidges != NULL)
	{
		LoadTextureFile(texRidges, "Ridges", capsule, grpRidges);
	}
	if (strlen(texVerrail1.file) != 0 && grpVerrail1 != NULL)
	{
		LoadTextureFile(texVerrail1, "Verrail1", capsule, grpVerrail1);
	}
	if (strlen(texVerrail2.file) != 0 && grpVerrail2 != NULL)
	{
		LoadTextureFile(texVerrail2, "Verrail2", capsule, grpVerrail2);
	}
	if (strlen(texMetal.file) != 0 && grpMetal != NULL)
	{
		LoadTextureFile(texMetal, "Metal", capsule, grpMetal);
	}
	if (strlen(texOrange.file) != 0 && grpOrange != NULL)
	{
		LoadTextureFile(texOrange, "Orange", capsule, grpOrange);
	}
	if (strlen(texPythr.file) != 0 && grpPythr != NULL)
	{
		LoadTextureFile(texPythr, "Pythr", capsule, grpPythr);
	}
	if (strlen(texRollthst.file) != 0 && grpRollthst != NULL)
	{
		LoadTextureFile(texRollthst, "Rollthst", capsule, grpRollthst);
	}
	if (strlen(texScanner.file) != 0 && grpScanner != NULL)
	{
		LoadTextureFile(texScanner, "Scanner", capsule, grpScanner);
	}
	if (strlen(texScope.file) != 0 && grpScope != NULL)
	{
		LoadTextureFile(texScope, "Scope", capsule, grpScope);
	}
	if (strlen(texSnorkel.file) != 0 && grpSnorkel != NULL)
	{
		LoadTextureFile(texSnorkel, "Snorkel", capsule, grpSnorkel);
	}
	if (strlen(texTrailmap.file) != 0 && grpTrailmap != NULL)
	{
		LoadTextureFile(texTrailmap, "Trailmap", capsule, grpTrailmap);
	}
	if (strlen(texTopboxes.file) != 0 && grpTopboxes != NULL)
	{
		LoadTextureFile(texTopboxes, "Topboxes", capsule, grpTopboxes);
	}
	if (strlen(texUsa.file) != 0 && grpUSA != NULL)
	{
		LoadTextureFile(texUsa, "USA", capsule, grpUSA);
	}
	if (strlen(texArtwork.file) != 0 && grpArtwork != NULL)
	{
		LoadTextureFile(texArtwork, "Artwork", capsule, grpArtwork);
	}
	if (strlen(texCrack.file) != 0 && grpCrack != NULL)
	{
		LoadTextureFile(texCrack, "Crack", capsule, grpCrack);
	}
	if (strlen(texGlass.file) != 0 && grpGlass != NULL)
	{
		LoadTextureFile(texGlass, "Glass", capsule, grpGlass);
	}
	if (strlen(texFoilwindow.file) != 0 && grpFoilwindow != NULL)
	{
		LoadTextureFile(texFoilwindow, "Foilwindow", capsule, grpFoilwindow);
	}
	if (strlen(texFoil.file) != 0 && grpFoil != NULL)
	{
		LoadTextureFile(texFoil, "Foil", capsule, grpFoil);
	}
	if (strlen(texWindowfr.file) != 0 && grpWindowfr != NULL)
	{
		LoadTextureFile(texWindowfr, "Windowfr", capsule, grpWindowfr);
	}
	if (strlen(texWfrfront.file) != 0 && grpWfrfront != NULL)
	{
		LoadTextureFile(texWfrfront, "Wfrfront", capsule, grpWfrfront);
	}
	if (strlen(texWindow.file) != 0 && grpWindow != NULL)
	{
		LoadTextureFile(texWindow, "Window", capsule, grpWindow);
	}
	if (strlen(texFlag.file) != 0 && grpFlag != NULL)
	{
		LoadTextureFile(texFlag, "Flag", capsule, grpFlag);
	}
	if (strlen(texBoilerplateCapsule.file) != 0 && grpBoilerplateCapsule != NULL)
	{
		LoadTextureFile(texBoilerplateCapsule, "BoilerplateCapsule", capsule, grpBoilerplateCapsule);
	} // BEGIN ANTHOUSE
	if (strlen(texMetalant.file) != 0 && grpMetalant != NULL)
	{
		LoadTextureFile(texMetalant, "MetalAnt", antennahouse, grpMetalant);
	}
	if (strlen(texAntridge.file) != 0 && grpAntridge != NULL)
	{
		LoadTextureFile(texAntridge, "Antridge", antennahouse, grpAntridge);
	}
	if (strlen(texScrew.file) != 0 && grpScrew != NULL)
	{
		LoadTextureFile(texScrew, "Screw", antennahouse, grpScrew);
	}
	if (strlen(texDialec.file) != 0 && grpDialec != NULL)
	{
		LoadTextureFile(texDialec, "Dialec", antennahouse, grpDialec);
	} // BEGIN RETRO
	if (strlen(texMetalret.file) != 0)
	{
		LoadTextureFile(texMetalret, "MetalRet", retro, 1); // group always the same
	}
	if (strlen(texDialecret.file) != 0)
	{
		LoadTextureFile(texDialecret, "DialecRet", retro, 2); // group always the same
	}
	if (strlen(texBw.file) != 0)
	{
		LoadTextureFile(texBw, "Bw", retro, 3); // group always the same
	}

	LoadRocketTextureReplacement();
}

inline void ProjectMercury::LoadTextureFile(ATEX tex, const char* type, MESHHANDLE mesh, DWORD meshTexIdx)
{
	char file[100] = "ProjectMercury\\Skins\\";
	strcat(file, tex.file);
	strcat(file, ".dds");

	oapiWriteLogV("Loading %s file >%s<", type, file);
	SURFHANDLE newTex = oapiLoadTexture(file, true);
	SURFHANDLE oldTex = oapiGetTextureHandle(mesh, meshTexIdx);
	oapiBlt(oldTex, newTex, 0, 0, 0, 0, tex.w, tex.h);
	oapiReleaseTexture(newTex);
	oapiWriteLogV("Finished loading %s texture", type);
}

inline void ProjectMercury::DefinePeriscopeAnimation(void)
{
	static UINT periscopeDoorGroups[1] = { 1 };

	static MGROUP_ROTATE DoorOpen(
		Capsule,
		periscopeDoorGroups, 1,
		_V(-0.126, -0.662, -0.187),
		_V(0.0, -sin(17.9 * RAD), -cos(17.9 * RAD)),
		float(150.0 * RAD)
	);

	static UINT periscopeGroups[2] = { 0, 2 };

	const double periscopeErectLength = 0.1;

	static MGROUP_TRANSLATE PeriscopeErect(
		Capsule,
		periscopeGroups, 2,
		_V(0.0, -cos(17.9 * RAD), sin(17.9 * RAD)) * periscopeErectLength
	);

	PeriscopeDeployAnim = CreateAnimation(0.0);
	AddAnimationComponent(PeriscopeDeployAnim, 0.0, 0.5, &DoorOpen);
	AddAnimationComponent(PeriscopeDeployAnim, 0.5, 1.0, &PeriscopeErect);
}

inline void ProjectMercury::DefineAntennaDestabiliser(void)
{
	static UINT destabiliserGroups[1] = { 0 };

	static MGROUP_ROTATE DestabiliserDeploy(
		Antennahouse,
		destabiliserGroups, 1,
		_V(0.0, -0.230, 0.265),
		_V(1.0, 0.0, 0.0),
		float(150.0 * RAD)
	);

	DestabiliserDeployAnim = CreateAnimation(0.0);
	AddAnimationComponent(DestabiliserDeployAnim, 0.0, 1.0, &DestabiliserDeploy);
}

void ProjectMercury::DefineDrogueAnimation(void)
{
	static UINT drogueDeployGroups[19] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };

	static MGROUP_SCALE DrogueZero(
		Drogue,
		drogueDeployGroups, 19,
		_V(0.0, 0.0, 0.0),
		_V(0.001, 0.001, 0.001)
	);

	static MGROUP_SCALE DrogueDeploy (
		Drogue,
		drogueDeployGroups, 19,
		_V(0.0, 0.0, 0.0),
		_V(0.001, 0.001, 0.001)
	);
	DrogueDeployAnim = CreateAnimation(0.0);
	DrogueDeployAnim1 = AddAnimationComponent(DrogueDeployAnim, 0.0, 1.0, &DrogueDeploy);
}

inline void ProjectMercury::DefineMainChuteAnimation(void)
{
	// Thanks to BrianJ for making his Dragon open source, so I could copy the solution!
	static UINT mainChuteDeployGroups[2] = { 0, 1 };

	static MGROUP_SCALE MainChuteZero(
		Mainchute,
		mainChuteDeployGroups, 2,
		_V(0.0, 0.0, 0.0),
		_V(0.01, 0.01, 0.01)
	);
	static MGROUP_SCALE MainChuteDeploy (
		Mainchute,
		mainChuteDeployGroups, 2,
		_V(0.0, 0.0, 0.0),
		_V(12.0, 12.0, 100.0)
	);
	static MGROUP_SCALE MainChuteDeployUnreef (
		Mainchute,
		mainChuteDeployGroups, 2,
		_V(0.0, 0.0, 0.0),
		_V(8.33, 8.33, 1.0)
	);

	MainChuteDeployAnim = CreateAnimation(0.0);
	AddAnimationComponent(MainChuteDeployAnim, 0.0, 0.01, &MainChuteZero);
	MainChuteDeployAnim1 = AddAnimationComponent(MainChuteDeployAnim, 0.01, 0.5, &MainChuteDeploy);
	AddAnimationComponent(MainChuteDeployAnim, 0.5, 1.0, &MainChuteDeployUnreef);
}

inline void ProjectMercury::DefineLandingBagAnimation(void)
{
	static UINT landingBagDeployGroups[13] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

	static MGROUP_SCALE LandingBagZero(
		Landingbag,
		landingBagDeployGroups, 13,
		_V(0.0, 0.0, MERCURY_LENGTH_LANDBAG / 2.0),
		_V(1, 1, 0.001)
	);

	static MGROUP_SCALE LandingBagDeploy(
		Landingbag,
		landingBagDeployGroups, 13,
		_V(0.0, 0.0, MERCURY_LENGTH_LANDBAG / 2.0),
		_V(1.0, 1.0, 1000.0)
	);

	static UINT heatShieldGroups[1] = { (UINT)heatShieldGroup }; // can't be static, as it changes from capsule to capsule

	static MGROUP_TRANSLATE HeatShieldLower(
		Capsule,
		heatShieldGroups, 1,
		_V(0.0, 0.0, -MERCURY_LENGTH_LANDBAG)
	);

	LandingBagDeployAnim = CreateAnimation(0.0);
	AddAnimationComponent(LandingBagDeployAnim, 0.0, 1e-5, &LandingBagZero);
	LandingBagDeployAnim1 = AddAnimationComponent(LandingBagDeployAnim, 1e-5, 1.0, &LandingBagDeploy);
	AddAnimationComponent(LandingBagDeployAnim, 0.0, 1.0, &HeatShieldLower);
}

inline void ProjectMercury::AnimateAntennaDestabiliser(double simt, double simdt)
{
	double animationStep = simdt * DESTABILISER_OPERATING_SPEED;

	if (DestabiliserStatus == P_OPENING)
	{
		if (destabiliserProgress > 1.0) // finished
		{
			destabiliserProgress = 1.0;
			DestabiliserStatus = P_DEPLOYED;
		}
		else
		{
			destabiliserProgress += animationStep;
		}
		SetAnimation(DestabiliserDeployAnim, destabiliserProgress);
	}
	else if (DestabiliserStatus == P_DEPLOYED) // wind drag
	{
		double drag = GetDrag();
		double limit = 90.0 / 150.0;
		double targetProcess = 1.0 + drag / 2e7 - drag * drag / 1e11;
		if (targetProcess < limit) targetProcess = limit;
		if (targetProcess > 1.0) targetProcess = 1.0;

		SetAnimation(DestabiliserDeployAnim, targetProcess);

	}
}

inline void ProjectMercury::AnimatePeriscope(double simt, double simdt)
{
	double animationStep = simdt * PERISCOPE_OPERATING_SPEED;

	if (PeriscopeStatus == P_OPENING) // initial deployment
	{
		if (periscopeProgress > 1.0) // finished
		{
			periscopeProgress = 1.0;
			PeriscopeStatus = P_DEPLOYED;
		}
		else // carry on deploying
		{
			periscopeProgress += animationStep;
		}
		SetAnimation(PeriscopeDeployAnim, periscopeProgress);
	}
	else if (PeriscopeStatus == P_CLOSING) // we want to close periscope
	{
		if (periscopeProgress < 0.0)
		{
			periscopeProgress = 0.0;
			PeriscopeStatus = P_CLOSED;
		}
		else // carry on retracting
		{
			periscopeProgress -= animationStep;
		}
		SetAnimation(PeriscopeDeployAnim, periscopeProgress);
	}
}

void ProjectMercury::vlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	// This works ish. For later improvement with matrix taking mach into consideration, see https://www.orbiter-forum.com/showthread.php?t=40607
	static const double cmp[13] = {
		0, -0.204, -0.193, -0.238, -0.337, -0.223, 0.03, 0.23, 0.31, 0.235, 0.194, 0.192, 0
	};

	static const double clp[13] = {
		0, -0.42, -0.04, 0.38, -0.38, -0.45, 0.06, 0.4, 0.24, -0.38, 0.02, 0.42, 0.0
	};

	/*static const double cdp[13] = {
		1.34, 1.25, 1.09, 1.24, 1.28, 0.99, 0.88, 0.99, 1.31, 1.26, 1.09, 1.24, 1.34
	};*/

	static const  double mach[14] = {
		0.0, 0.50, 0.7, 0.90, 1.00, 1.1, 1.30, 1.60, 2.00, 3.0, 5.00, 7.0, 9.6, 20.0
	};
	static const double cdp[14] = { // drag coeff at 0 AoA (blunt side first) for different mach numbers
		1.0, 1.02, 1.1, 1.23, 1.34, 1.4, 1.46, 1.49, 1.53, 1.6, 1.56, 1.5, 1.5, 1.5
	};

	double aoastep = 30.0 * RAD;
	aoa += PI;
	int idx = max(0, min(11, (int)(aoa / aoastep)));
	double d = aoa / aoastep - idx;

	int i = 0;
	while (i < 14 && M > mach[i])
	{
		i++;
	}

	if (i == 14)
	{
		*cd = cdp[13];
	}
	else if (i == 0)
	{
		*cd = cdp[0];
	}
	else
	{
		*cd = cdp[i - 1] + (cdp[i] - cdp[i - 1]) * (M - mach[i - 1]) / (mach[i] - mach[i - 1]);
	}

	*cl = clp[idx] + (clp[idx + 1] - clp[idx]) * d;
	*cm = cmp[idx] + (cmp[idx + 1] - cmp[idx]) * d;
	// *cd = cdp[idx] + (cdp[idx + 1] - cdp[idx]) * d;

	// function for calculating cd for different mach
	double logistA = 0.000008;
	double logistB = 6.4;
	double scaleFunction = (0.852 * M * M * M - 0.212 * M * M + 1.09 * M + 1.80) * (1.0 / (1.0 + logistA * exp(logistB * M))) + 5.81 / (1.0 + 4.91 * exp(-0.589 * M)) * 1.0 / (1.0 + 1.0 / logistA * exp(-logistB * M));
	double cdAtZeroAoA = *cd / scaleFunction;
	// Don't need this as we've already corrected for mach

	// function for calculating cd for different aoa
	//aoa -= PI;
	aoa *= DEG; // scale to deg. Check if needed to subtract 180 deg
	if (aoa > 180.0)
		aoa = 360.0 - (aoa);
	double scaleFunction2 = 2.211624962e-19 * pow(aoa, 10.0) - 1.922017370e-16 * pow(aoa, 9.0) + 6.976725705e-14 * pow(aoa, 8.0) - 1.368875959e-11 * pow(aoa, 7.0) +
		1.570125054e-9 * pow(aoa, 6.0) - 1.064292277e-7 * pow(aoa, 5.0) + 0.000004100646042 * pow(aoa, 4.0) - 0.00007879839851 * pow(aoa, 3.0) + 0.0003604511201 * pow(aoa, 2.0) + 0.002102445273 * aoa + 0.9993934527;

	*cd *= scaleFunction2; // this _SHOULD_ give a close to correct cd for any aoa and mach
	*cd *= 0.5;
}

void ProjectMercury::hlift(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	static const double cmp[13] = {
		0, 0.204, 0.193, 0.238, 0.337, 0.223, -0.03, -0.23, -0.31, -0.235, -0.194, -0.192, 0
	};

	// Old and thrustworthy code giving only coeffs from Mach 1 values
	static const double clp[13] = {
		0, -0.42, -0.04, 0.38, -0.38, -0.45, 0.06, -0.45, -0.38, 0.38, -0.04, -0.42, 0.0
	};

	static const  double mach[14] = {
		0.0, 0.50, 0.7, 0.90, 1.00, 1.1, 1.30, 1.60, 2.00, 3.0, 5.00, 7.0, 9.6, 20.0
	};
	static const double cdp[14] = { // drag coeff at 0 AoA (blunt side first) for different mach numbers
		1.0, 1.02, 1.1, 1.23, 1.34, 1.4, 1.46, 1.49, 1.53, 1.6, 1.56, 1.5, 1.5, 1.5
	};

	int i = 0;
	while (i < 14 && M > mach[i])
	{
		i++;
	}

	if (i == 14)
	{
		*cd = cdp[13];
	}
	else if (i == 0)
	{
		*cd = cdp[0];
	}
	else
	{
		*cd = cdp[i - 1] + (cdp[i] - cdp[i - 1]) * (M - mach[i - 1]) / (mach[i] - mach[i - 1]);
	}

	double aoastep = 30.0 * RAD;
	aoa += PI;
	int idx = max(0, min(11, (int)(aoa / aoastep)));
	double d = aoa / aoastep - idx;
	*cl = clp[idx] + (clp[idx + 1] - clp[idx]) * d;
	//*cm = cmp[idx] + (cmp[idx + 1] - cmp[idx]) * d;
	*cm = 0.0;
	// *cd = cdp[idx] + (cdp[idx + 1] - cdp[idx]) * d;

	// function for calculating cd for different mach
	double logistA = 0.000008;
	double logistB = 6.4;
	double scaleFunction = (0.852 * M * M * M - 0.212 * M * M + 1.09 * M + 1.80) * (1.0 / (1.0 + logistA * exp(logistB * M))) + 5.81 / (1.0 + 4.91 * exp(-0.589 * M)) * 1.0 / (1.0 + 1.0 / logistA * exp(-logistB * M));
	double cdAtZeroAoA = *cd / scaleFunction;
	// Don't need this as we've already corrected for mach

	// function for calculating cd for different aoa
	//aoa -= PI;
	aoa *= DEG; // scale to deg. Check if needed to subtract 180 deg
	if (aoa > 180.0)
		aoa = 360.0 - (aoa);
	double scaleFunction2 = 2.211624962e-19 * pow(aoa, 10.0) - 1.922017370e-16 * pow(aoa, 9.0) + 6.976725705e-14 * pow(aoa, 8.0) - 1.368875959e-11 * pow(aoa, 7.0) +
		1.570125054e-9 * pow(aoa, 6.0) - 1.064292277e-7 * pow(aoa, 5.0) + 0.000004100646042 * pow(aoa, 4.0) - 0.00007879839851 * pow(aoa, 3.0) + 0.0003604511201 * pow(aoa, 2.0) + 0.002102445273 * aoa + 0.9993934527;

	*cd *= scaleFunction2; // this _SHOULD_ give a close to correct cd for any aoa and mach.
	*cd *= 0.5;
}

void ProjectMercury::vliftEscape(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	// This works ish. For later improvement with matrix taking mach into consideration, see https://www.orbiter-forum.com/showthread.php?t=40607
	static const double cmp[13] = {
		0, -0.45, -0.50, -0.58, -0.38, -0.11, 0.03, 0.11, 0.38, 0.58, 0.50, 0.45, 0
	};

	static const double clp[13] = { //	-180   -150 -120  -90    -60   -30   0
		0, 0.50, 0.53, -0.38, -0.3, 0.24, 0.0, 0.24, -0.3, -0.38, 0.53, 0.50, 0.0
	};

	static const  double mach[12] = {
		0.0, 0.50, 0.7, 0.90, 1.00, 1.15, 1.5, 2.0, 3.0, 5.0, 7.0, 9.6
	};
	static const double cdp[12] = { // drag coeff at 0 AoA (tip first) for different mach numbers.
		0.63, 0.64, 0.64, 0.72, 0.92, 0.9, 0.78, 0.66, 0.46, 0.3, 0.23, 0.18
	};

	double aoastep = 30.0 * RAD;
	aoa += PI;
	int idx = max(0, min(11, (int)(aoa / aoastep)));
	double d = aoa / aoastep - idx;

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

	*cl = clp[idx] + (clp[idx + 1] - clp[idx]) * d;
	*cm = cmp[idx] + (cmp[idx + 1] - cmp[idx]) * d;

	*cd *= 0.5;
}

void ProjectMercury::hliftEscape(VESSEL* v, double aoa, double M, double Re, void* context, double* cl, double* cm, double* cd)
{
	static const double cmp[13] = {
		0, -0.45, -0.50, -0.58, -0.38, -0.11, 0.03, 0.11, 0.38, 0.58, 0.50, 0.45, 0
	};

	static const double clp[13] = { //	 -180   -150 -120  -90    -60   -30   0
		0, 0.50, 0.53, -0.38, -0.3, 0.24, 0.0, 0.24, -0.3, -0.38, 0.53, 0.50, 0.0
	};

	static const  double mach[12] = {
		0.0, 0.50, 0.7, 0.90, 1.00, 1.15, 1.5, 2.0, 3.0, 5.0, 7.0, 9.6
	};
	static const double cdp[12] = { // drag coeff at 0 AoA (blunt side first) for different mach numbers. If stable is front first, then use 180 deg
		0.63, 0.64, 0.64, 0.72, 0.92, 0.9, 0.78, 0.66, 0.46, 0.3, 0.23, 0.18
	};

	int i = 0;
	while (i < 12 && M > mach[i])
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

	double aoastep = 30.0 * RAD;
	aoa += PI;
	int idx = max(0, min(11, (int)(aoa / aoastep)));
	double d = aoa / aoastep - idx;
	*cl = clp[idx] + (clp[idx + 1] - clp[idx]) * d;
	*cm = 0.0;

	*cd *= 0.5;
}