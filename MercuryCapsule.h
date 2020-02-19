#pragma once
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

inline void ProjectMercury::DisableAutopilot(bool turnOff)
{
	if (turnOff)
			autoPilot = false;

	if (VesselStatus == FLIGHT || VesselStatus == REENTRY || VesselStatus == REENTRYNODROGUE) // RCS exists
	{
		if ((int)GetThrusterCount() < 18)
		{
			char cbuf[256];
			sprintf(cbuf, "Won't disable autopilot, only has %i thrusters, i.e. no RCS created", (int)GetThrusterCount());
			oapiWriteLog(cbuf);
			return;
		}

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

void ProjectMercury::AuxDampingAuto(bool highThrust) // return number of active engines
{
	THRUSTER_HANDLE py0, py1, py2, py3, roll0, roll1;
	double rateFactor = 1.0;

	if ((int)GetThrusterCount() < 18)
	{
		char cbuf[256];
		sprintf(cbuf, "Won't damp, only has %i thrusters, i.e. no RCS created", (int)GetThrusterCount());
		oapiWriteLog(cbuf);
		return;
	}

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
		rateFactor = 0.1;
	}

	VECTOR3 angVel;
	GetAngularVel(angVel);

	if (angVel.x > 0.009)
		SetThrusterLevel(py1, 1.0);
	else
		SetThrusterLevel(py1, 0.0);

	if (angVel.x < -0.009)
		SetThrusterLevel(py0, 1.0);
	else
		SetThrusterLevel(py0, 0.0);

	if (angVel.y > 0.009)
		SetThrusterLevel(py2, 1.0);
	else
		SetThrusterLevel(py2, 0.0);

	if (angVel.y < -0.009)
		SetThrusterLevel(py3, 1.0);
	else
		SetThrusterLevel(py3, 0.0);

	if (angVel.z > 0.009)
		SetThrusterLevel(roll0, 1.0);
	else
		SetThrusterLevel(roll0, 0.0);

	if (angVel.z < -0.009)
		SetThrusterLevel(roll1, 1.0);
	else
		SetThrusterLevel(roll1, 0.0);
}

void ProjectMercury::RetroAttitudeAuto(double simt, double simdt) // finished
{
	bool highThrust = false;
	if (oapiGetTimeAcceleration() < 1.5)
		highThrust = true;

	if (autoPilot && oapiGetTimeAcceleration() < 15.0) // for high time acc things get unstable
	{
		bool result[3] = { false, false, false };
		result[0] = SetPitchAuto(-34.0, highThrust);
		result[1] = SetYawAuto(highThrust);
		result[2] = SetRollAuto(highThrust);
	}
}

bool ProjectMercury::SetPitchAuto(double targetPitch, bool highThrust)
{
	THRUSTER_HANDLE py0, py1, py2, py3, roll0, roll1;
	double rateFactor = 1.0;

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
		rateFactor = 0.1;
	}

	VECTOR3 angVel;
	GetAngularVel(angVel);
	double pitch = GetPitch();
	pitch -= targetPitch * RAD; // target pitch is -34 degrees, so that would give 0 deg if correct pitch
	double autoVel = 0.0;
	int sign = (pitch > 0) - (pitch < 0);
	double lvlH, lvlL;
	if (pitch > 0)
	{
		lvlH = 1.0;
		lvlL = 0.0;
	}
	else // inverted
	{
		lvlH = 0.0;
		lvlL = 1.0;
	}

	int autoMode;


	if (abs(pitch) > 0.5 * RAD)
	{
		if (abs(pitch) > 10.0 * RAD)
		{
			autoVel = 10.0 * RAD * rateFactor;
		}
		else if (abs(pitch) > 5.0 * RAD)
		{
			autoVel = 5.0 * RAD * rateFactor;
		}
		else
		{
			autoVel = 0.5 * RAD;
		}

		SetThrusterLevel(thruster_auto_py_1lb[0], 0.0); // UP
		SetThrusterLevel(thruster_auto_py_1lb[1], 0.0); // DOWN ... or maybe opposite

		if (sign * angVel.x > 0.0) // pitching away from target
		{
			SetThrusterLevel(py0, lvlL); // double check
			SetThrusterLevel(py1, lvlH);
			autoMode = 11;
		}
		else if (abs(angVel.x) < 0.90 * autoVel) // pitching towards target, and too slow
		{// 0.90 factor to not get oscillation about autoVel
			SetThrusterLevel(py0, lvlL);
			SetThrusterLevel(py1, lvlH);
			autoMode = 12;
		}
		else if (abs(angVel.x) > autoVel) // pitching towards target, but too fast
		{
			SetThrusterLevel(py0, lvlH);
			SetThrusterLevel(py1, lvlL);
			autoMode = 13;
		}
		else // coast
		{
			SetThrusterLevel(py0, 0.0);
			SetThrusterLevel(py1, 0.0);
			autoMode = 14;
		}
	}
	else
	{
		SetThrusterLevel(py0, 0.0);
		SetThrusterLevel(py1, 0.0);

		if (angVel.x > 0.001)
		{
			SetThrusterLevel(thruster_auto_py_1lb[0], lvlL);
			SetThrusterLevel(thruster_auto_py_1lb[1], lvlH);
			autoMode = 21;
		}
		else if (angVel.x < -0.001)
		{
			SetThrusterLevel(thruster_auto_py_1lb[0], lvlH);
			SetThrusterLevel(thruster_auto_py_1lb[1], lvlL);
			autoMode = 22;
		}
		else
		{
			SetThrusterLevel(thruster_auto_py_1lb[0], 0.0);
			SetThrusterLevel(thruster_auto_py_1lb[1], 0.0);
			autoMode = 23;
			return true; // success
		}
	}
	return false;
}

