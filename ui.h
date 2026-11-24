#ifndef UI_H
#define UI_H

#include "magasin.h"

/* Affiche le menu principal et retourne le choix (0 à 5). */
int  ui_menu_principal(void);

void ui_menu_articles(Magasin *m);
void ui_menu_clients(Magasin *m);
void ui_nouvelle_vente(Magasin *m);
void ui_statistiques(Magasin *m);

#endif
