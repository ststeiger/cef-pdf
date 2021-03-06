
// A program to read, write, and crop BMP image files.
#include "Bmp.h"



//   Make a copy of a string on the heap.
// - Postcondition: the caller is responsible to free
//   the memory for the string.
char *_string_duplicate(const char *string)
{
	char *copy = (char*)malloc(sizeof(*copy) * (strlen(string) + 1));
	if (copy == NULL)
	{
		// return "Not enough memory for error message";
		const char* error_message = "Not enough memory for error message";
		size_t len = strlen(error_message);
		char* error = (char*)malloc(len * sizeof(char) + 1);
		strcpy(error, error_message);
		return error;
	}

	strcpy(copy, string);
	return copy;
}


// Check condition and set error message.
bool _check(bool condition, char **error, const char *error_message)
{
	bool is_valid = true;
	if (!condition)
	{
		is_valid = false;
		if (*error == NULL)  // to avoid memory leaks
		{
			*error = _string_duplicate(error_message);
		}
	}
	return is_valid;
}


//   Write an image to an already open file.
// - Postcondition: it is the caller's responsibility to free the memory
//   for the error message.
// - Return: true if and only if the operation succeeded.
bool write_bmp(FILE *fp, BMPImage *image, char **error)
{
	// Write header
	rewind(fp);
	size_t num_read = fwrite(&image->header, sizeof(image->header), 1, fp);
	if (!_check(num_read == 1, error, "Cannot write image"))
	{
		return false;
	}
	// Write image data
	num_read = fwrite(image->data, image->header.image_size_bytes, 1, fp);
	if (!_check(num_read == 1, error, "Cannot write image"))
	{
		return false;
	}

	return true;
}

// Free all memory referred to by the given BMPImage.
void free_bmp(BMPImage *image)
{
	free(image->data);
	free(image);
}


// Open file. In case of error, print message and exit.
FILE *_open_file(const char *filename, const char *mode)
{
	FILE *fp = fopen(filename, mode);
	if (fp == NULL)
	{
		fprintf(stderr, "Could not open file %s\n", filename);

		exit(EXIT_FAILURE);
	}

	return fp;
}

// Close file and release memory.void _clean_up(FILE *fp, BMPImage *image, char **error)
void _clean_up(FILE *fp, BMPImage *image, char **error)
{
	if (fp != NULL)
	{
		fclose(fp);
	}
	free_bmp(image);
	free(*error);
}


// Print error message and clean up resources.
void _handle_error(char **error, FILE *fp, BMPImage *image)
{
	fprintf(stderr, "ERROR: %s\n", *error);
	_clean_up(fp, image, error);

	exit(EXIT_FAILURE);
}

void write_image(const char *filename, BMPImage *image, char **error)
{
	FILE *output_ptr = _open_file(filename, "wb");

	if (!write_bmp(output_ptr, image, error))
	{
		_handle_error(error, output_ptr, image);
	}

	fflush(output_ptr);
	fclose(output_ptr);
	_clean_up(output_ptr, image, error);
}



//   Return the size of an image row in bytes.
// - Precondition: the header must have the width of the image in pixels.
uint32_t computeImageSize(BMPHeader *bmp_header)
{
	uint32_t bytes_per_pixel = bmp_header->bits_per_pixel / BITS_PER_BYTE;
	uint32_t bytes_per_row_without_padding = bmp_header->width_px * bytes_per_pixel;
	uint32_t padding = (4 - (bmp_header->width_px * bytes_per_pixel) % 4) % 4;

	uint32_t row_size_bytes = bytes_per_row_without_padding + padding;

	return row_size_bytes * bmp_header->height_px;
}



#ifdef USE_GDI

#pragma warning(disable:4189)
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		} // if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) 

	} // Next j 

	free(pImageCodecInfo);
	return -1;  // Failure
}


// https://github.com/lvandeve/lodepng

static bool notInitialized = true;


