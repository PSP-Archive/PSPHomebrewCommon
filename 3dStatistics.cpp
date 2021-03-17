/*
 * 3dStatistics.cpp
 *
 *
 * Copyright (C) 2010 André Borrmann
 *
 * This program is free software;
 * you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program;
 * if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "3dStatistics.h"
#include "ClTextHelper.h"
extern "C"{
#include <psptypes.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <psprtc.h>
#include <stdio.h>
}

#define vramBase (0x40000000 | 0x04000000)

Cl3dStatistics* Cl3dStatistics::_instance = 0;

Cl3dStatistics::~Cl3dStatistics() {
	// TODO Auto-generated destructor stub
}
Cl3dStatistics::Cl3dStatistics() {
	// TODO Auto-generated constructor stub
	sceRtcGetCurrentTick(&last_tick);
	tick_frequency = sceRtcGetTickResolution();
	frame_count = 0;
	curr_ms = 1.0f;
	pspDebugScreenInit();
}

void Cl3dStatistics::displayStatistic(void* screen){
	pspDebugScreenSetBase((u32*)((u32)screen | vramBase));
	//pspDebugScreenSetXY(0,0);
	displayFPS();
	char txt[100];
	sprintf(txt, "Triangles: %d", this->triangleCount);
	pspDebugScreenSetXY(0,1);
	pspDebugScreenPrintf(txt);

	displayDynamicStats();
	//clTextHelper::getInstance()->writeText(SANSERIF_SMALL_REGULAR, txt, 0, 22);
}
void Cl3dStatistics::init()
{
	this->triangleCount = 0;
	this->dynStats.clear();
}

void Cl3dStatistics::addTriangleCount(unsigned int triangles)
{
	this->triangleCount+=triangles;
}

void Cl3dStatistics::addDynamicStatistic(char* text){
	this->dynStats.push_back(text);
}

void Cl3dStatistics::displayFPS(){
	float curr_fps = 1.0f / curr_ms;

	char txt[100];
	//pspDebugScreenSetBase((u32*)screen);
	sprintf(txt, "FPS: %d.%03d",(int)curr_fps,(int)((curr_fps-(int)curr_fps) * 1000.0f));
	pspDebugScreenSetXY(0,0);
	pspDebugScreenPrintf(txt);
	//clTextHelper::getInstance()->writeText(SANSERIF_SMALL_REGULAR, txt, 0, 10);
	//pspDebugScreenPrintf();

	// simple frame rate counter
	++frame_count;
	u64 curr_tick;
	sceRtcGetCurrentTick(&curr_tick);
	if ((curr_tick-last_tick) >= tick_frequency)
	{
		float time_span = ((int)(curr_tick-last_tick)) / (float)tick_frequency;
		curr_ms = time_span / frame_count;

		frame_count = 0;
		sceRtcGetCurrentTick(&last_tick);
	}
}

void Cl3dStatistics::displayDynamicStats(){
	for (int i = 0; i< this->dynStats.size();i++){
		pspDebugScreenSetXY(0,i+2);
		pspDebugScreenPrintf(this->dynStats[i]);
	}
}
