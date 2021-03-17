/*
 * Mp3Helper.h Helper class to enable playing MP3 sounds in
 *             PSP homebrew applications
 *
 */

#ifndef MP3HELPER_H_
#define MP3HELPER_H_

extern "C"{
#include <stdio.h>
#include <pspkerneltypes.h>
#include <pspaudio.h>
}

#include <vector>

class ClMp3Helper {
public:
	static bool initMP3();
	static void closeMP3();
	static int playMP3(const char* fileName, unsigned short volume = PSP_AUDIO_VOLUME_MAX, bool repeat = false);
	static bool isMP3Playing(int mp3ID);
	static void stopMP3(int mp3ID);
	static void setMP3Volume(int mp3ID, unsigned short volume);

private:
	static int threadId;
	static int fillStreamBuffer(FILE* fileHandle, int mp3Handle);
	static int fillStreamBuffer(SceUID fd, int mp3Handle);
	static int playThread(SceSize args, void *argp);
	static int getID3TagSize(SceUID fd);

	typedef struct _MP3_THREADS{
		int mp3Id;
		int threadId;
	} Mp3Threads;

	static std::vector<Mp3Threads> mp3Threads;

protected:
	ClMp3Helper();
	virtual ~ClMp3Helper();
};

#endif /* MP3HELPER_H_ */
