
#include <cstdio>
#include <cstdlib> 
#include <cstring>

using std::malloc;
using std::free;
using std::size_t;

using std::FILE;
using std::fopen;
using std::fclose;
using std::fprintf;
using std::fread;
using std::fseek;

using std::perror;

void bwtorgba(unsigned char *b,unsigned char *l,int n) {
    while(n--) {
		l[0] = *b;
		l[1] = *b;
		l[2] = *b;
		l[3] = 0xff;
		l += 4; 
		b++;
    }
}

void latorgba(unsigned char *b, unsigned char *a,unsigned char *l,int n) {
    while(n--) {
		l[0] = *b;
		l[1] = *b;
		l[2] = *b;
		l[3] = *a;
		l += 4; 
		b++; 
		a++;
	}
}

void rgbtorgba(unsigned char *r,unsigned char *g,unsigned char *b,unsigned char *l,int n) {
    while(n--) {
		l[0] = r[0];
		l[1] = g[0];
		l[2] = b[0];
		l[3] = 0xff;
		l += 4; 
		r++; 
		g++; 
		b++;
	}
}

void rgbatorgba(unsigned char *r,unsigned char *g,unsigned char *b,unsigned char *a,unsigned char *l,int n) {
    while(n--) {
		l[0] = r[0];
		l[1] = g[0];
		l[2] = b[0];
		l[3] = a[0];
		l += 4; 
		r++; 
		g++; 
		b++; 
		a++;
    }
}

typedef struct _ImageRec {
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short xsize, ysize, zsize;
    unsigned int min, max;
    unsigned int wasteBytes;
    char name[80];
    unsigned long colorMap;
    FILE *file;
    unsigned char *tmp, *tmpR, *tmpG, *tmpB;
    unsigned long rleEnd;
    unsigned int *rowStart;
    int *rowSize;
} ImageRec;

static void ConvertShort(unsigned short *array, long length) {
    unsigned b1, b2;
    unsigned char *ptr;
    ptr = (unsigned char *) array;
    while (length--) {
		b1 = *ptr++;
		b2 = *ptr++;
		*array++ = (b1 << 8) | (b2);
	}
}

static void ConvertLong(unsigned *array, long length) {
    unsigned long b1, b2, b3, b4;
    unsigned char *ptr;
    ptr = (unsigned char *)array;
    while (length--) {
		b1 = *ptr++;
		b2 = *ptr++;
		b3 = *ptr++;
		b4 = *ptr++;
		*array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
	}
}

static ImageRec *ImageOpen(const char *fileName){
    union {
		int testWord;
		char testByte[4];
	} 
	endianTest;
    ImageRec *image;
    int swapFlag;
    int x;
    endianTest.testWord = 1;
    if (endianTest.testByte[0] == 1) {
		swapFlag = 1;
    } 
	else {
		swapFlag = 0;
    }
    image = (ImageRec *)malloc(sizeof(ImageRec));
    if (image == NULL) {
		goto error;
    }
    if ((image->file = fopen(fileName, "rb")) == NULL) {
		perror(fileName);
        free(image);
		return NULL;
    }
    fread(image, 1, 12, image->file);
    if (swapFlag) {
		ConvertShort(&image->imagic, 6);
    }

    image->tmp  = (unsigned char *)malloc(image->xsize*256);
    image->tmpR = (unsigned char *)malloc(image->xsize*256);
    image->tmpG = (unsigned char *)malloc(image->xsize*256);
    image->tmpB = (unsigned char *)malloc(image->xsize*256);
    if (image->tmp == NULL || image->tmpR == NULL || image->tmpG == NULL ||image->tmpB == NULL) {
        goto error;
    }

    if ((image->type & 0xFF00) == 0x0100) {
		x = image->ysize * image->zsize * sizeof(unsigned);
		image->rowStart = (unsigned *)malloc(x);
		image->rowSize = (int *)malloc(x);
		if (image->rowStart == NULL || image->rowSize == NULL) {
            goto error;
		}
		image->rleEnd = 512 + (2 * x);
		fseek(image->file, 512, SEEK_SET);
		fread(image->rowStart, 1, x, image->file);
		fread(image->rowSize, 1, x, image->file);
		if (swapFlag) {
			ConvertLong(image->rowStart, x/sizeof(unsigned));
			ConvertLong((unsigned *)image->rowSize, x/sizeof(int));
		}
    }
    return image;
error:
    if (image) { 
        if (image->rowSize) free(image->rowSize); 
        if (image->rowStart) free(image->rowStart); 
        if (image->tmpB) free(image->tmpB); 
        if (image->tmpG) free(image->tmpG); 
        if (image->tmpR)free(image->tmpR); 
        if (image->tmp) free(image->tmp); 
        if (image->file) fclose(image->file); 
        free(image); 
    } 
    fprintf(stderr, "Out of memory!\n"); 
    return NULL; 
}

