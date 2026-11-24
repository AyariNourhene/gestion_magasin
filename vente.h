#ifndef VENTE_H
#define VENTE_H

#include "article.h"
#include "client.h"

#define TAILLE_DATE 11          /* JJ/MM/AAAA + '\0' */

#define VENTE_OK            0
#define VENTE_ERR_ARG      (-1)
#define VENTE_ERR_ARTICLE  (-2)  /* article inexistant */
#define VENTE_ERR_STOCK    (-3)  /* quantité demandée > stock disponible */
#define VENTE_ERR_QTE      (-4)  /* quantité <= 0 */
#define VENTE_ERR_MEM      (-5)
#define VENTE_ERR_VIDE     (-6)  /* panier vide */
#define VENTE_ERR_INDEX    (-7)

/* Une ligne du panier (prix figé au moment de l'ajout). */
typedef struct {
    int   id_article;
    int   quantite;
    float prix_unitaire;
} LigneVente;

/* Une vente : les lignes sont dans un tableau dynamique (malloc/realloc),
   l'historique des ventes est une liste chaînée. */
typedef struct Vente {
    int   id;
    char  date[TAILLE_DATE];
    int   id_client;            /* 0 = client anonyme */
    float total;
    LigneVente *lignes;
    int   nb_lignes;
    int   capacite;
    struct Vente *suivant;
} Vente;

Vente *vente_creer(int id, const char *date, int id_client);

/* Panier */
int   vente_ajouter_ligne(Vente *v, const Article *stock, int id_article, int qte);
int   vente_retirer_ligne(Vente *v, int index);
float vente_calculer_total(const Vente *v);

/* Validation : contrôle le stock, le décrémente et fige le total. */
int   vente_valider(Vente *v, Article *stock);

/* Historique */
Vente *historique_ajouter_fin(Vente *tete, Vente *v);
const Vente *vente_trouver(const Vente *tete, int id);
int   historique_prochain_id(const Vente *tete);
float historique_chiffre_affaires(const Vente *tete);
int   historique_quantite_vendue(const Vente *tete, int id_article);
const Article *historique_article_plus_vendu(const Vente *tete, const Article *stock,
                                             int *quantite);

/* Affichage */
void vente_afficher(const Vente *v, const Article *stock, const Client *clients,
                    const char *titre);
void historique_afficher(const Vente *tete, const Client *clients);

void vente_liberer(Vente *v);
void historique_liberer(Vente *tete);

#endif
