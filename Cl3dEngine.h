/*
 * Cl3dEngine.h
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

#ifndef CL3DENGINE_H_
#define CL3DENGINE_H_

extern "C"{
#include <pspdisplay.h>
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psptypes3d.h>
}

#include <ClTextureMgr.h>

typedef unsigned int PspColor;
typedef unsigned int PspU32;

class Cl3dEngine {
public:
	Cl3dEngine();
	virtual ~Cl3dEngine();

	virtual void enableStatistics();

	virtual bool beginScene(bool clearScreen, bool clearZBuffer, PspColor backColor);
	virtual bool endScene(bool waitVBlank);

	virtual void drawRectangle(short x, short y, short w, short h, PspColor color);

	virtual void drawImage(Texture* image, short alpha);
	virtual void drawImage(Texture* image, short x, short y, short width, short height, short alpha);

	virtual void setModelMatrix(ScePspFMatrix4* modelMatrix);
	virtual void setTexture(Texture* texture);
	virtual void drawMesh(int primitive, void* vertices, int vertexCount, int vertexType);
	virtual void drawMesh(int primitive, void* vertices, void* index, int vertexCount, int vertexType);

protected:
	bool statistics;
	void* screen;
};

#endif /* CL3DENGINE_H_ */
