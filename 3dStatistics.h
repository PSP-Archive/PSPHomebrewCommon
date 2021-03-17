/*
 * 3dStatistics.h
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

#ifndef CL3DSTATISTICS_H_
#define CL3DSTATISTICS_H_

#include <psptypes.h>
#include <vector>

class Cl3dStatistics
{
protected:
	static Cl3dStatistics* _instance;
public:
	static Cl3dStatistics* getInstance(){
		if (!_instance)
			_instance = new Cl3dStatistics();
		return _instance;
	}

	void init();
	void addTriangleCount(unsigned int triangles);
	void addDynamicStatistic(char* text);
	void displayStatistic(void* screen);

	virtual ~Cl3dStatistics();
protected:
	Cl3dStatistics();

	float curr_ms;
	u64 last_tick;
	u32 tick_frequency;
	int frame_count;
	unsigned int triangleCount;

	std::vector<char*> dynStats;
	void displayFPS();
	void displayDynamicStats();
};

#endif /* 3DSTATISTICS_H_ */
