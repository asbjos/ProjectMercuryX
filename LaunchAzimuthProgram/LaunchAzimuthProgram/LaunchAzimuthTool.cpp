// LaunchAzimuthTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// Created by Asbjørn ("asbjos" on Orbiter-Forum).
// This program is included in the Project Mercury X package
//
// It is used to calculate the target heading and inclination for an Earth launch where you know where you're launching from,
// and where you want to land, plus how many orbits you want to make before passing over the landing coordinates.
//
// Source code for this file is open source, and based on the NTRS 19980227091 document

#include <iostream>

// TrA in radians
inline double EccentricAnomaly(double ecc, double TrA)
{
	return 2.0 * atan(sqrt((1.0 - ecc) / (1.0 + ecc)) * tan(TrA / 2.0));
}

// TrA and Eanomaly in radians
inline double TimeFromPerigee(double period, double ecc, double TrA)
{
	double PI = 3.1415926535897932384626;
	double PI2 = PI * 2.0;
	return period / PI2 * (EccentricAnomaly(ecc, TrA) - ecc * sin(EccentricAnomaly(ecc, TrA)));
}

void TargetAzimuth(double cutoffAlt, double launchLong, double launchLat, double targLong, double targLat, int numOrbits, double *azimuth, double *inclination)
{
	// We target an orbit with perigee at current alt and target apogee set by user
	double planetRad = 6371e3;
	double planetMu = 3.986004418e14;
	double earthAngularVel = 360.0 / 86400.0;
	double PI = 3.1415926535897932384626;
	double PI2 = PI * 2.0;
	double RAD = PI / 180.0;
	double DEG = 180.0 / PI;

	double targetElevationAngle = 0.0;
	double rCutoff = cutoffAlt + planetRad;
	double vc = sqrt(planetMu / rCutoff);
	double cutoffVel = sqrt(planetMu * (2.0 / rCutoff - 2.0 / (270e3 + planetRad + rCutoff)));

	double targetSMa = 1.0 / (2.0 / rCutoff - cutoffVel * cutoffVel / planetMu);

	double pOverr1 = pow(cutoffVel / vc * cos(targetElevationAngle * RAD), 2.0);
	double TrACutoff = atan(tan(targetElevationAngle * RAD) * pOverr1 / (pOverr1 - 1.0));
	double targetEcc = (pOverr1 - 1) / cos(TrACutoff);
	double targetPeriod = PI2 * sqrt(pow(planetRad, 3.0) / planetMu) * pow(targetSMa / planetRad, 1.5);
	double targetSemilatusRectum = pOverr1 * rCutoff;

	double targetLongEquivalent = targLong + numOrbits * earthAngularVel * targetPeriod;

	// iterative values
	double dOmegaSmall = 0.0;
	double dPhi2 = 0.0;
	double dOmegaLarge = 0.0;
	double dLambda2 = 0.0;
	double tOfTheta2eMinusTTheta1 = targetPeriod / 360.0 * (targetLongEquivalent - launchLong);
	double dLambda1minus2e = 0.0;
	double cosTrA2eMinusTrAI = 0.0;
	double tTrA2e = 0.0;
	double TrA2e = 0.0;
	double sinAzimuth1 = 0.0;
	double cosInclination = 0.0;

	int iterationNumber = 1;

	for (int i = 1; i <= 6; i++)
	{
		targetLongEquivalent = targLong - dLambda2 + numOrbits * earthAngularVel * targetPeriod;


		if (i > 1)
		{
			tOfTheta2eMinusTTheta1 = TimeFromPerigee(targetPeriod, targetEcc, TrA2e * RAD) - TimeFromPerigee(targetPeriod, targetEcc, TrACutoff);
			dLambda1minus2e = targetLongEquivalent - launchLong + earthAngularVel * tOfTheta2eMinusTTheta1;
		}
		else
		{
			dLambda1minus2e = targLong + numOrbits * earthAngularVel * targetPeriod + earthAngularVel * tOfTheta2eMinusTTheta1 - launchLong;
		}

		cosTrA2eMinusTrAI = sin(targLat * RAD - dPhi2 * RAD) * sin(launchLat * RAD) + cos(targLat * RAD - dPhi2 * RAD) * cos(launchLat * RAD) * cos(dLambda1minus2e * RAD);

		TrA2e = fmod(acos(cosTrA2eMinusTrAI) * DEG + TrACutoff * DEG, 360.0);
		tTrA2e = TimeFromPerigee(targetPeriod, targetEcc, TrA2e * RAD);

		sinAzimuth1 = sin(dLambda1minus2e * RAD) * cos(targLat * RAD - dPhi2 * RAD) / sin(TrA2e * RAD - TrACutoff);

		cosInclination = sinAzimuth1 * cos(launchLat * RAD);

		if (i == iterationNumber)
		{
			dOmegaSmall = 3.4722e-3 / 60.0 * pow(planetRad / targetSemilatusRectum, 2.0) * pow(planetRad / targetSMa, 1.5) * (5.0 * cosInclination * cosInclination - 1.0) * (numOrbits * targetPeriod + TimeFromPerigee(targetPeriod, targetEcc, TrA2e * RAD) - 0.0);

			double omegaPlusTrA2e = asin(sin(launchLat * RAD) / sin(acos(cosInclination))) - TrACutoff + TrA2e * RAD;
			dPhi2 = sin(acos(cosInclination)) * cos(omegaPlusTrA2e) / cos(targLat * RAD) * dOmegaSmall;

			dOmegaLarge = -6.9444e-3 / 60.0 * pow(planetRad / targetSemilatusRectum, 2.0) * pow(planetRad / targetSMa, 1.5) * cosInclination * (numOrbits * targetPeriod + TimeFromPerigee(targetPeriod, targetEcc, TrA2e * RAD) - 0.0);

			dLambda2 = cosInclination / pow(cos(omegaPlusTrA2e), 2.0) / (1.0 + cosInclination * cosInclination * pow(tan(omegaPlusTrA2e), 2.0)) * dOmegaSmall + dOmegaLarge;
		}
	}

	*azimuth = asin(sinAzimuth1) * DEG;
	*inclination = acos(cosInclination) * DEG;

	// Including Earth's rotation
	double targetHeading = *azimuth;
	*azimuth = atan((vc * sin(targetHeading * RAD) - PI2 * planetRad / 86400.0 * cos(launchLat * RAD)) / (vc * cos(targetHeading * RAD))) * DEG;
}

