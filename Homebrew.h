/*
 * Homebrew.h
 *
 * This class provides core functionality needed by any homebrew
 * For reuse create your own application class inheriting this one
 * and implement the methods you want to extend or change the behavior of
 *
 * (c) André Borrmann 2010
 * Permission to copy, use, modify and distribute of this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to it's suitability for any purpose
 */

#ifndef HOMEBREW_H_
#define HOMEBREW_H_
extern "C"{
#include <pspdebug.h>
}

class ClHomebrew {
public:
	/*
	 * Initialisierung des Homebrews
	 */
	bool init();
	/*
	 * Homebrew ausführen bis "HOME" Taste + Beenden
	 */
	void run();
	/*
	 * Homebrew soll beendet werden, ggfl. speicher freigeben
	 */
	virtual void exit();
	/*
	 * Destructor
	 */
	virtual ~ClHomebrew();

protected:
	/**
	 * die Klasse kann nicht direkt instanziiert werden -> singleton
	 * Konstrukor ist protected
	 */
	ClHomebrew();
    virtual void mainthread();


};

#endif /* HOMEBREW_H_ */
