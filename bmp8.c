#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bmp8.h"

t_bmp8 *bmp8_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return NULL;
    }

    t_bmp8 *img = malloc(sizeof(t_bmp8));
    if (!img) {
        fclose(file);
        return NULL;
    }

    // Lecture de l'en-tête BMP
    if (fread(img->header, 1, 54, file) != 54) {
        free(img);
        fclose(file);
        return NULL;
    }

    // Lecture de la table des couleurs (palette)
    if (fread(img->colorTable, 1, 1024, file) != 1024) {
        free(img);
        fclose(file);
        return NULL;
    }

    // Extraction des informations du header
    img->width = *(unsigned int *)&img->header[18];
    img->height = *(unsigned int *)&img->header[22];
    img->colorDepth = *(unsigned short *)&img->header[28];
    img->dataSize = *(unsigned int *)&img->header[34];

    if (img->colorDepth != 8) {
        printf("Image non 8 bits !\n");
        free(img);
        fclose(file);
        return NULL;
    }

    // Si dataSize est 0, le calculer (parfois c'est le cas)
    if (img->dataSize == 0) {
        int rowSize = ((img->width + 3) / 4) * 4; // lignes alignées sur 4 octets
        img->dataSize = rowSize * img->height;
    }

    img->data = malloc(img->dataSize);
    if (!img->data) {
        free(img);
        fclose(file);
        return NULL;
    }

    if (fread(img->data, 1, img->dataSize, file) != img->dataSize) {
        free(img->data);
        free(img);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return img;
}


void bmp8_saveImage(const char *filename, t_bmp8 *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Erreur de sauvegarde");
        return;
    }

    fwrite(img->header, sizeof(unsigned char), 54, file);
    fwrite(img->colorTable, sizeof(unsigned char), 1024, file);
    fwrite(img->data, sizeof(unsigned char), img->dataSize, file);
    fclose(file);
}

void bmp8_free(t_bmp8 *img) {
    if (img) {
        free(img->data);
        free(img);
    }
}

void bmp8_printInfo(t_bmp8 *img) {
    printf("Image Info:\n");
    printf("Width: %u\n", img->width);
    printf("Height: %u\n", img->height);
    printf("Color Depth: %u\n", img->colorDepth);
    printf("Data Size: %u\n", img->dataSize);
}

void bmp8_negative(t_bmp8 *img) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

void bmp8_brightness(t_bmp8 *img, int value) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        int pixel = img->data[i] + value;

        // Clamping entre 0 et 255
        if (pixel < 0) pixel = 0;
        if (pixel > 255) pixel = 255;

        img->data[i] = (unsigned char)pixel;
    }
}


unsigned int *bmp8_computeHistogram(t_bmp8 *img) {
    unsigned int *hist = calloc(256, sizeof(unsigned int));
    for (unsigned int i = 0; i < img->dataSize; i++) {
        hist[img->data[i]]++;
    }
    return hist;
}

unsigned int *bmp8_computeCDF(unsigned int *hist, int totalPixels) {
    unsigned int *cdf = malloc(256 * sizeof(unsigned int));
    unsigned int cdfmin = 0;

    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++)
        cdf[i] = cdf[i - 1] + hist[i];

    for (int i = 0; i < 256; i++) {
        if (cdf[i] != 0) {
            cdfmin = cdf[i];
            break;
        }
    }

    unsigned int *hist_eq = malloc(256 * sizeof(unsigned int));
    for (int i = 0; i < 256; i++) {
        hist_eq[i] = round(((float)(cdf[i] - cdfmin) / (totalPixels - cdfmin)) * 255);
    }

    free(cdf);
    return hist_eq;
}

void bmp8_equalize(t_bmp8 *img) {
    unsigned int *hist = bmp8_computeHistogram(img);
    unsigned int *hist_eq = bmp8_computeCDF(hist, img->dataSize);

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = hist_eq[img->data[i]];
    }

    free(hist);
    free(hist_eq);
}
