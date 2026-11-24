-- Jeu de données de démonstration (facultatif)
--     sqlite3 magasin.db < sql/schema.sql
--     sqlite3 magasin.db < sql/seed.sql

INSERT INTO article (id, nom, categorie, prix, quantite, seuil_alerte) VALUES
    (1, 'Lait 1L',          'Epicerie',  1.250, 120, 20),
    (2, 'Pain baguette',    'Boulangerie', 0.250, 60, 15),
    (3, 'Huile d''olive 1L','Epicerie', 14.900, 25, 5),
    (4, 'Cahier 100p',      'Papeterie', 2.500, 8, 10),
    (5, 'Stylo bleu',       'Papeterie', 0.800, 200, 30);

INSERT INTO client (id, nom, telephone) VALUES
    (1, 'Ahmed Ben Salah', '20123456'),
    (2, 'Sana Trabelsi',   '98765432');

INSERT INTO vente (id, date_vente, id_client, total) VALUES
    (1, '20/11/2026', 1, 4.100),
    (2, '21/11/2026', 0, 15.700);

INSERT INTO ligne_vente (id_vente, id_article, quantite, prix_unitaire) VALUES
    (1, 1, 2, 1.250),
    (1, 5, 2, 0.800),
    (2, 3, 1, 14.900),
    (2, 5, 1, 0.800);
