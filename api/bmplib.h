#ifndef	BITMAP_CLASS_H
#define	BITMAP_CLASS_H

typedef unsigned char uchar; // We're lazy so typedef "unsigned char" as "uchar"

// We will use this class to load 24 and 32-bit .bmp for us
class DIB_BITMAP
{
	public:

		// Constructor() -- Zero's out DIB_BITMAP
		DIB_BITMAP():hbitmap(NULL),surface_bits(NULL),bmp_width(0),bmp_height(0),
					 bmp_channels(0),bmp_stride(0) { GdiFlush(); /* Guarantee that writing to
																 DIB_BITMAP is okay */ }

		// Data Access Functions ************
		
			inline int get_width() const { return bmp_width; } 
			inline int get_height() const { return bmp_height; }
			inline int get_channels() const { return bmp_channels; }
			inline int get_stride() const { return bmp_stride; }
					
		// ****** End of Data Access Functions

		// Creates a "empty" DIB_BITMAP with the "traits" of the parameters passed in
		// Returns true for success -- false otherwise
		// If set_size is called on a DIB_BITMAP that already has memory associated with it
		// that memory is freed and the new size is implemented
		bool set_size(int width, int height, int channels);

		// Loads a bmp with specified file_name -- Returns true on success, false otherwise
		// If loadBMP() is called on a DIB_BITMAP that already has memory associated with
		// it, that memory is freed and the .bmp is loaded
		bool loadBMP(const char *file_name);

		uchar* getLinePtr(int which_line); // returns a pointer to the line passed in
		
		// Deconstructor();
		~DIB_BITMAP();
		

	private:

		int bmp_width; // The width of the bitmap
		int bmp_height; // The height of the bitmap
		int bmp_channels; // How many channels is the bitmap (3 == 24-bit, 4 == 32-bit)
		int bmp_stride; // The TRUE number of bytes in a scan line (in a line of pixels
					   // in memory)

		HBITMAP hbitmap; // This will be the handle to our bitmap
		
		uchar *surface_bits; // This is a pointer to the actual pixels of the bitmap

		void FreeDIB_BMP(); // Frees all memory associated with DIB_BITMAP
};

#endif
