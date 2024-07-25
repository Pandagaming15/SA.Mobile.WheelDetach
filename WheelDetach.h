#pragma once

#include "library.h"
#include "funcs/rain.h"
#include "GTASA/CDamageManager.h"
#include "GTASA/CTimer.h"
#include "GTASA/CVehicle.h"
#include "GTASA/CAutoMobile.h"

class WheelDetach
{
	public:
	static void ProcessFall(CVehicle* vehicle);
	static void ProcessImpact(CVehicle* vehicle);
};

extern float randomChanceToBurst;
extern float minIntensity;
extern float maxHealth;
extern float radius;
extern float minFallSpeed;
extern float suspCompressBurst;
extern float suspCompressDetach;
