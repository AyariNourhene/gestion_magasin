#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vente.h"

Vente *vente_creer(int id, const char *date, int id_client)
{
    Vente *v = calloc(1, sizeof *v);
    if (!v)
        return NULL;
    v->id = id;
    snprintf(v->date, sizeof v->date, "%s", date ? date : "");
    v->id_client = id_client;
    return v;
}

static int trouver_ligne(const Vente *v, int id_article)
{
    for (int i = 0; i < v->nb_lignes; i++)
        if (v->lignes[i].id_article == id_article)
            return i;
    return -1;
}

int vente_ajouter_ligne(Vente *v, const Article *stock, int id_article, int qte)
{
    if (!v)
        return VENTE_ERR_ARG;
    if (qte <= 0)
        return VENTE_ERR_QTE;

    const Article *a = article_trouver(stock, id_article);
    if (!a)
        return VENTE_ERR_ARTICLE;

    int idx = trouver_ligne(v, id_article);
    int deja = (idx >= 0) ? v->lignes[idx].quantite : 0;
    if (qte > a->quantite - deja)                 /* tient compte du panier */
        return VENTE_ERR_STOCK;

    if (idx >= 0) {                               /* article déjà dans le panier */
        v->lignes[idx].quantite += qte;
    } else {
        if (v->nb_lignes == v->capacite) {        /* agrandir le tableau */
            int nouvelle = v->capacite ? v->capacite * 2 : 4;
            LigneVente *tmp = realloc(v->lignes, (size_t)nouvelle * sizeof *tmp);
            if (!tmp)
                return VENTE_ERR_MEM;
            v->lignes = tmp;
            v->capacite = nouvelle;
        }
        v->lignes[v->nb_lignes].id_article = id_article;
        v->lignes[v->nb_lignes].quantite = qte;
        v->lignes[v->nb_lignes].prix_unitaire = a->prix;
        v->nb_lignes++;
    }
    v->total = vente_calculer_total(v);
    return VENTE_OK;
}

int vente_retirer_ligne(Vente *v, int index)
{
    if (!v)
        return VENTE_ERR_ARG;
    if (index < 0 || index >= v->nb_lignes)
        return VENTE_ERR_INDEX;
    for (int i = index; i < v->nb_lignes - 1; i++)
        v->lignes[i] = v->lignes[i + 1];
    v->nb_lignes--;
    v->total = vente_calculer_total(v);
    return VENTE_OK;
}

float vente_calculer_total(const Vente *v)
{
    float total = 0.0f;
    if (!v)
        return total;
    for (int i = 0; i < v->nb_lignes; i++)
        total += v->lignes[i].quantite * v->lignes[i].prix_unitaire;
    return total;
}

int vente_valider(Vente *v, Article *stock)
{
    if (!v)
        return VENTE_ERR_ARG;
    if (v->nb_lignes == 0)
        return VENTE_ERR_VIDE;

    /* 1) tout vérifier avant de modifier quoi que ce soit */
    for (int i = 0; i < v->nb_lignes; i++) {
        const Article *a = article_trouver(stock, v->lignes[i].id_article);
        if (!a)
            return VENTE_ERR_ARTICLE;
        if (a->quantite < v->lignes[i].quantite)
            return VENTE_ERR_STOCK;
    }
    /* 2) décrémenter le stock */
    for (int i = 0; i < v->nb_lignes; i++) {
        Article *a = article_chercher_id(stock, v->lignes[i].id_article);
        a->quantite -= v->lignes[i].quantite;
    }
    v->total = vente_calculer_total(v);
    return VENTE_OK;
}

Vente *historique_ajouter_fin(Vente *tete, Vente *v)
{
    if (!v)
        return tete;
    v->suivant = NULL;
    if (!tete)
        return v;
    Vente *cur = tete;
    while (cur->suivant)
        cur = cur->suivant;
    cur->suivant = v;
    return tete;
}

const Vente *vente_trouver(const Vente *tete, int id)
{
    for (const Vente *v = tete; v; v = v->suivant)
        if (v->id == id)
            return v;
    return NULL;
}

int historique_prochain_id(const Vente *tete)
{
    int max = 0;
    for (const Vente *v = tete; v; v = v->suivant)
        if (v->id > max)
            max = v->id;
    return max + 1;
}

float historique_chiffre_affaires(const Vente *tete)
{
    float ca = 0.0f;
    for (const Vente *v = tete; v; v = v->suivant)
        ca += v->total;
    return ca;
}

int historique_quantite_vendue(const Vente *tete, int id_article)
{
    int total = 0;
    for (const Vente *v = tete; v; v = v->suivant)
        for (int i = 0; i < v->nb_lignes; i++)
            if (v->lignes[i].id_article == id_article)
                total += v->lignes[i].quantite;
    return total;
}

const Article *historique_article_plus_vendu(const Vente *tete, const Article *stock,
                                             int *quantite)
{
    const Article *meilleur = NULL;
    int max = 0;
    for (const Article *a = stock; a; a = a->suivant) {
        int q = historique_quantite_vendue(tete, a->id);
        if (q > max) {
            max = q;
            meilleur = a;
        }
    }
    if (quantite)
        *quantite = max;
    return meilleur;
}

static void nom_client(const Client *clients, int id, char *dest, size_t taille)
{
    if (id == 0) {
        snprintf(dest, taille, "Anonyme");
        return;
    }
    const Client *c = client_trouver(clients, id);
    if (c)
        snprintf(dest, taille, "%s", c->nom);
    else
        snprintf(dest, taille, "Client #%d (supprimé)", id);
}

void vente_afficher(const Vente *v, const Article *stock, const Client *clients,
                    const char *titre)
{
    char client[TAILLE_NOM + 32];
    nom_client(clients, v->id_client, client, sizeof client);

    printf("\n============ %s #%d ============\n", titre, v->id);
    printf("Date   : %s\n", v->date);
    printf("Client : %s\n", client);
    if (v->nb_lignes == 0) {
        printf("(panier vide)\n");
        return;
    }
    printf("%-4s %-24s %5s %11s %12s\n", "N", "Article", "Qte", "P.U. (DT)", "Total (DT)");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < v->nb_lignes; i++) {
        const LigneVente *l = &v->lignes[i];
        const Article *a = article_trouver(stock, l->id_article);
        printf("%-4d %-24.24s %5d %11.3f %12.3f\n", i + 1,
               a ? a->nom : "(article supprimé)", l->quantite, l->prix_unitaire,
               l->quantite * l->prix_unitaire);
    }
    printf("------------------------------------------------------------\n");
    printf("TOTAL A PAYER : %.3f DT\n", vente_calculer_total(v));
}

void historique_afficher(const Vente *tete, const Client *clients)
{
    if (!tete) {
        printf("Aucune vente enregistrée.\n");
        return;
    }
    printf("%-5s %-11s %-24s %8s %12s\n", "ID", "Date", "Client", "Lignes", "Total (DT)");
    printf("-------------------------------------------------------------------\n");
    for (const Vente *v = tete; v; v = v->suivant) {
        char client[TAILLE_NOM + 32];
        nom_client(clients, v->id_client, client, sizeof client);
        printf("%-5d %-11s %-24.24s %8d %12.3f\n", v->id, v->date, client,
               v->nb_lignes, v->total);
    }
}

void vente_liberer(Vente *v)
{
    if (!v)
        return;
    free(v->lignes);
    free(v);
}

void historique_liberer(Vente *tete)
{
    while (tete) {
        Vente *suivant = tete->suivant;
        vente_liberer(tete);
        tete = suivant;
    }
}