void WriteBitmapToFile(const char *filename, int width, int height, const void* buffer)
{
	// HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (notInitialized)
	{
		// https://docs.microsoft.com/en-us/windows/desktop/api/gdiplusinit/nf-gdiplusinit-gdiplusstartup
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::Status isOk = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		if (isOk != Gdiplus::Status::Ok)
		{
			printf("Failed on GdiplusStartup\n");
		}

		notInitialized = false;
		// defer
		// GdiplusShutdown(gdiplusToken);
	} // End if (notInitialized) 


	// https://docs.microsoft.com/en-us/windows/desktop/gdiplus/-gdiplus-constant-image-pixel-format-constants
	Gdiplus::Bitmap* myBitmap = new Gdiplus::Bitmap(width, height, width * 4, PixelFormat32bppARGB, (BYTE*)buffer);
	// myBitmap->RotateFlip(Gdiplus::Rotate180FlipY);




	CLSID pngClsid;

	// int result = GetEncoderClsid(L"image/tiff", &tiffClsid);
	int result = GetEncoderClsid(L"image/png", &pngClsid);
	printf("End GetEncoderClsid:\n");

	if (result == -1)
		printf("Error: GetEncoderClsid\n");
	// throw std::runtime_error("Bitmap::Save");

// if (Ok != myBitmap->Save(L"D\foobartest.png", &pngClsid)) printf("Error: Bitmap::Save");

// WTF ? I guess a standard C/C++-stream would have been too simple ? 
	IStream* oStream = nullptr;
	if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&oStream) != S_OK)
		printf("Error on creating an empty IStream\n");

	Gdiplus::EncoderParameters encoderParameters;

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;

	ULONG quality = 100;
	encoderParameters.Parameter[0].Value = &quality;



	// https://docs.microsoft.com/en-us/windows/desktop/api/gdiplusheaders/nf-gdiplusheaders-image-save(inistream_inconstclsid_inconstencoderparameters)
	if (Gdiplus::Status::Ok != myBitmap->Save(oStream, &pngClsid, &encoderParameters))
		printf("Error: Bitmap::Save\n");
	// throw std::runtime_error("Bitmap::Save");


	ULARGE_INTEGER ulnSize;
	LARGE_INTEGER lnOffset;
	lnOffset.QuadPart = 0;
	oStream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize);
	oStream->Seek(lnOffset, STREAM_SEEK_SET, NULL);

	uint8_t *pBuff = new uint8_t[(unsigned int)ulnSize.QuadPart];
	ULONG ulBytesRead;
	oStream->Read(pBuff, (ULONG)ulnSize.QuadPart, &ulBytesRead);


	FILE *output_ptr = _open_file(filename, "wb");
	fwrite((void*)pBuff, sizeof(uint8_t), (unsigned int)ulnSize.QuadPart, output_ptr);
	fflush(output_ptr);
	fclose(output_ptr);
	oStream->Release();


	delete pBuff;
	delete myBitmap;

	// https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp
	// std::string rotated_string = base64_encode((const unsigned char*)pBuff, ulnSize.QuadPart);	
}


#pragma warning(default:4189)

#else

// TODO: PNG-Encoder 
// https://github.com/lvandeve/lodepng
// https://lodev.org/lodepng/
BMPImage * CreateBitmapFromScan0(int32_t w, int32_t h, uint8_t* scan0)
{
	BMPImage *new_image = (BMPImage *)malloc(sizeof(*new_image));
	BMPHeader *header = (BMPHeader *)malloc(sizeof(*header));

	new_image->header = *header;
	new_image->header.type = MAGIC_VALUE;
	new_image->header.bits_per_pixel = BITS_PER_PIXEL;
	new_image->header.width_px = w;
	new_image->header.height_px = h;
	new_image->header.image_size_bytes = computeImageSize(&new_image->header);
	new_image->header.size = BMP_HEADER_SIZE + new_image->header.image_size_bytes;
	new_image->header.dib_header_size = DIB_HEADER_SIZE;
	new_image->header.offset = (uint32_t) sizeof(BMPHeader);
	new_image->header.num_planes = 1;
	new_image->header.compression = 0;
	new_image->header.reserved1 = 0;
	new_image->header.reserved2 = 0;
	new_image->header.num_colors = 0;
	new_image->header.important_colors = 0;

	new_image->header.x_resolution_ppm = 3780; // image->header.x_resolution_ppm;
	new_image->header.y_resolution_ppm = 3780; // image->header.y_resolution_ppm;

	new_image->data = (uint8_t*)malloc(sizeof(*new_image->data) * new_image->header.image_size_bytes);
	memcpy(new_image->data, scan0, new_image->header.image_size_bytes);

	return new_image;
}


void WriteBitmapToFile(const char *filename, int width, int height, const void* buffer)
{
	BMPImage * image = CreateBitmapFromScan0((int32_t)width, (int32_t)height, (uint8_t*)buffer);
	char *error = NULL;
	write_image(filename, image, &error);
}

#endif 