bool ProjectMercury::SetYawAuto(bool highThrust)
{
	THRUSTER_HANDLE py0, py1, py2, py3, roll0, roll1;
	double rateFactor = 1.0;

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
		rateFactor = 0.1;
	}

	VECTOR3 angVel;
	GetAngularVel(angVel);
	double yaw = GetSlipAngle();
	double autoVel = 0.0;
	int sign = (yaw > 0) - (yaw < 0);
	double lvlH, lvlL;
	if (yaw > 0)
	{
		lvlH = 1.0;
		lvlL = 0.0;
	}
	else // inverted
	{
		lvlH = 0.0;
		lvlL = 1.0;
	}

	int autoMode;

	if (abs(yaw) < 179.5 * RAD)
	{
		if (abs(yaw) < 170.0 * RAD)
		{
			autoVel = 10.0 * RAD * rateFactor;
		}
		else if (abs(yaw) < 175.0 * RAD)
		{
			autoVel = 5.0 * RAD * rateFactor;
		}
		else
		{
			autoVel = 0.5 * RAD;
		}

		SetThrusterLevel(thruster_auto_py_1lb[2], 0.0); // RIGHT
		SetThrusterLevel(thruster_auto_py_1lb[3], 0.0); // LEFT ... or maybe opposite

		if (sign * angVel.y > 0.0) // yawing away from target
		{
			SetThrusterLevel(py2, lvlH); // double check
			SetThrusterLevel(py3, lvlL);
			autoMode = 11;
		}
		else if (abs(angVel.y) < 0.90 * autoVel) // yawing towards target, and too slow
		{// 0.90 factor to not get oscillation about autoVel
			SetThrusterLevel(py2, lvlH);
			SetThrusterLevel(py3, lvlL);
			autoMode = 12;
		}
		else if (abs(angVel.y) > autoVel) // yawing towards target, but too fast
		{
			SetThrusterLevel(py2, lvlL);
			SetThrusterLevel(py3, lvlH);
			autoMode = 13;
		}
		else // coast
		{
			SetThrusterLevel(py2, 0.0);
			SetThrusterLevel(py3, 0.0);
			autoMode = 14;
		}
	}
	else
	{
		SetThrusterLevel(py2, 0.0);
		SetThrusterLevel(py3, 0.0);

		if (angVel.y > 0.001)
		{
			SetThrusterLevel(thruster_auto_py_1lb[2], lvlH); // RIGHT
			SetThrusterLevel(thruster_auto_py_1lb[3], lvlL); // LEFT
			autoMode = 21;
		}
		else if (angVel.y < -0.001)
		{
			SetThrusterLevel(thruster_auto_py_1lb[2], lvlL); // RIGHT
			SetThrusterLevel(thruster_auto_py_1lb[3], lvlH); // LEFT
			autoMode = 22;
		}
		else
		{
			SetThrusterLevel(thruster_auto_py_1lb[2], 0.0); // RIGHT
			SetThrusterLevel(thruster_auto_py_1lb[3], 0.0); // LEFT
			autoMode = 23;
			return true; // success
		}
	}
	return false;
}

bool ProjectMercury::SetRollAuto(bool highThrust)
{
	THRUSTER_HANDLE py0, py1, py2, py3, roll0, roll1;
	double rateFactor = 1.0;

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
		rateFactor = 0.1;
	}

	VECTOR3 angVel;
	GetAngularVel(angVel);
	angVel.z *= -1.0; // WARNING: ORBITER IS LEFT-HANDED, SO WE HAVE TO FLIP THE SIGN FOR ANGVELZ
	double roll = GetBank();
	double autoVel = 0.0;
	int sign = (roll > 0) - (roll < 0);
	double lvlH, lvlL;
	if (roll > 0)
	{
		lvlH = 1.0;
		lvlL = 0.0;
	}
	else // inverted
	{
		lvlH = 0.0;
		lvlL = 1.0;
	}

	int autoMode;

	if (abs(roll) > 0.5 * RAD)
	{
		if (abs(roll) > 10.0 * RAD)
		{
			autoVel = 10.0 * RAD * rateFactor;
		}
		else if (abs(roll) > 5.0 * RAD)
		{
			autoVel = 5.0 * RAD * rateFactor;
		}
		else
		{
			autoVel = 0.5 * RAD;
		}

		SetThrusterLevel(thruster_auto_roll_1lb[0], 0.0); // ROLL RIGHT
		SetThrusterLevel(thruster_auto_roll_1lb[1], 0.0); // ROLL LEFT ... or maybe opposite

		if (sign * angVel.z > 0.0) // rolling away from target
		{
			SetThrusterLevel(roll0, lvlH); // double check
			SetThrusterLevel(roll1, lvlL);
			autoMode = 11;
		}
		else if (abs(angVel.z) < 0.90 * autoVel) // rolling towards target, and too slow
		{// 0.90 factor to not get oscillation about autoVel
			SetThrusterLevel(roll0, lvlH);
			SetThrusterLevel(roll1, lvlL);
			autoMode = 12;
		}
		else if (abs(angVel.z) > autoVel) // rolling towards target, but too fast
		{
			SetThrusterLevel(roll0, lvlL);
			SetThrusterLevel(roll1, lvlH);
			autoMode = 13;
		}
		else // coast
		{
			SetThrusterLevel(roll0, 0.0);
			SetThrusterLevel(roll1, 0.0);
			autoMode = 14;
		}
	}
	else
	{
		SetThrusterLevel(roll0, 0.0);
		SetThrusterLevel(roll1, 0.0);

		if (angVel.z > 0.001)
		{
			SetThrusterLevel(thruster_auto_roll_1lb[0], lvlH);
			SetThrusterLevel(thruster_auto_roll_1lb[1], lvlL);
			autoMode = 21;
		}
		else if (angVel.z < -0.001)
		{
			SetThrusterLevel(thruster_auto_roll_1lb[0], lvlL);
			SetThrusterLevel(thruster_auto_roll_1lb[1], lvlH);
			autoMode = 22;
		}
		else
		{
			SetThrusterLevel(thruster_auto_roll_1lb[0], 0.0);
			SetThrusterLevel(thruster_auto_roll_1lb[1], 0.0);
			autoMode = 23;
			return true; // success
		}
	}
	return false;
}

