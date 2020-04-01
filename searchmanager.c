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
// int COMPLETED_PASSAGES; // X out of TOTAL_PASSAGES

sem_t completed_passages;
sem_t completed_prefixes;

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
    printf("completed: %d", completed);
    sem_getvalue(&completed_prefixes, &completed);
    for (i=0; i<TOTAL_PREFIXES; i++) {
        if (i<completed) {
            printf("%s - done\n", PREFIXES[i]);
        }
        else if (i == completed) {
            //look at completed_passages
            sem_getvalue(&completed_passages, &completed);
            printf("%s - %d out of %d\n", PREFIXES[i], completed, TOTAL_PREFIXES);
        }
        else {
            printf("%s - pending\n", PREFIXES[i]);
        }
    }
}

// //CTRL-C HANDLER
// void handler (int signum) {
//     int i;

//     //LOCK GLOBAL VARIABLES UNTIL SIGINT IS DONE WITH THEM
//     //PREVENTS THEM FROM BEING UPDATED WHILE HANDLER IS USING THEM
//     pthread_mutex_lock(&LOCK);
//         if (COMPLETED_PASSAGES == 0) {
//             for (i=0; i<TOTAL_PREFIXES; i++){
//                 printf("%s - pending\n", PREFIXES[i]);
//             }       
//         }
//         else {
//             for (i=0; i<TOTAL_PREFIXES; i++){
//                 if (COMPLETED_PASSAGES/TOTAL_PASSAGES > i) {
//                     printf("%s - done\n", PREFIXES[i]);
//                 }
//                 else if (COMPLETED_PASSAGES == i){
//                     printf("%s - %d out of %d\n", PREFIXES[i], COMPLETED_PASSAGES%TOTAL_PASSAGES, TOTAL_PASSAGES);
//                 }
//                 else {
//                     printf("%s - pending\n", PREFIXES[i]);
//                 }
//             }
//         }
//     pthread_mutex_unlock(&LOCK);
// }

int main(int argc, char** argv) {

    //edit out the command call and second parameter
    TOTAL_PREFIXES = argc-2;
    PREFIXES = argv+2;
    sem_init(&completed_prefixes, 0, 0);
    sem_init(&completed_passages, 0, 0);
    // COMPLETED_PASSAGES = 0;
    // pthread_mutex_init(&LOCK, NULL);
    signal(SIGINT, initHandler);


    if (argc < 3) {
        fprintf(stderr, "wrong format for commandline\n");
        return -1;
    }

    int wait = atoi(argv[1]);        //time to wait between sending prefixes
    int i;
    
    response_buf response;
    response_buf* responseArray = NULL;

    for (i=2; i<argc; i++) {        //loop to read all prefixes
        /**send prefix via System V ipc message (send prefix ID starting at 1)
         * wait for results (count how many have returned) (if count==denominator statement)
         * print results
         * wait sec time (?)
         */

        //removing prefixes of incorrect length
        if (strlen(argv[i])<3 || strlen(argv[i])>20){
            fprintf(stderr, "invalid prefix length");
        }

        //removing prefixes with non-alphabet characters
        int j;
        for (j=0; j<strlen(argv[j]); j++) {
            if (isalpha(argv[i][j]) == 0) { //isn't alphabet
                fprintf(stderr, "invalid character found in prefix %d", i);
                break;
            }
        }

        //send the message!
        send(i-1, argv[i]);
        sleep(wait);

        //receive the results and print
        response = receive();

        //update semaphore
        TOTAL_PASSAGES = response.count;
        sem_post(&completed_passages);
        signal(SIGINT, mainHandler);

        // pthread_mutex_lock(&LOCK);
            // COMPLETED_PASSAGES++;
        // pthread_mutex_unlock(&LOCK);

        if (responseArray == NULL)
            responseArray = (response_buf*) malloc (sizeof(response_buf)*response.count);

        responseArray[response.index] = response;

        //j=1 because 1 message is already received
        for (j=1; j<response.count; j++) {
            // pthread_mutex_lock(&LOCK);
            //     COMPLETED_PASSAGES++;
            // pthread_mutex_unlock(&LOCK);
            sem_post(&completed_passages);
            response = receive();
            responseArray[response.index] = response;
        }

        sem_post(&completed_prefixes);

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
}
