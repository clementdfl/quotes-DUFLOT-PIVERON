#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

// ============================================================================
// DÉFINITIONS DES STRUCTURES
// ============================================================================

// Structure pour image 8-bits en niveaux de gris
typedef struct {
    unsigned char header[54];
    unsigned char colorTable[1024];
    unsigned char * data;
    unsigned int width;
    unsigned int height;
    unsigned int colorDepth;
    unsigned int dataSize;
} t_bmp8;

// Structure pour un pixel couleur
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} t_pixel;

// Structure pour l'en-tête BMP
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} t_bmp_header;

// Structure pour les informations BMP
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

// Structure pour image 24-bits couleur
typedef struct {
    t_bmp_header header;
    t_bmp_info header_info;
    int width;
    int height;
    int colorDepth;
    t_pixel **data;
} t_bmp24;

// Structure pour les valeurs YUV
typedef struct {
    float y;  // Luminance
    float u;  // Chrominance bleue
    float v;  // Chrominance rouge
} t_yuv;

// ============================================================================
// PROTOTYPES DES FONCTIONS
// ============================================================================

// Fonctions pour images 8-bits
unsigned int * bmp8_computeHistogram(t_bmp8 * img);
unsigned int * bmp8_computeCDF(unsigned int * hist);
void bmp8_equalize(t_bmp8 * img);

// Fonctions pour images couleur
t_yuv rgb_to_yuv(t_pixel pixel);
t_pixel yuv_to_rgb(t_yuv yuv);
unsigned int * bmp24_computeLuminanceHistogram(t_bmp24 * img);
void bmp24_equalize(t_bmp24 * img);

// Fonctions utilitaires
void print_histogram_sample(unsigned int * hist, const char * title);

// ============================================================================
// IMPLÉMENTATION - ÉGALISATION POUR IMAGES 8-BITS
// ============================================================================

/**
 * @brief Calcule l'histogramme d'une image 8-bits en niveau de gris
 * @param img Pointeur vers l'image 8-bits
 * @return Tableau de 256 entiers représentant l'histogramme
 */
unsigned int * bmp8_computeHistogram(t_bmp8 * img) {
    // Vérification de validité de l'image
    if (img == NULL || img->data == NULL) {
        printf("Erreur: Image invalide pour le calcul d'histogramme\n");
        return NULL;
    }
    
    // Allocation mémoire pour l'histogramme (256 niveaux de gris)
    // calloc initialise automatiquement à zéro
    unsigned int * histogram = (unsigned int *)calloc(256, sizeof(unsigned int));
    
    if (histogram == NULL) {
        printf("Erreur: Échec de l'allocation mémoire pour l'histogramme\n");
        return NULL;
    }
    
    // Parcours de tous les pixels de l'image
    for (unsigned int i = 0; i < img->dataSize; i++) {
        // Récupération de la valeur du pixel (0-255)
        unsigned char pixel_value = img->data[i];
        
        // Incrémentation du compteur pour ce niveau de gris
        histogram[pixel_value]++;
    }
    
    printf("Histogramme calculé: %u pixels traités\n", img->dataSize);
    return histogram;
}

/**
 * @brief Calcule la fonction de distribution cumulative (CDF) et la normalise
 * @param hist Histogramme d'origine (256 valeurs)
 * @return Tableau de transformation pour l'égalisation
 */
unsigned int * bmp8_computeCDF(unsigned int * hist) {
    if (hist == NULL) {
        printf("Erreur: Histogramme NULL pour le calcul CDF\n");
        return NULL;
    }
    
    // Allocation pour la CDF temporaire
    unsigned int * cdf = (unsigned int *)malloc(256 * sizeof(unsigned int));
    if (cdf == NULL) {
        printf("Erreur: Allocation CDF échouée\n");
        return NULL;
    }
    
    // Allocation pour l'histogramme égalisé (table de transformation)
    unsigned int * hist_eq = (unsigned int *)malloc(256 * sizeof(unsigned int));
    if (hist_eq == NULL) {
        printf("Erreur: Allocation histogramme égalisé échouée\n");
        free(cdf);
        return NULL;
    }
    
    // Calcul de la CDF (Cumulative Distribution Function)
    cdf[0] = hist[0];  // Premier élément
    
    for (int i = 1; i < 256; i++) {
        // Formule CDF: cdf[i] = cdf[i-1] + hist[i]
        // Chaque valeur = somme de toutes les valeurs précédentes
        cdf[i] = cdf[i - 1] + hist[i];
    }
    
    // Recherche de la valeur minimale non-nulle de la CDF
    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] != 0) {
            cdf_min = cdf[i];
            break;
        }
    }
    
    // Le nombre total de pixels est la dernière valeur de la CDF
    unsigned int total_pixels = cdf[255];
    
    printf("CDF calculée: cdf_min=%u, total_pixels=%u\n", cdf_min, total_pixels);
    
    // Normalisation de la CDF pour créer la table de transformation
    for (int i = 0; i < 256; i++) {
        if (cdf[i] == 0) {
            // Si aucun pixel n'a cette valeur, transformation = 0
            hist_eq[i] = 0;
        } else {
            // Formule de normalisation:
            // new_value = round((cdf[i] - cdf_min) / (total - cdf_min) * 255)
            float normalized = ((float)(cdf[i] - cdf_min) / (float)(total_pixels - cdf_min)) * 255.0f;
            hist_eq[i] = (unsigned int)round(normalized);
        }
    }
    
    // Libération de la mémoire temporaire
    free(cdf);
    
    return hist_eq;
}

