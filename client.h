#ifndef CLIENT_H
#define CLIENT_H

#include "article.h"      /* TAILLE_NOM */

#define TAILLE_TEL 15

#define CLIENT_OK              0
#define CLIENT_ERR_INTROUVABLE (-1)

typedef struct Client {
    int  id;
    char nom[TAILLE_NOM];
    char telephone[TAILLE_TEL];
    struct Client *suivant;
} Client;

Client *client_creer(int id, const char *nom, const char *telephone);
Client *client_ajouter_fin(Client *tete, Client *nouveau);
Client *client_supprimer(Client *tete, int id, int *succes);

Client       *client_chercher_id(Client *tete, int id);
const Client *client_trouver(const Client *tete, int id);
int           client_modifier(Client *tete, int id, const char *nom, const char *telephone);

void client_afficher_tous(const Client *tete);
int  client_rechercher_afficher(const Client *tete, const char *motif);

int  client_prochain_id(const Client *tete);
void client_liberer(Client *tete);

#endif
