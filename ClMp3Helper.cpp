/*
 * Mp3Helper.cpp
 *
 *  Created on: Feb 10, 2010
 *      Author: andreborrmann
 */

extern "C"{
#include <pspkernel.h>
#include <pspaudio.h>
#include <pspmp3.h>
#include <psputility.h>
//#include <pspdebug.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
}

#include "ClMp3Helper.h"
#include "ClHBLogger.h"

#define ENDIAN_SWAP_32BIT(n) (n = ((n&0xFF000000)>>24)|((n&0x00FF0000)>>8)|((n&0x0000FF00)<<8)|((n&0x000000FF)<<24))
#define ID3_TAG_LENGTH_TO_INT(n) (n = (((n&0x7f000000)>>3)|((n&0x7f0000)>>2)|((n&0x7f00)>>1)|(n&0x7f)))

/*
char	mp3Buf[16*1024]  __attribute__((aligned(64)));
short	pcmBuf[16*(1152/2)]  __attribute__((aligned(64)));
*/
/*
 * Bits send to the MP3 thread to get control from outside
 * they are or'd together
 * additional attributes will be stored at bit position 16 to 32
 */
enum Mp3ControlBits {
	MP3_STOP_BIT       = 0b00001,
	MP3_SET_VOLUME_BIT = 0b00010,
	MP3_ALL_BITS       = 0xffffffff
};

typedef struct ThreadParams{
	const char* fileName;
	int eventId;
	unsigned short volume;
	bool repeat;
	int threadId;
}ThreadParams;

int ClMp3Helper::threadId = 0;
std::vector<ClMp3Helper::Mp3Threads> ClMp3Helper::mp3Threads;

ClMp3Helper::ClMp3Helper() {
	// TODO Auto-generated constructor stub

}

int ClMp3Helper::fillStreamBuffer(FILE* fileHandle, int mp3Handle){
	unsigned char* destination;
	int bytesToWrite;
	int srcPos;

	int mp3Status;
	int bytesRead;

	//this will get the amount of data needed to be red and the
	//memory will be allocated accordingly
	mp3Status = sceMp3GetInfoToAddStreamData(mp3Handle, &destination, &bytesToWrite, &srcPos);
	if (mp3Status < 0) return -1;
	//go to the file position requested
	mp3Status = fseek( fileHandle, srcPos, SEEK_SET );
	if (mp3Status < 0) return -1;
	//now read the data requested
	bytesRead = fread( destination, sizeof(char), bytesToWrite, fileHandle );
	if (bytesRead <0) return -1;
	if (bytesRead == 0) return 0; //reached end of file

	//notify MP3 lib how many bytes we have processed
	mp3Status = sceMp3NotifyAddStreamData( mp3Handle, bytesRead );
	if (mp3Status <0) return -1;

	return (srcPos > 0);
}

int ClMp3Helper::getID3TagSize(SceUID fd)
{
	char header[10];
	int size = 0;

	if (fd < 0)
		return 0;

	sceIoLseek( fd, 0, PSP_SEEK_SET );
	sceIoRead(fd, header, sizeof(header));

	if (!strncmp((char *) header, "ea3", 3)
		|| !strncmp((char *) header, "EA3", 3)
		|| !strncmp((char *) header, "ID3", 3)) {
		// get the real size from the syncsafe int
		int tagSize;
		sceIoLseek(fd, 6, PSP_SEEK_SET);
		sceIoRead(fd, &tagSize, 4);
		ENDIAN_SWAP_32BIT(tagSize);
		ID3_TAG_LENGTH_TO_INT(tagSize);

		tagSize += 10;
		return tagSize;
	}
	return 0;
}

int ClMp3Helper::fillStreamBuffer(SceUID fd, int mp3Handle){
	unsigned char* destination;
	int bytesToWrite;
	int srcPos;

	int mp3Status;
	int bytesRead;

	//this will get the amount of data needed to be red and the
	//memory will be allocated accordingly
	mp3Status = sceMp3GetInfoToAddStreamData(mp3Handle, &destination, &bytesToWrite, &srcPos);
	if (mp3Status < 0) return -1;
	//go to the file position requested
	mp3Status = sceIoLseek( fd, srcPos, PSP_SEEK_SET );
	if (mp3Status < 0) return -1;
	//now read the data requested
	bytesRead = sceIoRead( fd, (char*)destination, bytesToWrite);
	if (bytesRead <0) return -1;
	if (bytesRead == 0) return 0; //reached end of file

	//notify MP3 lib how many bytes we have processed
	mp3Status = sceMp3NotifyAddStreamData( mp3Handle, bytesRead );
	if (mp3Status <0) return -1;

	return (srcPos > 0);
}

