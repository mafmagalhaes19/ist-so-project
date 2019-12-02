#include "fs.h"
#include "tecnicofs-api-constants.h"
#include "lib/inodes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "sync.h"

ficheiro_aberto tabela_ficheiros_abertos[4];
int numero_ficheiros_abertos = 0;

tecnicofs* new_tecnicofs(){
	tecnicofs* fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	inode_table_init();
	for (int i=0;i<5;i++){
		tabela_ficheiros_abertos[i].owner = FREE_INODE;
        tabela_ficheiros_abertos[i].inumber = -1;
		tabela_ficheiros_abertos[i].ownerPermissions =-1;
		tabela_ficheiros_abertos[i].othersPermissions =-1;
		tabela_ficheiros_abertos[i].file = NULL;
	}
	sync_init(&(fs->bstLock));
	return fs;
}
	

void free_tecnicofs(tecnicofs* fs){
	inode_table_destroy();
	free(fs);
	sync_destroy(&(fs->bstLock));
	return;
	}


int create(tecnicofs* fs, char *name, int cliente_uid, permission ownerPermissions, permission otherPermissions){
	sync_wrlock(&(fs->bstLock));
	if (lookup(fs,name) != -1){
		return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
	}
	int inumber = inode_create(cliente_uid, ownerPermissions, otherPermissions);
	fs->bstRoot = insert(fs->bstRoot, name, inumber);
	sync_unlock(&(fs->bstLock));
	return 0;
}

int deletes(tecnicofs* fs, char *name, int cliente_uid){
	uid_t owner;
	int inumber = lookup(fs, name);
	if(inumber == -1)
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
	sync_wrlock(&(fs->bstLock));
	inode_get(inumber,&owner,NULL , NULL, NULL, 0);
	if(owner != cliente_uid){
		sync_unlock(&(fs->bstLock));
		return TECNICOFS_ERROR_PERMISSION_DENIED;
	}
	inode_delete(inumber);
	fs->bstRoot = remove_item(fs->bstRoot, name);
	sync_unlock(&(fs->bstLock));
	return 0;
}

int lookup(tecnicofs* fs, char *name){
	sync_rdlock(&(fs->bstLock));
	int inumber = -1;
	node* searchNode = search(fs->bstRoot, name);
	if ( searchNode ) {
		inumber = searchNode->inumber;
	}
	sync_unlock(&(fs->bstLock));
	return inumber;
}


int renameFile(tecnicofs* fs, char *name, char *newname,int cliente_uid){
	uid_t owner;
	int inumber = lookup(fs, name);
	if(inumber == -1)
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
	sync_wrlock(&(fs->bstLock));
	node* name1 = search(fs->bstRoot, name);
	node* name2 = search(fs->bstRoot, newname);
	inode_get(inumber,&owner,NULL , NULL, NULL, 0);
	if (owner != cliente_uid){
		return TECNICOFS_ERROR_PERMISSION_DENIED;
	}
	if(name1){
		if(!name2){
		fs->bstRoot = remove_item(fs->bstRoot, name1->key);
		fs->bstRoot = insert(fs->bstRoot, name2->key, inumber);
	}
	}
	sync_unlock(&(fs->bstLock));
	return 0;
	}


int openFile(tecnicofs* fs, char* name, permission mode,int cliente_uid){
	uid_t owner;
	FILE *fp;
	permission ownerPermission, userPermission;
    int inumber =  lookup(fs, name);
	sync_wrlock(&(fs->bstLock));
    if(inumber == -1){
        return TECNICOFS_ERROR_FILE_NOT_FOUND;}
	inode_get(inumber, &owner, &ownerPermission, &userPermission, NULL, 0);
	if (owner != cliente_uid){
		if(mode != userPermission){
		sync_unlock(&(fs->bstLock));
        return TECNICOFS_ERROR_PERMISSION_DENIED;
	}
	}
	if (owner == cliente_uid){
		if(mode != ownerPermission){
			sync_unlock(&(fs->bstLock));
        	return TECNICOFS_ERROR_PERMISSION_DENIED;
		}
	}
	numero_ficheiros_abertos++;
	if (numero_ficheiros_abertos>5)
		return TECNICOFS_ERROR_MAXED_OPEN_FILES;
    for (int i=0; i<5; i++){
        if (tabela_ficheiros_abertos[i].owner==FREE_INODE){
			tabela_ficheiros_abertos[i].owner=owner;
            tabela_ficheiros_abertos[i].ownerPermissions=ownerPermission;
			tabela_ficheiros_abertos[i].othersPermissions=userPermission;
			tabela_ficheiros_abertos[i].inumber=inumber;
			if (mode == 2){
                fp = fopen(name, "w");
				tabela_ficheiros_abertos[i].file=fp;
				sync_unlock(&(fs->bstLock));
                return i;
                }
            if (mode == 1){
                fp = fopen(name, "r");
				tabela_ficheiros_abertos[i].file=fp;
				sync_unlock(&(fs->bstLock));
                return i;
                }
            if (mode == 3){
                fp = fopen(name, "r+");
				tabela_ficheiros_abertos[i].file=fp;
				sync_unlock(&(fs->bstLock));
                return i;
				}
				}
	}
 return -1;
}

int closeFile(tecnicofs* fs, int fd, int cliente_uid ){
FILE* fp;
for (int i=0; i<5; i++){
	if (tabela_ficheiros_abertos[i].owner != FREE_INODE){
    	if (fd>=0 && fd<=4){
			fp=tabela_ficheiros_abertos[fd].file;
			fclose(fp);
			tabela_ficheiros_abertos[i].owner = FREE_INODE;
        	tabela_ficheiros_abertos[i].inumber = -1;
			tabela_ficheiros_abertos[i].ownerPermissions =-1;
			tabela_ficheiros_abertos[i].othersPermissions =-1;
			tabela_ficheiros_abertos[i].file=NULL;
    		numero_ficheiros_abertos--;
    		return 0;
			}
    }
}
return TECNICOFS_ERROR_FILE_NOT_OPEN;
}



int readFile (tecnicofs* fs, int fd, char *buffer, int len, int cliente_uid){
	int inumber;
	uid_t owner;
	permission ownerPerm, othersPerm;
	inumber=tabela_ficheiros_abertos[fd].inumber;
	if (inumber==-1){
		return TECNICOFS_ERROR_FILE_NOT_OPEN;
	}
    inode_get(inumber, &owner, &ownerPerm, &othersPerm, buffer, 0);
	if (ownerPerm != READ || othersPerm != RW){
		if (ownerPerm != READ || othersPerm != RW){
			return TECNICOFS_ERROR_PERMISSION_DENIED;
        }
    }
	len=strlen(buffer);
	return len;
    }


int writeFile (tecnicofs *fs, int fd, char *buffer, int cliente_uid){
	return 0;
}


void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
		print_tree(fp, fs->bstRoot);
		fflush(fp);
	}
