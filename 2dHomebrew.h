/*
 * 2dHomebrew.h
 * This class defines a base class for simple 2D graphic homebrew applications.
 * There is no usage of any hardware acceleration but direct screen
 * manipulation....
 *
 * (c) André Borrmann 2010
 * Permission to copy, use, modify and distribute of this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to it's suitability for any purpose
 */

#ifndef CL2DHOMEBREW_H_
#define CL2DHOMEBREW_H_

#define PSP_LINE_SIZE 512
#define SCREEN_HEIGHT 272
#define FRAMESIZE PSP_LINE_SIZE*SCREEN_HEIGHT

#define RGB(r, g, b) ((r & 255) | ((g & 255) << 8) | ((b & 255) << 16))

extern "C"{
#include <pspdebug.h>
#include <pspdisplay.h>
#include <psptypes.h>
}

#include "Homebrew.h"

class Cl2dHomebrew : public ClHomebrew
{
public:
	static Cl2dHomebrew* getInstance();
	virtual ~Cl2dHomebrew();
	virtual bool init();
	virtual void exit();

protected:
	Cl2dHomebrew();
	/*
	 * put a pixel on the screen with the given color
	 */
	void setPixel(int x, int y, int color);

	u32 *pgGetVramAddr();

	virtual void mainthread();
	virtual void render(u32* vram);

private:
	static Cl2dHomebrew* _instance;
	static u32* pg_vRamTop;
	bool pg_drawframe;

};

#endif /* 2DHOMEBREW_H_ */
