package edu.cs300;
import CtCILibrary.*;

import java.util.ArrayList;
import java.util.concurrent.*;
import java.util.Scanner;
import java.io.File;
import java.util.regex.Pattern;

class Worker extends Thread{

  Trie textTrieTree;
  ArrayBlockingQueue<String> prefixRequestArray;
  ArrayBlockingQueue<String> resultsOutputArray;
  int id;
  String passageName;
  String path;
  int prefixCount;

  public Worker(String path,int id,ArrayBlockingQueue<String> prefix, ArrayBlockingQueue<String> results){
    // this.textTrieTree=new Trie(words);
    //trietree is unknown at this point - words is yet to be found - done in main now
    this.textTrieTree = null;
    this.prefixRequestArray=prefix;
    this.resultsOutputArray=results;
    this.id=id;
    
    //print out name of passage
    String[] pns = path.split("/");
    this.passageName=pns[pns.length-1];

    this.path=path;
    this.prefixCount=0;
  }

  //function to parse all the valid words in a document
  public static ArrayList<String> parsePassage(String path) {
    /**
     * open the path file
     * make regex scanner
     * put things in array, separated by the scanner
     * throw out anything <3 or with an ' or -
     */
    String testing;
    ArrayList<String> okWords = new ArrayList<String>();

    Scanner word;
    try {
        word = new Scanner(new File(path));
    } catch (Exception e) {
      System.err.println("file not found");
      return null;
    }
    word.useDelimiter("[^a-zA-Z\'-]");
      while (word.hasNext()) {
        testing = word.next();
        if (testing.length() < 3)
            continue;
          
        if ((testing.contains("\'")) || (testing.contains("-")))
            continue;
            
        okWords.add(testing);
      }
    return okWords;
  }

  //run when thread is started
  public void run() {
    System.out.println("Worker-"+this.id+" ("+this.passageName+") thread started ...");
    this.textTrieTree = new Trie(parsePassage(path));
    while (true){
      try {
        String prefix=(String)this.prefixRequestArray.take();
        
        //kill the worker with a prefix <3
        if (prefix.length() < 3) {
          return;
        }

        this.prefixCount++;
        boolean found = this.textTrieTree.contains(prefix);
        
        if (!found){
          //System.out.println("Worker-"+this.id+" "+req.requestID+":"+ prefix+" ==> not found ");
          resultsOutputArray.put("Worker-" + this.id + " " + this.prefixCount + ":" + prefix + " ==> not found");
        } else{
          //System.out.println("Worker-"+this.id+" "+req.requestID+":"+ prefix+" ==> "+word);
          resultsOutputArray.put("Worker-" + this.id + " " + this.prefixCount + ":" + prefix + " ==> " + this.textTrieTree.plss(prefix));
        }
      } catch(InterruptedException e){
          System.err.println(e.getMessage());
        }
    }
  }

}
