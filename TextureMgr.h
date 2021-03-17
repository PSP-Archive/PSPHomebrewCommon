/*
 * TextureMgr.h
 * This class provides functionality to manage textures used in your
 * PSP GU homebrews
 *
 * (c) André Borrmann 2010
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

/*
 * define a general structure how a texture would be
 * seen from outside this class
 */
typedef struct Texture{
	unsigned int id; //the id of the texture
	int          type; //type of the texture - see GU_PSM* enumerations
	int          width, height; //width and height as power of 2
	int          stride; //buffer width of texture as power of 2
	bool         swizzled; //true if texture is swizzled
	void*        data; //pointer to texture data
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
	 * loads a texture from a PNG file
	 * usually creates GU_PSM_8888 textures
	 */
	Texture* loadFromPNG(const char* filename);

	/*
	 * get a specific texture by id
	 */
	Texture* getTexture(unsigned int id);

protected:
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
	static ClTextureMgr* _instance; //the singleton instance
	int managedTextures; //counter of managed textures
	Texture* textures;   //the list of managed textures of size managedTextures

};

#endif /* TEXTUREMGR_H_ */
