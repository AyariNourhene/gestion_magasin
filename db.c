#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

#include "db.h"

/* Même contenu que sql/schema.sql */
static const char *SCHEMA =
    "PRAGMA foreign_keys = ON;"
    "CREATE TABLE IF NOT EXISTS article ("
    "  id INTEGER PRIMARY KEY,"
    "  nom TEXT NOT NULL,"
    "  categorie TEXT,"
    "  prix REAL NOT NULL CHECK (prix > 0),"
    "  quantite INTEGER NOT NULL CHECK (quantite >= 0),"
    "  seuil_alerte INTEGER NOT NULL DEFAULT 5 CHECK (seuil_alerte >= 0));"
    "CREATE TABLE IF NOT EXISTS client ("
    "  id INTEGER PRIMARY KEY,"
    "  nom TEXT NOT NULL,"
    "  telephone TEXT);"
    "CREATE TABLE IF NOT EXISTS vente ("
    "  id INTEGER PRIMARY KEY,"
    "  date_vente TEXT NOT NULL,"
    "  id_client INTEGER NOT NULL DEFAULT 0,"
    "  total REAL NOT NULL CHECK (total >= 0));"
    "CREATE TABLE IF NOT EXISTS ligne_vente ("
    "  id_vente INTEGER NOT NULL,"
    "  id_article INTEGER NOT NULL,"
    "  quantite INTEGER NOT NULL CHECK (quantite > 0),"
    "  prix_unitaire REAL NOT NULL CHECK (prix_unitaire > 0),"
    "  FOREIGN KEY (id_vente) REFERENCES vente(id) ON DELETE CASCADE);";

static int exec_sql(sqlite3 *db, const char *sql)
{
    char *err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Erreur SQL : %s\n", err ? err : "inconnue");
        sqlite3_free(err);
        return DB_ERR;
    }
    return DB_OK;
}

