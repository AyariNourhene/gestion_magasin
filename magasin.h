#ifndef MAGASIN_H
#define MAGASIN_H

#include "article.h"
#include "client.h"
#include "vente.h"

/* État complet du programme, tenu en mémoire (listes chaînées).
   La base SQLite ne sert qu'à la sauvegarde / au rechargement. */
typedef struct {
    Article *articles;
    Client  *clients;
    Vente   *ventes;
    int prochain_id_article;
    int prochain_id_client;
    int prochain_id_vente;
} Magasin;

void magasin_init(Magasin *m);
void magasin_liberer(Magasin *m);

#endif
