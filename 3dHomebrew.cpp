/*
 * 3dHomwbrew.cpp
 *
 *  Created on: 14.01.2010
 *      Author: andreborrmann
 */

extern "C" {
#include <pspdisplay.h>
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <malloc.h>
#include <string.h>
}

#include "3dHomebrew.h"
#include "3dStatistics.h"

static unsigned int __attribute__((aligned(16))) list[426144];

char key[] = "QTAK319JQKJ952HA";
char titleShow[] = "New Save\0";
PspUtilitySavedataListSaveNewData gameData;

char nameMultiple[][20] =	// End list with ""
{
 "0000",
 "0001",
 "0002",
 "0003",
 "0004",
 ""
};

void *Cl3dHomebrew::drawBuff = 0x0;

short gu_PSM;

Cl3dHomebrew::Cl3dHomebrew() {
	// TODO Auto-generated constructor stub

	//set the default init parameters
	this->initParams.gu_PSM = GU_PSM_8888;
	this->initParams.gu_WaitVblank = true;
	this->initParams.statistics = false;
}

bool Cl3dHomebrew::init(){
	//zuerst die Superklassen Methode rufen
	if (!ClHomebrew::init()) return false;
	//nun die eigene Initialisierung der GU
	sceGuInit();

	drawBuff = (void*)0x0;
	//die Adresse des AnzeigePuffers sollte nach dem Zeichenpuffer beginnen
	//dabei müssen die Adressen weitgenug auseinander liegen, so
	//dass der komplette Bildschirminhalt in diesen Bereich passt.
	//bei 480×272 Bildpunkten und 4 Byte Farbtiefe sind das 522240 Bytes.
	//die PSP arbeitet intern aber immer mit einer Breite die eine Potenz von 2 ist. Damit werden
	//im Puffer 512×272 Bildpunkte und 4 Bytes pro Punkt belegt. Das sind dann 557056 Bytes
	//die Adresse muss also (in Hex) 0x88000 höher sein als die Zeichenpufferaddresse
	unsigned int bufLen;
	switch (initParams.gu_PSM) {
		case GU_PSM_8888:
			bufLen = BUF_WIDTH*SCR_HEIGHT*4;
			break;
		case GU_PSM_4444:
		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_T16:
			bufLen = BUF_WIDTH*SCR_HEIGHT*2;
			break;
		case GU_PSM_T8:
			bufLen = BUF_WIDTH*SCR_HEIGHT;
			break;
	}
	gu_PSM = initParams.gu_PSM;
	void* displayBuff = (void*)bufLen;//0x88000;
	bufLen*=2;
	//die Z-Puffer Adresse muss um die selbe Größe hinter dem BackPuffer kommen
	void* depthBuff = (void*)bufLen;//0x110000;
	//nun teilen wir der GU diese Speicheraddressen für den jeweiligen zweck mit.
	//Dazu starten wir erstmal die Liste
	sceGuStart(GU_DIRECT,list);
	//beim festlegen des Zeichenpuffers müssen wir auch dessen Format mitgeben.
	//also 4 Bytes pro Punkt (8bit rot, 8Bit grün, 8Bit blau, 8bit Alpha) und 512 Punkte breit
	sceGuDrawBuffer(initParams.gu_PSM,drawBuff,BUF_WIDTH);
	//dem Darstellungspuffer müssen wir zusätzlich mitgeben wie breit und hoch der Bildschirm ist.
	//plus der Pufferbreite. Interessanterweise kein Pixelformat
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,displayBuff,BUF_WIDTH);
	//nun noch den Tiefenpuffer. Auch hier muss die Pufferbreite entsprechend angegeben werden
	sceGuDepthBuffer(depthBuff,BUF_WIDTH);

	//der virtuelle Bildschirm soll zentriert sein
	//und die Breite und Höhe des PSP Schirmes haben
	//die GU geht hierbei immer von der Tiefe 0 aus.
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	//der Koordinatensystemoffset (Nullpunkt) wird abhängig von der virtuellen
	//Bildschirmgröße gesetzt. Wieso ? das ist nicht ganz klar…
	//ich hätte hier - vermutlich wie Ihr auch - einen Offset bei 2048/2048 erwartet.
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	//setzen des maximalen Tiefenbereiches
	sceGuDepthRange(65535,0);
	//aktivieren des “Zuschneiders”.
	//damit wird das Berechnete Bild auf die Masse des Bildschirms zugeschnitten.
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	//aktivieren der Tiefenprüfung
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	//Nochmals festlegen, dass “vorne” im Uhrzeigersinn ist (ClockWise)
	sceGuFrontFace(GU_CW);
	// aktiviere dass Flächen von “hinten” nicht gezeichnet werden
	sceGuEnable(GU_CULL_FACE);
	//aktiviere das Clipping
	sceGuEnable(GU_CLIP_PLANES);
	//aktiviere das Licht
	sceGuEnable(GU_LIGHTING);
	//setze des Umgebunbgslicht auf einen hellen Wert.
	//Die Farbe wird dabei in Hexform als ABGR - also
	//Alpha-Blau-Grün-Rot Wert die jeweils von 0 bis FF gehen abgebildet
	sceGuAmbient(0xFFFFFFFF);
	//abschließen der GU Liste führt zur sofortigen Abarbeitung
	//in diesem Falle, da wir die Liste mit GU_DIRECT gestartet haben
	sceGuFinish();
	sceGuSync(0,0);
	//Aktiviere die Anzeige der GU auf dem realen Bildschirm
	sceGuDisplay(GU_TRUE);

	//um die Matrizenfunktionen nutzen zu können initialisieren wir noch
	//diesen Teil der GU
	gumInit();

	sceGuStart(GU_DIRECT, list);
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(45.0f, 480.0f/272.0f, 0.1f, 1000.0f);
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumUpdateMatrix();
	sceGuFinish();
	sceGuSync(0,0);

	engine = new Cl3dEngine();
	if (initParams.statistics)
		this->engine->enableStatistics();

	return true;
}

