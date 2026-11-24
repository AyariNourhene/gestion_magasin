# Gestion de magasin (stocks et ventes) — Projet de programmation C

Mini-projet réalisé en binôme dans le cadre du cours de programmation C.
Application console en C pour gérer les **articles en stock**, les **clients** et les **ventes**.
Les données sont tenues **en mémoire** dans des listes chaînées et **sauvegardées dans une base SQLite**.

---

## 1. Arborescence

```
projet_magasin/
├── Makefile            # compilation (make, make run, make valgrind, make clean)
├── README.md           # cette documentation
├── main.c              # point d'entrée : chargement, menu principal itératif, sauvegarde
├── magasin.h / .c      # structure Magasin (état global : 3 listes + compteurs d'ID)
├── article.h / .c      # struct Article, liste chaînée : CRUD, recherche, tri, stock faible
├── client.h / .c       # struct Client, liste chaînée : CRUD, recherche
├── vente.h / .c        # struct Vente / LigneVente : panier (tableau dynamique),
│                       #   validation, historique (liste chaînée), statistiques, tickets
├── db.h / .c           # persistance SQLite : db_charger() et db_sauvegarder()
├── ui.h / .c           # interface console : tous les menus et sous-menus
├── utils.h / .c        # saisies sécurisées, date du jour, comparaisons de chaînes
└── sql/
    ├── schema.sql      # schéma des 4 tables (documentation / création manuelle)
    └── seed.sql        # jeu de données de démonstration (facultatif)
```

Le fichier de base de données `magasin.db` est créé automatiquement dans le dossier d'exécution.

---

## 2. Architecture

| Couche | Rôle | Fichiers |
|---|---|---|
| **Front** | Menus console, affichage en tableaux, saisies | `ui.c`, `utils.c` |
| **Back** | Logique métier sur listes chaînées / tableau dynamique | `article.c`, `client.c`, `vente.c`, `magasin.c` |
| **BD** | Sauvegarde et rechargement (SQLite) | `db.c` |

```
Démarrage :   SQLite  --db_charger()-->  listes chaînées (mémoire)
Utilisation : toutes les opérations travaillent en mémoire (aucun SQL)
Sauvegarde :  listes chaînées  --db_sauvegarder()-->  SQLite (1 transaction)
```

- **Structures de données** : liste chaînée pour les articles, les clients et l'historique des ventes ;
  tableau dynamique (`malloc` / `realloc`) pour les lignes d'un panier.
- **Sauvegarde** : à chaque sauvegarde, les tables sont vidées puis réécrites **dans une transaction**
  (`BEGIN` … `COMMIT`). En cas d'erreur : `ROLLBACK`, l'ancienne sauvegarde reste intacte.
- Le programme sauvegarde automatiquement en quittant (option 0) et à la demande (option 5).

---

## 3. Base de données SQL

### Faut-il créer la base à la main ? **Non.**

Au premier lancement, `db_charger()` ouvre (ou crée) le fichier `magasin.db` et exécute les
`CREATE TABLE IF NOT EXISTS` : **la base et les tables sont créées automatiquement**.
Il suffit que la bibliothèque SQLite soit installée (voir §4).

### Création manuelle (facultative)

Utile pour inspecter la base ou charger des données de démonstration :

```bash
# Création de la base avec ses tables
sqlite3 magasin.db < sql/schema.sql

# (facultatif) données de démonstration
sqlite3 magasin.db < sql/seed.sql

# Inspection
sqlite3 magasin.db "SELECT * FROM article;"
```

Sans l'outil `sqlite3` en ligne de commande, on peut utiliser **DB Browser for SQLite**
(interface graphique) ou Python :
`python3 -c "import sqlite3; c=sqlite3.connect('magasin.db'); c.executescript(open('sql/schema.sql').read())"`.

Pour repartir de zéro : supprimer le fichier `magasin.db`.

### Schéma relationnel

```
ARTICLE     (id, nom, categorie, prix, quantite, seuil_alerte)
CLIENT      (id, nom, telephone)
VENTE       (id, date_vente, id_client)
LIGNE_VENTE (#id_vente, id_article, quantite, prix_unitaire)
```

### Tables

**article**

