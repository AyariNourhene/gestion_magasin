#ifndef DB_H
#define DB_H

#include "magasin.h"

#define DB_OK    0
#define DB_ERR (-1)

/* Charge la base SQLite dans les listes chaînées (la base et ses tables sont
   créées automatiquement si elles n'existent pas). */
int db_charger(Magasin *m, const char *fichier);

/* Sauvegarde les listes chaînées dans la base, dans une seule transaction :
   en cas d'erreur, ROLLBACK et l'ancienne sauvegarde reste intacte. */
int db_sauvegarder(const Magasin *m, const char *fichier);

#endif
