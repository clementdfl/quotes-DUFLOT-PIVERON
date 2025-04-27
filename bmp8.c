#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"

// Fonction pour charger une image 8 bits
t_bmp8 * bmp8_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur : impossible d'ouvrir %s\n", filename);
        return NULL;
    }

    t_bmp8 *img = (t_bmp8 *)malloc(sizeof(t_bmp8));
    if (!img) {
        printf("Erreur : malloc\n");
        fclose(file);
        return NULL;
    }

    fread(img->header, sizeof(unsigned char), 54, file);
    img->width = *(unsigned int *)&img->header[18];
    img->height = *(unsigned int *)&img->header[22];
    img->colorDepth = *(unsigned int *)&img->header[28];
    img->dataSize = *(unsigned int *)&img->header[34];

    if (img->colorDepth != 8) {
        printf("Erreur : L'image doit être en niveaux de gris 8 bits.\n");
        free(img);
        fclose(file);
        return NULL;
    }

    fread(img->colorTable, sizeof(unsigned char), 1024, file);

    img->data = (unsigned char *)malloc(sizeof(unsigned char) * img->dataSize);
    if (!img->data) {
        printf("Erreur : malloc pour data\n");
        free(img);
        fclose(file);
        return NULL;
    }

    fread(img->data, sizeof(unsigned char), img->dataSize, file);

    fclose(file);
    return img;
}

// Fonction pour sauvegarder une image 8 bits
void bmp8_saveImage(const char *filename, t_bmp8 *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erreur : impossible d'ouvrir %s en écriture\n", filename);
        return;
    }

    fwrite(img->header, sizeof(unsigned char), 54, file);
    fwrite(img->colorTable, sizeof(unsigned char), 1024, file);
    fwrite(img->data, sizeof(unsigned char), img->dataSize, file);

    fclose(file);
}

// Fonction pour libérer une image
void bmp8_free(t_bmp8 *img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}

// Fonction pour afficher les infos d'une image
void bmp8_printInfo(t_bmp8 *img) {
    if (img) {
        printf("Image Info:\n");
        printf("Width: %u\n", img->width);
        printf("Height: %u\n", img->height);
        printf("Color Depth: %u\n", img->colorDepth);
        printf("Data Size: %u\n", img->dataSize);
    }
}

// Fonction pour inverser les couleurs
void bmp8_negative(t_bmp8 *img) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

// Fonction pour changer la luminosité
void bmp8_brightness(t_bmp8 *img, int value) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        int newValue = img->data[i] + value;
        if (newValue > 255) newValue = 255;
        if (newValue < 0) newValue = 0;
        img->data[i] = (unsigned char)newValue;
    }
}

// Fonction pour binariser l'image
void bmp8_threshold(t_bmp8 *img, int threshold) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
    }
}

// Appliquer un filtre (convolution simple)
void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize) {
    int n = kernelSize / 2;
    unsigned char *newData = (unsigned char *)malloc(sizeof(unsigned char) * img->dataSize);

    for (unsigned int y = 0; y < img->height; y++) {
        for (unsigned int x = 0; x < img->width; x++) {
            if (x < n || x >= img->width - n || y < n || y >= img->height - n) {
                newData[y * img->width + x] = img->data[y * img->width + x];
                continue;
            }

            float sum = 0.0f;
            for (int i = -n; i <= n; i++) {
                for (int j = -n; j <= n; j++) {
                    sum += kernel[i+n][j+n] * img->data[(y+i)*img->width + (x+j)];
                }
            }
            if (sum < 0) sum = 0;
            if (sum > 255) sum = 255;
            newData[y * img->width + x] = (unsigned char)sum;
        }
    }

    free(img->data);
    img->data = newData;
}