| Colonne | Type | Contraintes |
|---|---|---|
| id | INTEGER | clé primaire |
| nom | TEXT | NOT NULL |
| categorie | TEXT | |
| prix | REAL | NOT NULL, `> 0` |
| quantite | INTEGER | NOT NULL, `>= 0` |
| seuil_alerte | INTEGER | NOT NULL, défaut 5, `>= 0` |

**client**

| Colonne | Type | Contraintes |
|---|---|---|
| id | INTEGER | clé primaire |
| nom | TEXT | NOT NULL |
| telephone | TEXT | |

**vente**

| Colonne | Type | Contraintes |
|---|---|---|
| id | INTEGER | clé primaire |
| date_vente | TEXT | NOT NULL, format `JJ/MM/AAAA` |
| id_client | INTEGER | NOT NULL, défaut 0 (**0 = client anonyme**) |
| total | REAL | NOT NULL, `>= 0` |

**ligne_vente**

| Colonne | Type | Contraintes |
|---|---|---|
| id_vente | INTEGER | NOT NULL, clé étrangère → `vente(id)` `ON DELETE CASCADE` |
| id_article | INTEGER | NOT NULL |
| quantite | INTEGER | NOT NULL, `> 0` |
| prix_unitaire | REAL | NOT NULL, `> 0` (prix figé au moment de la vente) |

Choix de conception :
- Pas de clé étrangère vers `article` ni `client` : on peut supprimer un article ou un client du
  stock sans perdre l'historique des ventes (les tickets affichent « article supprimé » /
  « client supprimé »).
- `prix_unitaire` est copié dans la ligne : modifier un prix plus tard ne change pas les anciennes ventes.
- Les identifiants sont attribués par le programme (compteurs en mémoire, recalculés au chargement).

---

## 4. Prérequis, compilation et exécution

**Prérequis** : compilateur C (gcc), `make`, bibliothèque SQLite3 (en-têtes inclus).

```bash
# Debian / Ubuntu / WSL
sudo apt install build-essential libsqlite3-dev

# Fedora
sudo dnf install gcc make sqlite-devel

# macOS (Homebrew)
brew install sqlite
```

**Windows (MinGW)** : deux possibilités.
- MSYS2 : `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-sqlite3`, puis `mingw32-make`.
- MinGW autonome sans SQLite : télécharger « sqlite-amalgamation » sur sqlite.org, copier `sqlite3.c` et
  `sqlite3.h` dans le dossier du projet, puis `mingw32-make -f Makefile.windows` et lancer `magasin.exe`.

Pour un affichage correct des accents dans le terminal : `chcp 65001`.

**Compilation et lancement**

```bash
make            # produit l'exécutable ./magasin
./magasin       # utilise ./magasin.db
./magasin autre.db   # utilise un autre fichier de base

make valgrind   # vérification des fuites mémoire
make clean      # supprime les fichiers objets et l'exécutable
```

Compilation sans `make` :
`gcc -Wall -Wextra -std=c11 -o magasin main.c magasin.c article.c client.c vente.c db.c ui.c utils.c -lsqlite3`

---

## 5. Structures de données (C)

```c
typedef struct Article {            /* article.h : liste chaînée */
    int id; char nom[50]; char categorie[30];
    float prix; int quantite; int seuil_alerte;
    struct Article *suivant;
} Article;

typedef struct Client {             /* client.h : liste chaînée */
    int id; char nom[50]; char telephone[15];
    struct Client *suivant;
} Client;

typedef struct { int id_article; int quantite; float prix_unitaire; } LigneVente;

typedef struct Vente {              /* vente.h */
    int id; char date[11]; int id_client; float total;
    LigneVente *lignes;             /* tableau dynamique (malloc/realloc) */
    int nb_lignes, capacite;
    struct Vente *suivant;          /* historique = liste chaînée */
} Vente;

typedef struct {                    /* magasin.h : état global */
    Article *articles; Client *clients; Vente *ventes;
    int prochain_id_article, prochain_id_client, prochain_id_vente;
} Magasin;
```

---

## 6. Fonctionnalités

**Menu principal** (boucle itérative) : articles · clients · nouvelle vente · historique et
statistiques · sauvegarder · quitter.

