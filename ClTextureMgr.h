/*
 * ClTextureMgr.h
 *
 * This class provides functionality to manage textures used in your
 * PSP GU homebrews
 *
 * Copyright (c) 2010 André Borrmann
 * Permission to copy, use, modify and distribute of this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to it's suitability for any purpose
 */

#ifndef TEXTUREMGR_H_
#define TEXTUREMGR_H_

// define the maximum size of texture we will allow to handle
#define TEX_MAX_WIDTH  (512)
#define TEX_MAX_HEIGHT (512)
#define MAX_TEX_COUNT  100
#define MAX_MIPMAPS 4

#include <vector>
extern "C"{
#include <pspgu.h>
}

using namespace std;
typedef struct MipMap{
	int width, height, stride;
	u32* data;
}MipMap;
/*
 * define a general structure how a texture would be
 * seen from outside this class
 */
typedef struct Texture{
	unsigned int id; //the id of the texture
	char*        name; //name of the texture
	int          type; //type of the texture - see GU_PSM* enumerations
	int          width, height;//real width and height of texture
	int          width2, height2; //width and height as power of 2
	int          stride; //buffer width of texture as power of 2
	bool         swizzled; //true if texture is swizzled
	void*        data; //pointer to texture data
	MipMap       mipmaps[MAX_MIPMAPS];
	int          mmCount;
	unsigned int* palette;
}Texture;

class ClTextureMgr {
public:
	/*
	 * static methods to retrieve the singleton and
	 * clean up the same
	 */
	static ClTextureMgr* getInstance();
	static void freeInstance();

	/*
	 * set the base path all textures will be loaded
	 * from. A path provided to the texture load method
	 * is relative to this one
	 */
	void setBasePath(const char* path);

	/*
	 * loads a texture from a PNG file
	 * usually creates GU_PSM_8888 textures, however, does support palette images as well now
	 * if possible this texture will be swizzled
	 */
	Texture* loadFromPNG(const char* filename, bool swizzle = true);

	/*
	 * load a texture and create mipmaps
	 */
	Texture* loadFromPNG(const char* filename, unsigned short mipmaps, bool swizzle = true);

	/*
	 * get a specific texture by id
	 */
	Texture* getTexture(unsigned int id);

	/*
	 * to optimize texture usage we couls set a texture to be in use. as long as the
	 * same texture will be used it will come from the same VRAM location to speed things up
	 */
	void inline useTexture(Texture* usedTex){
	/*   	//check the last used texture, if the same do nothing
			if (usedTex == textureInUse.tex){
				//just enable texture usage
				sceGuEnable(GU_TEXTURE_2D);
				return;
			}*/
			//they are different.
			//set the new texture as being used
			textureInUse.tex = usedTex;
			//setup the GU to use this texture
			sceGuEnable(GU_TEXTURE_2D);
			sceGuTexMapMode(GU_TEXTURE_COORDS,0,0);//GU_TEXTURE_MATRIX, 0, 0); //texture mapping using matrix instead u, v
			sceGuTexProjMapMode(GU_UV); //texture mapped based on the position of the objects relative to the texture

			sceGuTexMode(usedTex->type, 0,0,usedTex->swizzled);
			sceGuTexImage(0,usedTex->width,usedTex->height,usedTex->stride, usedTex->data);
			sceGuTexOffset(0.0f, 0.0f);
			sceGuTexScale(1.0f, 1.0f);
			sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);//apply RGBA value of texture
			sceGuTexFilter(GU_LINEAR,GU_LINEAR); //interpolate to have smooth borders
			sceGuTexWrap(GU_REPEAT,GU_REPEAT); //do repeat the texture if necessary
			// in the case we do have mipmaps available do set them as well
			if (usedTex->mmCount>0){
				sceGuTexLevelMode(GU_TEXTURE_AUTO, -2.0f);
				sceGuTexMode(usedTex->type, usedTex->mmCount,0, usedTex->swizzled);
				sceGuTexFilter(GU_LINEAR_MIPMAP_LINEAR,GU_LINEAR_MIPMAP_LINEAR);
				for (short m = 0; m<usedTex->mmCount;m++){
					sceGuTexImage(m+1,usedTex->mipmaps[m].width,usedTex->mipmaps[m].height,usedTex->mipmaps[m].stride,usedTex->mipmaps[m].data );
				}
			}

	 /*   	//first restore the origin data pointer in last used texture
			if (textureInUse.tex)
				textureInUse.tex->data = textureInUse.originData;

			//set the new texture as being used
			textureInUse.tex = usedTex;
			textureInUse.originData = usedTex->data;
			//set VRAM address as texture data pointer
			textureInUse.tex->data = (void*)((unsigned int)sceGeEdramGetAddr() + 0x401D4000);
			//copy texture data to VRAM
			sceGuCopyImage(usedTex->type, 0,0, usedTex->width, usedTex->height, usedTex->stride, textureInUse.originData, 0, 0, usedTex->stride, textureInUse.tex->data);
		*/
	}

	/*
	 * release a given texture
	 */
	void freeTexture(unsigned int id);

protected:
	/*
	 * swizzle the texture contents
	 */
	void swizzle(unsigned char* out, unsigned char* in, unsigned int width, unsigned int height);
	/*
	 * the constructor and destructor are protected to prevent
	 * direct instantiation of the class
	 */
	ClTextureMgr();
	virtual ~ClTextureMgr();

	/*
	 * all private members are coming next
	 */
private:
	char*  basePath;
	static ClTextureMgr* _instance; //the singleton instance
	int managedTextures; //counter of managed textures
	Texture* textures;   //the list of managed textures of size managedTextures

	std::vector<Texture*> textureList;

	struct TextureInUse {
		Texture* tex;     //texture data used containing new vram address
		void* originData; //stores original data pointer in ram
	}textureInUse;
};
#endif /* TEXTUREMGR_H_ */
