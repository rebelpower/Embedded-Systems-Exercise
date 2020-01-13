#ifndef _DELAYABSTRACTION_H
#define _DELAYABSTRACTION_H

#include <stdint.h>

void delayAbstraction_delayMilliseconds(uint32_t waitingPeriodInMilliseconds);
void delayAbstraction_delayMicroseconds(uint32_t waitingPeriodInMicroseconds);
#endif // _DELAYABSTRACTION_H
