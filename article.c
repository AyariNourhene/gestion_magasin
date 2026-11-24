#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "article.h"
#include "utils.h"

Article *article_creer(int id, const char *nom, const char *categorie,
                       float prix, int quantite, int seuil)
{
    if (!nom || nom[0] == '\0' || !(prix > 0.0f) || quantite < 0 || seuil < 0)
        return NULL;

    Article *a = malloc(sizeof *a);
    if (!a)
        return NULL;

    a->id = id;
    snprintf(a->nom, sizeof a->nom, "%s", nom);
    snprintf(a->categorie, sizeof a->categorie, "%s", categorie ? categorie : "");
    a->prix = prix;
    a->quantite = quantite;
    a->seuil_alerte = seuil;
    a->suivant = NULL;
    return a;
}

Article *article_ajouter_fin(Article *tete, Article *nouveau)
{
    if (!nouveau)
        return tete;
    nouveau->suivant = NULL;
    if (!tete)
        return nouveau;
    Article *cur = tete;
    while (cur->suivant)
        cur = cur->suivant;
    cur->suivant = nouveau;
    return tete;
}

Article *article_supprimer(Article *tete, int id, int *succes)
{
    Article *prec = NULL, *cur = tete;
    while (cur && cur->id != id) {
        prec = cur;
        cur = cur->suivant;
    }
    if (!cur) {
        if (succes)
            *succes = 0;
        return tete;
    }
    if (prec)
        prec->suivant = cur->suivant;
    else
        tete = cur->suivant;
    free(cur);
    if (succes)
        *succes = 1;
    return tete;
}

Article *article_chercher_id(Article *tete, int id)
{
    for (Article *a = tete; a; a = a->suivant)
        if (a->id == id)
            return a;
    return NULL;
}

const Article *article_trouver(const Article *tete, int id)
{
    for (const Article *a = tete; a; a = a->suivant)
        if (a->id == id)
            return a;
    return NULL;
}

int article_nom_existe(const Article *tete, const char *nom)
{
    for (const Article *a = tete; a; a = a->suivant)
        if (egal_ignore_casse(a->nom, nom))
            return 1;
    return 0;
}

int article_modifier(Article *tete, int id, const char *nom, const char *categorie,
                     float prix, int quantite, int seuil)
{
    Article *a = article_chercher_id(tete, id);
    if (!a)
        return ARTICLE_ERR_INTROUVABLE;
    if (!(prix > 0.0f) || quantite < 0 || seuil < 0)
        return ARTICLE_ERR_VALEUR;

    if (nom && nom[0] != '\0')
        snprintf(a->nom, sizeof a->nom, "%s", nom);
    if (categorie && categorie[0] != '\0')
        snprintf(a->categorie, sizeof a->categorie, "%s", categorie);
    a->prix = prix;
    a->quantite = quantite;
    a->seuil_alerte = seuil;
    return ARTICLE_OK;
}

static int comparer(const Article *a, const Article *b, int critere)
{
    switch (critere) {
    case ARTICLE_TRI_PRIX:
        return (a->prix > b->prix) - (a->prix < b->prix);
    case ARTICLE_TRI_QUANTITE:
        return (a->quantite > b->quantite) - (a->quantite < b->quantite);
    case ARTICLE_TRI_NOM:
    default:
        return comparer_ignore_casse(a->nom, b->nom);
    }
}

/* Tri par insertion sur liste chaînée (stable). */
Article *article_trier(Article *tete, int critere, int croissant)
{
    Article *trie = NULL;
    while (tete) {
        Article *cur = tete;
        tete = tete->suivant;

        Article **p = &trie;
        while (*p) {
            int c = comparer(*p, cur, critere);
            if (!croissant)
                c = -c;
            if (c > 0)
                break;
            p = &(*p)->suivant;
        }
        cur->suivant = *p;
        *p = cur;
    }
    return trie;
}

static void afficher_entete(void)
{
    printf("%-5s %-24s %-14s %12s %7s %7s\n",
           "ID", "Nom", "Categorie", "Prix (DT)", "Qte", "Seuil");
    printf("---------------------------------------------------------------------------\n");
}

static void afficher_ligne(const Article *a)
{
    printf("%-5d %-24.24s %-14.14s %12.3f %7d %7d%s\n",
           a->id, a->nom, a->categorie, a->prix, a->quantite, a->seuil_alerte,
           a->quantite <= a->seuil_alerte ? "  (!)" : "");
}

void article_afficher_tous(const Article *tete)
{
    if (!tete) {
        printf("Aucun article en stock.\n");
        return;
    }
    afficher_entete();
    for (const Article *a = tete; a; a = a->suivant)
        afficher_ligne(a);
    printf("(!) = stock faible (quantité <= seuil d'alerte)\n");
}

int article_rechercher_afficher(const Article *tete, const char *motif)
{
    int n = 0;
    for (const Article *a = tete; a; a = a->suivant) {
        if (contient_ignore_casse(a->nom, motif) ||
            contient_ignore_casse(a->categorie, motif)) {
            if (n == 0)
                afficher_entete();
            afficher_ligne(a);
            n++;
        }
    }
    return n;
}

int article_afficher_stock_faible(const Article *tete)
{
    int n = 0;
    for (const Article *a = tete; a; a = a->suivant) {
        if (a->quantite <= a->seuil_alerte) {
            if (n == 0)
                afficher_entete();
            afficher_ligne(a);
            n++;
        }
    }
    return n;
}

int article_prochain_id(const Article *tete)
{
    int max = 0;
    for (const Article *a = tete; a; a = a->suivant)
        if (a->id > max)
            max = a->id;
    return max + 1;
}

void article_liberer(Article *tete)
{
    while (tete) {
        Article *suivant = tete->suivant;
        free(tete);
        tete = suivant;
    }
}
