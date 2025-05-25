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
            case 1: menu_bmp8(); break;
            case 2: menu_bmp24(); break;
            case 3: printf("Au revoir !\n"); break;
            default: printf("Choix invalide.\n");
        }
    } while (choix != 3);

    return 0;
}

void menu_bmp8() {
    t_bmp8 *img = NULL;
    int choix, val, seuil;
    char chemin[256];

    do {
        printf("\n--- IMAGE 8 BITS ---\n");
        printf("1. Ouvrir une image\n");
        printf("2. Sauvegarder l'image\n");
        printf("3. Appliquer filtre negatif\n");
        printf("4. Modifier luminosite\n");
        printf("5. Afficher infos\n");
        printf("6. Egalisation d'histogramme\n");
        printf("7. Retour\n");
        printf(">>> Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                printf("Chemin de l'image : ");
                scanf("%s", chemin);
                img = bmp8_loadImage(chemin);
                if (img) printf("Image chargee avec succes.\n");
                break;
            case 2:
                if (img) {
                    printf("Chemin de sauvegarde : ");
                    scanf("%s", chemin);
                    bmp8_saveImage(chemin, img);
                    printf("Image sauvegardee.\n");
                } else printf("Aucune image chargee.\n");
                break;
            case 3:
                if (img) bmp8_negative(img);
                else printf("Aucune image chargee.\n");
                break;
            case 4:
                if (img) {
                    printf("Valeur luminosite (-255 a 255) : ");
                    scanf("%d", &val);
                    bmp8_brightness(img, val);
                } else printf("Aucune image chargee.\n");
                break;

            case 5:
                if (img) bmp8_printInfo(img);
                else printf("Aucune image chargee.\n");
                break;

            case 6:
                if (img) {
                    bmp8_equalize(img);
                    printf("Egalisation d'histogramme effectuee.\n");
                } else {
                    printf("Aucune image chargee.\n");
                }
            break;

        }
    } while (choix != 7);

    if (img) bmp8_free(img);
}

void menu_bmp24() {
    t_bmp24 *img = NULL;
    int choix, val, filtre;
    char chemin[256];

    do {
        printf("\n--- IMAGE 24 BITS ---\n");
        printf("1. Ouvrir une image\n");
        printf("2. Sauvegarder l'image\n");
        printf("3. Appliquer filtre negatif\n");
        printf("4. Convertir en niveaux de gris\n");
        printf("5. Modifier luminosite\n");
        printf("6. Filtres de convolution\n");
        printf("7. Egalisation d'histogramme \n");
        printf("8. Retour\n");
        printf(">>> Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                printf("Chemin de l'image : ");
                scanf("%s", chemin);
                img = bmp24_loadImage(chemin);
                if (img) printf("Image chargee avec succes.\n");
                break;
            case 2:
                if (img) {
                    printf("Chemin du fichier : ");
                    scanf("%s", chemin);
                    bmp24_saveImage(img, chemin);
                    printf("Image sauvegardee.\n");
                } else printf("Aucune image chargee.\n");
                break;
            case 3:
                if (img) bmp24_negative(img);
                else printf("Aucune image chargee.\n");
                break;
            case 4:
                if (img) bmp24_grayscale(img);
                else printf("Aucune image chargee.\n");
                break;
            case 5:
                if (img) {
                    printf("Valeur luminosite (-255 à 255) : ");
                    scanf("%d", &val);
                    bmp24_brightness(img, val);
                } else printf("Aucune image chargee.\n");
                break;
            case 6:
                if (!img) {
                    printf("Veuillez d'abord charger une image.\n");
                    break;
                }
                printf("\n--- FILTRES DE CONVOLUTION ---\n");
                printf("1. Box Blur\n");
                printf("2. Gaussian Blur\n");
                printf("3. Contours (Outline)\n");
                printf("4. Relief (Emboss)\n");
                printf("5. Nettete (Sharpen)\n");
                printf(">>> Choix du filtre : ");
                scanf("%d", &filtre);

                switch (filtre) {
                    case 1: bmp24_boxBlur(img); break;
                    case 2: bmp24_gaussianBlur(img); break;
                    case 3: bmp24_outline(img); break;
                    case 4: bmp24_emboss(img); break;
                    case 5: bmp24_sharpen(img); break;
                    default: printf("Filtre inconnu.\n");
                }
                printf("Filtre appliqué.\n");
                break;

            case 7:
                if (img) {
                    bmp24_equalizeColor(img);
                    printf("Egalisation d'histogramme effectuee.\n");
                } else {
                    printf("Aucune image chargee.\n");
                }
            break;


        }
    } while (choix != 8);

    if (img) bmp24_free(img);
}
