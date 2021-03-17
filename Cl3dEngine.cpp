/*
 * Cl3dEngine.cpp
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

#include "Cl3dEngine.h"
#include "3dStatistics.h"

//display list used by the PSP GU to control the rendering
static unsigned int __attribute__((aligned(16))) list[462144];


Cl3dEngine::Cl3dEngine() {
	// TODO Auto-generated constructor stub
	screen = 0;
}

Cl3dEngine::~Cl3dEngine() {
	// TODO Auto-generated destructor stub
}

void Cl3dEngine::enableStatistics(){
	this->statistics = true;
}

bool Cl3dEngine::beginScene(bool clearScreen, bool clearZBuffer, PspColor backColor)
{
	int clearFlags = 0;
	if (this->statistics)
		Cl3dStatistics::getInstance()->init();

	sceKernelDcacheWritebackAll();
	sceGuStart(GU_DIRECT,list);
	if (clearScreen)
		clearFlags |= GU_COLOR_BUFFER_BIT;

	if (clearZBuffer)
		clearFlags |= GU_DEPTH_BUFFER_BIT;

	sceGuClearColor(backColor);
	sceGuClearDepth(0);
	sceGuClear(clearFlags);

	return true;
}

bool Cl3dEngine::endScene(bool waitVBlank)
{
	//right before finish, draw statistics if requested
	sceGuFinish();
	sceGuSync(0,0);
	if (this->statistics)
		Cl3dStatistics::getInstance()->displayStatistic(screen);

	screen = sceGuSwapBuffers();
	if (waitVBlank)
		sceDisplayWaitVblankStart();

	return true;
}

void Cl3dEngine::drawRectangle(short  x, short  y, short  w, short  h, PspColor color)
{
	ColorIVertex* vertex;
	vertex = (ColorIVertex*)sceGuGetMemory(sizeof(ColorIVertex)*2);
	vertex[0].x = x;
	vertex[0].y = y;
	vertex[0].z = 0xffff;
	vertex[0].color = color;

	vertex[1].x = x+w;
	vertex[1].y = y+h;
	vertex[1].z = 0xffff;
	vertex[1].color = color;

	int currentStates = sceGuGetAllStatus();

	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDrawArray(GU_SPRITES, GU_TRANSFORM_2D | GU_COLOR_8888 | GU_VERTEX_16BIT, 2, 0, vertex);

	sceGuSetAllStatus(currentStates);
}

void Cl3dEngine::drawImage(Texture *image, short  alpha)
{
	drawImage(image, 0, 0, image->width, image->height, alpha);
}

void Cl3dEngine::drawImage(Texture *image, short  x, short  y, short  width, short  height, short  alpha)
{
	TexFColIVertex* vertex;
	int currentStates = sceGuGetAllStatus();
	ClTextureMgr::getInstance()->useTexture(image);
	//set the Texture
	//sceGuTexScale(1.0f/image->width, 1.0f/image->height);
	float xs = width/image->width;
	sceGuTexScale(xs/image->width, 1.0f/image->height);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuDisable(GU_DEPTH_TEST);
	//do not render complete but in 64Pixel width blocks
	int w, slice, u;
	slice = 64;
	w = 0;
	while (w < width){

		vertex = (TexFColIVertex*)sceGuGetMemory(2*sizeof(TexFColIVertex));

		if (w+slice > width) slice = width - w;
		u = w*image->width/width;
		vertex[0].x = x+w;
		vertex[0].y = y;
		vertex[0].z = 0x00ff;
		vertex[0].u = u;
		vertex[0].v = 0;
		vertex[0].color = 0x00ffffff | ((alpha & 255) << 24);

		u = (w+slice)*image->width/width;
		vertex[1].x = x+w+slice;
		vertex[1].y = y+height;
		vertex[1].z = 0x00ff;
		vertex[1].u = u;
		vertex[1].v = image->height;
		vertex[1].color = 0x00ffffff | ((alpha & 255) << 24);

		//draw this part
		sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_TEXTURE_32BITF | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertex);

		w+=slice;
	}

	sceGuSetAllStatus(currentStates);
}

void Cl3dEngine::setTexture(Texture *texture)
{
	ClTextureMgr::getInstance()->useTexture(texture);
}



void Cl3dEngine::drawMesh(int primitive, void *vertices, int vertexCount, int vertexType)
{
	unsigned int triangles;
	sceGumDrawArray(primitive, vertexType, vertexCount, 0, vertices);
	switch (primitive){
	case GU_TRIANGLE_STRIP:
		triangles = (vertexCount - 2 );
		break;
	default:
		triangles = vertexCount / 3;
		break;
	}
	Cl3dStatistics::getInstance()->addTriangleCount(triangles);
}

void Cl3dEngine::drawMesh(int primitive, void *vertices, void* index, int vertexCount, int vertexType)
{
	unsigned int triangles;
	sceGumDrawArray(primitive, vertexType | GU_INDEX_16BIT, vertexCount, index, vertices);
	switch (primitive){
	case GU_TRIANGLE_STRIP:
		triangles = (vertexCount - 2 );
		break;
	default:
		triangles = vertexCount / 3;
		break;
	}
	Cl3dStatistics::getInstance()->addTriangleCount(triangles);
}

void Cl3dEngine::setModelMatrix(ScePspFMatrix4 *modelMatrix)
{
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadMatrix(modelMatrix);
	sceGumUpdateMatrix();
}









