/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include "sync.h"
#include "tecnicofs-api-constants.h"

typedef struct tecnicofs {
    node* bstRoot;
    int nextINumber;
    syncMech bstLock;
} tecnicofs;

typedef struct ficheiro_aberto {
    uid_t owner;
    permission ownerPermissions;
    permission othersPermissions;
    int inumber;
    FILE* file;
} ficheiro_aberto;


int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs);
int deletes(tecnicofs* fs, char *name, int cliente_uid);
int create(tecnicofs* fs, char *name,int cliente_uid, permission ownerPermissions, permission otherPermissions);
int lookup(tecnicofs* fs, char *name);
int readFile (tecnicofs* fs, int fd, char *buffer, int len, int cliente_uid);
int writeFile (tecnicofs *fs, int fd, char *buffer, int cliente_uid);
int closeFile(tecnicofs* fs, int fd, int cliente_uid );
int openFile(tecnicofs* fs, char* name, permission mode,int cliente_uid);
int renameFile(tecnicofs* fs, char *name, char *newname,int cliente_uid);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */
