/*
 * Mp4Decoder.cpp
 *
 * Copyright (C) 2010 André Borrmann
 *
 * Inspired by the initial code provided by cooleyes
 *   	   eyes.cooleyes@gmail.com
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 */

extern "C"{
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <pspdebug.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspmpeg.h>
#include <pspmpegbase.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <unistd.h>
#include "mem64.h"
}
#include "Mp4Decoder.h"

/*
 * define some internally used structures
 */
typedef struct AvcNalStruct{
   ScePVoid sps_buffer;
   SceInt32 sps_size;
   ScePVoid pps_buffer;
   SceInt32 pps_size;
   SceInt32 unkown0;
   ScePVoid nal_buffer;
   SceInt32 nal_size;
   SceInt32 mode;
} AvcNalStruct;

typedef struct AvcInfoStruct{
   SceInt32 unknown0;
   SceInt32 unknown1;
   SceInt32 width;
   SceInt32 height;
   SceInt32 unknown4;
   SceInt32 unknown5;
   SceInt32 unknown6;
   SceInt32 unknown7;
   SceInt32 unknown8;
   SceInt32 unknown9;
} AvcInfoStruct;

typedef struct AvcYuvStruct{
   ScePVoid buffer0;
   ScePVoid buffer1;
   ScePVoid buffer2;
   ScePVoid buffer3;
   ScePVoid buffer4;
   ScePVoid buffer5;
   ScePVoid buffer6;
   ScePVoid buffer7;
   SceInt32 unknown0;
   SceInt32 unknown1;
   SceInt32 unknown2;
} AvcYuvStruct;


typedef struct AvcCodecStruct{
   SceInt32 unknown0;
   SceInt32 unknown1;
   SceInt32 unknown2;
   SceInt32 unknown3;
   AvcInfoStruct* info_buffer;
   SceInt32 unknown5;
   SceInt32 unknown6;
   SceInt32 unknown7;
   SceInt32 unknown8;
   SceInt32 unknown9;
   SceInt32 unknown10;
   AvcYuvStruct* yuv_buffer;
   SceInt32 unknown12;
   SceInt32 unknown13;
   SceInt32 unknown14;
   SceInt32 unknown15;
   SceInt32 unknown16;
   SceInt32 unknown17;
   SceInt32 unknown18;
   SceInt32 unknown19;
   SceInt32 unknown20;
   SceInt32 unknown21;
   SceInt32 unknown22;
   SceInt32 unknown23;
} AvcCodecStruct;

typedef struct AvcCscStruct{
   SceInt32 height;
   SceInt32 width;
   SceInt32 mode0;
   SceInt32 mode1;
   ScePVoid buffer0;
   ScePVoid buffer1;
   ScePVoid buffer2;
   ScePVoid buffer3;
   ScePVoid buffer4;
   ScePVoid buffer5;
   ScePVoid buffer6;
   ScePVoid buffer7;
} AvcCscStruct;

typedef struct AvcDecodeStruct{
   ScePVoid mpeg_buffer;
   SceMpeg mpeg;
   SceMpegRingbuffer mpeg_ringbuffer;
   SceMpegAu* mpeg_au;
   SceInt32 mpeg_mode;
   SceInt32 mpeg_buffer_size;
   ScePVoid mpeg_ddrtop;
   ScePVoid mpeg_au_buffer;
   AvcNalStruct mpeg_nal;
   AvcCodecStruct* mpeg_codec_buffer;
   AvcYuvStruct* mpeg_yuv_buffer;
   AvcInfoStruct* mpeg_info_buffer;
} AvcDecodeStruct;

bool ClMp4Decoder::initialized = false;
unsigned char* ClMp4Decoder::frameBuffer[] = {0,0};
mp4ff_callback_t ClMp4Decoder::mp4_callback = {ClMp4Decoder::mp4_read, 0, ClMp4Decoder::mp4_seek, 0, 0};

int ClMp4Decoder::init(const char *prxFile){
	//initializing the MP4 decoder class to be able to
	//load and play MP4 files
	if (initialized) return 0;
	//set up the frame buffer for output of the video data to
	//screen, must be part of VRAM.
	//first static initialization
	frameBuffer[0] = (unsigned char*)(0x44000000);
	frameBuffer[1] = (unsigned char*)(0x44000000 | 0x88000); //assuming ColorMode 8888 and frame width 512

	//get decoder prx location
	char prxLocation[1024];
	getcwd(prxLocation, 256);
	//prxFile should start with /
	strcat(prxLocation, prxFile);

	//load the base audio/video prx into user mode
	int result;
	result = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	if (result != 0 && result != 0x80110f02) // - already loaded ?
		return MP4_ERROR_AV_MODULE_AVCCODEC;

	//load the decoding prx
	SceUID modid;
	int status;
	modid = sceKernelLoadModule(prxLocation, 0, NULL);
	if(modid >= 0) {
		modid = sceKernelStartModule(modid, 0, 0, &status, NULL);
	} else
		return MP4_ERROR_MPEG_VSHPRX;

	//no init MPEG
	result = sceMpegInit();
	if (result != 0)
		return MP4_ERROR_MPEG_INIT;

	//if everything where fine the MP4 Decoder is initialized
	initialized = true;
	return 0;
}