//void ProjectMercury::TurnAroundAuto(double simt, double simdt) // not used currently (I believe?)
//{
//	VECTOR3 angVel;
//	GetAngularVel(angVel);
//	double yaw = GetSlipAngle();
//	double autoVel = 0.0;
//
//	int autoMode;
//
//	if (abs(yaw) < 179.5 * RAD)
//	{
//		if (abs(yaw) < 170.0 * RAD)
//		{
//			autoVel = -10.0 * RAD;
//		}
//		else if (abs(yaw) < 175.0 * RAD)
//		{
//			autoVel = -5.0 * RAD;
//		}
//		else
//		{
//			autoVel = -0.5 * RAD;
//		}
//
//		SetThrusterLevel(thruster_auto_py_1lb[2], 0.0); // RIGHT
//		SetThrusterLevel(thruster_auto_py_1lb[3], 0.0); // LEFT
//
//		if (angVel.y > 0.0)
//		{
//			SetThrusterLevel(thruster_auto_py[2], 1.0);
//			SetThrusterLevel(thruster_auto_py[3], 0.0);
//			autoMode = 11;
//		}
//		else if (angVel.y < 0.0 && angVel.y > 0.90 * autoVel) // 0.95 factor to not get oscillation about autoVel
//		{
//			SetThrusterLevel(thruster_auto_py[2], 1.0);
//			SetThrusterLevel(thruster_auto_py[3], 0.0);
//			autoMode = 12;
//		}
//		else if (angVel.y < 0.0 && angVel.y < autoVel)
//		{
//			SetThrusterLevel(thruster_auto_py[2], 0.0);
//			SetThrusterLevel(thruster_auto_py[3], 1.0);
//			autoMode = 13;
//		}
//		else // coast
//		{
//			SetThrusterLevel(thruster_auto_py[2], 0.0);
//			SetThrusterLevel(thruster_auto_py[3], 0.0);
//			autoMode = 14;
//		}
//
//		// eliminate x and z components too
//		/*if (angVel.x > 0.009)
//			SetThrusterLevel(thruster_auto_py[1], 1.0);
//		else
//			SetThrusterLevel(thruster_auto_py[1], 0.0);
//
//		if (angVel.x < -0.009)
//			SetThrusterLevel(thruster_auto_py[0], 1.0);
//		else
//			SetThrusterLevel(thruster_auto_py[0], 0.0);
//
//		if (angVel.z > 0.009)
//			SetThrusterLevel(thruster_auto_roll[0], 1.0);
//		else
//			SetThrusterLevel(thruster_auto_roll[0], 0.0);
//
//		if (angVel.z < -0.009)
//			SetThrusterLevel(thruster_auto_roll[1], 1.0);
//		else
//			SetThrusterLevel(thruster_auto_roll[1], 0.0);*/
//	}
//	else
//	{
//		SetThrusterLevel(thruster_auto_py[0], 0.0);
//		SetThrusterLevel(thruster_auto_py[1], 0.0);
//		SetThrusterLevel(thruster_auto_py[2], 0.0);
//		SetThrusterLevel(thruster_auto_py[3], 0.0);
//		SetThrusterLevel(thruster_auto_roll[0], 0.0);
//		SetThrusterLevel(thruster_auto_roll[1], 0.0);
//
//		if (angVel.y > 0.002)
//		{
//			SetThrusterLevel(thruster_auto_py_1lb[2], 1.0); // RIGHT
//			SetThrusterLevel(thruster_auto_py_1lb[3], 0.0); // LEFT
//			autoMode = 21;
//		}
//		else if (angVel.y < -0.002)
//		{
//			SetThrusterLevel(thruster_auto_py_1lb[2], 0.0); // RIGHT
//			SetThrusterLevel(thruster_auto_py_1lb[3], 1.0); // LEFT
//			autoMode = 22;
//		}
//		else
//		{
//			SetThrusterLevel(thruster_auto_py_1lb[2], 0.0); // RIGHT
//			SetThrusterLevel(thruster_auto_py_1lb[3], 0.0); // LEFT
//			autoMode = 23;
//
//			AutopilotStatus = PITCHHOLD;
//		}
//	}
//	sprintf(oapiDebugString(), "auto Turnaround, yaw: %.2f. Mode %i. AngVel.y: %.4f (%.4f rad)", yaw * DEG, autoMode, angVel.y * DEG, angVel.y);
//}

void ProjectMercury::ReentryAttitudeAuto(double simt, double simdt)
{
	bool highThrust = false;

	if (oapiGetTimeAcceleration() < 1.5)
	{
		highThrust = true;
	}

	if (autoPilot && oapiGetTimeAcceleration() < 15.0) // for high time acc things get unstable
	{
		if (suborbitalMission) // Do suborbital reentry attitude (40 deg pitch)
		{
			SetPitchAuto(40.0, highThrust);
			SetYawAuto(highThrust);
			SetRollAuto(highThrust);
		}
		else // Do orbital reentry attitude (1.5 degree pitch)
		{
			SetPitchAuto(1.5, highThrust);
			SetYawAuto(highThrust);
			SetRollAuto(highThrust);
		}
	}
	else
	{
		DisableAutopilot(false); // eliminate stuck thrusters
	}
}

//void ProjectMercury::LowTorqueAttAuto(double simt, double simdt)
//{
//}
//
//void ProjectMercury::PitchAttAuto(double simt, double simdt)
//{
//	VECTOR3 angVel;
//	GetAngularVel(angVel);
//	double yaw = GetSlipAngle();
//	double autoVel = 0.0;
//
//	int autoMode;
//}

void ProjectMercury::GRollAuto(double simt, double simdt)
{
	VECTOR3 angVel;
	GetAngularVel(angVel);

	if (angVel.z < 10 * RAD)
	{
		SetThrusterLevel(thruster_auto_roll[0], 1.0);
	}
	else
	{
		SetThrusterLevel(thruster_auto_roll[0], 0.0);
	}
	SetThrusterLevel(thruster_auto_roll[1], 0.0); // ensure not stuck

	// Dampen pitch and yaw to 2 deg/s each
	if (angVel.x > 2.0 * RAD)
		SetThrusterLevel(thruster_auto_py[1], 1.0);
	else
		SetThrusterLevel(thruster_auto_py[1], 0.0);

	if (angVel.x < -2.0 * RAD)
		SetThrusterLevel(thruster_auto_py[0], 1.0);
	else
		SetThrusterLevel(thruster_auto_py[0], 0.0);

	if (angVel.y > 2.0 * RAD)
		SetThrusterLevel(thruster_auto_py[2], 1.0);
	else
		SetThrusterLevel(thruster_auto_py[2], 0.0);

	if (angVel.y < -2.0 * RAD)
		SetThrusterLevel(thruster_auto_py[3], 1.0);
	else
		SetThrusterLevel(thruster_auto_py[3], 0.0);
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
	abortSepSequencePerformed = true;
	SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);
	SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);
	CGshifted = false;
	VesselStatus = REENTRY;
}

void ProjectMercury::OffPadAbortSeparate(void)
{
	SeparateTower(false);
	SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);
	SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);
	CGshifted = false;
	VesselStatus = REENTRY;
}

