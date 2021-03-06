package edu.cs300;

import java.util.Scanner;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.List;

public class PassageProcessor {
    static String passagePath = "passages.txt";

    public static void main(String[] args) {
        PassageProcessing();
    }

    public static void PassageProcessing() {
        /**
         * make thread for all text files
         * each thread makes a trie
         * wait for prefixes to come in
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
        
        ArrayList<String> paths = new ArrayList<String>();
        String prefix;
        int prefixCount = 0;
        ArrayList<Worker> workerList = new ArrayList<Worker>();

        try {
            Scanner passage = new Scanner(new File(passagePath));
            String line;
            //read in each passage path and give it to a worker
            while (passage.hasNextLine()) {
                line = passage.nextLine();
                paths.add(line);
            }

            ArrayBlockingQueue<String> results = new ArrayBlockingQueue<String>(paths.size()*10);
            ArrayList<ArrayBlockingQueue<String>> workers = new ArrayList<ArrayBlockingQueue<String>>();
             
            File newFile;
            //starts the trie creation
            for (int i=0; i<paths.size(); i++) {
                newFile = null;
                //test the passage paths
                try {
                    newFile = new File(paths.get(i));
                } catch (NullPointerException e) {
                    System.err.println("File " + paths.get(i) + " doesn't exist\n");
                    continue;
                }

                if (!newFile.exists()) {
                    System.err.println("File " + paths.get(i) + " doesn't exist\n");
                    continue;
                }

                //run the workers
                workers.add(new ArrayBlockingQueue<String>(1));
                workerList.add(new Worker(paths.get(i), workers.size()-1, workers.get(workers.size()-1), results));
                workerList.get(workerList.size()-1).start();
            }

            if (workerList.size() == 0) {
                System.err.println("No valid passages in passages.txt\n");
                return;
            }
             
            while (true) {

                //from system5_msg
                SearchRequest message = MessageJNI.readPrefixRequestMsg();
                prefix = message.prefix;
                prefixCount = message.requestID;

                System.out.println("**prefix(" + prefixCount + ") " + prefix + " received");
                
                //kill switch from searchmanager
                if ((prefix.length() < 3) || prefix.compareTo("   ") == 0) break;

                //give workers the prefix
                for (int i=0; i<workers.size(); i++) {
                    workers.get(i).add(prefix);
                }

                //give results (in results array) back to SearchManager
                String sendBack;
                int wID;
            
                for (int i=0; i<workerList.size(); i++) {
                    //pull results from the queue
                    sendBack = results.take();
                    
                    //parse out worker id number
                    String[] sBtwo = sendBack.split("-");
                    String[] sBthree = sBtwo[1].split(" ");
                    wID = Integer.parseInt(sBthree[0]);

                    //send back message to searchmanager
                    if (sendBack.contains("not found")) {
                        new MessageJNI().writeLongestWordResponseMsg(prefixCount, prefix, wID, workerList.get(wID).passageName, "----", workerList.size(), 0);
                    }
                    else {
                        new MessageJNI().writeLongestWordResponseMsg(prefixCount, prefix, wID, workerList.get(wID).passageName, sBthree[3].substring(1), workerList.size(), 1);
                    }
                }
            }

            //done with prefixes now - send in the killer
            String killer = "-1";
            for (int i=0; i<workers.size(); i++) {
                workers.get(i).add(killer);
            }
            //join the threads
            for (Worker w : workerList) {
                w.join();
            }
            System.out.println("Terminating ...\n");
        }  catch (Exception e) {System.err.println(e.getMessage());}
    }
}