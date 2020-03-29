javac edu/cs300/PassageProcessor.java edu/cs300/Worker.java
javac edu/cs300/*.java
javac CtCILibrary/Trie.java CtCILibrary/TrieNode.java
javac -h . edu/cs300/MessageJNI.java

java -cp . -Djava.library.path=. edu.cs300.PassageProcessor
PAUSE