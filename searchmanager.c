#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "longest_word_search.h"
#include "queue_ids.h"
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

//how to pass things from main to SIGINT
//signals work best by modifying global variables,
//so I make everything SIGINT needs a global variable
pthread_mutex_t LOCK;
int TOTAL_PREFIXES;     //IE ARGC
char** PREFIXES;        //IE ARGV
int TOTAL_PASSAGES;     // COMPLETED_PASSAGES out of X
sem_t completed_passages;

//taken from system5_msg.c
#ifndef mac
size_t                  /* O - Length of string */
strlcpy(char       *dst,        /* O - Destination string */
        const char *src,      /* I - Source string */
        size_t      size)     /* I - Size of destination string buffer */
{
    size_t    srclen;         /* Length of source string */


    /*
     * Figure out how much room is needed...
     */

    size --;

    srclen = strlen(src);

    /*
     * Copy the appropriate amount...
     */

    if (srclen > size)
        srclen = size;

    memcpy(dst, src, srclen);
    dst[srclen] = '\0';

    return (srclen);
}
#endif

//taken from msgsnd_pr.c
void send(int prefixID, char* prefix) {
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    prefix_buf sbuf;
    size_t buf_length;

    key = ftok(CRIMSON_ID,QUEUE_NUMBER);
    if ((msqid = msgget(key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }

    // We'll send message type 1
    sbuf.mtype = 1;
    strlcpy(sbuf.prefix,prefix,WORD_LENGTH);
    sbuf.id=prefixID;
    buf_length = strlen(sbuf.prefix) + sizeof(int)+1;//struct size without long int type

    // Send a message.
    if((msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT)) < 0) {
        int errnum = errno;
        fprintf(stderr,"%d, %ld, %s, %d\n", msqid, sbuf.mtype, sbuf.prefix, (int)buf_length);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
        exit(1);
    }
    else
        printf("\nMessage(%d): \"%s\" Sent (%d bytes)\n\n",sbuf.id, sbuf.prefix, (int)buf_length);

}

//taken from msgrcv_lwr.c
response_buf receive() {
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    response_buf rbuf;
    size_t buf_length;

    key = ftok(CRIMSON_ID,QUEUE_NUMBER);
    if ((msqid = msgget(key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }

    // msgrcv to receive message
    int ret;
    do {
      ret = msgrcv(msqid, &rbuf, sizeof(response_buf), 2, 0);//receive type 2 message
      int errnum = errno;
      if (ret < 0 && errno !=EINTR){
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
        fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
      }
    } while ((ret < 0 ) && (errno == 4));
    //fprintf(stderr,"msgrcv error return code --%d:$d--",ret,errno);

    return rbuf;
}

int isValidPrefix(char* prfx) {
    
    //removing prefixes of incorrect length
    if (strlen(prfx)<3 || strlen(prfx)>20){
        fprintf(stderr, "invalid prefix length for %s\n", prfx);
        return 0;
    }

    //removing prefixes with non-alphabet characters
    int j;
    for (j=0; j<strlen(prfx); j++) {
        if (isalpha(prfx[j]) == 0) { //isn't alphabet
            fprintf(stderr, "invalid character found in prefix %s\n", prfx);
            return 0;
        }
    }

    return 1;
}

//weed out invalid prefixes and update argc and argv to reflect the new list
char** makeValidPrefixList(int* origArgc, char** origArgv) {
    int i;
    int newArgc = 0;
    for (i=2; i<(*origArgc); i++) {
        if (isValidPrefix(origArgv[i]) == 1) {  //valid prefix
            newArgc++;
        }
    }

    //no valid prefixes - kill the program
    if (newArgc == 0) {
        fprintf(stderr, "no valid prefixes found in helper\n");
        exit(1); 
    }
    
    newArgc += 2;  //to account for first two that aren't prefixes

    char** newArgv = (char**)malloc(sizeof(char*)*(newArgc));

    newArgv[0] = origArgv[0];
    newArgv[1] = origArgv[1];
    newArgc = 2;        //first prefix goes at index 2

    for (i=2; i<(*origArgc); i++) {
        if (isValidPrefix(origArgv[i]) == 1) {  //valid prefix
            newArgv[newArgc] = origArgv[i];
            newArgc++;
        }
    }
    
    *origArgc = newArgc; //pass by reference - full size including first 2 parameters
    return newArgv;
}

//init handler - prints invalid prefixes too
void initHandler (int signum) {
    int i;
    for (i=0; i<TOTAL_PREFIXES; i++)
        printf("%s - pending\n", PREFIXES[i]);
}

//main handler
void mainHandler (int signum) {
    int i;
    int completed;
    PREFIXES = makeValidPrefixList(&TOTAL_PREFIXES, PREFIXES);
    sem_getvalue(&completed_passages, &completed);
    for (i=0; i<TOTAL_PREFIXES; i++){
        if (completed/TOTAL_PASSAGES > i) {
            printf("%s - done\n", PREFIXES[i]);
        }
        else if (completed == i){
            printf("%s - %d out of %d\n", PREFIXES[i], completed%TOTAL_PASSAGES, TOTAL_PASSAGES);
        }
        else {
            printf("%s - pending\n", PREFIXES[i]);
        }
    }
}

int main(int argc, char** argv) {

    //edit out the command call and second parameter
    TOTAL_PREFIXES = argc-2;
    PREFIXES = argv+2;
    sem_init(&completed_passages, 0, 0);
    signal(SIGINT, initHandler);

    argv = makeValidPrefixList(&argc, argv);

    if (argc < 3) {
        fprintf(stderr, "wrong format for commandline\n");
        return -1;
    }

    int wait = atoi(argv[1]);        //time to wait between sending prefixes
    int i, j;
    
    response_buf response;
    response_buf* responseArray = NULL;

    for (i=2; i<argc; i++) {        //loop to read all prefixes
        /**send prefix via System V ipc message (send prefix ID starting at 1)
         * wait for results (count how many have returned) (if count==denominator statement)
         * print results
         * wait sec time (?)
         */

        //send the message!
        send(i-1, argv[i]);
        sleep(wait);

        //receive the results and print
        response = receive();

        //update semaphore
        TOTAL_PASSAGES = response.count;
        sem_post(&completed_passages);
        signal(SIGINT, mainHandler);

        if (responseArray == NULL)
            responseArray = (response_buf*) malloc (sizeof(response_buf)*response.count);

        responseArray[response.index] = response;

        //j=1 because 1 message is already received
        for (j=1; j<response.count; j++) {
            sem_post(&completed_passages);
            response = receive();
            responseArray[response.index] = response;
        }

        printf("Report \"%s\"\n", argv[i]);
        for (j=0; j<response.count; j++) {
            if (responseArray[j].present == 1) {
                printf("Passage %d - %s - %s\n", responseArray[j].index, responseArray[j].location_description, responseArray[j].longest_word);
            }
            else {
                printf("Passage %d - %s - not found\n", responseArray[j].index, responseArray[j].location_description);
            }
        }
    }

    /**
     * send message to passage processer saying searchManager is done (no more prefixes to send)
     * send prefix ID of 0
     */

    send(0, "   ");
    printf("Exiting ...\n");
    free(responseArray);
    sem_destroy(&completed_passages);
}