/**
 * @brief Applique l'égalisation d'histogramme à une image 8-bits
 * @param img Image à égaliser (modifiée en place)
 */
void bmp8_equalize(t_bmp8 * img) {
    if (img == NULL || img->data == NULL) {
        printf("Erreur: Image invalide pour l'égalisation\n");
        return;
    }
    
    printf("Début de l'égalisation d'histogramme 8-bits...\n");
    
    // Étape 1: Calcul de l'histogramme original
    unsigned int * hist = bmp8_computeHistogram(img);
    if (hist == NULL) {
        printf("Erreur: Impossible de calculer l'histogramme\n");
        return;
    }
    
    // Étape 2: Calcul de la table de transformation (CDF normalisée)
    unsigned int * hist_eq = bmp8_computeCDF(hist);
    if (hist_eq == NULL) {
        printf("Erreur: Impossible de calculer la CDF\n");
        free(hist);
        return;
    }
    
    // Étape 3: Application de la transformation à chaque pixel
    for (unsigned int i = 0; i < img->dataSize; i++) {
        // Remplacement: nouvelle_valeur = table_transformation[ancienne_valeur]
        unsigned char old_value = img->data[i];
        img->data[i] = (unsigned char)hist_eq[old_value];
    }
    
    // Nettoyage mémoire
    free(hist);
    free(hist_eq);
    
    printf("Égalisation d'histogramme 8-bits terminée avec succès\n");
}

// ============================================================================
// IMPLÉMENTATION - CONVERSION RGB/YUV POUR IMAGES COULEUR
// ============================================================================

/**
 * @brief Convertit un pixel RGB vers l'espace colorimétrique YUV
 * @param pixel Pixel RGB à convertir
 * @return Structure YUV correspondante
 */
t_yuv rgb_to_yuv(t_pixel pixel) {
    t_yuv yuv;
    
    // Normalisation des valeurs RGB de [0-255] vers [0.0-1.0]
    float r = pixel.red / 255.0f;
    float g = pixel.green / 255.0f;
    float b = pixel.blue / 255.0f;
    
    // Formules de conversion RGB → YUV
    // Y (luminance): information de luminosité
    yuv.y = 0.299f * r + 0.587f * g + 0.114f * b;
    
    // U (chrominance bleue): différence bleu-luminance
    yuv.u = -0.14713f * r - 0.28886f * g + 0.436f * b;
    
    // V (chrominance rouge): différence rouge-luminance
    yuv.v = 0.615f * r - 0.51499f * g - 0.10001f * b;
    
    return yuv;
}

/**
 * @brief Convertit un pixel YUV vers l'espace colorimétrique RGB
 * @param yuv Structure YUV à convertir
 * @return Pixel RGB correspondant
 */
t_pixel yuv_to_rgb(t_yuv yuv) {
    t_pixel pixel;
    
    // Formules de conversion YUV → RGB
    float r = yuv.y + 1.13983f * yuv.v;
    float g = yuv.y - 0.39465f * yuv.u - 0.58060f * yuv.v;
    float b = yuv.y + 2.03211f * yuv.u;
    
    // Conversion vers [0-255] avec clamping (limitation des bornes)
    // fmaxf(x, 0.0f) assure que x >= 0
    // fminf(x, 1.0f) assure que x <= 1
    pixel.red   = (uint8_t)(fminf(fmaxf(r, 0.0f), 1.0f) * 255.0f);
    pixel.green = (uint8_t)(fminf(fmaxf(g, 0.0f), 1.0f) * 255.0f);
    pixel.blue  = (uint8_t)(fminf(fmaxf(b, 0.0f), 1.0f) * 255.0f);
    
    return pixel;
}

// ============================================================================
// IMPLÉMENTATION - ÉGALISATION POUR IMAGES COULEUR
// ============================================================================

/**
 * @brief Calcule l'histogramme de la composante luminance (Y) d'une image couleur
 * @param img Image couleur 24-bits
 * @return Histogramme de la luminance (256 valeurs)
 */
unsigned int * bmp24_computeLuminanceHistogram(t_bmp24 * img) {
    if (img == NULL || img->data == NULL) {
        printf("Erreur: Image couleur invalide\n");
        return NULL;
    }
    
    // Allocation de l'histogramme de luminance
    unsigned int * histogram = (unsigned int *)calloc(256, sizeof(unsigned int));
    if (histogram == NULL) {
        printf("Erreur: Allocation histogramme luminance échouée\n");
        return NULL;
    }
    
    // Parcours de tous les pixels de l'image couleur
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            // Conversion du pixel RGB en YUV
            t_yuv yuv = rgb_to_yuv(img->data[y][x]);
            
            // Conversion de la luminance [0.0-1.0] vers [0-255]
            int luminance_level = (int)round(yuv.y * 255.0f);
            
            // Sécurité: limitation des bornes
            if (luminance_level < 0) luminance_level = 0;
            if (luminance_level > 255) luminance_level = 255;
            
            // Incrémentation du compteur
            histogram[luminance_level]++;
        }
    }
    
    printf("Histogramme de luminance calculé pour image %dx%d\n", img->width, img->height);
    return histogram;
}

