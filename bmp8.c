#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
