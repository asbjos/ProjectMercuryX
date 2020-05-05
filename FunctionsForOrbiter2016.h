#pragma once

void ProjectMercury::VersionDependentTouchdown(VECTOR3 touch1, VECTOR3 touch2, VECTOR3 touch3, VECTOR3 touch4, double stiff, double damp, double mu)
{
	mu = 3.0; // it's best like this
	TOUCHDOWNVTX touchdownPoints[4] = {
		// pos, stiff, damping, mu, mu long
		{touch1, stiff, damp, mu},
		{touch2, stiff, damp, mu},
		{touch3, stiff, damp, mu},
		{touch4, stiff, damp, mu},
	};
	SetTouchdownPoints(touchdownPoints, 4);
}

inline void ProjectMercury::VersionDependentPanelClick(int id, const RECT& pos, int texidx, int draw_event, int mouse_event, PANELHANDLE hPanel, const RECT& texpos, int bkmode)
{
	RegisterPanelArea(hPanel, id, pos, texidx, texpos, draw_event, mouse_event, bkmode);
}

void ProjectMercury::VersionDependentPadHUD(oapi::Sketchpad* skp, double simt, int* yIndexUpdate, char* cbuf, VESSEL *v)
{
	sprintf(cbuf, "  Height: %.2f km", v->GetAltitude(ALTMODE_GROUND) / 1000.0);
	skp->Text(TextX0 * secondColumnHUDx, (*yIndexUpdate) * LineSpacing + TextY0, cbuf, strlen(cbuf));
	*yIndexUpdate += 1;

	double longR, latR, radR, longP, latP, radP;
	v->GetEquPos(longR, latR, radR);
	OBJHANDLE plt = GetEquPos(longP, latP, radP);
	double angleDownrange = oapiOrthodome(longR, latR, longP, latP);
	sprintf(cbuf, "  Downrange: %.2f km", angleDownrange * oapiGetSize(plt) / 1000.0);
	skp->Text(TextX0 * secondColumnHUDx, (*yIndexUpdate) * LineSpacing + TextY0, cbuf, strlen(cbuf));
	*yIndexUpdate += 1;

	sprintf(cbuf, "  Speed: %.1f m/s", v->GetGroundspeed());
	skp->Text(TextX0 * secondColumnHUDx, (*yIndexUpdate) * LineSpacing + TextY0, cbuf, strlen(cbuf));
	*yIndexUpdate += 1;
}