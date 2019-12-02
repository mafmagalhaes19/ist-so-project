 #include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"	
#include <assert.h>

#define MAX_INPUT 512
#define CONST_BUFFER 100

int fd;
int length;
int sessao=0;


char *socket_path = "\0hidden";




int tfsMount(char* address){
	struct sockaddr_un addr;
  	char buf[CONST_BUFFER];

  	socket_path=address;

  	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    	perror("socket error");
    	exit(-1);
  	}
  	memset(&addr, 0, sizeof(addr));
  	addr.sun_family = AF_UNIX;
  	if (*socket_path == '\0') {
    	*addr.sun_path = '\0';
    strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
  	} 
  	else {
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
  	}

  	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("connect error");
    exit(-1);
  }

	return 0;
}

int tfsUnmount(){
	if(sessao == 0){
		return TECNICOFS_ERROR_NO_OPEN_SESSION;
	}
    else{
		close(fd);
        sessao = 0;
        return 0;
    }
}

int trata_comando(char* message){
	char buffer[MAX_INPUT];
	int n  = strlen(message)+1;
	write(fd,message,n);
	n=read(fd,buffer,MAX_INPUT+1);
	return atoi(buffer);
      }


int tfsCreate(char* filename, permission ownerPermissions, permission otherPermissions){
	char comando[MAX_INPUT];
	char p1[2];
	char p2[2];
	int i;
	strcpy(comando, "c");
    strcat(comando, " ");
    strcat(comando, filename);
	sprintf(p1, "%d", ownerPermissions);
	sprintf(p2, "%d", otherPermissions);
	strcat(p1,p2);
    strcat(comando,p1);
	i=trata_comando(comando);
	return i;
}

int tfsDelete(char* filename){
	char comando[MAX_INPUT];
	int i;
	strcpy(comando, "d");
    strcat(comando, " ");
	strcat(comando, filename);
    i=trata_comando(comando);
	return i;
}
	
int tfsRename(char *filenameOld, char *filenameNew){
	char comando[MAX_INPUT];
	int i;
	strcpy(comando, "r");
    strcat(comando, " ");
	strcat(comando, filenameOld);
	strcat(comando, " ");
	strcat(comando, filenameNew);
	i=trata_comando(comando);
	return i;
}

int tfsOpen(char *filename, permission mode){
	char comando[MAX_INPUT];
	int i;
	char cmode[2];
	strcpy(comando, "o");
    strcat(comando, " ");
	strcat(comando , filename);
	sprintf(cmode, "%d", mode);
	strcat(comando, cmode);
	i=trata_comando(comando);
	return i;
}	

int tfsClose(int fd){
	char comando[MAX_INPUT];
	int i;
	char cfd[2];
	strcpy(comando, "x");
    strcat(comando, " ");
	sprintf(cfd, "%d", fd);
	strcat(comando, cfd);
	i=trata_comando(comando);
	return i;
}
	

int tfsRead(int fd, char *buffer, int length){
	char comando[MAX_INPUT];
	int i;
	char cfd[2];
	char clength[2];
	strcpy(comando, "l");
    strcat(comando, " ");
	sprintf(cfd, "%d", fd);
	sprintf(clength, "%d", length);
	strcat(comando, cfd);
	strcat(comando, " ");
	strcat(comando, clength);
	i=trata_comando(comando);
	return i;
}

int tfsWrite(int fd, char *buffer, int len){
	char comando[MAX_INPUT];
	int i;
	char cfd[10];
	strcpy(comando, "w");
    strcat(comando, " ");
	sprintf(cfd, "%d", fd);
	strcat(comando, cfd);
	strcat(comando, " ");
	strcat(comando, buffer);
	i=trata_comando(comando);
	return i;
}