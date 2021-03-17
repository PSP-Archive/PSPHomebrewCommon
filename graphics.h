#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <psptypes.h>

#define	PSP_LINE_SIZE 512
#define PSP_PIXEL_SIZE 4
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

typedef u32 Color;
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))

typedef struct
{
	int textureWidth;  // the real width of data, 2^n with n>=0
	int textureHeight;  // the real height of data, 2^n with n>=0
	int imageWidth;  // the image width
	int imageHeight;
	Color* data;
} Image;

/**
 * Load a PNG image.
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image to load
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
extern Image* loadImage(const char* filename);

/**
 * Create an empty image.
 *
 * @pre width > 0 && height > 0 && width <= 512 && height <= 512
 * @param width - width of the new image
 * @param height - height of the new image
 * @return pointer to a new allocated Image struct, all pixels initialized to color 0, or NULL on failure
 */
extern Image* createImage(int width, int height);

/**
 * Frees an allocated image.
 *
 * @pre image != null
 * @param image a pointer to an image struct
 */
extern void freeImage(Image* image);

/**
 * Initialize all pixels of an image with a color.
 *
 * @pre image != NULL
 * @param color - new color for the pixels
 * @param image - image to clear
 */
extern void clearImage(Color color, Image* image);

/**
 * Save an image or the screen in PNG format.
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image
 * @param data - start of Color type pixel data (can be getVramDisplayBuffer())
 * @param width - logical width of the image or SCREEN_WIDTH
 * @param height - height of the image or SCREEN_HEIGHT
 * @param lineSize - physical width of the image or PSP_LINE_SIZE
 * @param saveAlpha - if 0, image is saved without alpha channel
 */
extern void saveImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha);

/**
 * Draw a line to screen.
 *
 * @pre x0 >= 0 && x0 < image->imageWidth && y0 >= 0 && y0 < image->imageHeight &&
 *      x1 >= 0 && x1 < image->imageWidth && y1 >= 0 && y1 < image->imageHeight
 * @param x0 - x line start position
 * @param y0 - y line start position
 * @param x1 - x line end position
 * @param y1 - y line end position
 */
extern void drawLineImage(int x0, int y0, int x1, int y1, Color color, Image* image);

#endif
