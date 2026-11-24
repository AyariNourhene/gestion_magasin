#include <string.h>

#include "magasin.h"

void magasin_init(Magasin *m)
{
    memset(m, 0, sizeof *m);
    m->prochain_id_article = 1;
    m->prochain_id_client = 1;
    m->prochain_id_vente = 1;
}

void magasin_liberer(Magasin *m)
{
    article_liberer(m->articles);
    client_liberer(m->clients);
    historique_liberer(m->ventes);
    magasin_init(m);
}
