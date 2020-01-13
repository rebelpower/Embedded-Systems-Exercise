#include "unity.h"
#include "delayAbstraction.h"

void setUp(void)
{
}

void tearDown(void)
{
}

extern uint32_t fakeDelayMillisecondsCalled; 
extern uint32_t fakeDelayMicrosecondsCalled;

void test_platformFunctions_haveVariableDelayedMsFunction(void){
	fakeDelayMillisecondsCalled = 0;
	uint32_t waitingPeriodInMs = 20;
	delayAbstraction_delayMilliseconds(waitingPeriodInMs);
	TEST_ASSERT_EQUAL_UINT32(waitingPeriodInMs, fakeDelayMillisecondsCalled);
}

void test_platformFunctions_haveVariableDelayedMicrosecondsFunction(void){
	fakeDelayMicrosecondsCalled = 0;
	uint32_t waitingPeriodInMicroseconds = 20;
	delayAbstraction_delayMicroseconds(waitingPeriodInMicroseconds);
	TEST_ASSERT_EQUAL_UINT32(waitingPeriodInMicroseconds, fakeDelayMicrosecondsCalled);
}