/**
 * @brief Applique l'égalisation d'histogramme à une image couleur
 * Seule la luminance (Y) est égalisée, les chrominances (U,V) sont préservées
 * @param img Image couleur à égaliser (modifiée en place)
 */
void bmp24_equalize(t_bmp24 * img) {
    if (img == NULL || img->data == NULL) {
        printf("Erreur: Image couleur invalide pour l'égalisation\n");
        return;
    }
    
    printf("Début de l'égalisation d'histogramme couleur...\n");
    
    // Étape 1: Calcul de l'histogramme de luminance
    unsigned int * hist = bmp24_computeLuminanceHistogram(img);
    if (hist == NULL) {
        printf("Erreur: Impossible de calculer l'histogramme de luminance\n");
        return;
    }
    
    // Étape 2: Calcul de la table de transformation
    unsigned int * hist_eq = bmp8_computeCDF(hist);
    if (hist_eq == NULL) {
        printf("Erreur: Impossible de calculer la CDF pour l'image couleur\n");
        free(hist);
        return;
    }
    
    // Étape 3: Application de la transformation pixel par pixel
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            // Conversion RGB → YUV
            t_yuv yuv = rgb_to_yuv(img->data[y][x]);
            
            // Sauvegarde des chrominances originales
            float original_u = yuv.u;
            float original_v = yuv.v;
            
            // Transformation de la luminance
            int original_luminance = (int)round(yuv.y * 255.0f);
            if (original_luminance < 0) original_luminance = 0;
            if (original_luminance > 255) original_luminance = 255;
            
            // Application de l'égalisation sur Y uniquement
            int new_luminance = hist_eq[original_luminance];
            yuv.y = new_luminance / 255.0f;
            
            // Restauration des chrominances (préservation des couleurs)
            yuv.u = original_u;
            yuv.v = original_v;
            
            // Conversion YUV → RGB et mise à jour du pixel
            img->data[y][x] = yuv_to_rgb(yuv);
        }
    }
    
    // Nettoyage mémoire
    free(hist);
    free(hist_eq);
    
    printf("Égalisation d'histogramme couleur terminée avec succès\n");
}

// ============================================================================
// FONCTIONS UTILITAIRES
// ============================================================================

/**
 * @brief Affiche un échantillon de l'histogramme (valeurs non-nulles)
 * @param hist Histogramme à afficher
 * @param title Titre pour l'affichage
 */
void print_histogram_sample(unsigned int * hist, const char * title) {
    if (hist == NULL) {
        printf("Histogramme NULL\n");
        return;
    }
    
    printf("\n=== %s ===\n", title);
    printf("Échantillon des valeurs non-nulles:\n");
    
    int count = 0;
    for (int i = 0; i < 256 && count < 10; i++) {
        if (hist[i] > 0) {
            printf("  Niveau %3d: %6u pixels\n", i, hist[i]);
            count++;
        }
    }
    
    if (count == 10) {
        printf("  ... (autres valeurs omises)\n");
    }
    printf("\n");
}

// ============================================================================
// FONCTION PRINCIPALE ET TESTS
// ============================================================================

/**
 * @brief Fonction principale pour démonstration
 */
/*int main() {
    printf("===========================================\n");
    printf("  ÉGALISATION D'HISTOGRAMME - PARTIE 3\n");
    printf("===========================================\n\n");
    
    printf("Ce programme implémente les fonctions d'égalisation d'histogramme\n");
    printf("pour les images BMP 8-bits (niveaux de gris) et 24-bits (couleur).\n\n");
    
    printf("Fonctions disponibles:\n");
    printf("• bmp8_computeHistogram()  - Calcul histogramme 8-bits\n");
    printf("• bmp8_computeCDF()        - Calcul fonction de transformation\n");
    printf("• bmp8_equalize()          - Égalisation image 8-bits\n");
    printf("• bmp24_equalize()         - Égalisation image couleur\n\n");
    
    printf("Exemple d'utilisation:\n");
    printf("  t_bmp8 * img = bmp8_loadImage(\"barbara_gray.bmp\");\n");
    printf("  bmp8_equalize(img);\n");
    printf("  bmp8_saveImage(\"barbara_equalized.bmp\", img);\n\n");
    
    printf("Pour utiliser ces fonctions:\n");
    printf("1. Intégrez ce code avec vos fonctions des parties 1 et 2\n");
    printf("2. Compilez avec: gcc -o main main.c -lm\n");
    printf("3. Le flag -lm est nécessaire pour les fonctions mathématiques\n\n");
    
    printf("Programme terminé.\n");
    
    return 0;
}*/