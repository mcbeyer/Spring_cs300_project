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
        String prefix = "con";
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
                System.out.println("while");

                SearchRequest message = MessageJNI.readPrefixRequestMsg();
                // prefix = message.prefix;
                prefixCount = message.requestID;

                System.out.println("search request");

                //kill switch
                if (prefix.length() < 3) break;

                //give workers the prefix
                for (int i=0; i<workers.length; i++) {
                    workers[i].add(prefix);
                }

                //give results (in results array) back to SearchManager
                String sendBack;
                int wID;
            
                for (int i=0; i<paths.size(); i++) {
                    sendBack = results.take();
                    
                    //parse out worker id number - doesn't work, adds prefix count
                    String[] sBtwo = sendBack.split("-");
                    String[] sBthree = sBtwo[1].split(" ");
                    wID = Integer.parseInt(sBthree[0]);
                    
                    System.out.println("message:" + sBthree[3]);

                    if (sendBack.contains("not found")) {
                        System.out.println(new MessageJNI().readPrefixRequestMsg());
                        new MessageJNI().writeLongestWordResponseMsg(prefixCount, prefix, wID, paths.get(wID), "----", paths.size(), 0);
                    }
                    else {
                        System.out.println(new MessageJNI().readPrefixRequestMsg());
                        new MessageJNI().writeLongestWordResponseMsg(prefixCount, prefix, wID, paths.get(wID), sBthree[3], paths.size(), 0);
                    }
                }

                prefix = "-1";
            }

            //done with prefixes now
            String killer = "-1";
            for (int i=0; i<workers.length; i++) {
                workers[i].add(killer);
            }

        }  catch (Exception e) {};
    }
}