void ProjectMercury::CreatePosigradeRockets(void)
{
	VECTOR3 m_exhaust_pos;
	VECTOR3 m_exhaust_ref;

	m_exhaust_pos = _V(-0.305, MERCURY_OFS_RETRO.y - 0.11, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(0.329, 0.119, 0.95);
	thruster_posigrade[0] = CreateThruster(m_exhaust_pos, m_exhaust_ref, POSIGRADE_THRUST, posigrade_propellant[0], POSIGRADE_ISP);
	AddExhaust(thruster_posigrade[0], 0.5, 0.02);

	m_exhaust_pos = _V(0.065, MERCURY_OFS_RETRO.y + 0.285, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(-0.075, -0.337, 0.95);
	thruster_posigrade[1] = CreateThruster(m_exhaust_pos, m_exhaust_ref, POSIGRADE_THRUST, posigrade_propellant[1], POSIGRADE_ISP);
	AddExhaust(thruster_posigrade[1], 0.5, 0.02);

	m_exhaust_pos = _V(.214, MERCURY_OFS_RETRO.y - 0.23, MERCURY_OFS_RETRO.z - .2);
	m_exhaust_ref = _V(-0.235, 0.253, 0.95);
	thruster_posigrade[2] = CreateThruster(m_exhaust_pos, m_exhaust_ref, POSIGRADE_THRUST, posigrade_propellant[2], POSIGRADE_ISP);
	AddExhaust(thruster_posigrade[2], 0.5, 0.02);

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

	m_exhaust_pos = _V(-0.196, 0.095, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(0.9, -0.436, 6.7);
	thruster_retro[0] = CreateThruster(m_exhaust_pos, m_exhaust_ref, RETRO_THRUST, retro_propellant[0], RETRO_ISP);
	AddExhaust(thruster_retro[0], 1.8, 0.05);
	LightEmitter* retro1_light = AddPointLight(m_exhaust_pos, 20, 0.1, -0.01, 0.2, col_d, col_s, col_a);
	retro1_light->SetIntensityRef(&RETRO_THRUST_LEVEL[0]);

	m_exhaust_pos = _V(-0.005, -0.19, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(0.026, 1, 7.7);
	thruster_retro[1] = CreateThruster(m_exhaust_pos, m_exhaust_ref, RETRO_THRUST, retro_propellant[1], RETRO_ISP);
	AddExhaust(thruster_retro[1], 1.8, 0.05);
	LightEmitter* retro2_light = AddPointLight(m_exhaust_pos, 20, 0.1, -0.01, 0.2, col_d, col_s, col_a);
	retro2_light->SetIntensityRef(&RETRO_THRUST_LEVEL[1]);

	m_exhaust_pos = _V(0.175, 0.107, MERCURY_OFS_RETRO.z - 0.2);
	m_exhaust_ref = _V(-0.853, -0.522, 7.1);
	thruster_retro[2] = CreateThruster(m_exhaust_pos, m_exhaust_ref, RETRO_THRUST, retro_propellant[2], RETRO_ISP);
	AddExhaust(thruster_retro[2], 1.8, 0.05);
	LightEmitter* retro3_light = AddPointLight(m_exhaust_pos, 20, 0.1, -0.01, 0.2, col_d, col_s, col_a);
	retro3_light->SetIntensityRef(&RETRO_THRUST_LEVEL[2]);
}

void ProjectMercury::PrepareReentry(void)
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

void ProjectMercury::CreateRCS(void)
{
	if (rcsExists)
		return;

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
	thruster_man_py[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_auto, MERCURY_ISP_ATT);
	att_ref = _V(-0.06, -.41, 1.05);
	att_dir = _V(0, -1, 0);
	//AddExhaust(thruster_man_py[0], .2, .02, att_ref, att_dir);
	rcsStream[0] = AddExhaustStream(thruster_man_py[0], att_ref, &RCSHigh);

	att_ref = _V(0.0, .41, 1.05);
	att_dir = _V(0, -1, 0);
	thruster_man_py[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_auto, MERCURY_ISP_ATT);
	att_ref = _V(0.06, .41, 1.05);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_man_py[1], .2, .02, att_ref, att_dir);
	rcsStream[1] = AddExhaustStream(thruster_man_py[1], att_ref, &RCSHigh);

	att_ref = _V(-.41, 0.0, 1.05);
	att_dir = _V(1, 0, 0);
	thruster_man_py[2] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_auto, MERCURY_ISP_ATT);
	att_ref = _V(-.41, 0.06, 1.05);
	att_dir = _V(-1, 0, 0);
	// AddExhaust(thruster_man_py[2], .2, .02, att_ref, att_dir);
	rcsStream[2] = AddExhaustStream(thruster_man_py[2], att_ref, &RCSHigh);

	att_ref = _V(.41, 0.0, 1.05);
	att_dir = _V(-1, 0, 0);
	thruster_man_py[3] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_auto, MERCURY_ISP_ATT);
	att_ref = _V(.41, -0.06, 1.05);
	att_dir = _V(1, 0, 0);
	// AddExhaust(thruster_man_py[3], .2, .02, att_ref, att_dir);
	rcsStream[3] = AddExhaustStream(thruster_man_py[3], att_ref, &RCSHigh);

	att_ref = _V(0.80, .05, 0.0);
	att_dir = _V(0, -1, 0);
	thruster_man_roll[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ROLL_ATT, fuel_auto, MERCURY_ISP_ATT);
	att_ref = _V(0.80, .05, -.6);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_man_roll[0], .2, .02, att_ref, att_dir);
	rcsStream[4] = AddExhaustStream(thruster_man_roll[0], att_ref, &RCSHigh);

	att_ref = _V(0.80, -.05, 0.0);
	att_dir = _V(0, 1, 0);
	thruster_man_roll[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ROLL_ATT, fuel_auto, MERCURY_ISP_ATT);
	att_ref = _V(0.80, -.05, -.6);
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_man_roll[1], .2, .02, att_ref, att_dir);
	rcsStream[5] = AddExhaustStream(thruster_man_roll[1], att_ref, &RCSHigh);

	att_ref = _V(0.0, -.41, 1.05);
	att_dir = _V(0, 1, 0);
	thruster_auto_py[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_auto_py[0], .2, .02, att_ref, att_dir);
	rcsStream[6] = AddExhaustStream(thruster_auto_py[0], att_ref, &RCSHigh);

	att_ref = _V(0.0, -.41, 1.05);
	att_dir = _V(0, 1, 0);
	thruster_auto_py_1lb[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(0.07, -.41, 1.05);
	//att_ref = _V(0.07,-.41,1.05);
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_auto_py_1lb[0], .07, .02, att_ref, att_dir);
	rcsStream[7] = AddExhaustStream(thruster_auto_py_1lb[0], att_ref, &RCSLow);

	att_ref = _V(0.0, .41, 1.05);
	att_dir = _V(0, -1, 0);
	thruster_auto_py[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_auto_py[1], .2, .02, att_ref, att_dir);
	rcsStream[8] = AddExhaustStream(thruster_auto_py[1], att_ref, &RCSHigh);

	att_ref = _V(0.0, .41, 1.05);
	att_dir = _V(0, -1, 0);
	thruster_auto_py_1lb[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.07, .41, 1.05);
	//att_ref = _V(-0.12,.41,1.05);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_auto_py_1lb[1], .07, .02, att_ref, att_dir);
	rcsStream[9] = AddExhaustStream(thruster_auto_py_1lb[1], att_ref, &RCSLow);

	att_ref = _V(-.41, 0.0, 1.05);
	att_dir = _V(1, 0, 0);
	thruster_auto_py[2] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_dir = _V(-1, 0, 0);
	// AddExhaust(thruster_auto_py[2], .2, .02, att_ref, att_dir);
	rcsStream[10] = AddExhaustStream(thruster_auto_py[2], att_ref, &RCSHigh);

	att_ref = _V(-.41, 0.0, 1.05);
	att_dir = _V(1, 0, 0);
	thruster_auto_py_1lb[2] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-.41, -0.07, 1.05);
	att_dir = _V(-1, 0, 0);
	// AddExhaust(thruster_auto_py_1lb[2], .07, .02, att_ref, att_dir);
	rcsStream[11] = AddExhaustStream(thruster_auto_py_1lb[2], att_ref, &RCSLow);

	att_ref = _V(.41, 0.0, 1.05);
	att_dir = _V(-1, 0, 0);
	thruster_auto_py[3] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_dir = _V(1, 0, 0);
	// AddExhaust(thruster_auto_py[3], .2, .02, att_ref, att_dir);
	rcsStream[12] = AddExhaustStream(thruster_auto_py[3], att_ref, &RCSHigh);

	att_ref = _V(.41, 0.0, 1.05);
	att_dir = _V(-1, 0, 0);
	thruster_auto_py_1lb[3] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(.41, 0.07, 1.05);
	att_dir = _V(1, 0, 0);
	// AddExhaust(thruster_auto_py_1lb[3], .07, .02, att_ref, att_dir);
	rcsStream[13] = AddExhaustStream(thruster_auto_py_1lb[3], att_ref, &RCSLow);

	att_ref = _V(-0.80, -.05, 0.0);
	att_dir = _V(0, 1, 0);
	thruster_auto_roll[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ROLL_ATT, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.80, -.05, -.6);
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_auto_roll[0], .2, .02, att_ref, att_dir);
	rcsStream[14] = AddExhaustStream(thruster_auto_roll[0], att_ref, &RCSHigh);

	att_ref = _V(-0.80, -.05, 0.0);
	att_dir = _V(0, 1, 0);
	thruster_auto_roll_1lb[0] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.80, -.05, -.63);
	att_dir = _V(0, -1, 0);
	// AddExhaust(thruster_auto_roll_1lb[0], .07, .02, att_ref, att_dir);
	rcsStream[15] = AddExhaustStream(thruster_auto_roll_1lb[0], att_ref, &RCSLow);

	att_ref = _V(-0.80, .05, 0.0);
	att_dir = _V(0, -1, 0);
	thruster_auto_roll[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ROLL_ATT, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.80, .05, -.6);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_auto_roll[1], .2, .02, att_ref, att_dir);
	rcsStream[16] = AddExhaustStream(thruster_auto_roll[1], att_ref, &RCSHigh);

	att_ref = _V(-0.80, .05, 0.0);
	att_dir = _V(0, -1, 0);
	thruster_auto_roll_1lb[1] = CreateThruster(att_ref, att_dir, MERCURY_THRUST_ATT_LOW, fuel_auto, MERCURY_ISP_ATT); // fuel auto
	att_ref = _V(-0.80, .05, -.63);
	att_dir = _V(0, 1, 0);
	// AddExhaust(thruster_auto_roll_1lb[1], .07, .02, att_ref, att_dir);
	rcsStream[17] = AddExhaustStream(thruster_auto_roll_1lb[1], att_ref, &RCSLow);

	// Manual groups
	CreateThrusterGroup(&thruster_man_py[0], 1, THGROUP_ATT_PITCHUP);
	CreateThrusterGroup(&thruster_man_py[1], 1, THGROUP_ATT_PITCHDOWN);
	CreateThrusterGroup(&thruster_man_py[2], 1, THGROUP_ATT_YAWRIGHT);
	CreateThrusterGroup(&thruster_man_py[3], 1, THGROUP_ATT_YAWLEFT);
	CreateThrusterGroup(&thruster_man_roll[0], 1, THGROUP_ATT_BANKRIGHT);
	CreateThrusterGroup(&thruster_man_roll[1], 1, THGROUP_ATT_BANKLEFT);

	// Auto groups
	pitchup = CreateThrusterGroup(&thruster_auto_py[0], 1, THGROUP_ATT_PITCHUP);
	pitchdown = CreateThrusterGroup(&thruster_auto_py[1], 1, THGROUP_ATT_PITCHDOWN);
	yawright = CreateThrusterGroup(&thruster_auto_py[2], 1, THGROUP_ATT_YAWRIGHT);
	yawleft = CreateThrusterGroup(&thruster_auto_py[3], 1, THGROUP_ATT_YAWLEFT);
	bankright = CreateThrusterGroup(&thruster_auto_roll[0], 1, THGROUP_ATT_BANKRIGHT);
	bankleft = CreateThrusterGroup(&thruster_auto_roll[1], 1, THGROUP_ATT_BANKLEFT);

	RcsStatus = AUTOHIGH;
	SetDefaultPropellantResource(fuel_auto);
	//attitudeFuelAuto = true;
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

