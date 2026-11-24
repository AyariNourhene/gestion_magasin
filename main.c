#include <stdio.h>

#include "db.h"
#include "magasin.h"
#include "ui.h"
#include "utils.h"

#define FICHIER_DB_DEFAUT "magasin.db"

int main(int argc, char **argv)
{
    const char *fichier = (argc > 1) ? argv[1] : FICHIER_DB_DEFAUT;
    Magasin m;
    magasin_init(&m);

    int sauvegarde_auto = 1;
    if (db_charger(&m, fichier) != DB_OK) {
        fprintf(stderr, "\nAttention : le chargement de '%s' a échoué.\n", fichier);
        fprintf(stderr, "La sauvegarde automatique à la sortie est désactivée "
                        "pour ne pas écraser vos données.\n");
        magasin_liberer(&m);
        sauvegarde_auto = 0;
    } else {
        printf("Base '%s' chargée : %s.\n", fichier,
               m.articles || m.clients || m.ventes ? "données récupérées"
                                                   : "nouvelle base vide");
    }

    int choix;
    do {
        choix = ui_menu_principal();
        switch (choix) {
        case 1: ui_menu_articles(&m); break;
        case 2: ui_menu_clients(&m); break;
        case 3: ui_nouvelle_vente(&m); break;
        case 4: ui_statistiques(&m); break;
        case 5:
            if (!sauvegarde_auto &&
                !confirmer("Le chargement avait échoué : écraser la base existante"))
                break;
            if (db_sauvegarder(&m, fichier) == DB_OK) {
                printf("Sauvegarde réussie.\n");
                sauvegarde_auto = 1;
            } else {
                printf("Échec de la sauvegarde !\n");
            }
            break;
        default: break;
        }
    } while (choix != 0);

    if (sauvegarde_auto) {
        if (db_sauvegarder(&m, fichier) == DB_OK)
            printf("Données sauvegardées dans '%s'. Au revoir !\n", fichier);
        else
            fprintf(stderr, "Attention : la sauvegarde finale a échoué !\n");
    } else {
        printf("Au revoir (aucune sauvegarde effectuée).\n");
    }

    magasin_liberer(&m);
    return 0;
}
