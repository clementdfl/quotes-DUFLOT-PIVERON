//Ce fichier bmp24.h déclare les prototypes des fonctions que l'on utilise dans le fichier bmp24.c et le fichier main.c
//Ce fichier a été modifié par Clément Duflot et Lucas Piveron



#ifndef BMP24_H
#define BMP24_H

#include <stdint.h>


#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} t_bmp_header;

#pragma pack(push, 1) // Le pragma sert à ne pas ajouter de padding, il garantit que tous les champs soient bien placés à la suite des autres sans que les valeurs de certaines variables soient échangées avec celles des autres.
typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits;
    uint32_t compression;
    uint32_t imagesize;
    int32_t xresolution;
    int32_t yresolution;
    uint32_t ncolors;
    uint32_t importantcolors;
} t_bmp_info;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} t_pixel;

typedef struct {
    t_bmp_header header;
    t_bmp_info header_info;
    int width;
    int height;
    int colorDepth;
    t_pixel **data;
} t_bmp24;

// Alloc / Free
t_pixel ** bmp24_allocateDataPixels(int width, int height);
void bmp24_freeDataPixels(t_pixel **pixels, int height);
t_bmp24 * bmp24_allocate(int width, int height, int colorDepth);
void bmp24_free(t_bmp24 *img); //Fonction pour libérer l'espace alloué à l'image

// Load / Save
t_bmp24 * bmp24_loadImage(const char * filename); //Fonction qui sert à charger une image
void bmp24_saveImage(t_bmp24 * img, const char * filename); //Fonction qui sert à sauvegarder une image

// Traitement
void bmp24_negative(t_bmp24 * img); //Application de filtre négatif
void bmp24_grayscale(t_bmp24 * img);
void bmp24_brightness(t_bmp24 * img, int value); //Modification de la luminosité

// Convolution
t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float **kernel, int kernelSize); // Ces 6 fonctions servent à appliqer tous les filtres de convolution présent dans la partie 2 du projet
void bmp24_boxBlur(t_bmp24 *img);
void bmp24_gaussianBlur(t_bmp24 *img);
void bmp24_outline(t_bmp24 *img);
void bmp24_emboss(t_bmp24 *img);
void bmp24_sharpen(t_bmp24 *img);

void bmp24_equalizeColor(t_bmp24 *img); //Fonction pour égaliser l'image flowers_color



#endif
