#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "bmp24.h"

t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur ouverture image");
        return NULL;
    }

    // Lire header (14 octets) + header_info (40 octets)
    t_bmp_header header;
    fread(&header, sizeof(t_bmp_header), 1, file);

    t_bmp_info header_info;
    fread(&header_info, sizeof(t_bmp_info), 1, file);

    // Vérifier profondeur

    if (header_info.bits != 24) {
        printf("Erreur : image non 24 bits !\n");
        fclose(file);
        return NULL;
    }

    // Allouer structure
    t_bmp24 *img = bmp24_allocate(header_info.width, header_info.height, 24);
    if (!img) {
        fclose(file);
        return NULL;
    }

    img->header = header;
    img->header_info = header_info;

    // Se placer à l'offset des données
    fseek(file, header.offset, SEEK_SET);

    // Lire pixels (format BMP : lignes inversées et BGR)
    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            uint8_t bgr[3];
            fread(bgr, sizeof(uint8_t), 3, file);
            img->data[y][x].blue = bgr[0];
            img->data[y][x].green = bgr[1];
            img->data[y][x].red = bgr[2];
        }
    }

    fclose(file);
    return img;
}




void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Erreur sauvegarde");
        return;
    }

    // Écrire les headers
    fwrite(&img->header, sizeof(t_bmp_header), 1, file);
    fwrite(&img->header_info, sizeof(t_bmp_info), 1, file);

    // Se placer à l'offset des données
    fseek(file, img->header.offset, SEEK_SET);

    // Écrire pixels (en BGR, ligne du bas en premier)
    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            uint8_t bgr[3] = {
                img->data[y][x].blue,
                img->data[y][x].green,
                img->data[y][x].red
            };
            fwrite(bgr, sizeof(uint8_t), 3, file);
        }
    }

    fclose(file);
}






t_pixel **bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = malloc(height * sizeof(t_pixel *));
    if (!pixels) return NULL;

    for (int i = 0; i < height; i++) {
        pixels[i] = malloc(width * sizeof(t_pixel));
        if (!pixels[i]) {
            for (int j = 0; j < i; j++) free(pixels[j]);
            free(pixels);
            return NULL;
        }
    }
    return pixels;
}

void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

t_bmp24 *bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = malloc(sizeof(t_bmp24));
    if (!img) return NULL;

    img->data = bmp24_allocateDataPixels(width, height);
    if (!img->data) {
        free(img);
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;
    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (img) {
        bmp24_freeDataPixels(img->data, img->height);
        free(img);
    }
}

void bmp24_negative(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue = 255 - img->data[y][x].blue;
        }
    }
}

void bmp24_grayscale(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t r = img->data[y][x].red;
            uint8_t g = img->data[y][x].green;
            uint8_t b = img->data[y][x].blue;
            uint8_t gray = (r + g + b) / 3;
            img->data[y][x].red = gray;
            img->data[y][x].green = gray;
            img->data[y][x].blue = gray;
        }
    }
}

void bmp24_brightness(t_bmp24 *img, int value) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int r = img->data[y][x].red + value;
            int g = img->data[y][x].green + value;
            int b = img->data[y][x].blue + value;

            img->data[y][x].red = r < 0 ? 0 : (r > 255 ? 255 : r);
            img->data[y][x].green = g < 0 ? 0 : (g > 255 ? 255 : g);
            img->data[y][x].blue = b < 0 ? 0 : (b > 255 ? 255 : b);
        }
    }
}


static float kernel_box[3][3] = {
    {1/9.0, 1/9.0, 1/9.0},
    {1/9.0, 1/9.0, 1/9.0},
    {1/9.0, 1/9.0, 1/9.0}
};

static float kernel_gaussian[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}
};

static float kernel_outline[3][3] = {
    {-1, -1, -1},
    {-1,  8, -1},
    {-1, -1, -1}
};

static float kernel_emboss[3][3] = {
    {-2, -1,  0},
    {-1,  1,  1},
    { 0,  1,  2}
};

static float kernel_sharpen[3][3] = {
    { 0, -1,  0},
    {-1,  5, -1},
    { 0, -1,  0}
};