void ProjectMercury::SwitchAttitudeMode(void)
{
	if (RcsStatus == AUTOHIGH)
	{
		DelThrusterGroup(pitchup);
		DelThrusterGroup(pitchdown);
		DelThrusterGroup(yawright);
		DelThrusterGroup(yawleft);
		DelThrusterGroup(bankleft);
		DelThrusterGroup(bankright);

		pitchup = CreateThrusterGroup(&thruster_auto_py[0], 1, THGROUP_ATT_PITCHUP);
		pitchdown = CreateThrusterGroup(&thruster_auto_py[1], 1, THGROUP_ATT_PITCHDOWN);
		yawright = CreateThrusterGroup(&thruster_auto_py[2], 1, THGROUP_ATT_YAWRIGHT);
		yawleft = CreateThrusterGroup(&thruster_auto_py[3], 1, THGROUP_ATT_YAWLEFT);
		bankright = CreateThrusterGroup(&thruster_auto_roll[0], 1, THGROUP_ATT_BANKRIGHT);
		bankleft = CreateThrusterGroup(&thruster_auto_roll[1], 1, THGROUP_ATT_BANKLEFT);
	}
	else if (RcsStatus == MANUAL)
	{
		DelThrusterGroup(pitchup);
		DelThrusterGroup(pitchdown);
		DelThrusterGroup(yawright);
		DelThrusterGroup(yawleft);
		DelThrusterGroup(bankleft);
		DelThrusterGroup(bankright);

		pitchup = CreateThrusterGroup(&thruster_man_py[0], 1, THGROUP_ATT_PITCHUP);
		pitchdown = CreateThrusterGroup(&thruster_man_py[1], 1, THGROUP_ATT_PITCHDOWN);
		yawright = CreateThrusterGroup(&thruster_man_py[2], 1, THGROUP_ATT_YAWRIGHT);
		yawleft = CreateThrusterGroup(&thruster_man_py[3], 1, THGROUP_ATT_YAWLEFT);
		bankright = CreateThrusterGroup(&thruster_man_roll[0], 1, THGROUP_ATT_BANKRIGHT);
		bankleft = CreateThrusterGroup(&thruster_man_roll[1], 1, THGROUP_ATT_BANKLEFT);
	}
	else
	{
		DelThrusterGroup(pitchup);
		DelThrusterGroup(pitchdown);
		DelThrusterGroup(yawright);
		DelThrusterGroup(yawleft);
		DelThrusterGroup(bankleft);
		DelThrusterGroup(bankright);

		pitchup = CreateThrusterGroup(&thruster_auto_py_1lb[0], 1, THGROUP_ATT_PITCHUP);
		pitchdown = CreateThrusterGroup(&thruster_auto_py_1lb[1], 1, THGROUP_ATT_PITCHDOWN);
		yawright = CreateThrusterGroup(&thruster_auto_py_1lb[2], 1, THGROUP_ATT_YAWRIGHT);
		yawleft = CreateThrusterGroup(&thruster_auto_py_1lb[3], 1, THGROUP_ATT_YAWLEFT);
		bankright = CreateThrusterGroup(&thruster_auto_roll_1lb[0], 1, THGROUP_ATT_BANKRIGHT);
		bankleft = CreateThrusterGroup(&thruster_auto_roll_1lb[1], 1, THGROUP_ATT_BANKLEFT);
	}
}

