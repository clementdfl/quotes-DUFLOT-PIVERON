
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bmp8.h"
#include "bmp24.h"

int main() {


    t_bmp8 *image8 = NULL;
    t_bmp24 *image24 = NULL;
    int choixPrincipal, choixFiltre, value, threshold;
    char filename[256];

    while (1) {
        printf("\n=== MENU PRINCIPAL ===\n");
        printf("1. Ouvrir une image 8 bits (niveau de gris)\n");
        printf("2. Ouvrir une image 24 bits (couleur)\n");
        printf("3. Sauvegarder l'image 8 bits\n");
        printf("4. Sauvegarder l'image 24 bits\n");
        printf("5. Appliquer un filtre 8 bits\n");
        printf("6. Appliquer un filtre 24 bits\n");
        printf("7. Afficher infos image 8 bits\n");
        printf("8. Quitter\n");
        printf(">>> Votre choix : ");
        scanf("%d", &choixPrincipal);

        switch (choixPrincipal) {
            case 1:
                printf("Chemin du fichier 8 bits : ");
                scanf("%s", filename);
                image8 = bmp8_loadImage(filename);
                if (image8)
                    printf("Image 8 bits chargée avec succès.\n");
                break;

            case 2:
                printf("Chemin du fichier 24 bits : ");
                scanf("%s", filename);
                image24 = bmp24_loadImage(filename);
                if (image24)
                    printf("Image 24 bits chargée avec succès.\n");
                break;

            case 3:
                if (image8) {
                    printf("Chemin du fichier de sortie 8 bits : ");
                    scanf("%s", filename);
                    bmp8_saveImage(filename, image8);
                    printf("Image 8 bits sauvegardée.\n");
                } else {
                    printf("Aucune image 8 bits chargée.\n");
                }
                break;

            case 4:
                if (image24) {
                    printf("Chemin du fichier de sortie 24 bits : ");
                    scanf("%s", filename);
                    bmp24_saveImage(image24, filename);
                    printf("Image 24 bits sauvegardée.\n");
                } else {
                    printf("Aucune image 24 bits chargée.\n");
                }
                break;

            case 5:
                if (!image8) {
                    printf("Aucune image 8 bits chargée.\n");
                    break;
                }
                printf("\n--- Filtres 8 bits ---\n");
                printf("1. Négatif\n");
                printf("2. Luminosité\n");
                printf("3. Binarisation (Seuil)\n");
                printf(">>> Votre choix : ");
                scanf("%d", &choixFiltre);

                switch (choixFiltre) {
                    case 1:
                        bmp8_negative(image8);
                        printf("Filtre négatif appliqué.\n");
                        break;
                    case 2:
                        printf("Valeur de luminosité (+ ou -) : ");
                        scanf("%d", &value);
                        bmp8_brightness(image8, value);
                        printf("Luminosité ajustée.\n");
                        break;
                    case 3:
                        printf("Valeur du seuil (0-255) : ");
                        scanf("%d", &threshold);
                        bmp8_threshold(image8, threshold);
                        printf("Binarisation effectuée.\n");
                        break;
                    default:
                        printf("Choix invalide.\n");
                        break;
                }
                break;

            case 6:
                if (!image24) {
                    printf("Aucune image 24 bits chargée.\n");
                    break;
                }
                printf("\n--- Filtres 24 bits ---\n");
                printf("1. Négatif\n");
                printf("2. Niveaux de gris\n");
                printf("3. Luminosité\n");
                printf(">>> Votre choix : ");
                scanf("%d", &choixFiltre);

                switch (choixFiltre) {
                    case 1:
                        bmp24_negative(image24);
                        printf("Filtre négatif appliqué.\n");
                        break;
                    case 2:
                        bmp24_grayscale(image24);
                        printf("Conversion en niveaux de gris effectuée.\n");
                        break;
                    case 3:
                        printf("Valeur de luminosité (+ ou -) : ");
                        scanf("%d", &value);
                        bmp24_brightness(image24, value);
                        printf("Luminosité ajustée.\n");
                        break;
                    default:
                        printf("Choix invalide.\n");
                        break;
                }
                break;

            case 7:
                if (image8) {
                    bmp8_printInfo(image8);
                } else {
                    printf("Aucune image 8 bits chargée.\n");
                }
                break;

            case 8:
                printf("Fermeture du programme...\n");
                if (image8) bmp8_free(image8);
                if (image24) bmp24_free(image24);
                return 0;

            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }
    }

    return 0;
}


