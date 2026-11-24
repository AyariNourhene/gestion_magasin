-- Schéma de la base de données du projet « Gestion de magasin »
-- NB : le programme crée automatiquement ces tables au premier lancement.
-- Ce fichier sert à la documentation et à une création manuelle éventuelle :
--     sqlite3 magasin.db < sql/schema.sql

PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS article (
    id            INTEGER PRIMARY KEY,
    nom           TEXT    NOT NULL,
    categorie     TEXT,
    prix          REAL    NOT NULL CHECK (prix > 0),
    quantite      INTEGER NOT NULL CHECK (quantite >= 0),
    seuil_alerte  INTEGER NOT NULL DEFAULT 5 CHECK (seuil_alerte >= 0)
);

CREATE TABLE IF NOT EXISTS client (
    id         INTEGER PRIMARY KEY,
    nom        TEXT NOT NULL,
    telephone  TEXT
);

CREATE TABLE IF NOT EXISTS vente (
    id          INTEGER PRIMARY KEY,
    date_vente  TEXT    NOT NULL,                 -- JJ/MM/AAAA
    id_client   INTEGER NOT NULL DEFAULT 0,       -- 0 = client anonyme
    total       REAL    NOT NULL CHECK (total >= 0)
);

CREATE TABLE IF NOT EXISTS ligne_vente (
    id_vente       INTEGER NOT NULL,
    id_article     INTEGER NOT NULL,
    quantite       INTEGER NOT NULL CHECK (quantite > 0),
    prix_unitaire  REAL    NOT NULL CHECK (prix_unitaire > 0),
    FOREIGN KEY (id_vente) REFERENCES vente(id) ON DELETE CASCADE
);