static sqlite3 *ouvrir(const char *fichier)
{
    sqlite3 *db = NULL;
    if (sqlite3_open(fichier, &db) != SQLITE_OK) {
        fprintf(stderr, "Impossible d'ouvrir la base '%s' : %s\n", fichier,
                db ? sqlite3_errmsg(db) : "mémoire insuffisante");
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

/* ------------------------------ CHARGEMENT ------------------------------ */

static int charger_articles(sqlite3 *db, Magasin *m)
{
    sqlite3_stmt *st;
    if (sqlite3_prepare_v2(db,
            "SELECT id, nom, categorie, prix, quantite, seuil_alerte "
            "FROM article ORDER BY id;", -1, &st, NULL) != SQLITE_OK)
        return DB_ERR;

    int rc = DB_OK;
    while (sqlite3_step(st) == SQLITE_ROW) {
        const char *cat = (const char *)sqlite3_column_text(st, 2);
        Article *a = article_creer(sqlite3_column_int(st, 0),
                                   (const char *)sqlite3_column_text(st, 1),
                                   cat ? cat : "",
                                   (float)sqlite3_column_double(st, 3),
                                   sqlite3_column_int(st, 4),
                                   sqlite3_column_int(st, 5));
        if (!a) {
            rc = DB_ERR;
            break;
        }
        m->articles = article_ajouter_fin(m->articles, a);
    }
    sqlite3_finalize(st);
    return rc;
}

static int charger_clients(sqlite3 *db, Magasin *m)
{
    sqlite3_stmt *st;
    if (sqlite3_prepare_v2(db, "SELECT id, nom, telephone FROM client ORDER BY id;",
                           -1, &st, NULL) != SQLITE_OK)
        return DB_ERR;

    int rc = DB_OK;
    while (sqlite3_step(st) == SQLITE_ROW) {
        const char *tel = (const char *)sqlite3_column_text(st, 2);
        Client *c = client_creer(sqlite3_column_int(st, 0),
                                 (const char *)sqlite3_column_text(st, 1),
                                 tel ? tel : "");
        if (!c) {
            rc = DB_ERR;
            break;
        }
        m->clients = client_ajouter_fin(m->clients, c);
    }
    sqlite3_finalize(st);
    return rc;
}

static int charger_lignes(sqlite3 *db, Vente *v)
{
    sqlite3_stmt *st;
    if (sqlite3_prepare_v2(db,
            "SELECT id_article, quantite, prix_unitaire FROM ligne_vente "
            "WHERE id_vente = ?;", -1, &st, NULL) != SQLITE_OK)
        return DB_ERR;
    sqlite3_bind_int(st, 1, v->id);

    int rc = DB_OK;
    while (sqlite3_step(st) == SQLITE_ROW) {
        if (v->nb_lignes == v->capacite) {
            int nouvelle = v->capacite ? v->capacite * 2 : 4;
            LigneVente *tmp = realloc(v->lignes, (size_t)nouvelle * sizeof *tmp);
            if (!tmp) {
                rc = DB_ERR;
                break;
            }
            v->lignes = tmp;
            v->capacite = nouvelle;
        }
        v->lignes[v->nb_lignes].id_article = sqlite3_column_int(st, 0);
        v->lignes[v->nb_lignes].quantite = sqlite3_column_int(st, 1);
        v->lignes[v->nb_lignes].prix_unitaire = (float)sqlite3_column_double(st, 2);
        v->nb_lignes++;
    }
    sqlite3_finalize(st);
    return rc;
}

static int charger_ventes(sqlite3 *db, Magasin *m)
{
    sqlite3_stmt *st;
    if (sqlite3_prepare_v2(db,
            "SELECT id, date_vente, id_client, total FROM vente ORDER BY id;",
            -1, &st, NULL) != SQLITE_OK)
        return DB_ERR;

    int rc = DB_OK;
    while (sqlite3_step(st) == SQLITE_ROW) {
        Vente *v = vente_creer(sqlite3_column_int(st, 0),
                               (const char *)sqlite3_column_text(st, 1),
                               sqlite3_column_int(st, 2));
        if (!v) {
            rc = DB_ERR;
            break;
        }
        v->total = (float)sqlite3_column_double(st, 3);
        if (charger_lignes(db, v) != DB_OK) {
            vente_liberer(v);
            rc = DB_ERR;
            break;
        }
        m->ventes = historique_ajouter_fin(m->ventes, v);
    }
    sqlite3_finalize(st);
    return rc;
}

int db_charger(Magasin *m, const char *fichier)
{
    sqlite3 *db = ouvrir(fichier);
    if (!db)
        return DB_ERR;

    int rc = exec_sql(db, SCHEMA);
    if (rc == DB_OK) rc = charger_articles(db, m);
    if (rc == DB_OK) rc = charger_clients(db, m);
    if (rc == DB_OK) rc = charger_ventes(db, m);
    sqlite3_close(db);

    m->prochain_id_article = article_prochain_id(m->articles);
    m->prochain_id_client = client_prochain_id(m->clients);
    m->prochain_id_vente = historique_prochain_id(m->ventes);
    return rc;
}

/* ------------------------------ SAUVEGARDE ------------------------------ */

static int sauver_articles(sqlite3 *db, const Article *a)
{
    sqlite3_stmt *st;
    if (sqlite3_prepare_v2(db,
            "INSERT INTO article(id, nom, categorie, prix, quantite, seuil_alerte) "
            "VALUES(?,?,?,?,?,?);", -1, &st, NULL) != SQLITE_OK)
        return DB_ERR;

    int rc = DB_OK;
    for (; a && rc == DB_OK; a = a->suivant) {
        sqlite3_reset(st);
        sqlite3_bind_int(st, 1, a->id);
        sqlite3_bind_text(st, 2, a->nom, -1, SQLITE_STATIC);
        sqlite3_bind_text(st, 3, a->categorie, -1, SQLITE_STATIC);
        sqlite3_bind_double(st, 4, a->prix);
        sqlite3_bind_int(st, 5, a->quantite);
        sqlite3_bind_int(st, 6, a->seuil_alerte);
        if (sqlite3_step(st) != SQLITE_DONE)
            rc = DB_ERR;
    }
    sqlite3_finalize(st);
    return rc;
}

static int sauver_clients(sqlite3 *db, const Client *c)
{
    sqlite3_stmt *st;
    if (sqlite3_prepare_v2(db, "INSERT INTO client(id, nom, telephone) VALUES(?,?,?);",
                           -1, &st, NULL) != SQLITE_OK)
        return DB_ERR;

    int rc = DB_OK;
    for (; c && rc == DB_OK; c = c->suivant) {
        sqlite3_reset(st);
        sqlite3_bind_int(st, 1, c->id);
        sqlite3_bind_text(st, 2, c->nom, -1, SQLITE_STATIC);
        sqlite3_bind_text(st, 3, c->telephone, -1, SQLITE_STATIC);
        if (sqlite3_step(st) != SQLITE_DONE)
            rc = DB_ERR;
    }
    sqlite3_finalize(st);
    return rc;
}

static int sauver_ventes(sqlite3 *db, const Vente *v)
{
    sqlite3_stmt *sv, *sl;
    if (sqlite3_prepare_v2(db,
            "INSERT INTO vente(id, date_vente, id_client, total) VALUES(?,?,?,?);",
            -1, &sv, NULL) != SQLITE_OK)
        return DB_ERR;
    if (sqlite3_prepare_v2(db,
            "INSERT INTO ligne_vente(id_vente, id_article, quantite, prix_unitaire) "
            "VALUES(?,?,?,?);", -1, &sl, NULL) != SQLITE_OK) {
        sqlite3_finalize(sv);
        return DB_ERR;
    }

    int rc = DB_OK;
    for (; v && rc == DB_OK; v = v->suivant) {
        sqlite3_reset(sv);
        sqlite3_bind_int(sv, 1, v->id);
        sqlite3_bind_text(sv, 2, v->date, -1, SQLITE_STATIC);
        sqlite3_bind_int(sv, 3, v->id_client);
        sqlite3_bind_double(sv, 4, v->total);
        if (sqlite3_step(sv) != SQLITE_DONE) {
            rc = DB_ERR;
            break;
        }
        for (int i = 0; i < v->nb_lignes && rc == DB_OK; i++) {
            sqlite3_reset(sl);
            sqlite3_bind_int(sl, 1, v->id);
            sqlite3_bind_int(sl, 2, v->lignes[i].id_article);
            sqlite3_bind_int(sl, 3, v->lignes[i].quantite);
            sqlite3_bind_double(sl, 4, v->lignes[i].prix_unitaire);
            if (sqlite3_step(sl) != SQLITE_DONE)
                rc = DB_ERR;
        }
    }
    sqlite3_finalize(sv);
    sqlite3_finalize(sl);
    return rc;
}

int db_sauvegarder(const Magasin *m, const char *fichier)
{
    sqlite3 *db = ouvrir(fichier);
    if (!db)
        return DB_ERR;

    int rc = exec_sql(db, SCHEMA);
    if (rc == DB_OK)
        rc = exec_sql(db, "BEGIN TRANSACTION;");
    if (rc == DB_OK) {
        rc = exec_sql(db, "DELETE FROM ligne_vente; DELETE FROM vente;"
                          "DELETE FROM article; DELETE FROM client;");
        if (rc == DB_OK) rc = sauver_articles(db, m->articles);
        if (rc == DB_OK) rc = sauver_clients(db, m->clients);
        if (rc == DB_OK) rc = sauver_ventes(db, m->ventes);

        if (exec_sql(db, rc == DB_OK ? "COMMIT;" : "ROLLBACK;") != DB_OK)
            rc = DB_ERR;
    }
    sqlite3_close(db);
    return rc;
}
