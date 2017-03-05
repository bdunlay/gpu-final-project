#pragma once
#include "CL\cl.h"

// http://stackoverflow.com/questions/1737726/how-to-perform-rgb-yuv-conversion-in-c-c#14697130

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// RGB -> YUV
#define RGB2Y(R, G, B) CLIP(( (  66 * (R) + 129 * (G) +  25 * (B) + 128) >> 8) +  16)
#define RGB2U(R, G, B) CLIP(( ( -38 * (R) -  74 * (G) + 112 * (B) + 128) >> 8) + 128)
#define RGB2V(R, G, B) CLIP(( ( 112 * (R) -  94 * (G) -  18 * (B) + 128) >> 8) + 128)

// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

#define WIDTH_INDEX			18
#define HEIGHT_INDEX		22
#define IMAGE_DATA_OFFSET_INDEX	10

typedef enum {
	CONVERT_RGB2YUV,
	CONVERT_YUV2RGB
} COLORSPACE_CONVERSION_TYPE;

typedef enum {
	CONVERT_BMP2CLBUF,
	CONVERT_CLBUG2BMP
} BUFFER_CONVERSION_TYPE;

/*
Reads a BMP file into a buffer and returns a pointer to it.
*/
unsigned char* read_bmp(
	_In_ const char* filename, 
	_Out_ int& width, 
	_Out_ int& height, 
	_Out_ int& filesize);

/*
Writes a BMP file from a buffer.
*/
void write_bmp(
	_In_ const char* filename, 
	_In_ const unsigned char* data, 
	_In_ int filesize);

/*
Converts BMP colorspaces (RGB -> YUV or YUV -> RGB)
*/
void convert_colorspace(
	_Inout_ unsigned char* data,
	_In_ const COLORSPACE_CONVERSION_TYPE direction);

void convert_buffer(
	_Inout_ unsigned char* bmp_data,
	_Inout_ cl_uchar3* cl_buffer,
	_In_ const BUFFER_CONVERSION_TYPE direction);