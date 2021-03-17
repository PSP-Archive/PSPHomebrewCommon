/*
 * SimpleTimer.cpp
 *
 *  Created on: Dec 30, 2009
 *      Author: andreborrmann
 */

#include "ClSimpleTimer.h"

ClSimpleTimer::ClSimpleTimer() {
	// TODO Auto-generated constructor stub
	sceRtcGetCurrentTick(&lastTick);
	firstTick = lastTick;
	tickFrequency = 1.0f / sceRtcGetTickResolution();
	reseted = false;
}

ClSimpleTimer::~ClSimpleTimer() {
	// TODO Auto-generated destructor stub
}

/*
 * get the seconds since the last call of reset()
 */
float ClSimpleTimer::getDeltaSeconds(){
	u64 currTick;
	sceRtcGetCurrentTick(&currTick);

	return ((currTick - lastTick)*tickFrequency);
}

float ClSimpleTimer::getTime(){
	u64 currTick;
	sceRtcGetCurrentTick(&currTick);

	return ((currTick - firstTick)*tickFrequency);
}

/*
 * reset the timer
 */
void ClSimpleTimer::reset(){
	sceRtcGetCurrentTick(&lastTick);
	reseted = true;
}

/*
 *
 */
bool ClSimpleTimer::isReset(){
	return reseted;
}


