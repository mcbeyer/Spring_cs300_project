#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {

    if (argc < 3) {
        printf("wrong format dumbass\n");
        return -1;
    }

    int sec = atoi(argv[1]);        //time to wait between sending prefixes
    int i;
    for (i=2; i<argc; i++) {        //loop to read all prefixes
        /**send prefix via System V ipc message (send prefix ID starting at 1)
         * wait for results (count how many have returned) (if count==denominator statement)
         * print results
         * wait sec time (?)
         */
    }

    //don't forget to do the SIGINT if ^C is called
    //returns status of searchManager

    /**
     * send message to passage processer saying searchManager is done (no more prefixes to send)
     * send prefix ID of 0
     */

}