bool ClMp3Helper::initMP3(){
	//load the modules for the codecs
	int status = sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	if (status < 0) return false;
	status = sceUtilityLoadModule(PSP_MODULE_AV_MP3);
	if (status < 0) return false;
	status = sceMp3InitResource();
	if (status < 0) return false;

	return true;
}


int ClMp3Helper::playMP3(const char *fileName, unsigned short volume, bool repeat){

	//now start playing in a separate thread to allow the main application
	//continue running
	static short id = 0;
	char threadName[100];
	Mp3Threads threadInfo;
	sprintf(threadName, "playSound%d", id);
	id++;
	threadId = sceKernelCreateThread(threadName, ClMp3Helper::playThread, 0x11, 0x10000, THREAD_ATTR_USER, 0);
	if (threadId >= 0){
		ThreadParams params;
		params.fileName = fileName;
		//to allow control of the thread from outside
		//we create a EventFlag the thread is waiting for
		//the method will return the ID of this EventFlag
		SceUID threadEvent = sceKernelCreateEventFlag(threadName, 0,0,0);//PSP_EVENT_WAITMULTIPLE, 0, 0);
		params.eventId = threadEvent;
		params.volume = volume;
		params.repeat = repeat;
		params.threadId = threadId;
		sceKernelStartThread(threadId, sizeof(params), &params);
		//sprintf(threadName, "thread started %d", threadId);
		//ClHBLogger::log(threadName);
		threadInfo.mp3Id = threadEvent;
		threadInfo.threadId = threadId;
		mp3Threads.push_back(threadInfo);

		return threadEvent;
	}

	return -1;
}

void ClMp3Helper::setMP3Volume(int mp3ID, unsigned short volume){
	sceKernelSetEventFlag(mp3ID, MP3_SET_VOLUME_BIT | volume << 16);
}


void ClMp3Helper::stopMP3(int mp3ID){
	sceKernelSetEventFlag(mp3ID, MP3_STOP_BIT);
	sceKernelDelayThread(100);
}

bool ClMp3Helper::isMP3Playing(int mp3ID){
	short i;
	SceKernelThreadInfo status;
	status.size = sizeof(SceKernelThreadInfo);

	for (i=0;i<mp3Threads.size();i++){
		if (mp3ID == mp3Threads[i].mp3Id){
			if (sceKernelReferThreadStatus(mp3Threads[i].threadId,&status)==0){
				//as long as we recieve a thread status it seem to be running
				return true;
			} else
				return false;
		}
	}
	return false;
}

