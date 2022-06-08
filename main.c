#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

/**
    * @brief Permet de récupérer l'en-tête d'un fichier .ppm
    * @param image l'image dont on souhaite récupérer l'en-tête
    * @param type pointeur vers la variable contenant le type
    * @param taille_x pointeur vers la variable contenant la longueur
    * @param taille_y pointeur vers la variable contenant la largeur
    * @param max pointeur vers la variable contenant la valeur maximale de la couleur
    */
void recup_entete(FILE* image, unsigned char* type, int* taille_x, int* taille_y, int* max)
{

    fscanf(image, "%s", type);
    fscanf(image, "%d", taille_x);
    fscanf(image, "%d", taille_y);
    fscanf(image, "%d", max);
    fseek(image, 1L, SEEK_CUR);

}

/**
    *@brief Permet de connaître la taille du fichier à partir du point où le curseur se situe
    *@param fichier Fichier dont on veut connaître la taille
    *@return Retourne un long contenant la taille du fichier
    */
long taille_du_fichier(FILE* fichier)
{
    long emplacement_actuel;
    long taille;

    emplacement_actuel = ftell(fichier); //on récupère l'emplacement du fichier pour y retourner plus tard
    fseek(fichier, 0L, SEEK_END); //on va à la fin du fichier
    taille = ftell(fichier); //on regarde combien d'octet on a parcouru depuis le début
    fseek(fichier, emplacement_actuel, SEEK_SET);
    return taille;
}

/**
    *@brief Convertit les octets contenus dans un fichier en un tableau de caractère à partir du point où le curseur est
    *@param fichier Fichier dont on veut convertir le contenu
    *@return retourne un tableau contenant le contenu du fichier
    */
unsigned char* conv_octets_tableau(FILE* fichier)
{
    long taille;
    unsigned char* tableau_fichier;


    taille = taille_du_fichier(fichier); //recup_entete() ayant déjà été utilisé on ne stocke que l'information contenue dans le fichier
    tableau_fichier = (unsigned char*)malloc(taille * sizeof(unsigned char));

    for (int i = 0; i < taille; i++) {
        fscanf(fichier, "%c", &tableau_fichier[i]);
    }

    return tableau_fichier;
}

/**
    *@brief Convertit un fichier contenant une suite d'entiers en un tableau d'entier et permet de connaître le nombres d'entier dedans
    *@param fichier_cle Fichier dont on veut convertir le contenu
    *@param longueur Pointeur vers la variable contenant la longueur de la clé
    *@return retourne un tableau contenant chaque entier contenu dans le fichier
    */
long* conv_cle_tableau(FILE* fichier_cle, long* longueur)
{
    long taille;
    long* tableau_cle;

    *longueur = 0;
    taille = taille_du_fichier(fichier_cle);
    tableau_cle = (long*)malloc(taille * sizeof(long)); //on génère d'abord un tableau aussi grand que la taille du fichier contenant la clé, c'est à dire trop grand

    while (fscanf(fichier_cle, "%ld", &tableau_cle[*longueur]) > 0) { //tant qu'on atteint pas le bout (donc que scanf > 0) on continue d'écrire dans le tableau et on augmente la longueur de 1
        (*longueur)++;
    }

    return tableau_cle;
}

/**
    *@brief Permet de permuter 2 entiers
    *@param a pointeur vers l'entier 1
    *@param b pointeur vers l'entier 2
    */
void permuter (long* a, long* b)
{
    long swap;

    swap = *a;
    *a = *b;
    *b = swap;
}

/**
    *@brief Génère un tableau d'entiers allant de 0 à taille disposés dans un ordre aléatoire
    *@param taille Taille du tableau et nombre d'entier
    *@return Retourne un tableau d'entiers allant de 0 à taille disposés dans un ordre aléatoire
    */
long* creer_cle(long taille)
{
    long* cle;
    long aleatoire;

    cle = (long*)malloc(taille*sizeof(long));

    for (int i = 0; i < taille; i++) {
        cle[i] = i;
    } // on génère un tableau contenant tous les entiers naturels de 0 à taille

    for (int i = 0; i < taille; i++) {
        aleatoire = rand()%taille;
        permuter(&cle[i], &cle[aleatoire]);
    } //on les permute de façon aléatoire afin d'avoir une clé pseudo-aléatoire


    return cle;

}

/**
    *@brief Lit les LSB (lower significant bit) de chaque octet de l'image d'entrée afin de récupérer le message caché
    *@param image_entree Image dans laquelle on cherche le message.
    *@param message Fichier texte contenant le message à récupérer
    *@param cle Tableau d'entier formant la clé
    *@param taille_message Variable contenant le message
    *@return Retourne un tableau d'entiers allant de 0 à taille disposés dans un ordre aléatoire
    */
void traiter_image(unsigned char* image_entree, FILE* message, long* cle, int taille_message)
{
    unsigned char octet_entree;
    unsigned char octet_sortie = 0;
    unsigned char masque_binaire;


         for (int i = 0; i < taille_message/8; i++) {
                octet_sortie = 0;
            for (int j = 0; j < 8; j++) {
                octet_entree = image_entree[cle[i*8+j]]; //pour chaque bit du message caché on récupère l'octet indiqué par la clé afin d'en récupérer le LSB
                octet_sortie = octet_sortie | ((1&octet_entree) << (j)); // 7-j pour faire dans l'ordre "droit" ou j pour faire dans l'ordre "croisé"
            }
            fprintf(message, "%c", octet_sortie);
         }

}

/**
    *@brief Procédure réalisant l'écriture du message dans l'image
    */