static void ImageClose(ImageRec *image) {
    fclose(image->file);
    free(image->tmp);
    free(image->tmpR);
    free(image->tmpG);
    free(image->tmpB);
    free(image);
}

static void ImageGetRow(ImageRec *image, unsigned char *buf, int y, int z) {
    unsigned char *iPtr, *oPtr, pixel;
    int count;
    if ((image->type & 0xFF00) == 0x0100) {
		fseek(image->file, image->rowStart[y+z*image->ysize], SEEK_SET);
		fread(image->tmp, 1, (unsigned int)image->rowSize[y+z*image->ysize],image->file);
		iPtr = image->tmp;
		oPtr = buf;
		while (1) {
			pixel = *iPtr++;
			count = (int)(pixel & 0x7F);
			if (!count) {
				return;
			}
			if (pixel & 0x80) {
				while (count--) {
					*oPtr++ = *iPtr++;
				}
			} 
			else {
				pixel = *iPtr++;
				while (count--) {
					*oPtr++ = pixel;
				}
			}
		}
    } 
	else {
		fseek(image->file, 512+(y*image->xsize)+(z*image->xsize*image->ysize),SEEK_SET);
		fread(buf, 1, image->xsize, image->file);
    }
}

//-------------------------TGA LOADER-------------------------

//	=============
//	checkSize
//	Make sure its a power of 2.
//	=============
int checkSize (int x){
	return 1;
	if (x == 2	 || x == 4   || 
        x == 8	 || x == 16  || 
        x == 32  || x == 64  ||
        x == 128 || x == 256 || x == 512)
		return 1;
    else 
		return 0;
}


unsigned int texFormat;

unsigned char* getRGBA (FILE *s, size_t size){
    unsigned char *rgba;
    unsigned char temp;
    size_t bread;
    size_t i;
    rgba=(unsigned char*)malloc(size * 4); 
    if (rgba == NULL) return 0;
    bread = fread (rgba, sizeof (unsigned char), size * 4, s); 
    // TGA is stored in BGRA, make it RGBA 
    if (bread != size * 4) {
        free (rgba);
        return 0;
    }
    for (i = 0; i < size * 4; i += 4 ){
        temp = rgba[i];
        rgba[i] = rgba[i + 2];
        rgba[i + 2] = temp;
    }
    return rgba;
}


//	=============
//	getRGB
//	Reads in RGB data for a 24bit image. 
//	=============
unsigned char* getRGB (FILE *s, size_t size){
    unsigned char *rgb;
    unsigned char temp;
    size_t bread;
    size_t i;
    rgb=(unsigned char*)malloc(size * 3); 
    if (rgb == NULL) return 0;
    bread = fread (rgb, sizeof (unsigned char), size * 3, s);
    if (bread != size * 3){
        free (rgb);
        return 0;
    }
    // TGA is stored in BGR, make it RGB 
    for (i = 0; i < size * 3; i += 3){
        temp = rgb[i];
        rgb[i] = rgb[i + 2];
        rgb[i + 2] = temp;
    }
    return rgb;
}