int ClMp3Helper::playThread(SceSize args, void *argp){

	int status;
	char path[255];
	memset(path, 0,255);
	getcwd(path, 255);
	ThreadParams* params = (ThreadParams*)argp;
	//FILE* fileHandle = fopen( params->fileName, "rb" );
	strcat(path, "/");
	strcat(path, params->fileName);
	SceUID fd = sceIoOpen( path, PSP_O_RDONLY | PSP_O_NBLOCK, 0777 );

	//if (!fileHandle) return -1;
	if (fd < 0){
		threadId=0;
		return sceKernelExitDeleteThread(0);
	}

	SceMp3InitArg mp3Init;
	mp3Init.mp3StreamStart = getID3TagSize(fd);
	//fseek( fileHandle, 0, SEEK_END );
	sceIoLseek(fd, 0, PSP_SEEK_END);
	mp3Init.mp3StreamEnd = sceIoLseek(fd, 0, PSP_SEEK_END);;//ftell(fileHandle);
	mp3Init.unk1 = 0;
	mp3Init.unk2 = 0;
	mp3Init.mp3Buf = memalign(64, 16*1024*sizeof(char));
	mp3Init.mp3BufSize = 16*1024*sizeof(char);
	mp3Init.pcmBuf = memalign(64, 16*(1152/2)*sizeof(short));
	mp3Init.pcmBufSize = 16*(1152/2)*sizeof(short);

	//int id3Size = getID3TagSize(fd);

	//get the MP3 handle
	int mp3Handle = sceMp3ReserveMp3Handle( &mp3Init );
	if (mp3Handle >= 0){
		//fill the stream Buffer
		//ClMp3Helper::fillStreamBuffer(fileHandle, mp3Handle);
		ClMp3Helper::fillStreamBuffer(fd, mp3Handle);

		sceMp3Init( mp3Handle );

		int samplingRate = sceMp3GetSamplingRate( mp3Handle );
		int numChannels = sceMp3GetMp3ChannelNum( mp3Handle );
		int kbits = sceMp3GetBitRate( mp3Handle );
		int lastDecoded = 0;
		int volume = params->volume;//PSP_AUDIO_VOLUME_MAX;
		bool playing = true;
		int channel = -1;
		unsigned int eventBits = 0;
		unsigned int waitTime = 100;
		if (params->repeat)
			status = sceMp3SetLoopNum( mp3Handle, -1 );
		else
			status = sceMp3SetLoopNum( mp3Handle, 0);

		while (playing) {

			//first check if we need new data
			// Check if we need to fill our stream buffer
			if (sceMp3CheckStreamDataNeeded( mp3Handle )>0)
			{
				//if there was nothing more read from the buffer finish playing
				//if(!ClMp3Helper::fillStreamBuffer( fileHandle, mp3Handle ))
				if (!ClMp3Helper::fillStreamBuffer( fd, mp3Handle)
					&& !params->repeat)
					playing = false;
			}

			// Decode some samples
			short* buf;
			int bytesDecoded;
			int retries = 0;
			//try top decode data and ask for more if necessary
			for (retries = 0;retries<1;retries++){

				bytesDecoded = sceMp3Decode( mp3Handle, &buf );
				if (bytesDecoded>0) break; //date decoded - everything fine
				//otherwise check if we need more data
				if (sceMp3CheckStreamDataNeeded( mp3Handle )<=0) break; //nothing more needed
				//need to get data - but if nothing available stop playing
				if(!ClMp3Helper::fillStreamBuffer( fd, mp3Handle))
					playing = false;
			}
			//error in decoding stops playing
			if (bytesDecoded<0 && bytesDecoded!=0x80671402){
				playing = false;
			}
			//if there is nothing more to decode, we have finished
			if (bytesDecoded==0 || bytesDecoded==0x80671402)
			{
				if (!params->repeat)
					playing = false;
				else
					//start from beginning
					sceMp3ResetPlayPosition(mp3Handle);
			} else {
				// Reserve the Audio channel for our output if not yet done
				if (channel<0 || lastDecoded!=bytesDecoded)
				{
					if (channel>=0)
						//sceAudioSRCChRelease();
						sceAudioChRelease(channel);

					//channel = sceAudioSRCChReserve( bytesDecoded/(2*numChannels), samplingRate, numChannels );
					channel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, PSP_AUDIO_SAMPLE_ALIGN(bytesDecoded/(2*numChannels)), PSP_AUDIO_FORMAT_STEREO);

					//sceKernelDelayThread(1000);
					lastDecoded = bytesDecoded;
				}
				// Output the decoded sample
				//sceAudioSRCOutputBlocking( volume, buf );
				if (channel >= 0)
					status = sceAudioOutputBlocking(channel, volume, buf);
					//status = sceAudioOutput(channel, volume, buf);
			}
			sceKernelDelayThread(100);
			//now wait for the EventFlag
			eventBits = 0;
			waitTime = 50;
			sceKernelWaitEventFlag(params->eventId, MP3_ALL_BITS, PSP_EVENT_WAITAND | PSP_EVENT_WAITCLEAR, &eventBits, &waitTime);
			if (eventBits & MP3_STOP_BIT){
				//we have recieved stop flag
				playing = false;
			}
			/*
			if (eventBits & MP3_SET_VOLUME_BIT){
				volume = eventBits >> 16;
				sceKernelSetEventFlag(params->eventId, 0);
			}*/
		}

		//if we have finished playing...some cleanup
		if (channel>=0)
			//sceAudioSRCChRelease();
			sceAudioChRelease(channel);

		sceMp3ReleaseMp3Handle( mp3Handle );
	} //mp3Handle >= 0
	//fclose( fileHandle );
	sceIoClose(fd);
	free(mp3Init.mp3Buf);
	free(mp3Init.pcmBuf);

	std::vector<Mp3Threads>::iterator it = mp3Threads.begin();
	while (it < mp3Threads.end()){
		if (it->threadId == params->threadId){
			it = mp3Threads.erase(it);
		} else
			it++;
	}
	threadId=0;
	//char log[255];
	//sprintf(log, "thread stopped %d", params->threadId);
	//ClHBLogger::log(log);
	return sceKernelExitDeleteThread(0);
}

void ClMp3Helper::closeMP3(){

	short i;

	for (i=0;i<mp3Threads.size();i++){
		stopMP3(mp3Threads[i].mp3Id);
	}
	sceKernelDelayThread(1000);
	mp3Threads.clear();

	sceMp3TermResource();
}

ClMp3Helper::~ClMp3Helper() {
	// TODO Auto-generated destructor stub
}
