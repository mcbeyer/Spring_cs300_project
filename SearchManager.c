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
    else
        fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);

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
        fprintf(stderr,"Message(%d): \"%s\" Sent (%d bytes)\n", sbuf.id, sbuf.prefix,(int)buf_length);

}

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
    else
        fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);


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

int main(int argc, char** argv) {

    if (argc < 3) {
        printf("wrong format dumbass\n");
        return -1;
    }

    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0){ //the child - java one
        int state = system("java -cp . -Djava.library.path=. edu.cs300.PassageProcessor");
        return(0);
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
            printf("invalid prefix length");
        }

        //removing prefixes with non-alphabet characters
        int j;
        for (j=0; j<strlen(argv[j]); j++) {
            if (isalpha(argv[i][j]) == 0) { //isn't alphabet
                printf("invalid character found");
                break;
            }
        }

        //send the message!
        send(i-1, argv[i]);
        sleep(wait);

        //receive the results and print
        response = receive();
        if (responseArray == NULL)
            responseArray = (response_buf*) malloc (sizeof(response_buf)*response.count);

        responseArray[response.index] = response;

        //j=1 because 1 message is already received
        for (j=1; j<response.count; j++) {
            response = receive();
            responseArray[response.index] = response;
        }

        for (j=0; j<response.count; j++) {
            if (responseArray[j].present == 1) {
                printf("Passage %d - %s - %s\n", responseArray[j].index, responseArray[j].location_description, responseArray[j].longest_word);
            }
            else {
                printf("Passage %d - %s - not found\n", responseArray[j].index, responseArray[j].location_description);
            }
        }

    }

    //don't forget to do the SIGINT if ^C is called
    //returns status of searchManager

    /**
     * send message to passage processer saying searchManager is done (no more prefixes to send)
     * send prefix ID of 0
     */

    send(0, "-1");

}