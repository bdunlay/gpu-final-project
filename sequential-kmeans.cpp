#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// From http://stackoverflow.com/questions/1737726/how-to-perform-rgb-yuv-conversion-in-c-c#14697130
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

typedef unsigned int centroid_t;
#define centroid(u,v) (u << 8 | v)
#define get_x(c) (c>>24)
#define get_y(c) (c>>16 & 0xFF)
#define get_u(c) (c>>8 & 0xFF)
#define get_v(c) (c & 0xFF)

typedef enum {
    RGB2YUV,
    YUV2RGB
} DIRECTION;

unsigned char* ReadBMP(const char* filename, int& width, int& height, int& filesize) {
    FILE* f = fopen(filename, "rb");

    fseek(f, 0L, SEEK_END);
    filesize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    // unsigned char info[54];
    // fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    unsigned char* data = new unsigned char[filesize];
    fread(data, sizeof(unsigned char), filesize, f);
    fclose(f);

    // extract image height and width from header
    // int row_padded = width*3 + (4 - ((width * 3) % 4));
    width = *(int*)&data[18];
    height = *(int*)&data[22];

    return data;
}

void convertColorSpace(unsigned char* data, const int width, const int height, DIRECTION direction) {
    int image_data_offset = (int)data[0x0a];
    //int padded_row = width*3 + 4 - ((width * 3) % 4);
    int padded_row = 4*((24*width+31)/32);// - 1;

    for (int row = 0; row < height; row++) {
        unsigned char* row_data = data + image_data_offset + row*padded_row;
        for (int pixel = 0; pixel < width; pixel++) {
            int pixel_pos = pixel*3;

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
            } else {
                out_a = YUV2B(a, b, c);
                out_b = YUV2G(a, b, c);
                out_c = YUV2R(a, b, c);
            }

            // Store converted value back in data
            row_data[pixel_pos] =  out_a;
            row_data[pixel_pos + 1] = out_b;
            row_data[pixel_pos + 2] = out_c;
        }
    }
}

void writeFile(const char* filename, unsigned char* data, int filesize) {
    FILE *fp;
    fp=fopen(filename, "wb");
    if (fp == NULL) {
        printf("ERROR!\n");
        return;
    }

    fwrite(data, sizeof(data[0]), filesize, fp);
    fclose(fp);
}

centroid_t get_centroid(int x, int y,  unsigned char* data, const int width, const int height) {
    int image_data_offset = (int)data[0x0a];
    //int padded_row = width*3 + 4 - ((width * 3) % 4);
    int padded_row = 4*((24*width+31)/32);// - 1;

    unsigned char* pixel = &data[image_data_offset + padded_row*(y) + 3*(x)];

    return centroid(pixel[1],pixel[2]);
}

void clusterPixels(
        unsigned char* data,
        const int width,
        const int height,
        centroid_t* centroids,
        const int k,
        bool colorize = false) {
    int image_data_offset = (int)data[0x0a];
    int padded_row = 4*((24*width+31)/32);

    int* cluster_counts = (int*)calloc(k, sizeof(int));
    unsigned int* mean_centroids = (unsigned int*)calloc(2*k, sizeof(unsigned int));

    for (int row = 0; row < height; row++) {
        unsigned char* row_data = &data[image_data_offset + row*padded_row];
        for (int pixel = 0; pixel < width; pixel++) {
            int pixel_pos = pixel*3;

            // Extract the original values
            unsigned char u = row_data[pixel_pos + 1];
            unsigned char v = row_data[pixel_pos + 2];

            int distance = -1;
            int cluster = -1;

            for (int i = 0; i < k; i++) {
                unsigned char c_u = get_u(centroids[i]);
                unsigned char c_v = get_v(centroids[i]);
                int d = sqrt(pow((c_u-u),2) + pow((c_v-v),2));
                if (distance <= 1) {
                    cluster = i;
                    distance = d;
                } else if (d < distance) {
                    cluster = i;
                    distance = d;
                }
                mean_centroids[2*cluster + 0] += u;
                mean_centroids[2*cluster + 1] += v;
                cluster_counts[cluster]++;
            }

            if (colorize) {
              //  if (cluster != 0) {
              //      row_data[pixel_pos] = RGB2Y(0,0,0);
              //      row_data[pixel_pos+1] = RGB2U(0,0,0);
              //      row_data[pixel_pos+2] = RGB2V(0,0,0);
              //  } else {
                    unsigned char c_u = get_u(centroids[cluster]);
                    unsigned char c_v = get_v(centroids[cluster]);
                     // Extract the original values
                    // Store converted value back in data
                    row_data[pixel_pos] = RGB2Y(255,0,0);
                    row_data[pixel_pos + 1] = c_u;
                    row_data[pixel_pos + 2] = c_v;
              //  }
           }


        }
    }
    for (int i = 0; i < k; i++) {
        mean_centroids[2*i + 0] /= cluster_counts[i];
        mean_centroids[2*i + 1] /= cluster_counts[i];
        centroids[i] = mean_centroids[2*i + 0]<<8;
        centroids[i] |= mean_centroids[2*i + 1];
    }
    free(mean_centroids);
}

bool compareCentroids(centroid_t* centroids, centroid_t* cached_centroids, const int k) {
    for (int i = 0; i < k; i++) {
        if (get_u(cached_centroids[i]) != get_u(centroids[i])) {
            return true;
        }
        if (get_v(cached_centroids[i]) != get_v(centroids[i])) {
            return true;
        }
    }
    return false;
}

void kmeans(unsigned char* data, const int width, const int height, const int k, centroid_t* centroids) {
    centroid_t* cached_centroids = (centroid_t*)calloc(k, sizeof(centroid_t));

    // Cluster pixels, compute mean centroid of clusters, and compare
    // with previous centroids until respective centroids converge
    clock_t start = clock();

    do {
        memcpy(cached_centroids, centroids, k*sizeof(*cached_centroids));
        clusterPixels(data, width, height, centroids, k);
        // printf("\noriginal uv: (%u, %u)\n", get_u(cached_centroids[0]), get_v(cached_centroids[0]));
        // printf("average  uv: (%u, %u)\n", get_u(centroids[0]), get_v(centroids[0]));
    }
    while (compareCentroids(cached_centroids, centroids, k));

    printf("%.5f sec\n", (double)(clock()-start)/CLOCKS_PER_SEC);

    // Colorize
    clusterPixels(data, width, height, centroids, k, true);
    free(cached_centroids);
}


int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Error: missing parameters\n");
        printf("Required: k <image>\n");
        return -1;
    }

    const char write_filename[] = "image-out.bmp";

    int width = 0;
    int height = 0;
    int filesize = 0;

    // Read file, convert to YUV
    unsigned char* data = ReadBMP(argv[2], width, height, filesize);
    convertColorSpace(data, width, height, RGB2YUV);

    // Pick initial centroids
    int k = atoi(argv[1]);
    centroid_t* centroids = (centroid_t*)calloc(k, sizeof(centroid_t));
    for (int i = 0; i < k; i++) {
        int x,y;
        printf("x[%i]: ", i);
        fflush(stdout);
        std::cin >> x;

        printf("y[%i]: ", i);
        fflush(stdout);
        std::cin >> y;

        centroids[i] = get_centroid(x,y,data,width,height);
    }

    // Perform K-Means algorithm
    kmeans(data, width, height, k, centroids);

    // Convert to RGB, write file
    convertColorSpace(data, width, height, YUV2RGB);
    writeFile(write_filename, data, filesize);

    free(centroids);
    delete data;
    return 0;
}

// http://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file
