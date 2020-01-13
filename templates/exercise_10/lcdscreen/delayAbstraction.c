#include "delayAbstraction.h"
#include "platformConfig.h"

void delayAbstraction_delayMilliseconds(uint32_t waitingPeriodInMilliseconds){
	for (uint32_t i = 0; i < waitingPeriodInMilliseconds; ++i){
		delayForOneMillisecond();
	}
}

void delayAbstraction_delayMicroseconds(uint32_t waitingPeriodInMicroseconds){
	for (uint32_t i = 0; i < waitingPeriodInMicroseconds; ++i){
		delayForOneMicrosecond();
	}
}