void ecrire_image()
{
        FILE* image_ouverte;
        FILE* image_sortie;
        FILE* fichier_cle;
        char nom_image[100];
        char nom_image_code[100];
        char nom_image_cle[100];
        unsigned char* tab;

        int taille_x;
        int taille_y;
        int max;
        unsigned char type[3];

        int taille_message;
        unsigned char* message;

        int p_masque;
        unsigned char octet_bin[9];
        //unsigned char octet_lu;
        unsigned char masque;

        int nombre_alea;
        unsigned char* f_tableau;
        long* cle;

        //On récupère le nom de l'image et on ouvre les fichiers d'entrée et de sortie
        printf("Entrez le nom de l'image dans laquelle vous souhaitez ecrire (sans l'extension .ppm).");
        scanf("%s", nom_image);
        strcpy(nom_image_code, nom_image);
        strcpy(nom_image_cle, nom_image);
        strcat(nom_image, ".ppm");
        image_ouverte = fopen(nom_image, "rb");
        strcat(nom_image_code, "_code.ppm");
        image_sortie = fopen(nom_image_code, "wb");
        strcat(nom_image_cle, ".key");
        fichier_cle = fopen(nom_image_cle, "wb");


        printf("Entrez la taille du message que vous souhaitez ecrire.");
        scanf("%d", &taille_message);
        taille_message++;
        getchar();
        message = (unsigned char*)malloc((taille_message)*sizeof(unsigned char));
        printf("Entrez le message.");
        fgets(message, taille_message, stdin);

        // passer l'entête
        recup_entete(image_ouverte, type, &taille_x, &taille_y, &max);
        fprintf(image_sortie, "%s\n%d %d %d\n", type, taille_x, taille_y, max);

        cle = creer_cle(taille_message*8);
        f_tableau = conv_octets_tableau(image_ouverte);

        for (int i = 0; i < taille_message; i++) {
            for (p_masque = 0; p_masque < 8; p_masque++) {
                masque = pow(2, p_masque);
                unsigned char masqued_message = message[i]&masque;
                nombre_alea = rand()%2;
                if (masqued_message == masque) { // on regarde si l'octet que l'on doit obtenir est un 1
                    if (f_tableau[cle[i*8+p_masque]] &1 == 0) { //si l'octet que l'on modifie est un 0, on le modifie
                        if (nombre_alea == 0 || f_tableau[cle[i*8+p_masque]] == 0) { // on l'incrémente ou décrémente aléatoirement SAUF si l'octet vaut 0 auquel cas on l'incrémente
                            f_tableau[cle[i*8+p_masque]]++;
                        }
                        else {
                            f_tableau[cle[i*8+p_masque]]--;
                        }
                    }
                }

                else { // l'octet que l'on doit obtenir est un 0
                    if (f_tableau[cle[i*8+p_masque]] &1 == 1) { //si l'octet que l'on modifie est un 1, on le modifie
                        if (nombre_alea == 1 || f_tableau[cle[i*8+p_masque]] == 255) { // on l'incrémente ou décrémente aléatoirement SAUF si l'octet vaut 255 auquel cas on le décrémente
                            f_tableau[cle[i*8+p_masque]]--;
                        }
                        else {
                            f_tableau[cle[i*8+p_masque]]++;
                        }
                    }
                }
            }
        }

        // on écrit toute l'image dans le fichier de sortie
        for (int i = 0; i <= taille_x*taille_y*3; i++) {
            fprintf(image_sortie, "%c", f_tableau[i]);
        }



        //on écrit la clé dans le fichier de sortie
        for (int i = 0; i < taille_message*8; i++) {
            fprintf(fichier_cle, "%ld ", cle[i]);
        }

        fclose(image_ouverte);
        fclose(image_sortie);
        fclose(fichier_cle);

        printf("Votre image a ete encodee avec succes.");

}

/**
    *@brief Procédure principale du programme.
    */
int main()
{
    FILE* image_ouverte;
    FILE* image_sortie;
    FILE* fichier_cle;
    char nom_image[100];
    unsigned char* tab;

    int taille_x;
    int taille_y;
    int max;
    unsigned char type[3];

    int choix;
    long taille_message;
    unsigned char* message;

    int p_masque;
    unsigned char octet_bin[9];
    unsigned char octet_lu;
    unsigned char masque;

    long* cle;
    unsigned char* tabl_image;

    int nombre_alea;

    srand(time(NULL));
    printf("ENCODEUR-DECODEUR STEGANOGRAPHIE - PROJET NF05\nFait par Arthur Levasseur\n\n");
    printf("Souhaitez vous lire (0) ou ecrire (1) une image ?");
    scanf("%d", &choix);

    if (choix == 0) {


        printf("Entrez le nom de l'image que vous souhaitez ouvrir (avec l'extension .ppm).");
        scanf("%s", nom_image);
        image_ouverte = fopen(nom_image, "rb");
        printf("Entrez le nom de la cle.");
        scanf("%s", nom_image);
        fichier_cle = fopen(nom_image, "rb");
        image_sortie = fopen("message.txt", "wb");

        recup_entete(image_ouverte, type, &taille_x, &taille_y, &max);

        cle = conv_cle_tableau(fichier_cle, &taille_message);
        tabl_image = conv_octets_tableau(image_ouverte);

        traiter_image(tabl_image, image_sortie, cle, taille_message);

        printf("Votre image a bien ete lue. Le message est accessible dans le fichier message.txt.");

        fclose(image_ouverte);
        fclose(image_sortie);
    }
    else {

    ecrire_image();

}
}
