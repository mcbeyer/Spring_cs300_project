import java.util.ArrayList;
import java.util.concurrent.*;
import java.util.Scanner;
import java.io.File;
import java.util.regex.Pattern;

public class test {
    public static ArrayList<String> parse(String path) {
        /**
         * open the path file
         * make regex scanner
         * put things in array, separated by the scanner
         * throw out anything <3 or with an ' or -
         */

        try {
            Scanner word = new Scanner(new File(path));
            word.useDelimiter("[^a-zA-Z\'-]");
            while (word.hasNext()) {
                System.out.println(word.next());
            }
        }  catch (Exception e) {};
        return null;  //jank
    }

    public static void main() {
        parse("Little_Women.txt");
    }
}