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
        String testing;
        ArrayList<String> okWords = new ArrayList<String>();

        try {
            Scanner word = new Scanner(new File(path));
            word.useDelimiter("[^a-zA-Z\'-]");
            while (word.hasNext()) {
                testing = word.next();
                if (testing.length() < 3)
                    continue;
                
                if ((testing.contains("\'")) || (testing.contains("-"))
                    continue;

                okWords.add(testing);
            }
        }  catch (Exception e) {};
        return okWords;
    }

    public static void main(String[] args) {
        parse("Little_Women.txt");
    }
}