void Cl3dHomebrew::exit(){
	//GU beenden
	sceGuTerm();
	if (engine)
		delete(engine);
	//Superklassen Methode am Schluss
	ClHomebrew::exit();
}

void Cl3dHomebrew::initRenderTarget(){
	//festlegen des normalen Zeichenpuffers als Render-Target
	sceGuDrawBufferList(gu_PSM,(void*)drawBuff,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(45.0f, 480.0f/272.0f, 0.1f, 1000.0f);
}

void Cl3dHomebrew::finishGu(){
	sceGuFinish();
	sceGuSync(0,0);
}

void *Cl3dHomebrew::getDrawBuf(){
	return drawBuff;
}

void Cl3dHomebrew::startGu(){
	sceGuStart(GU_DIRECT,list);
}


void Cl3dHomebrew::mainthread(){
	//Die Hauptabarbeitungsschleife meiner 3D Applikation
	//hier einfach alles rendern...
	sceKernelDcacheWritebackAll();
	startGu();
	sceGuClearColor(0xff000000);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	render();

	finishGu();

	if (initParams.statistics){
		Cl3dStatistics::getInstance()->displayStatistic(drawBuff);
	}

	drawBuff = sceGuSwapBuffers();
	if (initParams.gu_WaitVblank)
		sceDisplayWaitVblankStart();
}

Cl3dHomebrew::~Cl3dHomebrew() {
	// TODO Auto-generated destructor stub
}

void Cl3dHomebrew::render(){
// Platzhalter für die RenderMethode die in der
// eigentlichen Klasse implementiert werden muss
}

void Cl3dHomebrew::showLoadSaveDialogue(PspUtilitySavedataMode mode)
{
	SceUtilitySavedataParam saveData;

	this->initializeSaveDialogue(&saveData, mode);

	int saveInit = sceUtilitySavedataInitStart(&saveData);

	int saveStatus;
	bool saveRunning = true;
	if (saveInit != 0)
		saveRunning = false;
	//handling the save dialogue
	//stop started drawing from GU
	this->finishGu();
	while (saveRunning){
		//TODO: pass to the application to draw save dialogue background
		this->startGu();
		sceGuClearColor(0xff000000);
		sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
		this->finishGu();

		saveStatus = sceUtilitySavedataGetStatus();
		switch(saveStatus){
			case PSP_UTILITY_DIALOG_VISIBLE :
				sceUtilitySavedataUpdate(1);
				break;

			case PSP_UTILITY_DIALOG_QUIT :
				sceUtilitySavedataShutdownStart();
				break;

			case PSP_UTILITY_DIALOG_FINISHED :
				if (mode == PSP_UTILITY_SAVEDATA_LISTLOAD){
					//TODO: as data was loaded pass data back to application
					this->getLoadData(saveData.dataBuf);
				}
				saveRunning = false;
				break;
			case PSP_UTILITY_DIALOG_NONE :
				saveRunning = false;
				break;
		}
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
		sceKernelDelayThread(10000);
	}

	//clean up some data
	if (saveData.icon0FileData.buf)
		free(saveData.icon0FileData.buf);

	//start drawing again with gu...
	this->startGu();
	return;
}


void Cl3dHomebrew::initializeSaveDialogue(SceUtilitySavedataParam* saveData, PspUtilitySavedataMode mode)
{
	memset(saveData, 0, sizeof(SceUtilitySavedataParam));
	saveData->base.size = sizeof(SceUtilitySavedataParam);
	saveData->base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	saveData->base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
	saveData->base.graphicsThread = 0x11;
	saveData->base.accessThread = 0x13;
	saveData->base.fontThread = 0x12;
	saveData->base.soundThread = 0x10;

	saveData->mode = mode;
	saveData->overwrite = 1;
	saveData->focus = PSP_UTILITY_SAVEDATA_FOCUS_LATEST; // Set initial focus to the newest file (for loading)

	#if _PSP_FW_VERSION >= 200
	strncpy(saveData->key, key, 16);
	#endif

	//saveData->dataSize = saveData->dataBufSize;
	//strcpy(saveData->gameName,"MAGICBOWLV1");
	strcpy(saveData->saveName,"0000");
	strcpy(saveData->fileName,"DATA.BIN");
	saveData->saveNameList = nameMultiple;

//	memset(&saveGame, 0, sizeof(saveGame));
//	saveData->dataBuf = &saveGame;
//	saveData->dataBufSize = sizeof(saveGame);
//	saveData->dataSize = sizeof(saveGame);

	if (mode == PSP_UTILITY_SAVEDATA_LISTSAVE)
	{
		//strcpy(saveData->sfoParam.title,"MagicBowlSave");
		//strcpy(saveData->sfoParam.savedataTitle,"Single player mode");
		//sprintf(saveData->sfoParam.detail, "Level:%d\nMana:%d", saveGame.level, saveGame.mana);
		//strcpy(saveData->sfoParam.detail,"Details");
		saveData->sfoParam.parentalLevel = 1;

		saveData->icon0FileData.buf = 0;
		saveData->icon0FileData.bufSize = 0;
		saveData->icon0FileData.size = 0;

		saveData->icon1FileData.buf = 0;
		saveData->icon1FileData.bufSize = 0;
		saveData->icon1FileData.size = 0;

		//I do not know where this will be shown...but leave it from the example
		saveData->pic1FileData.buf = 0;
		saveData->pic1FileData.bufSize = 0;
		saveData->pic1FileData.size = 0;

		saveData->snd0FileData.buf = 0;
		saveData->snd0FileData.bufSize = 0;
		saveData->snd0FileData.size = 0;

		gameData.title = "new Save Game";
		gameData.icon0.buf = 0;
		gameData.icon0.bufSize = 0;
		gameData.icon0.size = 0;

		saveData->newData = &gameData;

		saveData->focus = PSP_UTILITY_SAVEDATA_FOCUS_FIRSTEMPTY;
	}

	this->setLoadSaveData(saveData);
}

void Cl3dHomebrew::setLoadSaveData(SceUtilitySavedataParam* saveData){
	//to enable save "out-of-the-box"
	//do this default implementation
	strcpy(saveData->gameName, "MYGAME");
	saveData->dataBuf = (char*)malloc(10);
	strcpy((char*)saveData->dataBuf, "Test");
	saveData->dataBufSize = 10;
	saveData->dataSize = 10;

	strcpy(saveData->sfoParam.title,"MySave");
	strcpy(saveData->sfoParam.savedataTitle,"Data Title");
	//sprintf(sfoParam->detail, "Level:%d\nMana:%d", saveGame.level, saveGame.mana);
	strcpy(saveData->sfoParam.detail,"Details");
}

void Cl3dHomebrew::getLoadData(void* data){

}
