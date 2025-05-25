# IMAGE PROCESSING - Clément DUFLOT - Lucas PIVERON
Ce projet a pour but de concevoir un programme en langage C capable de charger, manipuler et enregistrer des images au format BMP, en 8 bits (niveaux de gris) et en 24 bits (couleur RGB).

L’utilisateur interagit avec un menu de commande, lui permettant d’appliquer différentes transformations sur les images, allant de simples filtres (négatif, luminosité) à des opérations plus complexes comme des filtres de convolution ou l’égalisation d’histogrammes.

En ce qui concerne les fonctionnalités implémentés on retrouve : 
- Le chargement et la sauvegarde d'une image BMP 8 bits et 24 bits
- L'affichage d'informations (dimensions, colordepth...)
- Ajout d'un filtre négatif que l'on peut ajouter sur les images
- Modification de la luminosité des images BMP 8 bits et 24 bits
- Filtres de convolution (Box flur, Gaussian flur, Outline, Emboss et Sharpen)
- L'Égalisation d’histogramme en niveaux de gris et en couleur

Enfin, il existe quelques bugs connus sur notre projet comme : 
- Le filtre Box flur / Gaussian flur où nous ne sommes pas sur à 100% que le filtre agis sur l'image à cause des légères modifications qu'il ajoute
- L'égalisation en niveau de gris qui ne marche pas exactement comme le veut le projet
- Le filtre Sharpen (netteté) qui ne marche pas du tout.

Ce READme a été réalisé par Clément DUFLOT.