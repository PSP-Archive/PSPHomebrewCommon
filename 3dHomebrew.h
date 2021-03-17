/*
 * 3dHomwbrew.h
 *
 * This class provides core functionality for any PSP homebrew that need GU
 * support. To create your own 3D homebrews using the GU you
 * need to your own class deriving from this one and implement additional
 * methods or re-implement the derived methods to adopt them to your needs
 *
 * (c) André Borrmann 2010
 * Permission to copy, use, modify and distribute of this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to it's suitability for any purpose
 */

#ifndef CL3DHOMEBREW_H_
#define CL3DHOMEBREW_H_

#include "Homebrew.h"
#include "Cl3dEngine.h"

extern "C"{
#include <psputility.h>
}

#define BUF_WIDTH  512
#define SCR_WIDTH  480
#define SCR_HEIGHT 272

class Cl3dHomebrew : public ClHomebrew
{
public:
	/*
	 * Initialize the GU Homebrew.
	 * The following default setup is used:
	 * GuDrawBuffer -> GU_PSM_8888
	 * GuDispBuffer -> SCR_WIDTH,SCR_HEIGHT
	 * GuViewport   -> 2048,2048,SCR_WIDTH,SCR_HEIGHT
	 * GuOffset     -> 2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2)
	 * GuDepthRange -> 65535,0
	 * GuDepthFunc  -> GU_GEQUAL
	 * GuFrontFace  -> GU_CW
	 * GuAmbient    -> 0xFFFFFFFF
	 *
	 * Active switches:
	 * 		- GU_SCISSOR_TEST
	 * 		- GU_DEPTH_TEST
	 * 		- GU_CULL_FACE
	 * 		- GU_CLIP_PLANES
	 * 		- GU_LIGHTNING
	 *
	 * Anything in addition should be initialized/set or changed
	 * in your
	 */
	virtual bool init();

	/*
	 * before exiting the homebrew we need to
	 * do some clean up
	 */
	virtual void exit();

	/*
	 * some static helper to enable the additional control
	 * of the GU list from outside this class and its childs
	 */
	static void finishGu();
	static void startGu();
	/*
	 * setup the default render target being the usual drawbuffer
	 */
	static void initRenderTarget();
	/*
	 * returns the current draw buffer
	 */
	static void* getDrawBuf();

protected:
	/*
	 * Constructor and destructor are protected to
	 * prevent direct instantiation of this class
	 */
	Cl3dHomebrew();
	virtual ~Cl3dHomebrew();

	/*
	 * inherit and extend the mainthread of the base class
	 * this would start the GU list and synch it as well
	 * It also flips the draw/display buffer
	 */
	virtual void mainthread();
	/**
	 * the real 3D application class should implement this render method
	 * GuStart and GuFinish are not needed there!
	 **/
	virtual void render();

	/**
	 * re-define this method to set application specific save/dialog
	 * behavior
	 */
	virtual void setLoadSaveData(SceUtilitySavedataParam* saveData);

	/**
	 * re-define this method to get data from load dialog back to application
	 */
	virtual void getLoadData(void* data);

	/**
	 * initialize SaveDialogue stuff. This need to be implemented by the
	 * application class to provide application specific behavior
	 */
	virtual void initializeSaveDialogue(SceUtilitySavedataParam* saveData, PspUtilitySavedataMode mode);

	/**
	 * display the load/save dialogue..the <home>-button is disabled untill you
	 * leave the dialog
	 */
	void showLoadSaveDialogue(PspUtilitySavedataMode mode);

	/*
	 * protected attributes
	 */
	static void *drawBuff; //the current draw buffer

	struct InitParams{
		short gu_PSM;
		bool gu_WaitVblank;
		bool statistics;
	}initParams;

	Cl3dEngine* engine;

	SceUtilitySavedataParam saveData;


};

#endif /* 3DHOMWBREW_H_ */
