package edu.cs300;

import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class PassageProcessor {
    static String passagePath = "../../passages.txt";
    public static void main(String[] args) {
        /**
         * make thread for all text files
         * each thread makes a trie
         * wait for prefixes to come in using Java Native Call
         * send prefix to each worker to process
         * worker finds longest word
         * recieve longest word from worker
         * send longest word to SearchManager (x/denominator)
         *  if found, send prefix id, passage id, passage name, number of passages
         *  (denominator), present=1
         *  if not found, send prefix id and present=0
         * wait for next prefix or process next prefix
         * if id=0, quit
         */

        //  ArrayBlockingQueue prefix = new ArrayBlockingQueue<>(10);
         ArrayBlockingQueue results = new ArrayBlockingQueue<>(10*100);
         ArrayList<String> paths = new ArrayList<String>();


         try {
             Scanner passage = new Scanner(new File(passagePath));
             String line;
             //read in each passage path and give it to a worker
             while (passage.hasNextLine()) {
                 line = passage.nextLine();
                 paths.add(line);
             }

             ArrayBlockingQueue[] workers = new ArrayBlockingQueue[paths.size()];
             
             for (int i=0; i<paths.size(); i++) {
                 new Worker(paths.get(i), i+1, workers[i], results).start();
             }

         }  catch (Exception e) {};
    }
}