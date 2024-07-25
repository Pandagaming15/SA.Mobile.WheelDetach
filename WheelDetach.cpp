#include "WheelDetach.h"

void HideAllAtomics(RwFrame *frame)
{
	if (!rwLinkListEmpty(&frame->objectList))
	{
		RwObjectHasFrame * atomic;

		RwLLLink * current = rwLinkListGetFirstLLLink(&frame->objectList);
		RwLLLink * end = rwLinkListGetTerminator(&frame->objectList);

		current = rwLinkListGetFirstLLLink(&frame->objectList);
		while (current != end) {
			atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);
			atomic->object.flags &= ~0x4; // clear

			current = rwLLLinkGetNext(current);
		}
	}
	return;
}

RwFrame * HideAllNodesRecursive(RwFrame *frame)
{
	HideAllAtomics(frame);

	if (RwFrame * nextFrame = frame->child) HideAllNodesRecursive(nextFrame);
	if (RwFrame * nextFrame = frame->next)  HideAllNodesRecursive(nextFrame);
	return frame;
}

bool IsNotLastWheel(CAutomobile *automobile, int wheelId)
{
	for (int i = 0; i < 4; ++i)
	{
		if (i != wheelId && automobile->m_damageManager.m_anWheelsStatus[i] != 2)
		{
			return true;
		}
	}
	return false;
}

void DetachThisWheel(CVehicle *vehicle, int wheelNode, int wheelId, bool isRear)
{
	CAutomobile *automobile = reinterpret_cast<CAutomobile*>(vehicle);
	if (automobile->IsComponentPresent(wheelNode) && IsNotLastWheel(automobile, wheelId))
	{
		if (wheelId != -1)
		{
			if (!isRear || !(vehicle->m_pHandlingData->m_nModelFlags & VEHICLE_HANDLING_MODEL_DOUBLE_RWHEELS))
			{
			    automobile->m_damageManager.SetWheelStatus(wheelId, WHEEL_STATUS_MISSING);
			}
		}
		automobile->SpawnFlyingComponent(wheelNode, 1);
		HideAllAtomics(automobile->m_aCarNodes[wheelNode]);
	}
}

#include <unordered_map>
std::unordered_map<CVehicle*, unsigned int> lastTimeFall;
std::unordered_map<CVehicle*, bool> isFall;

void WheelDetach::ProcessFall(CVehicle* vehicle)
{
	if (vehicle && (vehicle->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || vehicle->m_nVehicleSubClass == VEHICLE_QUAD) &&
				vehicle->m_nModelIndex != MODEL_RHINO)
			{
				CAutomobile *automobile = reinterpret_cast<CAutomobile*>(vehicle);
				int numContactWheels = automobile->GetNumContactWheels();
				if (numContactWheels == 0 && vehicle->m_vecMoveSpeed.z < minFallSpeed)
				{
					lastTimeFall[vehicle] = CTimer::m_snTimeInMilliseconds;
					isFall[vehicle] = true;
				}			
				else
				{
					bool &isFalling = isFall[vehicle];
                    unsigned int &lastTimeFalling = lastTimeFall[vehicle];
					if (isFalling)
					{
						for (int wheelId = 0; wheelId < 4; ++wheelId)
						{
							float suspAmount = reinterpret_cast<CAutomobile*>(vehicle)->wheelsDistancesToGround2[wheelId];
							if (suspAmount < suspCompressBurst)
							{
								if (suspAmount < suspCompressDetach)
								{
									int wheelNode;
									bool isRear = false;
									switch (wheelId)
									{
									case 0:
										wheelNode = eCarNodes::CAR_WHEEL_LF;
										break;
									case 1:
										wheelNode = eCarNodes::CAR_WHEEL_LB;
										isRear = true;
										break;
									case 2:
										wheelNode = eCarNodes::CAR_WHEEL_RF;
										break;
									case 3:
										wheelNode = eCarNodes::CAR_WHEEL_RB;
										isRear = true;
										break;
									default:
										break;
									}
									DetachThisWheel(vehicle, wheelNode, wheelId, isRear);
								}
								else
								{
									automobile->BurstTyre(wheelId, true);
								}
							}
						}
					}
					// Reset falling if touched the ground after time limit
					if (isFalling && (CTimer::m_snTimeInMilliseconds - lastTimeFalling) > 50)
					{
						isFall[vehicle] = false;
						lastTimeFall[vehicle] = 0;
					}
				}
			}
}

void WheelDetach::ProcessImpact(CVehicle* vehicle)
{
	if (vehicle && vehicle->m_fHealth < maxHealth && vehicle->m_fDamageIntensity > 1.0f &&
				(vehicle->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || vehicle->m_nVehicleSubClass == VEHICLE_QUAD) && vehicle->m_nModelIndex != MODEL_RHINO)
			{
				float minIntensityNow = minIntensity * (vehicle->m_fHealth / maxHealth) / vehicle->m_pHandlingData->m_fCollisionDamageMultiplier;
				if (vehicle->m_fDamageIntensity > minIntensityNow)
				{
					CAutomobile *automobile = reinterpret_cast<CAutomobile*>(vehicle);
					CVector collisionPos = vehicle->m_vecLastCollisionPosn;

					int wheelNode = 2;
					
					while (true)
					{
						if (automobile->m_aCarNodes[wheelNode] && DistanceBetweenPoints(automobile->m_aCarNodes[wheelNode]->ltm.pos, collisionPos) < radius) break;
						wheelNode++;
						if (wheelNode > 7) return;
					}

					int wheelId = -1;
					bool isRear = false;
					switch (wheelNode)
					{
					case eCarNodes::CAR_WHEEL_LF: //5
						wheelId = 0;
						break;
					case eCarNodes::CAR_WHEEL_LB: //7
						wheelId = 1;
						isRear = true;
						break;
					case eCarNodes::CAR_WHEEL_RF: //2
						wheelId = 2;
						break;
					case eCarNodes::CAR_WHEEL_RB: //4
						wheelId = 3;
						isRear = true;
						break;
					default:
						break;
					}

					if (automobile->IsComponentPresent(wheelNode) && IsNotLastWheel(automobile, wheelId))
					{
						if (wheelId != -1)
						{
							if (!isRear || !(vehicle->m_pHandlingData->m_nModelFlags & VEHICLE_HANDLING_MODEL_DOUBLE_RWHEELS))
							{
								automobile->m_damageManager.SetWheelStatus(wheelId, WHEEL_STATUS_MISSING);
							}
						}
						automobile->SpawnFlyingComponent(wheelNode, 1);
						HideAllAtomics(automobile->m_aCarNodes[wheelNode]);
					}
				}
			}
}
