/*
 * ClPspPad.cpp
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

#include "ClPspPad.h"
#include "ClSimpleTimer.h"

ClSimpleTimer buttonTimer;

SceCtrlData ClPspPad::lastPad;

unsigned int ClPspPad::getButton(float bt){
	SceCtrlData pad;
	unsigned int buttons = 0;
	sceCtrlPeekBufferPositive(&pad, 1);
	if (!buttonTimer.isReset())
		buttonTimer.reset();
	//if there was the same key pressed do not return this
	//unless a certain timeperiod has gone
	if (pad.Buttons != lastPad.Buttons
		|| buttonTimer.getDeltaSeconds() > bt){
		buttons = pad.Buttons;
		buttonTimer.reset();
	}

	lastPad = pad;
	return buttons;
}

SceCtrlData ClPspPad::getPad(){
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(&pad, 1);

	lastPad = pad;
	return lastPad;
}

