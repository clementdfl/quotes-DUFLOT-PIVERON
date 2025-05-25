#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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
