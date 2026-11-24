#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils.h"

#define TAILLE_LIGNE 256

/* Lit une ligne. Retourne 0 si OK, 1 si la ligne était trop longue.
   En cas de fin d'entrée (EOF), le programme s'arrête proprement. */
static int lire_ligne(char *buf, size_t taille)
{
    if (!fgets(buf, (int)taille, stdin)) {
        printf("\nFin de l'entrée standard : arrêt du programme.\n");
        exit(EXIT_FAILURE);
    }
    size_t n = strlen(buf);
    if (n > 0 && buf[n - 1] == '\n') {
        buf[n - 1] = '\0';
        return 0;
    }
    int c = getchar();
    if (c == EOF || c == '\n')
        return 0;
    while ((c = getchar()) != '\n' && c != EOF)
        ;                                   /* vide le reste de la ligne */
    return 1;
}

static void supprimer_espaces(char *s)
{
    size_t debut = 0, fin = strlen(s);
    while (s[debut] && isspace((unsigned char)s[debut]))
        debut++;
    while (fin > debut && isspace((unsigned char)s[fin - 1]))
        fin--;
    memmove(s, s + debut, fin - debut);
    s[fin - debut] = '\0';
}

void lire_texte(const char *msg, char *dest, size_t taille, int obligatoire)
{
    char buf[TAILLE_LIGNE];
    for (;;) {
        printf("%s", msg);
        if (lire_ligne(buf, sizeof buf)) {
            printf("Saisie trop longue.\n");
            continue;
        }
        supprimer_espaces(buf);
        if (buf[0] == '\0' && obligatoire) {
            printf("Ce champ est obligatoire.\n");
            continue;
        }
        if (strlen(buf) >= taille) {
            printf("Saisie trop longue (maximum %d caractères).\n", (int)(taille - 1));
            continue;
        }
        strcpy(dest, buf);
        return;
    }
}

int lire_entier(const char *msg, int min, int max)
{
    char buf[TAILLE_LIGNE];
    for (;;) {
        printf("%s", msg);
        if (lire_ligne(buf, sizeof buf)) {
            printf("Saisie trop longue.\n");
            continue;
        }
        supprimer_espaces(buf);
        if (buf[0] == '\0') {
            printf("Veuillez saisir un nombre.\n");
            continue;
        }
        char *fin;
        errno = 0;
        long v = strtol(buf, &fin, 10);
        if (*fin != '\0' || errno == ERANGE) {
            printf("Saisie invalide : entrez un nombre entier.\n");
            continue;
        }
        if (v < min || v > max) {
            printf("Valeur hors limites (entre %d et %d).\n", min, max);
            continue;
        }
        return (int)v;
    }
}

float lire_reel(const char *msg, float min, float max)
{
    char buf[TAILLE_LIGNE];
    for (;;) {
        printf("%s", msg);
        if (lire_ligne(buf, sizeof buf)) {
            printf("Saisie trop longue.\n");
            continue;
        }
        supprimer_espaces(buf);
        if (buf[0] == '\0') {
            printf("Veuillez saisir un nombre.\n");
            continue;
        }
        for (char *p = buf; *p; p++)        /* accepte la virgule décimale */
            if (*p == ',')
                *p = '.';
        char *fin;
        errno = 0;
        float v = strtof(buf, &fin);
        if (*fin != '\0' || errno == ERANGE) {
            printf("Saisie invalide : entrez un nombre (ex. 12.5).\n");
            continue;
        }
        if (!(v >= min && v <= max)) {      /* rejette aussi NaN */
            printf("Valeur hors limites (entre %g et %g).\n", min, max);
            continue;
        }
        return v;
    }
}

int confirmer(const char *msg)
{
    char buf[TAILLE_LIGNE];
    for (;;) {
        printf("%s (o/n) : ", msg);
        if (lire_ligne(buf, sizeof buf))
            continue;
        supprimer_espaces(buf);
        if (buf[0] && buf[1] == '\0') {
            char c = (char)tolower((unsigned char)buf[0]);
            if (c == 'o' || c == 'y')
                return 1;
            if (c == 'n')
                return 0;
        }
        printf("Répondez par 'o' (oui) ou 'n' (non).\n");
    }
}

void attendre_entree(void)
{
    char buf[TAILLE_LIGNE];
    printf("\nAppuyez sur Entrée pour continuer...");
    lire_ligne(buf, sizeof buf);
}

void date_du_jour(char *dest, size_t taille)
{
    time_t t = time(NULL);
    struct tm *info = localtime(&t);
    if (!info || strftime(dest, taille, "%d/%m/%Y", info) == 0)
        snprintf(dest, taille, "01/01/1970");
}

int comparer_ignore_casse(const char *a, const char *b)
{
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb)
            return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int egal_ignore_casse(const char *a, const char *b)
{
    return comparer_ignore_casse(a, b) == 0;
}

int contient_ignore_casse(const char *texte, const char *motif)
{
    size_t lm = strlen(motif);
    if (lm == 0)
        return 1;
    for (; *texte; texte++) {
        size_t i = 0;
        while (i < lm && texte[i] &&
               tolower((unsigned char)texte[i]) == tolower((unsigned char)motif[i]))
            i++;
        if (i == lm)
            return 1;
    }
    return 0;
}
