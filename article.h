#ifndef ARTICLE_H
#define ARTICLE_H

#define TAILLE_NOM        50
#define TAILLE_CATEGORIE  30

#define ARTICLE_TRI_NOM       1
#define ARTICLE_TRI_PRIX      2
#define ARTICLE_TRI_QUANTITE  3

#define ARTICLE_OK              0
#define ARTICLE_ERR_INTROUVABLE (-1)
#define ARTICLE_ERR_VALEUR      (-2)

/* Un article du stock : maillon d'une liste simplement chaînée. */
typedef struct Article {
    int   id;
    char  nom[TAILLE_NOM];
    char  categorie[TAILLE_CATEGORIE];
    float prix;
    int   quantite;
    int   seuil_alerte;
    struct Article *suivant;
} Article;

/* Création : retourne NULL si les valeurs sont invalides ou si malloc échoue. */
Article *article_creer(int id, const char *nom, const char *categorie,
                       float prix, int quantite, int seuil);

Article *article_ajouter_fin(Article *tete, Article *nouveau);
Article *article_supprimer(Article *tete, int id, int *succes);

Article       *article_chercher_id(Article *tete, int id);
const Article *article_trouver(const Article *tete, int id);   /* version const */
int            article_nom_existe(const Article *tete, const char *nom);

/* nom / categorie vides (ou NULL) = valeur inchangée. */
int article_modifier(Article *tete, int id, const char *nom, const char *categorie,
                     float prix, int quantite, int seuil);

/* Tri stable par déplacement de maillons (pas de copie de champs). */
Article *article_trier(Article *tete, int critere, int croissant);

void article_afficher_tous(const Article *tete);
int  article_rechercher_afficher(const Article *tete, const char *motif);
int  article_afficher_stock_faible(const Article *tete);

int  article_prochain_id(const Article *tete);   /* max(id) + 1, au moins 1 */
void article_liberer(Article *tete);

#endif
