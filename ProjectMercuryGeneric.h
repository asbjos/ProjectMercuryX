#pragma once

// ==============================================================
//				Header file for Project Mercury.
//				Created by Asbjørn "asbjos" Krüger
//					asbjorn.kruger@gmail.com
//						Made in 2020
//
// Includes genereal functions for all types of smaller projects,
// including Redstone, Atlas, Capsule, Little Joe and Scout.
//
// Thank you to all Orbiter-Forum users sharing code, tutorials,
// questions, and general help.
// 
// ==============================================================

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

inline VECTOR3 ProjectMercury::SwapXY(VECTOR3 vIn)
{
	VECTOR3 vOut;
	vOut.x = vIn.y;
	vOut.y = vIn.x;
	vOut.z = vIn.z;
	return vOut;
}

//inline double ProjectMercury::OrbitArea(double angle, double ecc)
//{
//	double AreaResult = (ecc * tan(angle / 2.0) / ((1.0 - ecc * ecc) * (ecc * tan(angle / 2.0) * tan(angle / 2.0) - tan(angle / 2.0) * tan(angle / 2.0) - ecc - 1.0)) - (-PI * floor(angle / PI2 + 0.5) + atan((ecc * tan(angle / 2.0) - tan(angle / 2.0)) / sqrt(1.0 - ecc * ecc))) / (sqrt(1 - ecc * ecc) * (1 - ecc * ecc)));
//	return AreaResult;
//}

