//Ce fichier bmp8.h déclare les prototypes des fonctions que l'on utilise dans le fichier bmp8.c et le fichier main.c
//Ce fichier a été modifié par Clément Duflot et Lucas Piveron


#ifndef BMP8_H
#define BMP8_H

typedef struct {
    unsigned char header[54];
    unsigned char colorTable[1024];
    unsigned char *data;
    unsigned int width;
    unsigned int height;
    unsigned int colorDepth;
    unsigned int dataSize;
} t_bmp8;

t_bmp8 * bmp8_loadImage(const char * filename); // Fonction pour charger une image
void bmp8_saveImage(const char * filename, t_bmp8 * img); // Fonction pour sauvegarder une image
void bmp8_free(t_bmp8 * img); // Fonction pour libérer l'espace alloué à une image
void bmp8_printInfo(t_bmp8 * img); // Fonction qui affiche les informations des images

void bmp8_negative(t_bmp8 * img); //Filtre négatif
void bmp8_brightness(t_bmp8 * img, int value); // Fonction pour modifier la luminosité
void bmp8_threshold(t_bmp8 * img, int threshold);
void bmp8_applyFilter(t_bmp8 * img, float **kernel, int kernelSize);

unsigned int *bmp8_computeHistogram(t_bmp8 *img);
unsigned int *bmp8_computeCDF(unsigned int *hist, int totalPixels);
void bmp8_equalize(t_bmp8 *img); // Fonction pour égaliser l'image barbara_grey
#endif
