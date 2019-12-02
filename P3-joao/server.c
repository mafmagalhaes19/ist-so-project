
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include "lib/timer.h"
#include "tecnicofs-api-constants.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fs.h"
#include <errno.h>
#include "sync.h"

#define MAXTHREADS 10
#define MAXLINE 1024

tecnicofs* fs;
pthread_t CommandThread[MAXTHREADS];   
pthread_mutex_t commandsLock;
char* global_outputFile;
FILE *file;
struct timeval startTime;
struct timeval stopTime;
int cliente_uid;
char *socket_path = "\0hidden";
int numero_threads=0;
int sock;
int sfd;

static sigset_t signal_mask;


static void displayUsage (const char* appaddr){
    printf("Usage: %s\n", appaddr);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 3) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
    socket_path = argv[1];
    global_outputFile = argv[2];
}

void errorParse(int lineNumber){
    fprintf(stderr, "Error: line %d invalid\n", lineNumber);
    exit(EXIT_FAILURE);
}

FILE * openOutputFile() {
    FILE *fp;
    fp = fopen(global_outputFile, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    return fp;
}

int applyCommands(char buffer[MAXLINE+1],int n){
    while(1){
        mutex_lock(&commandsLock);
            char token;
            char name1[MAXLINE];
            char name2[MAXLINE];
            int i=0;
            sscanf(buffer, "%c %s %s", &token, name1, name2);
            switch (token) {
                case 'c':
                    mutex_unlock(&commandsLock);
                    i=create(fs,name1, cliente_uid, atoi(name2)/10, atoi(name2)%10);
                    return i;
                case 'd':
                    mutex_unlock(&commandsLock);
                    i=deletes(fs,name1, cliente_uid);
                    return i;
                case 'r':
                    mutex_unlock(&commandsLock);
                    i=renameFile(fs,name1,name2, cliente_uid);
                    return i;
                case 'x':
                    mutex_unlock(&commandsLock);
                    closeFile(fs,atoi(name1), cliente_uid);
                    return i;
                case 'o':
                    mutex_unlock(&commandsLock);
                    i=openFile(fs,name1,atoi(name2), cliente_uid);
                    return i;
                case 'l':
                    mutex_unlock(&commandsLock);               
                    i=readFile(fs,atoi(name1),buffer,atoi(name2),cliente_uid);
                    return i;
                case 'w':
                    mutex_unlock(&commandsLock);
                    i=writeFile(fs,atoi(name1), buffer, cliente_uid);
                    return i;
                    n=1;
                default: { /* error */
                    mutex_unlock(&commandsLock);
                    fprintf(stderr, "Error: commands to apply\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        }

    


void* str_serv(void * args){
    int n;
    char buffer[MAXLINE+1];
    struct ucred ucred;
    //struct sockaddr_un cli_addr;
    int lucred = sizeof(struct ucred);
    int cfd = *((int *) args);
    printf("e");
    getsockopt(cfd, SOL_SOCKET, SO_PEERCRED,  &ucred, (socklen_t *) &lucred);
    cliente_uid = ucred.uid;
    n=read(cfd, buffer, MAXLINE);
    if (n<0)
        printf("erro no read");
    n=applyCommands(buffer,n);
    write(cfd, buffer, n);
    return NULL;
}


void* signal_thread(){
    double time;
    int  sig_caught;    /* signal caught       */
    sigwait (&signal_mask, &sig_caught);
    for(int i = 0; i < numero_threads; i++){
    	pthread_join(CommandThread[i], NULL);
    }
    close(sock);
    gettimeofday(&stopTime, NULL);
    time = (double) (stopTime.tv_sec - startTime.tv_sec) + (double) (stopTime.tv_usec - startTime.tv_usec)/100000;
    printf("TecnicoFS completed in %0.4f seconds.\n", time);
    free_tecnicofs(fs);
    exit(EXIT_SUCCESS);
}





int main(int argc, char* argv[]) {
    struct sockaddr_un addr;
    pthread_t  sig_thr_id;      /* signal handler thread ID */
    int cfd;
    struct sockaddr_un cli_addr;
    int cli_len = sizeof(cli_addr);
    printf("a");
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGINT);
    pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    pthread_create (&sig_thr_id, NULL, signal_thread, NULL);
    printf("B");
    mutex_init(&commandsLock);
    parseArgs(argc, argv);


    if ( (sfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
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
        unlink(socket_path);
    }

    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(-1);
    }
    gettimeofday(&startTime, NULL);
    if (listen(sfd, 5) == -1) {
        perror("listen error");
        exit(-1);
    }
    for(;;){
    if ( (cfd = accept(sfd, (struct sockaddr *) &cli_addr, (socklen_t *) &cli_len) ) == -1) {
        perror("accept error");
    }
    pthread_create(&CommandThread[numero_threads], NULL, str_serv, (void *) &cfd);
    numero_threads++;
    }

    
    mutex_destroy(&commandsLock);
    return 0;
}