//	=============
//	getGray
//	Gets the grayscale image data.  Used as an alpha channel.
//	=============
unsigned char* getGray (FILE *s, size_t size){
    unsigned char *grayData;
    size_t bread;
    grayData=(unsigned char*)malloc (size);
    if (grayData == NULL) return 0;
    bread = fread (grayData, sizeof (unsigned char), size, s);
    if (bread != size){
        free (grayData);
        return 0;
    }
    return grayData;
}


//	=============
//	getData
//	Gets the image data for the specified bit depth.
//	=============
unsigned char* getData (FILE *s, int sz, int iBits){
	switch (iBits){
	case 32 : return getRGBA (s, sz); break;
	case 24 : return getRGB  (s, sz); break;
	case 8  : return getGray (s, sz); break;
	default : return NULL;
	}
}


unsigned * read_tga_texture(char *name, int *width, int *height, int*) {   
    unsigned char type[4];
    unsigned char info[7];    
    unsigned *base;
	int imageBits, size;
    FILE *s;
	if (!(s = fopen (name, "r+bt"))) return NULL;
	fread (&type, sizeof (char), 3, s);  // read in colormap info and image type, byte 0 ignored
    fseek (s, 12, SEEK_SET);			 // seek past the header and useless info
    fread (&info, sizeof (char), 6, s);
	if (type[1] != 0 || (type[2] != 2 && type[2] != 3)) return NULL;
	(*width)  = info[0] + info[1] * 256; 
    (*height) = info[2] + info[3] * 256;
    imageBits =	info[4]; 
    size = (*width) * (*height); 
	// make sure dimension is a power of 2 
    if (!checkSize ((*width)) || !checkSize ((*height) )) return NULL;
	// make sure we are loading a supported type 
    if (imageBits != 32 && imageBits != 24 && imageBits != 8) return NULL;
	base = (unsigned int *)getData (s, size, imageBits);
	fclose(s);
	return (unsigned *) base;
}

unsigned * read_rgb_texture(const char *name, int *width, int *height, int *components) {
    unsigned *base, *lptr;
    unsigned char *rbuf, *gbuf, *bbuf, *abuf;
    ImageRec *image;
    int y;
    image = ImageOpen(name);
    if(!image) return NULL;
    (*width)=image->xsize;
    (*height)=image->ysize;
    (*components)=image->zsize;
    base = (unsigned *)malloc(image->xsize*image->ysize*sizeof(unsigned));
    rbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    gbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    bbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    abuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    if(!base || !rbuf || !gbuf || !bbuf) goto error;
    lptr = base;
	for(y=0; y<image->ysize; y++) {
		if(image->zsize>=4) {
			ImageGetRow(image,rbuf,y,0);
			ImageGetRow(image,gbuf,y,1);
			ImageGetRow(image,bbuf,y,2);
			ImageGetRow(image,abuf,y,3);
			rgbatorgba(rbuf,gbuf,bbuf,abuf,(unsigned char *)lptr,image->xsize);
			lptr += image->xsize;
		} else if(image->zsize==3) {
			ImageGetRow(image,rbuf,y,0);
			ImageGetRow(image,gbuf,y,1);
			ImageGetRow(image,bbuf,y,2);
			rgbtorgba(rbuf,gbuf,bbuf,(unsigned char *)lptr,image->xsize);
			lptr += image->xsize;
		} else if(image->zsize==2) {
			ImageGetRow(image,rbuf,y,0);
			ImageGetRow(image,abuf,y,1);
			latorgba(rbuf,abuf,(unsigned char *)lptr,image->xsize);
			lptr += image->xsize;
		} else {
			ImageGetRow(image,rbuf,y,0);
			bwtorgba(rbuf,(unsigned char *)lptr,image->xsize);
			lptr += image->xsize;
		}
    }
    ImageClose(image);
    free(rbuf);
    free(gbuf);
    free(bbuf);
    free(abuf);
    return (unsigned *) base;
error:
 	ImageClose(image); 
 	if (abuf) free(abuf); 
 	if (bbuf) free(bbuf); 
 	if (bbuf) free(gbuf); 
 	if (bbuf) free(rbuf); 
 	if (base) free(base); 
 	return NULL; 
}