t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float **kernel, int kernelSize) {
    int n = kernelSize / 2;
    float r = 0, g = 0, b = 0;

    for (int i = -n; i <= n; i++) {
        for (int j = -n; j <= n; j++) {
            int xi = x + i;
            int yj = y + j;

            if (xi >= 0 && xi < img->width && yj >= 0 && yj < img->height) {
                t_pixel p = img->data[yj][xi];
                float k = kernel[i + n][j + n];
                r += p.red * k;
                g += p.green * k;
                b += p.blue * k;
            }
        }
    }

    t_pixel result;
    result.red = fmin(fmax(round(r), 0), 255);
    result.green = fmin(fmax(round(g), 0), 255);
    result.blue = fmin(fmax(round(b), 0), 255);
    return result;
}

static void apply_filter(t_bmp24 *img, float kernel[3][3]) {
    int w = img->width;
    int h = img->height;
    t_pixel **newData = malloc(h * sizeof(t_pixel*));

    float *k[3] = {kernel[0], kernel[1], kernel[2]};

    for (int y = 0; y < h; y++) {
        newData[y] = malloc(w * sizeof(t_pixel));
        for (int x = 0; x < w; x++) {
            newData[y][x] = bmp24_convolution(img, x, y, k, 3);
        }
    }

    for (int y = 0; y < h; y++) {
        memcpy(img->data[y], newData[y], w * sizeof(t_pixel));
        free(newData[y]);
    }
    free(newData);
}

void bmp24_boxBlur(t_bmp24 *img)    { apply_filter(img, kernel_box); }
void bmp24_gaussianBlur(t_bmp24 *img) {
    float gk[3][3];
    float norm = 16.0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            gk[i][j] = kernel_gaussian[i][j] / norm;
    apply_filter(img, gk);
}
void bmp24_outline(t_bmp24 *img)    { apply_filter(img, kernel_outline); }
void bmp24_emboss(t_bmp24 *img)     { apply_filter(img, kernel_emboss); }
void bmp24_sharpen(t_bmp24 *img)    { apply_filter(img, kernel_sharpen); }


void bmp24_equalizeColor(t_bmp24 *img) {
    int w = img->width;
    int h = img->height;
    int size = w * h;

    float *Y = malloc(size * sizeof(float));
    float *U = malloc(size * sizeof(float));
    float *V = malloc(size * sizeof(float));
    unsigned int hist[256] = {0};

    // Étape 1 : Conversion RGB → YUV + histogramme de Y
    int idx = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            t_pixel p = img->data[y][x];

            float y_val = 0.299 * p.red + 0.587 * p.green + 0.114 * p.blue;
            float u = -0.14713 * p.red - 0.28886 * p.green + 0.436 * p.blue;
            float v =  0.615 * p.red - 0.51499 * p.green - 0.10001 * p.blue;

            Y[idx] = y_val;
            U[idx] = u;
            V[idx] = v;
            hist[(int)y_val]++;
            idx++;
        }
    }

    // Étape 2 : CDF + égalisation
    unsigned int cdf[256] = {0}, hist_eq[256] = {0};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + hist[i];
    }

    int cdfmin = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] != 0) {
            cdfmin = cdf[i];
            break;
        }
    }

    for (int i = 0; i < 256; i++) {
        hist_eq[i] = round(((float)(cdf[i] - cdfmin) / (size - cdfmin)) * 255);
    }

    // Étape 3 : nouvelle luminance Y -> RGB
    idx = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float y_new = hist_eq[(int)Y[idx]];
            float u = U[idx];
            float v = V[idx];

            float r = y_new + 1.13983 * v;
            float g = y_new - 0.39465 * u - 0.58060 * v;
            float b = y_new + 2.03211 * u;

            img->data[y][x].red = fmin(fmax(round(r), 0), 255);
            img->data[y][x].green = fmin(fmax(round(g), 0), 255);
            img->data[y][x].blue = fmin(fmax(round(b), 0), 255);

            idx++;
        }
    }

    free(Y);
    free(U);
    free(V);
}
