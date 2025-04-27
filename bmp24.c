#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bmp24.h"

// Allocation d'une matrice de pixels
t_pixel ** bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = (t_pixel **)malloc(sizeof(t_pixel *) * height);
    if (!pixels) return NULL;

    for (int i = 0; i < height; i++) {
        pixels[i] = (t_pixel *)malloc(sizeof(t_pixel) * width);
        if (!pixels[i]) return NULL;
    }
    return pixels;
}

// Libération d'une matrice de pixels
void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    if (!pixels) return;
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

// Allocation d'une image 24 bits
t_bmp24 * bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = (t_bmp24 *)malloc(sizeof(t_bmp24));
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

// Libération d'une image
void bmp24_free(t_bmp24 *img) {
    if (img) {
        bmp24_freeDataPixels(img->data, img->height);
        free(img);
    }
}

// Lecture d'une image BMP 24 bits
t_bmp24 * bmp24_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur ouverture fichier %s\n", filename);
        return NULL;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);

    int width = *(int *)&header[18];
    int height = *(int *)&header[22];
    int depth = *(short *)&header[28];

    if (depth != 24) {
        printf("Erreur : image pas en 24 bits.\n");
        fclose(file);
        return NULL;
    }

    t_bmp24 *img = bmp24_allocate(width, height, depth);

    fseek(file, 54, SEEK_SET);

    for (int i = height-1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            fread(&img->data[i][j].blue, sizeof(unsigned char), 1, file);
            fread(&img->data[i][j].green, sizeof(unsigned char), 1, file);
            fread(&img->data[i][j].red, sizeof(unsigned char), 1, file);
        }
    }

    fclose(file);
    return img;
}

// Sauvegarde d'une image BMP 24 bits
void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erreur écriture fichier %s\n", filename);
        return;
    }

    unsigned char header[54] = {0};

    header[0] = 'B';
    header[1] = 'M';
    int fileSize = 54 + img->width * img->height * 3;
    *(int *)&header[2] = fileSize;
    *(int *)&header[10] = 54;
    *(int *)&header[14] = 40;
    *(int *)&header[18] = img->width;
    *(int *)&header[22] = img->height;
    *(short *)&header[26] = 1;
    *(short *)&header[28] = 24;

    fwrite(header, sizeof(unsigned char), 54, file);

    for (int i = img->height-1; i >= 0; i--) {
        for (int j = 0; j < img->width; j++) {
            fwrite(&img->data[i][j].blue, sizeof(unsigned char), 1, file);
            fwrite(&img->data[i][j].green, sizeof(unsigned char), 1, file);
            fwrite(&img->data[i][j].red, sizeof(unsigned char), 1, file);
        }
    }

    fclose(file);
}

// Inverser les couleurs
void bmp24_negative(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue = 255 - img->data[y][x].blue;
        }
    }
}

// Mettre en niveaux de gris
void bmp24_grayscale(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t gray = (img->data[y][x].red + img->data[y][x].green + img->data[y][x].blue) / 3;
            img->data[y][x].red = img->data[y][x].green = img->data[y][x].blue = gray;
        }
    }
}

// Modifier la luminosité
void bmp24_brightness(t_bmp24 *img, int value) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int r = img->data[y][x].red + value;
            int g = img->data[y][x].green + value;
            int b = img->data[y][x].blue + value;
            if (r > 255) r = 255; if (r < 0) r = 0;
            if (g > 255) g = 255; if (g < 0) g = 0;
            if (b > 255) b = 255; if (b < 0) b = 0;
            img->data[y][x].red = (uint8_t)r;
            img->data[y][x].green = (uint8_t)g;
            img->data[y][x].blue = (uint8_t)b;
        }
    }
}
//
// Created by piver on 27/04/2025.
//