int ClMp4Decoder::playMp4(const char *filename, unsigned int stopButton, short pixelFormat){

	if (!initialized)
		return MP4_ERROR_NOT_INITIALIZED;

	int frame_index = 0;
	int frame_count = 0;
	u32 total_samples;
	u32 total_tracks;
	FILE* mp4_file;
	mp4ff_t* mp4_handle;
	unsigned char* sps_pps_buffer;
	unsigned int sps_size, pps_size;

	AvcDecodeStruct* avc = new AvcDecodeStruct;
	AvcCscStruct* csc    = new AvcCscStruct;

	//first try to access the file
	mp4_file = fopen(filename, "rb");
	if (!mp4_file)
		return MP4_ERROR_FILE_NOT_FOUND;

	//store the callback info
	mp4_callback.user_data = &mp4_file;
	//open and read the mp4 file
	mp4_handle = mp4ff_open_read(&mp4_callback);

	//no get some data for play back
	total_tracks = mp4ff_total_tracks(mp4_handle);
	total_samples = mp4ff_num_samples(mp4_handle, 0);
	if ( mp4ff_get_avc_sps_pps(mp4_handle, 0, &sps_pps_buffer, &sps_size, &pps_size) != 0 || sps_size == 0 || pps_size == 0 ){
		//cleanup data if failed
		delete(avc);
		delete(csc);
		mp4ff_close(mp4_handle);
		fclose(mp4_file);
		return MP4_ERROR_GET_AVC_SPS_SPP;
	}

	//set some decoding info for this file
	avc->mpeg_mode = 4;
	avc->mpeg_ddrtop =  memalign(0x400000, 0x400000);
	avc->mpeg_au_buffer = (void*)((int)avc->mpeg_ddrtop + 0x10000);

	//get decoding memory
	int result;
	result = sceMpegQueryMemSize(avc->mpeg_mode);
	if (result < 0){
		free(avc->mpeg_ddrtop);
		delete(avc);
		delete(csc);
		mp4ff_close(mp4_handle);
		fclose(mp4_file);
		return MP4_ERROR_QUERY_MEM_SIZE;
	}
	avc->mpeg_buffer_size = result;
	if ( (result & 0xF) != 0 )
	      result = (result & 0xFFFFFFF0) + 16;

	avc->mpeg_buffer = malloc_64(result);
	if (avc->mpeg_buffer == 0){
		free(avc->mpeg_ddrtop);
		delete(avc);
		delete(csc);
		mp4ff_close(mp4_handle);
		fclose(mp4_file);
		return MP4_ERROR_ALLOC_MPEG_BUFFER;
	}
	result = sceMpegCreate(&avc->mpeg, avc->mpeg_buffer, avc->mpeg_buffer_size, &avc->mpeg_ringbuffer, 512, avc->mpeg_mode, avc->mpeg_ddrtop);
	if (result != 0){
		free(avc->mpeg_ddrtop);
		free_64(avc->mpeg_buffer);
		delete(avc);
		delete(csc);
		mp4ff_close(mp4_handle);
		fclose(mp4_file);
		return MP4_ERROR_MPEG_CREATE;
	}

	avc->mpeg_au = (SceMpegAu*)malloc_64(64);
	if (avc->mpeg_au == 0){
		sceMpegDelete(&avc->mpeg);
		free(avc->mpeg_ddrtop);
		free_64(avc->mpeg_buffer);
		delete(avc);
		delete(csc);
		mp4ff_close(mp4_handle);
		fclose(mp4_file);
		return MP4_ERROR_ALLOC_MPEG_AU;
	}
	memset(avc->mpeg_au, 0xFF, 64);
	if ( sceMpegInitAu(&avc->mpeg, avc->mpeg_au_buffer, avc->mpeg_au) != 0 ){
		sceMpegDelete(&avc->mpeg);
		free_64(avc->mpeg_au);
		free(avc->mpeg_ddrtop);
		free_64(avc->mpeg_buffer);
		delete(avc);
		delete(csc);
		mp4ff_close(mp4_handle);
		fclose(mp4_file);
		return MP4_ERROR_ALLOC_INIT_AU;
	}

	unsigned char* nal_buffer = (unsigned char*)malloc_64(1024*1024);

	// now we seem to have everything in place to start playing the movie
	//first set the display buffer
	sceDisplayWaitVblankStart(); //8888
	sceDisplaySetFrameBuf(frameBuffer[frame_index], 512, pixelFormat, PSP_DISPLAY_SETBUF_IMMEDIATE);

	//now block the thread this movie will be played from until
	//it has stopped
	bool playing = true;
	//it should be possible to stop playing by pressing
	//a key
	SceCtrlData button;

	int pic_num; //current picture number
	while (playing){
		//the main task is now to setup data for decoding and display
		//the decoded images/frames of the movie
		avc->mpeg_nal.sps_buffer = (&sps_pps_buffer[0]);
		avc->mpeg_nal.sps_size = sps_size;
		avc->mpeg_nal.pps_buffer = (&sps_pps_buffer[sps_size]);
		avc->mpeg_nal.pps_size = pps_size;
		avc->mpeg_nal.unkown0 = 4;
		memset(nal_buffer, 0, 1024*1024);
		mp4ff_read_sample_v2(mp4_handle, 0, frame_count, nal_buffer);

		avc->mpeg_nal.nal_buffer = nal_buffer;
		avc->mpeg_nal.nal_size = mp4ff_read_sample_getsize(mp4_handle, 0, frame_count);//size1 ;
		if ( frame_count == 0 )
		   avc->mpeg_nal.mode = 3;
		else
		   avc->mpeg_nal.mode = 0;

		result = sceMpegGetAvcNalAu(&avc->mpeg, &avc->mpeg_nal, avc->mpeg_au);
		result = sceMpegAvcDecode(&avc->mpeg, avc->mpeg_au, 512, 0, &pic_num);
	    result = sceMpegAvcDecodeDetail2(&avc->mpeg, &avc->mpeg_codec_buffer);

	    //if we have something decoded check if it was a picture
	    if ( result == 0 ) {
		  avc->mpeg_yuv_buffer = avc->mpeg_codec_buffer->yuv_buffer;
		  avc->mpeg_info_buffer = avc->mpeg_codec_buffer->info_buffer;

		  if ( pic_num > 0 ) {
			 int i;
			 for(i=0;i<pic_num;i++) {
				int csc_mode = 0;//i % 2;
				csc->height = avc->mpeg_info_buffer->height >> 4;
				csc->width = avc->mpeg_info_buffer->width >> 4;
				csc->mode0 = csc_mode;
				csc->mode1 = csc_mode;
				if ( csc_mode == 0 ) {
				   csc->buffer0 = avc->mpeg_yuv_buffer->buffer0 ;
				   csc->buffer1 = avc->mpeg_yuv_buffer->buffer1 ;
				   csc->buffer2 = avc->mpeg_yuv_buffer->buffer2 ;
				   csc->buffer3 = avc->mpeg_yuv_buffer->buffer3 ;
				   csc->buffer4 = avc->mpeg_yuv_buffer->buffer4 ;
				   csc->buffer5 = avc->mpeg_yuv_buffer->buffer5 ;
				   csc->buffer6 = avc->mpeg_yuv_buffer->buffer6 ;
				   csc->buffer7 = avc->mpeg_yuv_buffer->buffer7 ;
				}
				else {
				   csc->buffer0 = avc->mpeg_yuv_buffer->buffer2 ;
				   csc->buffer1 = avc->mpeg_yuv_buffer->buffer3 ;
				   csc->buffer2 = avc->mpeg_yuv_buffer->buffer0 ;
				   csc->buffer3 = avc->mpeg_yuv_buffer->buffer1 ;
				   csc->buffer4 = avc->mpeg_yuv_buffer->buffer6 ;
				   csc->buffer5 = avc->mpeg_yuv_buffer->buffer7 ;
				   csc->buffer6 = avc->mpeg_yuv_buffer->buffer4 ;
				   csc->buffer7 = avc->mpeg_yuv_buffer->buffer5 ;
				}

				result = sceMpegBaseCscAvc(frameBuffer[frame_index],0,512,csc);
				if ( result == 0 ) {
				   sceDisplayWaitVblankStart();
				   sceDisplaySetFrameBuf(frameBuffer[frame_index], 512, pixelFormat, PSP_DISPLAY_SETBUF_IMMEDIATE);
				   frame_index = (frame_index+1) % 2;
				}
			 }
		  }
		  ++frame_count;
		 if ( frame_count >= total_samples )
			playing = false;
	    }
		//delay the thread to allow other threads to continue if
		//some
		sceKernelDelayThread(100);
		//seek for button pressed
		sceCtrlReadBufferPositive(&button, 1);
		if (button.Buttons & stopButton){
			playing = false;
		}
	}

	//after playing we could close the file and clean up
	sceMpegDelete(&avc->mpeg);
	free(avc->mpeg_ddrtop);
	free_64(nal_buffer);
	free_64(avc->mpeg_au);
	free_64(avc->mpeg_buffer);
	delete(avc);
	delete(csc);

	mp4ff_close(mp4_handle);
	fclose(mp4_file);

	return 0;
}


uint32_t ClMp4Decoder::mp4_seek(void *user_data, uint64_t position){
	FILE** fp = (FILE**)user_data;
	return fseek(*fp, position, PSP_SEEK_SET );
}

uint32_t ClMp4Decoder::mp4_read(void *user_data, void *buffer, uint32_t length){
	FILE** fp = (FILE**)user_data;
	uint32_t res = fread(buffer, length, 1, *fp);
	return (res*length);
}

ClMp4Decoder::ClMp4Decoder() {
	// TODO Auto-generated constructor stub

}
ClMp4Decoder::~ClMp4Decoder() {
	// TODO Auto-generated destructor stub
}