double ProjectMercury::OrbitalFrameSlipAngle(VECTOR3 pos, VECTOR3 vel)
{
	VECTOR3 axis1, axis2, axisVel;
	axisVel = vel;
	axisVel = unit(axisVel);
	VECTOR3 axisPos, axisH;
	axisPos = pos;
	axisH = crossp(axisVel, axisPos); // direction of orbital momentum
	axis2 = unit(crossp(axisH, axisVel));
	axis1 = crossp(axis2, axisVel);
	MATRIX3 Rref = _M(axis1.x, axis2.x, axisVel.x, axis1.y, axis2.y, axisVel.y, axis1.z, axis2.z, axisVel.z);

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

double ProjectMercury::OrbitalFrameSlipAngle2(VECTOR3 pos, VECTOR3 vel)
{
	VECTOR3 axis1, axis2, axis3;
	axis3 = vel;
	axis3 = unit(axis3);
	VECTOR3 vv, vm;
	vv = pos;
	vm = crossp(axis3, vv); // direction of orbital momentum
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
	euler.x = atan2(shipx.y, shipy.y);  // roll angle
	euler.y = asin(shipz.y);				// pitch angle
	euler.z = atan2(shipz.x, shipz.z);	// yaw angle

	return euler.z;
}

void ProjectMercury::AimEulerAngle(double pitch, double yaw)
{
	SetControlSurfaceLevel(AIRCTRL_AILERON, 0.0, true);
	SetControlSurfaceLevel(AIRCTRL_ELEVATOR, 0.0, true);
	SetControlSurfaceLevel(AIRCTRL_RUDDER, 0.0, true);

	yaw *= -1.0; // That's just how things are in this universe

	MATRIX3 R, rotR, rotPitch, rotYaw;
	GetRotationMatrix(R);
	rotPitch = _M(1.0, 0.0, 0.0,
		0.0, cos(pitch), sin(pitch),
		0.0, -sin(pitch), cos(pitch));
	rotYaw = _M(cos(yaw), 0.0, sin(yaw),
		0.0, 1.0, 0.0,
		-sin(yaw), 0.0, cos(yaw));
	rotR = mul(rotPitch, rotYaw);
	SetRotationMatrix(mul(R, rotR));

	VECTOR3 AngularVel;
	if (length(AngularVel) > 7.5 * RAD) SetAngularVel(_V(0, 0, 0)); // safety
}

// random number between 0 and 1
inline double ProjectMercury::GenerateRandom01(void)
{
	return double(rand() / (RAND_MAX + 1.0));
}

// Generate random gaussian number. Use a1 and a2 for different distribution.
// This is called the Box-Muller transform.
// This is for std 1 and exp 0, so to get specific std and exp, use std * genRandNorm + exp
inline double ProjectMercury::GenerateRandomNorm(double a1, double a2)
{
	double random1 = GenerateRandom01();
	double random2 = GenerateRandom01();

	return pow(-a1 * log(random1), a2) * cos(PI2 * random2);
}

// Generate gaussian truncated to an angle [-PI, PI]. Pick a1 and a2 for different distribution. If not picked carefully, you have very high probability of either -PI or PI
inline double ProjectMercury::GenerateRandomAngleNorm(double a1, double a2)
{
	double randGauss = GenerateRandomNorm(a1, a2);

	if (abs(randGauss) > PI)
		randGauss *= PI / abs(randGauss);

	return randGauss;
}

inline double ProjectMercury::NormAngleDeg(double ang)
{
	double deg = normangle(ang * RAD) * DEG;
	return deg;
}

inline void ProjectMercury::GetNoisyAngularVel(VECTOR3& avel, double stdDev)
{
	//VECTOR3 angVel;
	GetAngularVel(avel);

	if (oapiGetTimeAcceleration() > 1.0) stdDev /= oapiGetTimeAcceleration(); // it's not fun with noise when on time acc

	avel.x += GenerateRandomNorm() * stdDev;
	avel.y += GenerateRandomNorm() * stdDev;
	avel.z += GenerateRandomNorm() * stdDev;
}

inline bool ProjectMercury::InRadioContact(OBJHANDLE planet)
{
	if (FailureMode == RETROCALCOFF && oapiGetSimTime() - launchTime > timeOfError) // simulate loss of contact
	{
		sprintf(contactBase, "No radio contact");
		return false;
	}

	bool foundContact = false;
	for (int i = 0; i < int(oapiGetBaseCount(planet)); i++)
	{
		OBJHANDLE base = oapiGetBaseByIndex(planet, i);
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
			char cbuf[25];
			oapiGetObjectName(base, cbuf, 25);

			// Find out if a landing/contingency site:

			// Check if first character is a number
			int firstLetter = cbuf[0] - '0';
			bool aRealBase = true;
			if (firstLetter < 10 && firstLetter >= 0)
			{
				aRealBase = false;
			}

			if (aRealBase && (strncmp(cbuf, "MA", 2) == 0 || strncmp(cbuf, "MR", 2) == 0))
			{
				aRealBase = false;
			}

			VECTOR3 vertSpeed;
			GetGroundspeedVector(FRAME_HORIZON, vertSpeed);

			if (aRealBase)
			{
				sprintf(contactBase, "Radio contact with %s", cbuf);
				return true;
			}
			else if ((GetAltitude() < 35e3 && vertSpeed.y < 0.0) || GetAltitude() < 10.0) // going down, or bouncing on the surface
			{
				sprintf(contactBase, "Radio contact with %s", cbuf);
				return true;
			}
		}
	}
	sprintf(contactBase, "No radio contact");
	return false;
}

inline void ProjectMercury::DeleteRogueVessels(void)
{
	// Delete created vessels that bounce off into infinity (hopefully this will reduce crashes). Maybe use this for Redstone too, although we don't do time acc there
	for (int i = 0; i < stuffCreated; i++)
	{
		if (createdVessel[i] != NULL && oapiIsVessel(createdVessel[i]) && oapiGetFocusObject() != createdVessel[i]) // include condition to not delete if object is in focus (like if playing with staged parts)
		{
			VECTOR3 vesselPosition;
			GetRelativePos(createdVessel[i], vesselPosition);
			double vesselDistance = length(vesselPosition);
			if (vesselDistance > 2e5) // over 200 km away. Should be further than any realistic attempt at any rendezvous [, and also does not delete Atlas booster until after Atlas SECO --> no problem anymore, is handeled by core logic and config]
			{
				char vesselName[256];
				oapiGetObjectName(createdVessel[i], vesselName, 256);

				//if (oapiGetFocusObject() == createdVessel[i]) oapiSetFocusObject(GetHandle());

				bool deleteResult = oapiDeleteVessel(createdVessel[i]);
				if (deleteResult)
				{
					char cbuf[256];
					sprintf(cbuf, "Delete vessel %s due to distance %.2f km", vesselName, vesselDistance / 1000.0);
					oapiWriteLog(cbuf);
				}
				else
				{
					char cbuf[256];
					sprintf(cbuf, "Failed to delete vessel %s due to distance %.2f km", vesselName, vesselDistance / 1000.0);
					oapiWriteLog(cbuf);
				}

				createdVessel[i] = NULL;
				//createdAbove50km[i] = false;
			}
		}
	}
}

