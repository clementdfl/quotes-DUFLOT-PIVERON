#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"
#include "bmp24.h"

void menu_bmp8();
void menu_bmp24();

int main() {
    int choix;

    do {
        printf("\n--- MENU PRINCIPAL ---\n");
        printf("1. Image 8 bits (niveaux de gris)\n");
        printf("2. Image 24 bits (couleur)\n");
        printf("3. Quitter\n");
        printf(">>> Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                menu_bmp8();
            break;
            case 2:
                menu_bmp24();
            break;
            case 3:
                printf("Au revoir !\n");
            break;
            default:
                printf("Choix invalide.\n");
        }
    } while (choix != 3);

    return 0;
}

void menu_bmp8() {
    t_bmp8 *img = NULL;
    int choix, val;
    char chemin[256];

    do {
        printf("\n--- IMAGE 8 BITS ---\n");
        printf("1. Ouvrir une image\n");
        printf("2. Sauvegarder l'image\n");
        printf("3. Appliquer filtre négatif\n");
        printf("4. Modifier luminosité\n");
        printf("5. Afficher infos\n");
        printf("6. Retour\n");
        printf(">>> Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                printf("Chemin de l'image : ");
            scanf("%s", chemin);
            img = bmp8_loadImage(chemin);
            if (img) printf("Image chargée avec succès.\n");
            break;
            case 2:
                if (img) {
                    printf("Nom du fichier : ");
                    scanf("%s", chemin);
                    bmp8_saveImage(chemin, img);
                    printf("Image sauvegardée.\n");
                } else printf("Aucune image chargée.\n");
            break;
            case 3:
                if (img) bmp8_negative(img);
                else printf("Aucune image chargée.\n");
            break;
            case 4:
                if (img) {
                    printf("Valeur luminosité (-255 à 255) : ");
                    scanf("%d", &val);
                    bmp8_brightness(img, val);
                } else printf("Aucune image chargée.\n");
            break;
            case 5:
                if (img) bmp8_printInfo(img);
                else printf("Aucune image chargée.\n");
            break;
        }
    } while (choix != 6);

    if (img) bmp8_free(img);
}

void menu_bmp24() {
    t_bmp24 *img = NULL;
    int choix, val;
    char chemin[256];

    do {
        printf("\n--- IMAGE 24 BITS ---\n");
        printf("1. Ouvrir une image\n");
        printf("2. Sauvegarder l'image\n");
        printf("3. Appliquer filtre négatif\n");
        printf("4. Convertir en niveaux de gris\n");
        printf("5. Modifier luminosité\n");
        printf("6. Retour\n");
        printf(">>> Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                printf("Chemin de l'image : ");
            scanf("%s", chemin);
            img = bmp24_loadImage(chemin);
            if (img) printf("Image chargée avec succès.\n");
            break;
            case 2:
                if (img) {
                    printf("Nom du fichier : ");
                    scanf("%s", chemin);
                    bmp24_saveImage(img, chemin);
                    printf("Image sauvegardée.\n");
                } else printf("Aucune image chargée.\n");
            break;
            case 3:
                if (img) bmp24_negative(img);
                else printf("Aucune image chargée.\n");
            break;
            case 4:
                if (img) bmp24_grayscale(img);
                else printf("Aucune image chargée.\n");
            break;
            case 5:
                if (img) {
                    printf("Valeur luminosité (-255 à 255) : ");
                    scanf("%d", &val);
                    bmp24_brightness(img, val);
                } else printf("Aucune image chargée.\n");
            break;
        }
    } while (choix != 6);

    if (img) bmp24_free(img);
}


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
