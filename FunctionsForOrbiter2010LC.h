#pragma once

int FRAME_HORIZON = 0;

void MercuryLC5::VersionDependentTouchdown(VECTOR3 touch1, VECTOR3 touch2, VECTOR3 touch3, VECTOR3 touch4, double stiff, double damp, double mu)
{
	SetTouchdownPoints(touch1, touch2, touch3);
}

inline void MercuryLC5::VersionDependentPanelClick(int id, const RECT& pos, int texidx, int draw_event, int mouse_event, PANELHANDLE hPanel, const RECT& texpos, int bkmode)
{
	oapiRegisterPanelArea(id, pos, texidx, draw_event, mouse_event);
}

inline double MercuryLC5::normangle(double angle)
{
	double a = fmod(angle, PI2);
	return (a >= PI ? a - PI2 : a < -PI ? a + PI2 : a);
}

inline void MercuryLC5::oapiWriteLogV(const char* format, ...)
{
	oapiWriteLog("oapiWriteLogV is not supported in Orbiter2010. Sorry.");
}

inline double MercuryLC5::GetGroundspeed(void)
{
	return GetAirspeed();
}

inline double MercuryLC5::GetAnimation(UINT anim)
{
	return -1.0;
}

inline void MercuryLC5::GetGroundspeedVector(int frame, VECTOR3& v)
{
	oapiGetAirspeedVector(GetHandle(), &v);
}

inline double MercuryLC5::length2(VECTOR3 vec)
{
	return length(vec) * length(vec);
}



