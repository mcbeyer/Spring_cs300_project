package edu.cs300;

import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class PassageProcessor {
    static String passagePath = "passages.txt";
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
        
         ArrayList<String> paths = new ArrayList<String>();


         try {
             Scanner passage = new Scanner(new File(passagePath));
             String line;
             //read in each passage path and give it to a worker
             while (passage.hasNextLine()) {
                 line = passage.nextLine();
                 paths.add(line);
             }

             ArrayBlockingQueue<String> results = new ArrayBlockingQueue<String>(paths.size()*10);
             ArrayBlockingQueue<String>[] workers = new ArrayBlockingQueue[paths.size()];
             
             //starts the trie creation
             for (int i=0; i<paths.size(); i++) {
                 workers[i] = new ArrayBlockingQueue<String>(1);
                 new Worker(paths.get(i), i, workers[i], results).start();
             }

             //to be fixed later
             String tempprefix = "con";

             //give workers the prefix
             for (int i=0; i<workers.length; i++) {
                 workers[i].add(tempprefix);
             }

             tempprefix = "pre";

             //give workers the prefix
             for (int i=0; i<workers.length; i++) {
                 workers[i].add(tempprefix);
             }

             //give results (in results array) back to SearchManager
             String sendBack;
             String wID;
             
             for (int i=0; i<paths.size(); i++) {
                 wID = "";
                 sendBack = results.take();
                 System.out.println(sendBack);

                 //parse out worker id number
                 for (char c : sendBack.toCharArray()) {
                     if (Character.isDigit(c))
                         wID = wID + Character.toString(c);
                 }
            }

             //done with prefixes now
             String killer = "0";
             for (int i=0; i<workers.length; i++) {
                 workers[i].add(killer);
             }

         }  catch (Exception e) {};
    }
}