#include <cstdio>

#ifdef _WIN32
#include "windows.h"
#endif

#include "bmplib.h"

// Returns true for success -- false otherwise
bool DIB_BITMAP::set_size(int width, int height, int channels)	
{
 
	// If DIB_BITMAP has already been set -- clear it out first
	FreeDIB_BMP();

	// Create a temporary compatible device context
	HDC temp_hdc = CreateCompatibleDC(NULL);
		
		// Error Check
		if(!temp_hdc)
			return false;

	bmp_width = width; // Set the width
	bmp_height = height; // Set the height
	bmp_channels = channels; // Set the channels (3 == 24-bit, 4 == 32-bit)
	
	// Set stride -- The stride is the TRUE number of bytes in a line of pixels
	// Windows makes all the .bmps DWORD aligned (divisible evenly by 4)
	// So if you bitmap say was 103x103 pixels, Windows would add 1 "padding byte" to it
	// so in memory it would be 104x103 pixels.  The "padding bytes" do not get blit (drawn)
	// to the screen, they're just there so again everything is DWORD aligned which makes 
	// blitting (drawing to the screen) easier for the OS
	bmp_stride = bmp_width * bmp_channels;

	while((bmp_stride % 4) != 0) // Ensure bmp_stride is DWORD aligned
		bmp_stride++;
		
	BITMAPINFO bmp_info = {0};

	// Initialize the parameters that we care about
	bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmp_info.bmiHeader.biWidth = width;
	bmp_info.bmiHeader.biHeight = height;
	bmp_info.bmiHeader.biPlanes = 1; // Always equal 1
	bmp_info.bmiHeader.biBitCount = channels * 8;
	bmp_info.bmiHeader.biCompression = BI_RGB; // No compression
	bmp_info.bmiHeader.biClrUsed = 0; // Always equals 0 with a 24 or 32-bit .bmp

	// Create a DIBSection -- This returns us two things, an HBITMAP handle and
	// a memory pointer (pointer to the pixels) in surface_bits
	hbitmap = CreateDIBSection(temp_hdc, &bmp_info, DIB_RGB_COLORS,
							   (void**)&surface_bits, 0, 0);

	// Release our temporary HDC
	DeleteDC(temp_hdc);

		// Error Check -- Make sure the call to CreateDIBSection() DID NOT fail
		if(!hbitmap)
			return false;		
		
	return true; // We're sized :)

} // end of set_size(int width, int height, int channels)

bool DIB_BITMAP::loadBMP(const char *file_name)
{
	// If DIB_BITMAP has already been set -- clear it out first
	FreeDIB_BMP();

		// Error Check -- Make sure they passed in a valid file name
		if(!file_name)
			return false;

	FILE *bmp_file = fopen(file_name, "rb");

		// Error Check -- Make sure the file could be opened
		if(!bmp_file)
			return false;

	BITMAPFILEHEADER bmp_fileheader;

	// Read the BITMAPFILEHEADER
	if(!fread(&bmp_fileheader, sizeof(BITMAPFILEHEADER), 1, bmp_file))
	{
		fclose(bmp_file);
			return false;
	}

	// Check the type field to make sure we have a .bmp file
	if(memcmp(&bmp_fileheader.bfType, "BM", 2))
	{
		fclose(bmp_file);
			return false;
	}

	BITMAPINFOHEADER bmp_infoheader;
	
	// Read the BITMAPINFOHEADER.
	if(!fread(&bmp_infoheader, sizeof(BITMAPINFOHEADER), 1, bmp_file))
	{
		fclose(bmp_file);
			return false;
	}

	// We only support 24-bit and 32-bit .bmps so make sure that's what we have
	if((bmp_infoheader.biBitCount != 24) && (bmp_infoheader.biBitCount != 32))
	{
		fclose(bmp_file);
			return false;
	}

	// Set the size of our DIB_BITMAP, once we do this we're ready to store the pixel
	// data in it
	if(set_size(bmp_infoheader.biWidth,bmp_infoheader.biHeight,bmp_infoheader.biBitCount / 8) == false)
	{
		fclose(bmp_file);
			return false;
	}

	// Jump to the location where the pixel data is stored
	if(fseek(bmp_file, bmp_fileheader.bfOffBits, SEEK_SET))
	{
		fclose(bmp_file);
			return false;
	}
	
	unsigned int bytesPerLine = bmp_width * bmp_channels; // Bytes per line (number of bytes
														 // in a scan line)
	
	// Calculate how many "padding" bytes there are -- WE DO NOT want to read in the
	// padding bytes (we will just skip over those)
	// **Remember** Windows adds padding bytes to ensure ALL .bmps are DWORD aligned
	// (divisible evenly by 4)
	unsigned int padding = bmp_stride - bytesPerLine;

		// Loop over all the scan lines (all the rows of pixels in the image)
		for(int y = bmp_height-1; y >= 0; y--)
		{
			// Get the "current" line pointer
			uchar *LinePtr = getLinePtr(y);

			// Read the precise number of bytes that the scan line requires into the bitmap
			if(!fread(LinePtr, bytesPerLine, 1, bmp_file))
			{
				fclose(bmp_file);
					return false;
			}

			// Skip over any padding bytes.
			if(fseek(bmp_file, padding, SEEK_CUR))
			{
				fclose(bmp_file);
					return false;
			}
	
		} // end of for (int y = 0; y < bmp_infoheader.biHeight; y++)


	fclose(bmp_file);
		return true; // If we get here .bmp was read in successfully

} // end of loadBMP(char *file_name, HDC hdc)

// Returns the address in memory of the specified line. This gives you a pointer to at least 
// width * channels bytes. Lines are numbered such that when the bitmap
// is displayed line zero is at the top.
uchar* DIB_BITMAP::getLinePtr(int which_line)
{
	return (surface_bits + bmp_stride * which_line);
}

// Release the memory
void DIB_BITMAP::FreeDIB_BMP() 
{
	// If we created an HBITMAP, delete it
	if(hbitmap)
		DeleteObject(hbitmap);

	// Zero out all data associated with DIB_BITMAP
	hbitmap = NULL;
	surface_bits = NULL;
	bmp_width = bmp_height = bmp_channels = bmp_stride = 0;
		return;
}

// Deconstructor
DIB_BITMAP::~DIB_BITMAP() { FreeDIB_BMP(); }
	
