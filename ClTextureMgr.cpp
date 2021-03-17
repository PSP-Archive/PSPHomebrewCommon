/*
 * TextureMgr.cpp
 *
 *  Copyright (C) 2010 André Borrmann
 *
 * This program is free software;
 * you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program;
 * if not, see <http://www.gnu.org/licenses/>.
 */

//#define PNG_NO_STDIO

extern "C"{
#include <pspkernel.h>
#include <malloc.h>
#include <stdlib.h>
#include <png.h>
#include <psptypes.h>
#include <pspgu.h>
}

#include <memory>
#include <string>

using namespace std;

#include "ClTextureMgr.h"
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
	//textures = (Texture*)malloc(MAX_TEX_COUNT*sizeof(Texture));
	textureInUse.originData = 0;
	textureInUse.tex = 0;

	basePath = 0;
}

ClTextureMgr *ClTextureMgr::getInstance()
{
	if (_instance == 0) {
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
Texture *ClTextureMgr::loadFromPNG(const char *filename, bool swizzle){
	//set up data for new texture

	//try to find an already existing texture with the same name
	//and return this instead of loading it twice
	for (short t=0;t<textureList.size();t++){
		if (textureList[t]->name){
			if (strncmp(filename, textureList[t]->name, strlen(filename))==0){
				return textureList[t];
			}
		}
	}
	//we need to create a new texture
	// get the pointer of the new texture data structure
	Texture* newTex = (Texture*)((malloc(sizeof (Texture))));
	newTex->mmCount = 0;
        newTex->name = (char*)((malloc(strlen(filename) + 1)));
        strcpy(newTex->name, filename);
        char *filenamepath; //filename+base path
        if(!basePath){
        	filenamepath = (char*)filename;//(char*)((malloc(strlen(filename) + 1)));
            //strcpy(filenamepath, filename);
        }else{
        	int len = strlen(filename) + strlen(basePath) + 2;
            filenamepath = (char*)(malloc(len));
            strcpy(filenamepath, basePath);
            strcat(filenamepath, "/");
            strcat(filenamepath, filename);
        }
        // now load the data into the structure
        // this code is based on the png load routines of the
        // graphics delivered with pspsdk
        png_structp png_ptr;
        png_infop info_ptr;
        unsigned int sig_read = 0;
        png_uint_32 width, height;
        int bit_depth, color_type, interlace_type, x, y;
        u32 *line;
        FILE *fp;
        //SceUID file;
        //try to open the file
        if ((fp = fopen(filenamepath, "rb")) == NULL){
        //if ((fp = fopen(pFilenamePath->c_str(), "rb")) == NULL){
			//if ((file = sceIoOpen(filenamepath, PSP_O_RDONLY, 0777 )) < 0){
			//if it fails release the allocated memory for this texture
			//and return NULL
			free(newTex);
			if (basePath)
				free(filenamepath);
		return NULL;
	}
        //initiate PNG read
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) {
		//if it fails release the allocated memory for this texture
		//and return NULL
		free(newTex);
		if (basePath)
			free(filenamepath);
		fclose(fp);
		//sceIoClose(file);
		return NULL;
	}
        //set the errorhandling callbacks for PNG read
        png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, (png_error_ptr) NULL);
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
		//if it fails release the allocated memory for this texture
		//and return NULL
		free(newTex);
		if (basePath)
			free(filenamepath);
		fclose(fp);
		//sceIoClose(file);
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
		if (basePath)
			free(filenamepath);
		fclose(fp);
		//sceIoClose(file);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
        //store the width and the height of the texture
        newTex->width = width;
        newTex->height = height;
        newTex->width2 = getNextPower2(width);
        newTex->height2 = getNextPower2(height);
        newTex->stride = getNextPower2(width);
        //set the kind of accessing the image data
        png_set_strip_16(png_ptr);
        png_set_packing(png_ptr);
        //handle different color types of file and convert them to a single RGB scheme
        if (color_type == PNG_COLOR_TYPE_PALETTE){
        	//png_set_palette_to_rgb(png_ptr);
        	newTex->palette = (unsigned int*)memalign(16, sizeof(unsigned int)*256);
        	png_color palette[256];
        	int palSize;
        	png_get_PLTE(png_ptr, info_ptr, (png_colorp*)&palette, &palSize);
        	for (int p=0;p<palSize;p++){
        		newTex->palette[p] = GU_RGBA(palette[p].red, palette[p].green, palette[p].blue, 0xff);
        	}
        }
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
        //handle the alpha channel
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
        //now allocate the memory for the image data
        //as we convert all color modes to RGB we will have 32Bit for each pixel
        if (color_type == PNG_COLOR_TYPE_PALETTE){
        	newTex->type = GU_PSM_T8;
        	newTex->data = (void*)((malloc(sizeof(u8)*newTex->stride*newTex->height)));
        } else {
			newTex->type = GU_PSM_8888;
			newTex->data = (void*)((malloc(sizeof (u32) * newTex->stride * newTex->height)));
        }
        //if we could not get the memory return null and free the memory already allocated
        if(!newTex->data){
		free(newTex);
		if (basePath)
			free(filenamepath);
		fclose(fp);
		//sceIoClose(file);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
        //now read the image date line by line from file into buffer
        line = (u32*)((malloc(width * 8)));
        for (y = 0; y < height; y++){
			png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
			for (x = 0;x <  width; x++){
				if (newTex->type == GU_PSM_T8){
					u8 color = ((u8*)line)[x];
					((u8*)newTex->data)[x + y*newTex->stride] = color;
				}else{
					u32 color = line[x];
					((u32*)newTex->data)[x + y*newTex->stride] = color;
				}
			}
        }
        free(line);
        //now we are ready
        //close the PNG read and the file
        png_read_end(png_ptr, info_ptr);
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(fp);
        if (basePath)
        	free(filenamepath);
        //return the texture
        managedTextures++;
        newTex->id = managedTextures;
        newTex->swizzled = false;
        //do not swizzle if the texture is to small
        if(newTex->width >= 32 && swizzle && newTex->type == GU_PSM_8888){
            //try to swizzle the data
            //first get target memory
            void *swizzleData = malloc(sizeof (u32) * newTex->stride * newTex->height);
            if(swizzleData){
                this->swizzle((unsigned char*)((swizzleData)), (unsigned char*)((newTex->data)), newTex->stride * sizeof (u32), newTex->height);
                newTex->swizzled = true;
                free(newTex->data);
                newTex->data = swizzleData;
            }else
                newTex->swizzled = false;

        }
        textureList.push_back(newTex);
        fclose(fp);
        return newTex;
    }

	Texture *ClTextureMgr::loadFromPNG(const char *filename, unsigned short  mipmaps, bool swizzle)
	{
		//first load original texture, but un swizzled in any case
		Texture* tex = loadFromPNG(filename, false);
		if (tex == NULL) return NULL;
		unsigned short m = mipmaps;
		if (mipmaps > 0){
			if (m > MAX_MIPMAPS) m = MAX_MIPMAPS;
			tex->mmCount = m;
			for (int l = 0; l < m; l++){
				int mwidth = tex->width >> (l+1);
				int mstride = tex->stride >> (l+1);
				int mheight = tex->height >> (l+1);
				if (mwidth < 16 || mheight < 16){
					tex->mmCount = l-1; //not as many mipmaps possible as requested...
					l = m;
				}else {
					//first assume GU_PSM_8888
					if (tex->type == GU_PSM_8888){
						tex->mipmaps[l].data = (u32*)malloc(sizeof(u32)*mstride*mheight);
						for (int x = 0; x< mwidth;x++){
							for (int y= 0; y <mheight;y++){
								((u32*)tex->mipmaps[l].data)[x+y*mstride] = ((u32*)tex->data)[(x<<(l+1))+(y<<(l+1))*tex->stride];
								//test: tint mipmaps with different color to get e clue when it will be used
								//((u32*)tex->mipmaps[l].data)[x+y*mwidth] |= (0x0000ff << (l*16));
							}
						}
						if (mwidth >= 32 && swizzle){
							void *swizzleData = malloc(sizeof (u32) * mstride * mheight);
							if(swizzleData){
								this->swizzle((u8*)((swizzleData)), (u8*)((tex->mipmaps[l].data)), mstride * sizeof (u32), mheight);
								free(tex->mipmaps[l].data);
								tex->mipmaps[l].data = (u32*)swizzleData;
							}
						}
						tex->mipmaps[l].width = mwidth;
						tex->mipmaps[l].height = mheight;
						tex->mipmaps[l].stride = mstride;
					}
				}
			}
		}
		//once all mipmaps are done we can swizzle the original
		if(tex->width >= 32 && swizzle && tex->type == GU_PSM_8888){
			//try to swizzle the data
			//first get target memory
			void *swizzleData = malloc(sizeof (u32) * tex->stride * tex->height);
			if(swizzleData){
				this->swizzle((unsigned char*)((swizzleData)), (unsigned char*)((tex->data)), tex->stride * sizeof (u32), tex->height);
				tex->swizzled = true;
				free(tex->data);
				tex->data = swizzleData;
			}else
				tex->swizzled = false;

		}
		return tex;
	}

    void ClTextureMgr::swizzle(unsigned char *out, unsigned char *in, unsigned int width, unsigned int height)
    {
        unsigned int i, j;
        unsigned int rowblocks = (width / 16);
        for(j = 0;j < height;++j){
            for(i = 0;i < width;++i){
                unsigned int blockx = i / 16;
                unsigned int blocky = j / 8;
                unsigned int x = (i - blockx * 16);
                unsigned int y = (j - blocky * 8);
                unsigned int block_index = blockx + ((blocky) * rowblocks);
                unsigned int block_address = block_index * 16 * 8;
                out[block_address + x + y * 16] = in[i + j * width];
            }
        }

    }

    Texture *ClTextureMgr::getTexture(unsigned int id)
    {
        //if the requested ID is out of bounds we cannot
        //return a valid value
        if (id > textureList.size()){
		return NULL;
	}
        return textureList[id - 1];
    }

    void ClTextureMgr::freeTexture(unsigned int id)
    {
    	if (textureList[id - 1]->data){
			free(textureList[id - 1]->data);
			textureList[id - 1]->data = 0;
			for (short m = 0; m< textureList[id-1]->mmCount;m++){
				free(textureList[id-1]->mipmaps[m].data);
			}
    	}
    	if (textureList[id - 1]->name){
    		free(textureList[id - 1]->name);
    		textureList[id - 1]->name = 0;
    	}
        //textureList.erase(textureList);
    }

    void ClTextureMgr::setBasePath(const char *path)
    {
        if(basePath)
            free(basePath);

        basePath = (char*)((malloc(strlen(path) + 1)));
        strcpy(basePath, path);
    }



ClTextureMgr::~ClTextureMgr() {
	// TODO Auto-generated destructor stub
	//during cleanup we need to free all allocated memory
	//free all managed texture data
	for (int t=0;t<textureList.size();t++){
		if(textureList[t]->data)
			free(textureList[t]->data);
		for (short m = 0; m< textureList[t]->mmCount;m++){
			free(textureList[t]->mipmaps[m].data);
		}
		//if (textureList[t]->name)
			//free(textureList[t]->name);
	}
	//release at the the memory of the texture list
	textureList.clear(); //does call the "destructor" of all childs
	                     //but may not release the allocated memory (?)
}
