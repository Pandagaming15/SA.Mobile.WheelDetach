#include "library.h"
#include "mod/amlmod.h"
#include "mod/config.h"
#include "mod/logger.h"
#include "GTASA/common.h"

MYMODCFG(WheelDetach, GTASA, 1.0, PandaGaming)
NEEDGAME(com.rockstargames.gtasa)

using namespace plugin;

#include "WheelDetach.h"

float randomChanceToBurst;
float minIntensity;
float maxHealth;
float radius;
float minFallSpeed;
float suspCompressBurst;
float suspCompressDetach;

ConfigEntry *cBurst, *mInt, *mH, *rad, *mFall, *sCB, *sCD;

void CallDetach()
{
	CVehicle* vehicle = FindPlayerVehicle(-1, false);
	WheelDetach::ProcessFall(vehicle);
	WheelDetach::ProcessImpact(vehicle);
	
	mInt = cfg->Bind("MinIntensity", 300.0f, "Limits");
	minIntensity = mInt->GetFloat();
	
	cBurst =  cfg->Bind("RandomChanceToBurst", 10, "Limits");
    randomChanceToBurst = cBurst->GetInt();
	
	mH = cfg->Bind("MaxHealth", 700.0f, "Limits");
    maxHealth = mH->GetFloat();
	
	rad = cfg->Bind("Radius", 1.3f, "Limits");
    radius = rad->GetFloat();
	
	mFall = cfg->Bind("MinFallSpeed", -0.30f, "Limits");
    minFallSpeed = mFall->GetFloat();
	
	sCB = cfg->Bind("SuspCompressBurst", 0.1f, "Limits");
    suspCompressBurst = sCB->GetFloat();
	
	sCD = cfg->Bind("SuspCompressDetach", 0.01f, "Limits");
    suspCompressDetach = sCD->GetFloat();
}

DECL_HOOKv(vehicleRender, CVehicle* vehicle)
{
	CallDetach();
	vehicleRender(vehicle);
};

extern "C" void OnModLoad()
{
	HOOK(vehicleRender, libs.GetSym("_ZN8CVehicle6RenderEv"));
}
