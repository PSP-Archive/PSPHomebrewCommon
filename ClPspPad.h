/*
 * ClPspPad.h
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

#ifndef CLPSPPAD_H_
#define CLPSPPAD_H_

extern "C"{
#include <pspctrl.h>
}

class ClPspPad {
	static SceCtrlData lastPad;
public:
	static unsigned int getButton(float bt = 5.0f);
	static SceCtrlData getPad();
};

#endif /* CLPSPPAD_H_ */
