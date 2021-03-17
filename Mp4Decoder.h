/*
 * Mp4Decoder.h
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

#ifndef MP4DECODER_H_
#define MP4DECODER_H_

extern "C"{
#include <mp4ff.h>
}

/*
 * error codes returned by this class
 */
enum Mp4ErrorCode{
	MP4_ERROR_NOT_INITIALIZED    = 0x91000000,
	MP4_ERROR_AV_MODULE_AVCCODEC = 0x91000001,
	MP4_ERROR_MPEG_VSHPRX        = 0x91000002,
	MP4_ERROR_MPEG_INIT          = 0x91000003,
	MP4_ERROR_FILE_NOT_FOUND     = 0x91001001,
	MP4_ERROR_GET_AVC_SPS_SPP    = 0x91002001,
	MP4_ERROR_QUERY_MEM_SIZE     = 0x91002001,
	MP4_ERROR_ALLOC_MPEG_BUFFER  = 0x91002002,
	MP4_ERROR_MPEG_CREATE        = 0x91002003,
	MP4_ERROR_ALLOC_MPEG_AU      = 0x91002004,
	MP4_ERROR_ALLOC_INIT_AU      = 0x91002005
};

class ClMp4Decoder {
public:
	/**
	 * init
	 *
	 * @param prxFile - MPEG decoding PRX to be loaded during initialization
	 *                  the name should be provided including path relative to the
	 *                  eBoot.pbp location and should always start with /
	 * @return 0 if succesfull initialized, otherwise error code, see ::Mp4ErrorCode
	 */
	static int init(const char* prxFile = "/mpeg_vsh370.prx");

	/**
	 * playMp4
	 *
	 * @param filename - name of the file (incl. path) of the mp4 video file
	 * @param stopButton - the button which stops the playback of the video. If
	 *                     nothing provided the movie will be played from start
	 *                     to end before handing control back to the calling thread.
	 *                     However, other threads will still being executed.
	 *
	 * @return 0 if successfully played, otherwise error code, see ::Mp4ErrorCode
	 */
	static int playMp4(const char* filename, unsigned int stopButton, short pixelFormat = PSP_DISPLAY_PIXEL_FORMAT_8888);

protected:
	static bool initialized;
	static unsigned char* frameBuffer[];
	static mp4ff_callback_t mp4_callback;

	//call backs for reading data from mp4 file
	static uint32_t mp4_read(void *user_data, void *buffer, uint32_t length);
	static uint32_t mp4_seek(void *user_data, uint64_t position);

private:
	ClMp4Decoder();
	virtual ~ClMp4Decoder();
};

#endif /* MP4DECODER_H_ */