void ProjectMercury::SwitchPropellantSource(void)
{
	double currentAuto = GetPropellantMass(fuel_auto);
	double currentManual = GetPropellantMass(fuel_manual);
	double currentAutoMax = GetPropellantMaxMass(fuel_auto);
	double currentManualMax = GetPropellantMaxMass(fuel_manual);

	SetPropellantMaxMass(fuel_manual, currentAutoMax);
	SetPropellantMaxMass(fuel_auto, currentManualMax);
	SetPropellantMass(fuel_manual, currentAuto);
	SetPropellantMass(fuel_auto, currentManual);

	attitudeFuelAuto = !attitudeFuelAuto;
}

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

	if (GetFuelMass() == 0.0 && (GetPropellantMass(fuel_manual) != 0.0 || GetPropellantMass(fuel_auto) != 0.0))
	{
		SwitchPropellantSource();
	}

}

inline VECTOR3 ProjectMercury::FlipX(VECTOR3 vIn)
{
	VECTOR3 vOut;
	vOut.x = -vIn.x;
	vOut.y = vIn.y;
	vOut.z = vIn.z;
	return vOut;
}

inline VECTOR3 ProjectMercury::FlipY(VECTOR3 vIn)
{
	VECTOR3 vOut;
	vOut.x = vIn.x;
	vOut.y = -vIn.y;
	vOut.z = vIn.z;
	return vOut;
}

inline double ProjectMercury::OrbitArea(double angle, double ecc)
{
	double AreaResult = (ecc * tan(angle / 2.0) / ((1.0 - ecc * ecc) * (ecc * tan(angle / 2.0) * tan(angle / 2.0) - tan(angle / 2.0) * tan(angle / 2.0) - ecc - 1.0)) - (-PI * floor(angle / PI2 + 0.5) + atan((ecc * tan(angle / 2.0) - tan(angle / 2.0)) / sqrt(1.0 - ecc * ecc))) / (sqrt(1 - ecc * ecc) * (1 - ecc * ecc)));
	return AreaResult;
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
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
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
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
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
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
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
	if (GetAltitude() > 5e4)
		createdAbove50km[stuffCreated] = true;
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
		if (GetAltitude() > 5e4)
			createdAbove50km[stuffCreated] = true;
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
		if (GetAltitude() > 5e4)
			createdAbove50km[stuffCreated] = true;
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
		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Antenna", &vs);
		if (GetAltitude() > 5e4)
			createdAbove50km[stuffCreated] = true;
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
		Drogue = AddMesh(oapiLoadMeshGlobal("ProjectMercury\\merc_drogue"), &MERCURY_OFS_DROGUE);
		SetMeshVisibleInternal(Drogue, true);
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
		Mainchute = AddMesh(oapiLoadMeshGlobal("ProjectMercury\\merc_chute"), &MERCURY_OFS_MAINCHUTE);
		SetMeshVisibleInternal(Mainchute, true);
		DefineMainChuteAnimation();

		CreateVariableDragElement(&mainChuteProgressArea, 218.0, MERCURY_OFS_MAINCHUTE); // calibrated factor (second input) to sink rate 9.144 m/s (30 ft/s) at sea level
		SetRotDrag(MERCURY_ROT_DRAG_CHUTE);
	}
}

