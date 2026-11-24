#include <limits.h>
#include <stdio.h>

#include "ui.h"
#include "utils.h"

#define PRIX_MAX   1000000.0f
#define QTE_MAX    1000000

/* ===================================================================== */
/*                            MENU PRINCIPAL                             */
/* ===================================================================== */

int ui_menu_principal(void)
{
    printf("\n========================================\n");
    printf("          GESTION DE MAGASIN\n");
    printf("========================================\n");
    printf(" 1. Gestion des articles (stock)\n");
    printf(" 2. Gestion des clients\n");
    printf(" 3. Nouvelle vente\n");
    printf(" 4. Historique et statistiques\n");
    printf(" 5. Sauvegarder maintenant\n");
    printf(" 0. Quitter\n");
    printf("----------------------------------------\n");
    return lire_entier("Votre choix : ", 0, 5);
}

/* ===================================================================== */
/*                               ARTICLES                                */
/* ===================================================================== */

static void article_ajouter_ui(Magasin *m)
{
    char nom[TAILLE_NOM], cat[TAILLE_CATEGORIE];

    printf("\n--- Ajouter un article ---\n");
    lire_texte("Nom : ", nom, sizeof nom, 1);
    if (article_nom_existe(m->articles, nom)) {
        printf("Erreur : un article porte déjà ce nom.\n");
        return;
    }
    lire_texte("Catégorie (facultatif) : ", cat, sizeof cat, 0);
    float prix = lire_reel("Prix unitaire (DT) : ", 0.001f, PRIX_MAX);
    int qte = lire_entier("Quantité en stock : ", 0, QTE_MAX);
    int seuil = lire_entier("Seuil d'alerte : ", 0, QTE_MAX);

    Article *a = article_creer(m->prochain_id_article, nom, cat, prix, qte, seuil);
    if (!a) {
        printf("Erreur : mémoire insuffisante, article non ajouté.\n");
        return;
    }
    m->articles = article_ajouter_fin(m->articles, a);
    m->prochain_id_article++;
    printf("Article ajouté (ID %d).\n", a->id);
}

static void article_modifier_ui(Magasin *m)
{
    char nom[TAILLE_NOM], cat[TAILLE_CATEGORIE];

    printf("\n--- Modifier un article ---\n");
    if (!m->articles) {
        printf("Aucun article en stock.\n");
        return;
    }
    int id = lire_entier("ID de l'article : ", 1, INT_MAX);
    Article *a = article_chercher_id(m->articles, id);
    if (!a) {
        printf("Erreur : article introuvable.\n");
        return;
    }
    printf("Actuel : %s | %s | %.3f DT | qté %d | seuil %d\n",
           a->nom, a->categorie, a->prix, a->quantite, a->seuil_alerte);

    lire_texte("Nouveau nom (Entrée = inchangé) : ", nom, sizeof nom, 0);
    if (nom[0] && !egal_ignore_casse(nom, a->nom) && article_nom_existe(m->articles, nom)) {
        printf("Erreur : un autre article porte déjà ce nom.\n");
        return;
    }
    lire_texte("Nouvelle catégorie (Entrée = inchangée) : ", cat, sizeof cat, 0);
    float prix = lire_reel("Nouveau prix (DT) : ", 0.001f, PRIX_MAX);
    int qte = lire_entier("Nouvelle quantité : ", 0, QTE_MAX);
    int seuil = lire_entier("Nouveau seuil d'alerte : ", 0, QTE_MAX);

    if (article_modifier(m->articles, id, nom, cat, prix, qte, seuil) == ARTICLE_OK)
        printf("Article modifié.\n");
    else
        printf("Erreur : modification impossible.\n");
}

static void article_supprimer_ui(Magasin *m)
{
    printf("\n--- Supprimer un article ---\n");
    if (!m->articles) {
        printf("Aucun article en stock.\n");
        return;
    }
    int id = lire_entier("ID de l'article : ", 1, INT_MAX);
    const Article *a = article_trouver(m->articles, id);
    if (!a) {
        printf("Erreur : article introuvable.\n");
        return;
    }
    printf("Article : %s (qté %d)\n", a->nom, a->quantite);
    if (!confirmer("Confirmer la suppression"))
        return;

    int succes = 0;
    m->articles = article_supprimer(m->articles, id, &succes);
    printf(succes ? "Article supprimé.\n" : "Erreur : suppression impossible.\n");
}

static void article_rechercher_ui(const Magasin *m)
{
    char motif[TAILLE_NOM];
    printf("\n--- Rechercher un article ---\n");
    lire_texte("Nom ou catégorie contient : ", motif, sizeof motif, 1);
    if (article_rechercher_afficher(m->articles, motif) == 0)
        printf("Aucun article ne correspond à « %s ».\n", motif);
}

