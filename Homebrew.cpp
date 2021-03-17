/*
 * homebrew.cpp
 *
 * Diese Klasse ist die Basisklasse für jede Art von Homebrew Entwicklung
 */

#include "homebrew.h"

extern "C" {
#include "callbacks.h"
}

void ClHomebrew::run()
{
	// die Hauptschleife
	while(running()){
		/*
		 * so lange die Applikation nicht über die HOME-Taste
		 * beendet wurde diese Ausführen
		 */
		mainthread();
	}
}

/**
 * Initializing the homebrew. Set up of callbacks
 */
bool ClHomebrew::init(){
	//init debug screen
	pspDebugScreenInit();

	setupCallbacks();

	return true;
}

void ClHomebrew::exit(){
}

/**
 * Leerer Hauptthread/Programmablauf. Muss von der
 * Ableitenden Klasse implementiert werden
 */
void ClHomebrew::mainthread(){
}

ClHomebrew::ClHomebrew() {
}

ClHomebrew::~ClHomebrew() {
}