void ProjectMercury::DeployLandingBag(void)
{
	if (!landingBagDeployed)
	{
		landingBagDeployed = true;
		LandingBagStatus = L_OPENING;
		landingBagMoving = true;
		landingBagProgress = 0.0;

		Landingbag = AddMesh(oapiLoadMeshGlobal("ProjectMercury\\merc_LandingBag"), &(MERCURY_OFS_LANDBAG - MERCURY_OFS_CAPSULE));
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
		if (GetAltitude() > 5e4)
			createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(Mainchute);
		mainChuteSeparated = true;

		mainChuteProgress = 0.0;
		mainChuteProgressArea = 0.0;
		ClearVariableDragElements(); // Clear main chute drag
		SetSize(MERCURY_LENGTH_CAPSULE);
		SetRotDrag(MERCURY_ROT_DRAG);
	}
	else if (reserveChuteDeployed && !reserveChuteSeparated)
	{
		strcat(name, " Reserve chute");

		createdVessel[stuffCreated] = oapiCreateVesselEx(name, "ProjectMercury\\Mercury_Chute", &vs);
		if (GetAltitude() > 5e4)
			createdAbove50km[stuffCreated] = true;
		stuffCreated += 1;
		DelMesh(Mainchute);
		reserveChuteSeparated = true;

		mainChuteProgress = 0.0;
		mainChuteProgressArea = 0.0;
		ClearVariableDragElements(); // Clear main chute drag
		SetSize(MERCURY_LENGTH_CAPSULE);
		SetRotDrag(MERCURY_ROT_DRAG);
	}
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
	if (mainChuteMoving)
	{
		double animationStep = simdt * MAIN_CHUTE_OPERATING_SPEED;

		if (MainChuteStatus == OPENING) // initial deployment
		{
			if (mainChuteEndAnimation && mainChuteProgress == 1.0) // if reserve chute, then progress is reset, to allow new deployment
			{
				MainChuteStatus = REEFED;
				mainChuteReefedTime = simt;
				mainChuteEndAnimation = false;
			}
			else if (mainChuteProgress > 1.0) // finished
			{
				mainChuteProgress = 1.0;
				mainChuteEndAnimation = true;
				mainChuteMoving = false;
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
		else if (MainChuteStatus == REEFED && (simt - mainChuteReefedTime) > 0.0) // Reefed state. But don't see the point in the time check
		{
			MainChuteStatus = UNREEFING;
		}
	}
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

inline void ProjectMercury::MercuryGenericConstructor(void)
{
	adaptcover1 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover1");
	adaptcover2 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover2");
	adaptcover3 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptcover3");
	adaptring1 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring1");
	adaptring2 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring2");
	adaptring3 = oapiLoadMeshGlobal("ProjectMercury\\merc_adaptring3");
	tower = oapiLoadMeshGlobal("ProjectMercury\\merc_tower");
	antennahouse = oapiLoadMeshGlobal("ProjectMercury\\merc_anthouse");
	droguecover = oapiLoadMeshGlobal("ProjectMercury\\merc_DrogueCover");
	retro = oapiLoadMeshGlobal("ProjectMercury\\merc_retro");
	retrocover1 = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover1");
	retrocover2 = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover2");
	retrocover3 = oapiLoadMeshGlobal("ProjectMercury\\merc_retrocover3");
	retrostrap1 = oapiLoadMeshGlobal("ProjectMercury\\merc_strap1");
	retrostrap2 = oapiLoadMeshGlobal("ProjectMercury\\merc_strap2");
	retrostrap3 = oapiLoadMeshGlobal("ProjectMercury\\merc_strap3");
	explosivebolt = oapiLoadMeshGlobal("ProjectMercury\\merc_explosivebolt");

	VesselStatus = LAUNCH;
	PreviousVesselStatus = LAUNCH;
	DrogueStatus = CLOSED;
	MainChuteStatus = CLOSED;
	RcsStatus = AUTOHIGH;
	AutopilotStatus = AUTOLAUNCH;

	CGshifted = false;
}

inline void ProjectMercury::WriteFlightParameters(void)
{
	char cbuf[256];
	sprintf(cbuf, "%s Flight parameters:", GetName());
	oapiWriteLog(cbuf);
	if (historyReference == NULL) // probably spawned after launch
	{
		historyReference = GetSurfaceRef();
		oapiWriteLogV(" > FULL FLIGHT NOT RECORDED");
	}
	if (historyLandLat == NULL) // probably not landed when scenario exits
	{
		double radiusDontCare;
		GetEquPos(historyLandLong, historyLandLat, radiusDontCare);
		oapiWriteLogV(" > FULL FLIGHT NOT RECORDED");
	}
	oapiWriteLogV(" > Cut-off altitude: %.0f m (%.0f ft)", historyCutOffAlt, historyCutOffAlt / 0.3048);
	oapiWriteLogV(" > Cut-off space-fixed velocity: %.0f m/s (%.0f ft/s)", historyCutOffVel, historyCutOffVel / 0.3048);
	oapiWriteLogV(" > Cut-off flight path angle: %.4f deg", historyCutOffAngl);
	double range = oapiOrthodome(historyLaunchLong, historyLaunchLat, historyLandLong, historyLandLat) * oapiGetSize(historyReference) / 1000.0;
	oapiWriteLogV(" > Range: %.1f km (%.1f nm)", range, range / 1.852);

	if (suborbitalMission) // Redstone mission or Atlas abort
	{
		oapiWriteLogV(" > Maximum altitude: %.1f km (%.1f nm)", historyMaxAltitude / 1000.0, historyMaxAltitude / 1852.0);
		int weightM = (int)floor(historyWeightlessTime / 60.0);
		int weightS = (int)floor((historyWeightlessTime - weightM * 60.0));
		oapiWriteLogV(" > Period of weightlessness: %01i:%02i", weightM, weightS);
	}
	else // Non-aborted Atlas mission
	{
		oapiWriteLogV(" > Apogee altitude: %.1f km (%.1f nm)", historyMaxAltitude / 1000.0, historyMaxAltitude / 1852.0);
		oapiWriteLogV(" > Perigee altitude: %.1f km (%.1f nm)", historyPerigee / 1000.0, historyPerigee / 1852.0);
		oapiWriteLogV(" > Inclination angle: %.2f deg", historyInclination); // only true for full orbit
	}
	oapiWriteLogV(" > Maximum exit load factor: %.1f g", historyMaxLaunchAcc / G);
	oapiWriteLogV(" > Maximum reentry load factor: %.1f g", historyMaxReentryAcc / G);
	oapiWriteLogV(" > Earth-fixed velocity: %.0f m/s (%.0f ft/s)", historyMaxEarthSpeed, historyMaxEarthSpeed / 0.3048);
	oapiWriteLogV(" > Space-fixed velocity: %.0f m/s (%.0f ft/s)", historyMaxSpaceSpeed, historyMaxSpaceSpeed / 0.3048);
}

inline void ProjectMercury::ReadConfigSettings(FILEHANDLE cfg)
{
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
	if (!oapiReadItem_int(cfg, "HUD2ndColumnPos", secondColumnHUDx))
	{
		secondColumnHUDx = 28;
		oapiWriteLog("Mercury could not read HUD column position config.");
	}
}

inline void ProjectMercury::CreateCapsuleFuelTanks(void)
{
	// propellant resources, in order of last deleted to first
	fuel_manual = CreatePropellantResource(MERCURY_FUEL_MASS_MAN);
	//SetPropellantMass(fuel_manual, MERCURY_FUEL_MASS_MAN);

	fuel_auto = CreatePropellantResource(MERCURY_FUEL_MASS_AUTO);
	//SetPropellantMass(fuel_auto, MERCURY_FUEL_MASS_AUTO);

	attitudeFuelAuto = true;

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

	escape_tank = CreatePropellantResource(ABORT_MASS_FUEL);
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

	Antennahouse = AddMesh(antennahouse, &MERCURY_OFS_ANTHOUSE);
	Droguecover = AddMesh(droguecover, &MERCURY_OFS_DROGUECOVER);
	Retro = AddMesh(retro, &MERCURY_OFS_RETRO);
	Retrocover1 = AddMesh(retrocover1, &MERCURY_OFS_RETROCOVER1);
	Retrocover2 = AddMesh(retrocover2, &MERCURY_OFS_RETROCOVER2);
	Retrocover3 = AddMesh(retrocover3, &MERCURY_OFS_RETROCOVER3);
	Retrostrap1 = AddMesh(retrostrap1, &MERCURY_OFS_RETROSTRAP1);
	Retrostrap2 = AddMesh(retrostrap2, &MERCURY_OFS_RETROSTRAP2);
	Retrostrap3 = AddMesh(retrostrap3, &MERCURY_OFS_RETROSTRAP3);
	Explosivebolt = AddMesh(explosivebolt, &MERCURY_OFS_EXPLOSIVEBOLT);
}

inline void ProjectMercury::CapsuleGenericPostCreation(void)
{
	// HUD constants
	oapiGetViewportSize(&ScreenWidth, &ScreenHeight, &ScreenColour);
	TextX0 = (int)(0.025 * ScreenWidth);
	TextY0 = (int)(0.225 * ScreenHeight);
	LineSpacing = (int)(0.025 * ScreenHeight);

	if (!capsuleDefined) // something went wrong in the scenario reading
	{
		oapiWriteLog("No Mercury capsule specified");
		capsule = oapiLoadMeshGlobal("ProjectMercury\\merc_Freedom7II");
		Capsule = AddMesh(capsule, &MERCURY_OFS_CAPSULE);
	}

	if (GetDamageModel() == 0) // overwrite any scenario definition
		enableAbortConditions = false;
}

inline void ProjectMercury::DeleteRogueVessels(void)
{
	// Delete created vessels that bounce off into infinity (hopefully this will reduce crashes). Maybe use this for Redstone too, although we don't do time acc there
	for (int i = 0; i < stuffCreated; i++)
	{
		if (createdVessel[i] != NULL)
		{
			//VECTOR3 vesselPosition;
			double vesselHeight;
			oapiGetAltitude(createdVessel[i], &vesselHeight);
			//GetRelativePos(createdVessel[i], vesselPosition);
			if ((vesselHeight < 2e4 && createdAbove50km[i]) || vesselHeight > 1e6) // way close and was way away, or just way away
			{
				char vesselName[256];
				oapiGetObjectName(createdVessel[i], vesselName, 256);

				bool deleteResult = oapiDeleteVessel(createdVessel[i]);
				if (deleteResult)
				{
					char cbuf[256];
					sprintf(cbuf, "Delete vessel %s due to distance %.2f km", vesselName, vesselHeight / 1000.0);
					oapiWriteLog(cbuf);
				}
				else
				{
					char cbuf[256];
					sprintf(cbuf, "Failed to delete vessel %s due to distance %.2f km", vesselName, vesselHeight / 1000.0);
					oapiWriteLog(cbuf);
				}

				createdVessel[i] = NULL;
				createdAbove50km[i] = false;
				//stuffCreated -= 1;
			}
		}
	}
}

inline void ProjectMercury::CapsuleAutopilotControl(double simt, double simdt)
{
	if (autoPilot && VesselStatus == FLIGHT)
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

				// Turn off thrusters
				SetThrusterLevel(thruster_auto_py[0], 0.0);
				SetThrusterLevel(thruster_auto_py[1], 0.0);
				SetThrusterLevel(thruster_auto_py[2], 0.0);
				SetThrusterLevel(thruster_auto_py[3], 0.0);
				SetThrusterLevel(thruster_auto_roll[0], 0.0);
				SetThrusterLevel(thruster_auto_roll[1], 0.0);
			}
		}
		else if (AutopilotStatus == TURNAROUND)
		{
			RetroAttitudeAuto(simt, simdt);
		}
		else if (AutopilotStatus == PITCHHOLD)
		{
			if (RETRO_THRUST_LEVEL[0] + RETRO_THRUST_LEVEL[1] + RETRO_THRUST_LEVEL[2] != 0.0) // retros running
			{
				RetroAttitudeAuto(simt, simdt);
			}
			else if (engageRetro && simt - retroStartTime < 23.0) // shortly after shutoff (22 sec burntime)
			{
				DisableAutopilot(false);
			}
		}

		if (engageRetro && simt - retroStartTime > 60.0) // both for suborbital and orbital missions (19640056774 page 12 and doi:10.1002/j.2161-4296.1962.tb02524.x page 2)
		{
			prepareReentryAction = true;
			AutopilotStatus = REENTRYATT;
		}
	}
	else if (autoPilot && VesselStatus == REENTRY)
	{
		if (AutopilotStatus == REENTRYATT)
		{
			ReentryAttitudeAuto(simt, simdt);

			if (vesselAcceleration > 0.05 * G)
			{
				GetRelativeVel(GetSurfaceRef(), entryVel);
				GetRelativePos(GetSurfaceRef(), entryLoc);

				entryAng = -acos(dotp(entryLoc, entryVel) / length(entryLoc) / length(entryVel)) * DEG + 90.0;

				double radius;
				GetEquPos(lowGLong, lowGLat, radius);

				entryAngleToBase = oapiOrthodome(lowGLong, lowGLat, missionLandLong * RAD, missionLandLat * RAD);
				ELEMENTS el;
				ORBITPARAM prm;
				GetElements(GetSurfaceRef(), el, &prm, 0.0, FRAME_EQU);
				lowGInc = el.i * DEG;

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

inline void ProjectMercury::FlightReentryAbortControl(double simt, double simdt, double latit, double longit, double getAlt)
{
	// Rate damping if aborting (19670028606 page 97 and 98)
	if (abort && (VesselStatus == FLIGHT || VesselStatus == REENTRY) && !drogueDeployed && abortDamping)
	{
		DisableAutopilot(false); // remove stuck problems
		AuxDampingAuto(true);
	}

	// Calculate G force for activation of tower & retro separation at 0.25 G
	double m = GetMass();
	VECTOR3 F, W;
	GetForceVector(F);
	GetWeightVector(W);
	vesselAcceleration = length((F - W) / m);

	// Retrosequence
	if (engageRetro && simt > retroStartTime) // firing starts 30 sec after retrosequence start
	{
		if (!retroCoverSeparated[1 - 1])
		{
			separateRetroCoverAction[1 - 1] = true;
		}
		SetThrusterLevel(thruster_retro[0], 1.0);
		RETRO_THRUST_LEVEL[0] = GetThrusterLevel(thruster_retro[0]); // For local lights
		if (simt > (retroStartTime + 5.0))
		{
			if (!retroCoverSeparated[2 - 1])
			{
				separateRetroCoverAction[2 - 1] = true;
			}
			SetThrusterLevel(thruster_retro[1], 1.0);
			RETRO_THRUST_LEVEL[1] = GetThrusterLevel(thruster_retro[1]); // For local lights
			if (simt > (retroStartTime + 10.0))
			{
				if (!retroCoverSeparated[3 - 1])
				{
					separateRetroCoverAction[3 - 1] = true;
				}
				SetThrusterLevel(thruster_retro[2], 1.0);
				RETRO_THRUST_LEVEL[2] = GetThrusterLevel(thruster_retro[2]); // For local lights
			}
		}
	}

	// Jettison retro if aerodynamic stresses are too high (a la MA6)
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
		outfile.open("C:\\Orbiter2016\\MercuryEntryLog.txt", std::ios_base::app); // app = append
		outfile << entryAng << " , " << length(entryVel) << " , " << (length(entryLoc) - oapiGetSize(GetSurfaceRef())) << " , " << (entryAngleToBase * DEG) << " , " << (angleCovered * DEG) << " , " << (lowGLat * DEG) << " , " << lowGInc << "\n";

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

	if (VesselStatus == LAUNCH && GroundContact())
	{
		historyLaunchLong = longit;
		historyLaunchLat = latit;
		historyReference = GetSurfaceRef();
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
		if (autoPilot && AutopilotStatus == POSIGRADEDAMP)
		{
			sprintf(cbuf, "Capsule turnaround in %.2f s", 5.0 + posigradeDampingTime - simt);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (autoPilot && AutopilotStatus == TURNAROUND)
		{
			sprintf(cbuf, "Hold retro attitude");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  P: %.1f\u00B0, Y: %.1f\u00B0, R: %.1f\u00B0", GetPitch() * DEG, GetSlipAngle() * DEG, GetBank() * DEG);
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;
		}
		else if (autoPilot && AutopilotStatus == PITCHHOLD)
		{
			sprintf(cbuf, "Holding retro attitude");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  P: %.1f\u00B0, Y: %.1f\u00B0, R: %.1f\u00B0", GetPitch() * DEG, GetSlipAngle() * DEG, GetBank() * DEG);
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
	else if (autoPilot && VesselStatus == REENTRY)
	{
		if (AutopilotStatus == REENTRYATT)
		{
			sprintf(cbuf, "Holding reentry pitch:");
			skp->Text(secondColumnHUDx * TextX0, yIndex * LineSpacing + TextY0, cbuf, strlen(cbuf));
			yIndex += 1;

			sprintf(cbuf, "  %.1f\u00B0, %.1f\u00B0, %.1f\u00B0", GetPitch() * DEG, GetSlipAngle() * DEG, GetBank() * DEG);
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

	static UINT heatShieldGroups[1] = { heatShieldGroup };

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