#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* Saisies sécurisées : elles bouclent tant que la saisie est invalide. */
void  lire_texte(const char *msg, char *dest, size_t taille, int obligatoire);
int   lire_entier(const char *msg, int min, int max);
float lire_reel(const char *msg, float min, float max);
int   confirmer(const char *msg);            /* 1 = oui, 0 = non */
void  attendre_entree(void);

/* Date du jour au format JJ/MM/AAAA (taille >= 11). */
void  date_du_jour(char *dest, size_t taille);

/* Comparaisons de chaînes insensibles à la casse. */
int   egal_ignore_casse(const char *a, const char *b);
int   comparer_ignore_casse(const char *a, const char *b);
int   contient_ignore_casse(const char *texte, const char *motif);

#endif
