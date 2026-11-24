#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "utils.h"

Client *client_creer(int id, const char *nom, const char *telephone)
{
    if (!nom || nom[0] == '\0')
        return NULL;
    Client *c = malloc(sizeof *c);
    if (!c)
        return NULL;
    c->id = id;
    snprintf(c->nom, sizeof c->nom, "%s", nom);
    snprintf(c->telephone, sizeof c->telephone, "%s", telephone ? telephone : "");
    c->suivant = NULL;
    return c;
}

Client *client_ajouter_fin(Client *tete, Client *nouveau)
{
    if (!nouveau)
        return tete;
    nouveau->suivant = NULL;
    if (!tete)
        return nouveau;
    Client *cur = tete;
    while (cur->suivant)
        cur = cur->suivant;
    cur->suivant = nouveau;
    return tete;
}

Client *client_supprimer(Client *tete, int id, int *succes)
{
    Client *prec = NULL, *cur = tete;
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

Client *client_chercher_id(Client *tete, int id)
{
    for (Client *c = tete; c; c = c->suivant)
        if (c->id == id)
            return c;
    return NULL;
}

const Client *client_trouver(const Client *tete, int id)
{
    for (const Client *c = tete; c; c = c->suivant)
        if (c->id == id)
            return c;
    return NULL;
}

int client_modifier(Client *tete, int id, const char *nom, const char *telephone)
{
    Client *c = client_chercher_id(tete, id);
    if (!c)
        return CLIENT_ERR_INTROUVABLE;
    if (nom && nom[0] != '\0')
        snprintf(c->nom, sizeof c->nom, "%s", nom);
    if (telephone && telephone[0] != '\0')
        snprintf(c->telephone, sizeof c->telephone, "%s", telephone);
    return CLIENT_OK;
}

static void afficher_entete(void)
{
    printf("%-5s %-30s %-15s\n", "ID", "Nom", "Telephone");
    printf("-------------------------------------------------\n");
}

static void afficher_ligne(const Client *c)
{
    printf("%-5d %-30.30s %-15s\n", c->id, c->nom, c->telephone);
}

void client_afficher_tous(const Client *tete)
{
    if (!tete) {
        printf("Aucun client enregistré.\n");
        return;
    }
    afficher_entete();
    for (const Client *c = tete; c; c = c->suivant)
        afficher_ligne(c);
}

int client_rechercher_afficher(const Client *tete, const char *motif)
{
    int n = 0;
    for (const Client *c = tete; c; c = c->suivant) {
        if (contient_ignore_casse(c->nom, motif) ||
            contient_ignore_casse(c->telephone, motif)) {
            if (n == 0)
                afficher_entete();
            afficher_ligne(c);
            n++;
        }
    }
    return n;
}

int client_prochain_id(const Client *tete)
{
    int max = 0;
    for (const Client *c = tete; c; c = c->suivant)
        if (c->id > max)
            max = c->id;
    return max + 1;
}

void client_liberer(Client *tete)
{
    while (tete) {
        Client *suivant = tete->suivant;
        free(tete);
        tete = suivant;
    }
}
