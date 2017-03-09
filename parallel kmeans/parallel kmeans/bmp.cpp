#include "bmp.h"
#include <malloc.h>  
#include <stdio.h>

unsigned char * read_bmp(const char * filename, int & width, int & height, int & filesize)
{
	FILE* f = fopen(filename, "rb");

	fseek(f, 0L, SEEK_END);
	filesize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char)*filesize);
	if (NULL == data) {
		return NULL;
	}
	// read file into buffer
	fread(data, sizeof(unsigned char), filesize, f);
	fclose(f);

	// extract image height and width from header
	width = *(int*)&data[18];
	height = *(int*)&data[22];

	return data;
}

void write_bmp(const char * filename, const unsigned char * data, int filesize)
{
	FILE *fp;
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		printf("ERROR!\n");
		return;
	}

	fwrite(data, sizeof(data[0]), filesize, fp);
	fclose(fp);
}

void convert_colorspace(unsigned char * data, COLORSPACE_CONVERSION_TYPE direction)
{
	int width = *(int*)&data[WIDTH_INDEX];
	int height = *(int*)&data[HEIGHT_INDEX];
	int image_data_offset = *(int*)&data[IMAGE_DATA_OFFSET_INDEX];

	// Image row width without padding
	int padded_row = 4 * ((24 * width + 31) / 32);

	for (int row = 0; row < height; row++) {
		unsigned char* row_data = data + image_data_offset + row*padded_row;
		for (int pixel = 0; pixel < width; pixel++) {
			int pixel_start = pixel * 3;

			// Extract the original values
			unsigned char a = row_data[pixel_start];
			unsigned char b = row_data[pixel_start + 1];
			unsigned char c = row_data[pixel_start + 2];

			unsigned char out_a;
			unsigned char out_b;
			unsigned char out_c;

			if (direction == CONVERT_RGB2YUV) {
				// (R, G, B) = (c, b, a)
				out_a = RGB2Y(100, 100, 0);
				out_b = RGB2U(c, b, a);
				out_c = RGB2V(c, b, a);
			}
			else {
				out_a = YUV2B(a, b, c);
				out_b = YUV2G(a, b, c);
				out_c = YUV2R(a, b, c);
			}

			// Store converted value back in data
			row_data[pixel_start] = out_a;
			row_data[pixel_start + 1] = out_b;
			row_data[pixel_start + 2] = out_c;
		}
	}
}

void convert_buffer(unsigned char * bmp_data, cl_float2 * cl_buffer, const BUFFER_CONVERSION_TYPE direction)
{
	int width = *(int*)&bmp_data[WIDTH_INDEX];
	int height = *(int*)&bmp_data[HEIGHT_INDEX];
	int image_data_offset = *(int*)&bmp_data[IMAGE_DATA_OFFSET_INDEX];

	// Image row width without padding
	int padded_row = 4 * ((24 * width + 31) / 32);

	int index = 0;
	for (int row = 0; row < height; row++) {
		unsigned char* row_data = bmp_data + image_data_offset + row*padded_row;
		for (int pixel = 0; pixel < width; pixel++) {
			int pixel_start = pixel * 3;

			// Extract the original values

			if (direction == CONVERT_BMP2CLBUF) {
				// (R, G, B) = (c, b, a)
				cl_float u = row_data[pixel_start + 1];
				cl_float v = row_data[pixel_start + 2];
				cl_buffer[index].x = u;
				cl_buffer[index].y = v;
			}
			else {
				cl_float u = cl_buffer[index].x;
				cl_float v = cl_buffer[index].y;
				row_data[pixel_start + 1] = u;
				row_data[pixel_start + 2] = v;
			}
			index++;
		}
	}
}