| Module | Fonctionnalités |
|---|---|
| **Articles** | ajouter (nom unique), modifier, supprimer (avec confirmation), rechercher (nom / catégorie, partielle, insensible à la casse), afficher, **trier** (nom / prix / quantité, croissant ou décroissant), alerte **stock faible** |
| **Clients** | ajouter, modifier, supprimer, rechercher, afficher |
| **Vente** | choix du client (ou anonyme), panier : ajouter / retirer une ligne / voir, contrôle du stock (en tenant compte du panier), calcul du total, validation (mise à jour du stock), ticket |
| **Statistiques** | historique des ventes, détail d'une vente (ticket), chiffre d'affaires total, article le plus vendu |

### Gestion des erreurs et saisies invalides

- `utils.c` : lecture avec `fgets` + `strtol`/`strtof` (jamais `scanf`) ; boucle jusqu'à une saisie valide ;
  contrôle des bornes, des chaînes vides, des saisies trop longues ; la virgule décimale est acceptée.
- Retours d'erreur explicites (`VENTE_ERR_STOCK`, `ARTICLE_ERR_INTROUVABLE`, …) traduits en messages.
- Tous les `malloc` / `realloc` sont vérifiés.
- Fichier de base absent : créé automatiquement. Fichier illisible ou corrompu : message d'erreur,
  **sauvegarde automatique désactivée** pour ne pas écraser les données.
- Panier : le stock n'est modifié qu'à la validation, après vérification de toutes les lignes.

---

## 7. Conformité au cahier des charges

| Exigence | Où |
|---|---|
| Structures `struct` propres | `Article`, `Client`, `Vente`, `LigneVente`, `Magasin` |
| Structure de données du cours (tableaux dynamiques, listes chaînées) | listes chaînées (articles, clients, historique) + tableau dynamique (lignes de vente) |
| Menu principal itératif | `main.c` (`do … while`) + sous-menus dans `ui.c` |
| Fonctionnalités cohérentes (ajout, suppression, modification, recherche, affichage, tri…) | §6 |
| Persistance dans des fichiers (sauvegarde / rechargement) | base SQLite `magasin.db` via `db.c` |
| Organisation modulaire (`.c` / `.h`) | 8 modules, un rôle par module |
| Fonctions avec paramètres et valeurs de retour, `main()` léger | tout le code ; `main()` ne fait que boucler sur le menu |
| Gestion des erreurs et saisies invalides | §6 |
| Programme compilable et fonctionnel | `make` sans avertissement avec `-Wall -Wextra -pedantic`, testé sous Valgrind (0 fuite, 0 erreur) |

---

## 8. Répartition des tâches proposée (binôme)

| Membre A — Stock et persistance | Membre B — Ventes et interface |
|---|---|
| `article.c/.h` (liste chaînée, CRUD, recherche, tri, stock faible) | `utils.c/.h` (saisies sécurisées, date, comparaisons) |
| `db.c/.h` (chargement / sauvegarde SQLite), `sql/` | `vente.c/.h` (panier, validation, historique, statistiques, tickets) |
| Menus Articles dans `ui.c` | Menus Clients, Vente, Statistiques dans `ui.c` |
| `client.c/.h` | `main.c`, `magasin.c`, Makefile |
| Tests croisés (A teste le code de B), Valgrind | Tests croisés (B teste le code de A), README, démonstration |

Les `.h` sont le « contrat » entre les deux membres : à figer en premier.

---

## 9. Jeu d'essai conseillé pour la démonstration

1. Charger `sql/seed.sql` (ou ajouter 3-4 articles à la main).
2. Articles → afficher, rechercher « lait », trier par prix décroissant, voir le stock faible.
3. Clients → ajouter un client.
4. Nouvelle vente → ajouter un article, tenter une quantité supérieure au stock (erreur), valider, lire le ticket.
5. Historique → détail de la vente, chiffre d'affaires, article le plus vendu.
6. Quitter, relancer : les données sont rechargées depuis `magasin.db`.

---

## 10. Limites connues et pistes d'amélioration

- Montants stockés en `float` (suffisant pour le projet ; `long` en millimes serait plus rigoureux).
- La sauvegarde réécrit toute la base (simple et sûr pour un petit magasin).
- Pistes : retour de marchandises, remises, export HTML/CSV des tickets, gestion des catégories dans une table.
