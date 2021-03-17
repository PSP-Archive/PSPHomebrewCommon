/*
 * 2dHomebrew.cpp
 *
 *  Created on: 10.10.2009
 *      Author: MamaPapa
 */

#include "2dHomebrew.h"

extern "C" {
#include <string.h>
}

Cl2dHomebrew* Cl2dHomebrew::_instance = 0;
//set static VRAM Top for graphics -uncached area
u32* Cl2dHomebrew::pg_vRamTop = (u32 *)(0x40000000 | 0x04000000);
void Cl2dHomebrew::exit()
{
}

/**
 * Init 2D Homebrew
 * Setting of the FrameBuffer
 */
bool Cl2dHomebrew::init()
{
	if (!ClHomebrew::init()) return false;
	//first draw to the backbuffer
	pg_drawframe = true;
	return true;
}

Cl2dHomebrew *Cl2dHomebrew::getInstance()
{
	if (!_instance) {
		_instance = new Cl2dHomebrew();
	}

	return _instance;
}

Cl2dHomebrew::Cl2dHomebrew() {
	// TODO Auto-generated constructor stub

}

Cl2dHomebrew::~Cl2dHomebrew() {
	// TODO Auto-generated destructor stub
}

u32 *Cl2dHomebrew::pgGetVramAddr()
{
	if (pg_drawframe)
		return pg_vRamTop + FRAMESIZE;
	else
		return pg_vRamTop;
}

void Cl2dHomebrew::setPixel(int x, int y, int color){
	//Farbe setzen
	u32* point = pgGetVramAddr()+x+y*PSP_LINE_SIZE;
	*point = color;
}

/**
 * virtual. place holder for application rendering
 */
void Cl2dHomebrew::render(u32 *vram){
}


/**
 * main tread which will be executed in a loop while the homebrew is running
 */
void Cl2dHomebrew::mainthread()
{
	u32* vram;
	vram = pgGetVramAddr();
	//clear the whole screen
	memset(vram, 0, FRAMESIZE*sizeof(u32));
	//do the rendering
	render(vram);
	//display the buffer
	sceDisplaySetFrameBuf(vram, PSP_LINE_SIZE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE );
	//switch the buffer to draw to
	pg_drawframe = pg_drawframe?false:true;
	//wait for
	sceDisplayWaitVblankStart();
}


