package edu.cs300;

import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
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
        String prefix = "";
        int prefixCount = 0;


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

             
            while (true) {
                if (prefixCount == 0) {
                    prefix = "con";
                }
                else if (prefixCount == 1) {
                    prefix = "pre";
                }
                else prefix = "-1";

                //kill switch
                if (prefix.length() < 3) break;

                //give workers the prefix
                for (int i=0; i<workers.length; i++) {
                    workers[i].add(prefix);
                }

                //give results (in results array) back to SearchManager
                String sendBack;
                String wID;
            
                for (int i=0; i<paths.size(); i++) {
                    wID = "";
                    System.out.println("about to take");
                    sendBack = results.take();
                    System.out.println(sendBack);

                    //parse out worker id number - doesn't work, adds prefix count
                    Pattern p = Pattern.compile("(?<=Worker-)\\d+)");
                    Matcher m = p.matcher(sendBack);
                    if (m.find()) {
                        wID = m.group();
                    }


                      System.out.println(wID);

                    //send to SearchManager now along with count of workers
                }

                prefixCount++;
            }

            

            //done with prefixes now
            String killer = "-1";
            for (int i=0; i<workers.length; i++) {
                workers[i].add(killer);
            }

        }  catch (Exception e) {};
    }
}