static void article_trier_ui(Magasin *m)
{
    printf("\n--- Trier les articles ---\n");
    if (!m->articles) {
        printf("Aucun article en stock.\n");
        return;
    }
    printf(" 1. Par nom\n 2. Par prix\n 3. Par quantité\n");
    int critere = lire_entier("Critère : ", 1, 3);
    printf(" 1. Croissant\n 2. Décroissant\n");
    int ordre = lire_entier("Ordre : ", 1, 2);

    m->articles = article_trier(m->articles, critere, ordre == 1);
    article_afficher_tous(m->articles);
}

void ui_menu_articles(Magasin *m)
{
    int choix;
    do {
        printf("\n===== ARTICLES =====\n");
        printf(" 1. Ajouter\n 2. Modifier\n 3. Supprimer\n 4. Rechercher\n");
        printf(" 5. Afficher tous\n 6. Trier\n 7. Stock faible\n 0. Retour\n");
        choix = lire_entier("Votre choix : ", 0, 7);
        switch (choix) {
        case 1: article_ajouter_ui(m); break;
        case 2: article_modifier_ui(m); break;
        case 3: article_supprimer_ui(m); break;
        case 4: article_rechercher_ui(m); break;
        case 5: printf("\n"); article_afficher_tous(m->articles); break;
        case 6: article_trier_ui(m); break;
        case 7:
            printf("\n");
            if (article_afficher_stock_faible(m->articles) == 0)
                printf("Aucun article en stock faible.\n");
            break;
        default: break;
        }
    } while (choix != 0);
}

/* ===================================================================== */
/*                                CLIENTS                                */
/* ===================================================================== */

static void client_ajouter_ui(Magasin *m)
{
    char nom[TAILLE_NOM], tel[TAILLE_TEL];

    printf("\n--- Ajouter un client ---\n");
    lire_texte("Nom : ", nom, sizeof nom, 1);
    lire_texte("Téléphone (facultatif) : ", tel, sizeof tel, 0);

    Client *c = client_creer(m->prochain_id_client, nom, tel);
    if (!c) {
        printf("Erreur : mémoire insuffisante, client non ajouté.\n");
        return;
    }
    m->clients = client_ajouter_fin(m->clients, c);
    m->prochain_id_client++;
    printf("Client ajouté (ID %d).\n", c->id);
}

static void client_modifier_ui(Magasin *m)
{
    char nom[TAILLE_NOM], tel[TAILLE_TEL];

    printf("\n--- Modifier un client ---\n");
    if (!m->clients) {
        printf("Aucun client enregistré.\n");
        return;
    }
    int id = lire_entier("ID du client : ", 1, INT_MAX);
    const Client *c = client_trouver(m->clients, id);
    if (!c) {
        printf("Erreur : client introuvable.\n");
        return;
    }
    printf("Actuel : %s | %s\n", c->nom, c->telephone);
    lire_texte("Nouveau nom (Entrée = inchangé) : ", nom, sizeof nom, 0);
    lire_texte("Nouveau téléphone (Entrée = inchangé) : ", tel, sizeof tel, 0);

    if (client_modifier(m->clients, id, nom, tel) == CLIENT_OK)
        printf("Client modifié.\n");
    else
        printf("Erreur : modification impossible.\n");
}

static void client_supprimer_ui(Magasin *m)
{
    printf("\n--- Supprimer un client ---\n");
    if (!m->clients) {
        printf("Aucun client enregistré.\n");
        return;
    }
    int id = lire_entier("ID du client : ", 1, INT_MAX);
    const Client *c = client_trouver(m->clients, id);
    if (!c) {
        printf("Erreur : client introuvable.\n");
        return;
    }
    printf("Client : %s\n", c->nom);
    if (!confirmer("Confirmer la suppression"))
        return;

    int succes = 0;
    m->clients = client_supprimer(m->clients, id, &succes);
    printf(succes ? "Client supprimé.\n" : "Erreur : suppression impossible.\n");
}

static void client_rechercher_ui(const Magasin *m)
{
    char motif[TAILLE_NOM];
    printf("\n--- Rechercher un client ---\n");
    lire_texte("Nom ou téléphone contient : ", motif, sizeof motif, 1);
    if (client_rechercher_afficher(m->clients, motif) == 0)
        printf("Aucun client ne correspond à « %s ».\n", motif);
}

void ui_menu_clients(Magasin *m)
{
    int choix;
    do {
        printf("\n===== CLIENTS =====\n");
        printf(" 1. Ajouter\n 2. Modifier\n 3. Supprimer\n 4. Rechercher\n");
        printf(" 5. Afficher tous\n 0. Retour\n");
        choix = lire_entier("Votre choix : ", 0, 5);
        switch (choix) {
        case 1: client_ajouter_ui(m); break;
        case 2: client_modifier_ui(m); break;
        case 3: client_supprimer_ui(m); break;
        case 4: client_rechercher_ui(m); break;
        case 5: printf("\n"); client_afficher_tous(m->clients); break;
        default: break;
        }
    } while (choix != 0);
}

/* ===================================================================== */
/*                                 VENTES                                */
/* ===================================================================== */

