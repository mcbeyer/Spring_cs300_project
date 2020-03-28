package edu.cs300;
import CtCILibrary.*;

import java.util.ArrayList;
import java.util.concurrent.*;
import java.util.Scanner;
import java.io.File;
import java.util.regex.Pattern;

class Worker extends Thread{

  Trie textTrieTree;
  ArrayBlockingQueue prefixRequestArray;
  ArrayBlockingQueue resultsOutputArray;
  int id;
  String passageName;

  public Worker(String path,int id,ArrayBlockingQueue prefix, ArrayBlockingQueue results){
    // this.textTrieTree=new Trie(words);
    this.textTrieTree = null;
    this.prefixRequestArray=prefix;
    this.resultsOutputArray=results;
    this.id=id;
    this.passageName="Passage-"+Integer.toString(id)+".txt";//put name of passage here
  }

  public ArrayList<String> parse(String path) {
    /**
     * open the path file
     * make regex scanner
     * put things in array, separated by the scanner
     * throw out anything <3 or with an ' or -
     */

    try {
      Scanner word = new Scanner(new File(path));
      word.useDelimiter("[^a-zA-Z\'-]");

    }  catch (Exception e) {};
    return null;  //jank
  }

  public void run() {
    System.out.println("Worker-"+this.id+" ("+this.passageName+") thread started ...");
    //while (true){
      try {
        String prefix=(String)this.prefixRequestArray.take();
        boolean found = this.textTrieTree.contains(prefix);
        
        if (!found){
          //System.out.println("Worker-"+this.id+" "+req.requestID+":"+ prefix+" ==> not found ");
          resultsOutputArray.put(passageName+":"+prefix+" not found");
        } else{
          //System.out.println("Worker-"+this.id+" "+req.requestID+":"+ prefix+" ==> "+word);
          resultsOutputArray.put(passageName+":"+prefix+" found");
        }
      } catch(InterruptedException e){
        System.out.println(e.getMessage());
      }
    //}
  }

}
