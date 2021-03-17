/*
 * TextureMgr.cpp
 *
 *  Created on: 29.01.2010
 *      Author: andreborrmann
 */

extern "C"{
#include <malloc.h>
#include <stdlib.h>
#include <png.h>
#include <psptypes.h>
#include <pspgu.h>
}

#include "TextureMgr.h"

/*
 * define a simple static helper function to retrieve the next number
 * being power of 2 for the given one
 */
static int getNextPower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

//initialize the singleton instance to NIL
ClTextureMgr* ClTextureMgr::_instance = 0;

ClTextureMgr::ClTextureMgr() {
	// TODO Auto-generated constructor stub

	//initialize the member variables
	this->managedTextures = 0;
	this->textures = 0;

}

ClTextureMgr *ClTextureMgr::getInstance()
{
	if (!_instance) {
		_instance = new ClTextureMgr();
	}

	return _instance;
}

void ClTextureMgr::freeInstance(){
	if (_instance){
		delete(_instance);
	}
}

/*
 * load a new texture from PNG file
 */
Texture *ClTextureMgr::loadFromPNG(const char *filename){
	//set up data for new texture
	if (!managedTextures)
		textures = (Texture*)malloc(sizeof(Texture));
	else
		textures = (Texture*)realloc(textures, sizeof(Texture)*managedTextures+1);

	managedTextures++;

	// get the pointer of the new texture data structure
	Texture* newTex = &textures[managedTextures-1];

	// now load the data into the structure
	// this code is based on the png load routines of the
	// graphics delivered with pspsdk
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, x, y;
	u32* line;
	FILE *fp;

	//try to open the file
	if ((fp = fopen(filename, "rb")) == NULL){
		//if it fails release the allocated memory for this texture
		//and return NULL
		free(newTex);
		managedTextures--;
		return NULL;
	}
	//initiate PNG read
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		//if it fails release the allocated memory for this texture
		//and return NULL
		free(newTex);
		fclose(fp);
		managedTextures--;
		return NULL;
	}
	//set the errorhandling callbacks for PNG read
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, (png_error_ptr) NULL);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		//if it fails release the allocated memory for this texture
		//and return NULL
		free(newTex);
		fclose(fp);
		managedTextures--;
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	//prepare for reading
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	//read the info before image data
	png_read_info(png_ptr, info_ptr);
	//read image header
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	if (width > TEX_MAX_WIDTH || height > TEX_MAX_HEIGHT) {
		//if size of the image is to big
		//release the allocated memory for this texture
		//and return NULL
		free(newTex);
		fclose(fp);
		managedTextures--;
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}

	//store the width and the height of the texture
	newTex->width = getNextPower2(width);
	newTex->height = getNextPower2(height);
	newTex->stride = getNextPower2(width);

	//set the kind of accessing the image data
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	//handle different color types of file and convert them to a single RGB scheme
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	//handle the alpha channel
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	//now allocate the memory for the image data
	//as we convert all color modes to RGB we will have 32Bit for each pixel
	newTex->type = GU_PSM_8888;
	newTex->data = (void*)malloc(sizeof(u32)*newTex->stride*newTex->height);
	//if we could not get the memory return null and free the memory already allocated
	if(!newTex->data){
		free(newTex);
		fclose(fp);
		managedTextures--;
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	//now read the image date line by line from file into buffer
	line = (u32*)malloc(width*4);
	for (y = 0; y < height; y++){
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0;x <  width; x++){
			u32 color = line[x];
			((u32*)newTex->data)[x + y*newTex->stride] = color;
		}
	}
	free(line);

	//now we are ready
	//close the PNG read and the file
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);

	//return the texture
	newTex->id = managedTextures;
	newTex->swizzled = false;
	return newTex;
}


Texture *ClTextureMgr::getTexture(unsigned int id){
	//if the requested ID is out of bounds we cannot
	//return a valid value
	if (id > managedTextures){
		return NULL;
	}
	return &textures[id-1];
}

ClTextureMgr::~ClTextureMgr() {
	// TODO Auto-generated destructor stub
	//during cleanup we need to free all allocated memory
	//free all managed texture data
	for (int t=0;t<managedTextures;t++){
		if (textures[t].data) free(textures[t].data);
	}
	//release at the the memory of the texture list
	if (managedTextures)
		free(textures);
}
