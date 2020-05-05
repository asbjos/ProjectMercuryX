#pragma once

int FRAME_HORIZON = 0;

void ProjectMercury::VersionDependentTouchdown(VECTOR3 touch1, VECTOR3 touch2, VECTOR3 touch3, VECTOR3 touch4, double stiff, double damp, double mu)
{
	SetTouchdownPoints(touch1, touch2, touch3);
}

inline void ProjectMercury::VersionDependentPanelClick(int id, const RECT& pos, int texidx, int draw_event, int mouse_event, PANELHANDLE hPanel, const RECT& texpos, int bkmode)
{
	oapiRegisterPanelArea(id, pos, texidx, draw_event, mouse_event);
}

void ProjectMercury::VersionDependentPadHUD(oapi::Sketchpad* skp, double simt, int* yIndexUpdate, char* cbuf, VESSEL* v)
{
	// Empty, not supported
}

inline double ProjectMercury::normangle(double angle)
{
	double a = fmod(angle, PI2);
	return (a >= PI ? a - PI2 : a < -PI ? a + PI2 : a);
}

inline void ProjectMercury::oapiWriteLogV(const char* format, ...)
{
	//oapiWriteLog("oapiWriteLogV is not supported in Orbiter2010. Sorry.");
	char string[200];
	strcpy(string, "oapiWriteLogV not supported in Orbiter 2010: ");
	strcat(string, format);
	oapiWriteLog(string);
}

inline double ProjectMercury::GetGroundspeed(void)
{
	return GetAirspeed();
}

inline double ProjectMercury::GetAnimation(UINT anim)
{
	return -1.0;
}

inline void ProjectMercury::GetGroundspeedVector(int frame, VECTOR3& v)
{
	oapiGetAirspeedVector(GetHandle(), &v);
}

inline double ProjectMercury::length2(VECTOR3 vec)
{
	return length(vec) * length(vec);
}

inline void ProjectMercury::GetAirspeedVector(int frame, VECTOR3& v)
{
	oapiGetAirspeedVector(GetHandle(), &v);
}



