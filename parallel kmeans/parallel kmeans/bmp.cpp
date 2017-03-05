#include "bmp.h"

unsigned char * read_bmp(const char * filename, int & width, int & height, int & filesize)
{
	FILE* f = fopen(filename, "rb");

	fseek(f, 0L, SEEK_END);
	filesize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	unsigned char* data = new unsigned char[filesize];
	fread(data, sizeof(unsigned char), filesize, f);
	fclose(f);

	// extract image height and width from header
	width = *(int*)&data[18];
	height = *(int*)&data[22];

	return data;
}

void write_bmp(const char * filename, unsigned char * data, int filesize)
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

void convert_colorspace(unsigned char * data, const int width, const int height, CONVERSION_TYPE direction)
{
	int image_data_offset = (int)data[0x0a];
	int padded_row = 4 * ((24 * width + 31) / 32);

	for (int row = 0; row < height; row++) {
		unsigned char* row_data = data + image_data_offset + row*padded_row;
		for (int pixel = 0; pixel < width; pixel++) {
			int pixel_pos = pixel * 3;

			// Extract the original values
			unsigned char a = row_data[pixel_pos];
			unsigned char b = row_data[pixel_pos + 1];
			unsigned char c = row_data[pixel_pos + 2];

			unsigned char out_a;
			unsigned char out_b;
			unsigned char out_c;

			if (direction == RGB2YUV) {
				// (R, G, B) = (c, b, a)
				out_a = RGB2Y(c, b, a);
				out_b = RGB2U(c, b, a);
				out_c = RGB2V(c, b, a);
			}
			else {
				out_a = YUV2B(a, b, c);
				out_b = YUV2G(a, b, c);
				out_c = YUV2R(a, b, c);
			}

			// Store converted value back in data
			row_data[pixel_pos] = out_a;
			row_data[pixel_pos + 1] = out_b;
			row_data[pixel_pos + 2] = out_c;
		}
	}
}
