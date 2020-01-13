#ifndef _PLATFORM_CONFIG_H
#define _PLATFORM_CONFIG_H

#ifdef TEST
#include <stdint.h>
#include <stdio.h>
uint32_t fakeDelayMillisecondsCalled = 0;
uint32_t fakeDelayMicrosecondsCalled = 0;

void fakeDelayForOneMillisecond(){
	fakeDelayMillisecondsCalled++;
}

#define delayForOneMillisecond() fakeDelayForOneMillisecond()

void fakeDelayForOneMicrosecond(){
	fakeDelayMicrosecondsCalled++;
}
#define delayForOneMicrosecond() fakeDelayForOneMicrosecond()

#else // TEST

#include <util/delay.h>
#define delayForOneMillisecond() _delay_ms(1);
#define delayForOneMicrosecond() _delay_us(1);

#endif //TEST

#endif //_PLATFORM_CONFIG_H