int main()
{
	std::cout << "Launch Azimuth Tool\n";
	std::cout << "Created by Asbjoern ('asbjos'), 2020.\n";
	std::cout << "Included in Project Mercury X for Orbiter Space Flight Simulator.\n";
	std::cout << "-------------------\n";
	std::cout << "Input latitude (north/south) and longitude (east/west) as decimal values in degrees.\nFor south and west, use negative numbers.\n";
	std::cout << "-------------------\n";

	double cutoffAlt = 160.9e3;
	double launchLong = -80.5;
	double launchLat = 28.5;
	double targLong = -68.3;
	double targLat = 21.3;
	int numOrbits = 3;

	int capeLaunch;
	std::cout << "Launching from Cape Canaveral? (1 = yes / 0 = no): ";
	std::cin >> capeLaunch;
	if (capeLaunch == 1)
	{
		launchLong = -80.5;
		launchLat = 28.5;
	}
	else
	{
		std::cout << "Launching latitude (Cape Canaveral is 28.5): ";
		std::cin >> launchLat;

		std::cout << "Launching longitude (Cape Canaveral is -80.5): ";
		std::cin >> launchLong;
	}

	std::cout << "-------------------\n";

	std::cout << "Landing latitude: ";
	std::cin >> targLat;

	std::cout << "Landing longitude: ";
	std::cin >> targLong;

	std::cout << "Number of orbits for mission: ";
	std::cin >> numOrbits;

	while (numOrbits != 0)
	{

		targLong = fmod(targLong, 360.0);
		launchLong = fmod(launchLong, 360.0);

		double azimuth;
		double inclination;
		TargetAzimuth(160.9e3, launchLong, launchLat, targLong, targLat, numOrbits, &azimuth, &inclination);

		std::cout << "> Target azimuth: " << azimuth << " deg.\n";
		std::cout << "> Target inclination: " << inclination << " deg.\n";

		std::cout << "-------------------\n";

		std::cout << "Numer of orbits for mission (0 to exit): ";
		std::cin >> numOrbits;
	}
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
