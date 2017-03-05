#pragma once
#include <stdio.h>

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

typedef enum {
	RGB2YUV,
	YUV2RGB
} CONVERSION_TYPE;


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
	_In_ const int width, 
	_In_ const int height, 
	_In_ const CONVERSION_TYPE direction);