// MnA in radians
inline double ProjectMercury::MnA2TrA(double MnA, double Ecc)
{
	double TrA = MnA + (2.0 * Ecc - pow(Ecc, 3) / 4.0) * sin(MnA) + 5.0 / 4.0 * pow(Ecc, 2) * sin(2.0 * MnA) + 13.0 / 12.0 * pow(Ecc, 3) * sin(3.0 * MnA);

	return TrA;
}

inline double ProjectMercury::TrA2MnA(double TrA, double Ecc)
{
	//double MnA = TrA - 2.0 * Ecc * sin(TrA) + (3.0 / 4.0 * pow(Ecc, 2) + pow(Ecc, 4) / 8.0) * sin(2.0 * TrA) - pow(Ecc, 3) / 3.0 * sin(3.0 * TrA) + 5.0 / 32.0 * pow(Ecc, 4) * sin(4.0 * TrA);
	// Old Taylor series diverges for high Ecc and is in general shit, when we can use analytical result.

	double EccAnom = EccentricAnomaly(Ecc, TrA);
	//if (Ecc > 1.0) return Ecc * sinh(EccAnom) - EccAnom; // hyperbolic
	return EccAnom - Ecc * sin(EccAnom); // elliptic
}

// TrA in radians
inline double ProjectMercury::EccentricAnomaly(double ecc, double TrA)
{
	return 2.0 * atan(sqrt((1.0 - ecc) / (1.0 + ecc)) * tan(TrA / 2.0));
}

void ProjectMercury::myStrncpy(char* writeTo, const char* readFrom, int len)
{
	// Because strncpy sucks, I have to make my own >:(
	int i = 0;
	while (i < len && readFrom[i] != '\0')
	{
		writeTo[i] = readFrom[i];
		i++;
	}
	writeTo[i] = '\0';
}

// TrA and Eanomaly in radians
inline double ProjectMercury::TimeFromPerigee(double period, double ecc, double TrA)
{
	return period / PI2 * (EccentricAnomaly(ecc, TrA) - ecc * sin(EccentricAnomaly(ecc, TrA)));
}

void ProjectMercury::GetEquPosInTime(double t, double SMa, double Ecc, double Inc, double Per, double LPe, double LAN, double M, double longAtNow, double* longitude, double* latitude)
{
	// This method is partly from NTRS document 20160000809

	double planetRad = oapiGetSize(GetSurfaceRef());
	double planetMu = oapiGetMass(GetSurfaceRef()) * GGRAV;

	double M0 = M;
	// TrA in x seconds
	M = fmod(M + PI2 * t / Per, PI2);
	double TrA = MnA2TrA(M, Ecc);
	double TrA0 = MnA2TrA(M0, Ecc);

	double u = LPe - LAN + TrA;
	double u0 = LPe - LAN + TrA0;
	double alpha = atan2(cos(u) * sin(LAN) + sin(u) * cos(LAN) * cos(Inc), cos(u) * cos(LAN) - sin(u) * sin(LAN) * cos(Inc));
	double alpha0 = atan2(cos(u0) * sin(LAN) + sin(u0) * cos(LAN) * cos(Inc), cos(u0) * cos(LAN) - sin(u0) * sin(LAN) * cos(Inc));
	alpha -= alpha0;

	double longi = alpha + longAtNow - PI2 / oapiGetPlanetPeriod(GetSurfaceRef()) * t;
	longi = normangle(longi);

	double lati = asin(sin(u) * sin(Inc));

	*longitude = longi;
	*latitude = lati;
}