static const char *message_erreur_vente(int code)
{
    switch (code) {
    case VENTE_ERR_ARTICLE: return "Article introuvable.";
    case VENTE_ERR_STOCK:   return "Stock insuffisant (en tenant compte du panier).";
    case VENTE_ERR_QTE:     return "Quantité invalide.";
    case VENTE_ERR_MEM:     return "Mémoire insuffisante.";
    case VENTE_ERR_VIDE:    return "Le panier est vide.";
    case VENTE_ERR_INDEX:   return "Numéro de ligne invalide.";
    default:                return "Erreur inconnue.";
    }
}

static int choisir_client(const Magasin *m)
{
    for (;;) {
        int id = lire_entier("ID du client (0 = client anonyme) : ", 0, INT_MAX);
        if (id == 0 || client_trouver(m->clients, id))
            return id;
        printf("Client introuvable (consultez le menu Clients pour voir les ID).\n");
    }
}

void ui_nouvelle_vente(Magasin *m)
{
    printf("\n===== NOUVELLE VENTE =====\n");
    if (!m->articles) {
        printf("Aucun article en stock : impossible de vendre.\n");
        return;
    }

    int id_client = choisir_client(m);
    char date[TAILLE_DATE];
    date_du_jour(date, sizeof date);

    Vente *v = vente_creer(m->prochain_id_vente, date, id_client);
    if (!v) {
        printf("Erreur : mémoire insuffisante.\n");
        return;
    }

    int fini = 0;
    while (!fini) {
        printf("\n--- Panier (%d ligne(s), total %.3f DT) ---\n", v->nb_lignes, v->total);
        printf(" 1. Ajouter un article\n 2. Retirer une ligne\n 3. Voir le panier\n");
        printf(" 4. Valider la vente\n 5. Afficher les articles\n 0. Annuler la vente\n");
        int choix = lire_entier("Votre choix : ", 0, 5);

        switch (choix) {
        case 1: {
            int id = lire_entier("ID de l'article : ", 1, INT_MAX);
            int qte = lire_entier("Quantité : ", 1, QTE_MAX);
            int rc = vente_ajouter_ligne(v, m->articles, id, qte);
            if (rc == VENTE_OK)
                printf("Ligne ajoutée.\n");
            else
                printf("Erreur : %s\n", message_erreur_vente(rc));
            break;
        }
        case 2:
            if (v->nb_lignes == 0) {
                printf("Le panier est vide.\n");
            } else {
                vente_afficher(v, m->articles, m->clients, "PANIER");
                int n = lire_entier("Numéro de la ligne à retirer : ", 1, v->nb_lignes);
                vente_retirer_ligne(v, n - 1);
                printf("Ligne retirée.\n");
            }
            break;
        case 3:
            vente_afficher(v, m->articles, m->clients, "PANIER");
            break;
        case 4: {
            int rc = vente_valider(v, m->articles);
            if (rc == VENTE_OK) {
                m->ventes = historique_ajouter_fin(m->ventes, v);
                m->prochain_id_vente++;
                vente_afficher(v, m->articles, m->clients, "TICKET DE VENTE");
                printf("\nVente enregistrée, stock mis à jour.\n");
                v = NULL;
                fini = 1;
            } else {
                printf("Erreur : %s\n", message_erreur_vente(rc));
            }
            break;
        }
        case 5:
            printf("\n");
            article_afficher_tous(m->articles);
            break;
        case 0:
            if (v->nb_lignes > 0 && !confirmer("Abandonner ce panier"))
                break;
            fini = 1;
            printf("Vente annulée.\n");
            break;
        }
    }
    vente_liberer(v);          /* NULL si la vente a été validée */
}

void ui_statistiques(Magasin *m)
{
    int choix;
    do {
        printf("\n===== HISTORIQUE ET STATISTIQUES =====\n");
        printf(" 1. Historique des ventes\n 2. Détail d'une vente\n");
        printf(" 3. Chiffre d'affaires total\n 4. Article le plus vendu\n 0. Retour\n");
        choix = lire_entier("Votre choix : ", 0, 4);
        switch (choix) {
        case 1:
            printf("\n");
            historique_afficher(m->ventes, m->clients);
            break;
        case 2:
            if (!m->ventes) {
                printf("Aucune vente enregistrée.\n");
            } else {
                int id = lire_entier("ID de la vente : ", 1, INT_MAX);
                const Vente *v = vente_trouver(m->ventes, id);
                if (v)
                    vente_afficher(v, m->articles, m->clients, "TICKET DE VENTE");
                else
                    printf("Erreur : vente introuvable.\n");
            }
            break;
        case 3:
            printf("\nChiffre d'affaires total : %.3f DT\n",
                   historique_chiffre_affaires(m->ventes));
            break;
        case 4: {
            int qte = 0;
            const Article *a = historique_article_plus_vendu(m->ventes, m->articles, &qte);
            if (a)
                printf("\nArticle le plus vendu : %s (%d unités)\n", a->nom, qte);
            else
                printf("\nAucune vente enregistrée.\n");
            break;
        }
        default: break;
        }
    } while (choix != 0);
}
