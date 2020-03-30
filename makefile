JObj = CtCILibrary/Trie.class CtCILibrary/TrieNode.class edu/cs300/TextSamples.class edu/cs300/PassageProcessor.class edu/cs300/Worker.class edu_cs300_MessageJNI.h
CObj = SearchManager msgsnd msgrcv edu_cs300_MessageJNI.o

all: $(JObj) $(CObj)

pull:
	git pull origin master

CtCILibrary/TrieNode.class: CtCILibrary/TrieNode.java
	javac CtCILibrary/TrieNode.java

CtCILibrary/Trie.class: CtCILibrary/Trie.java CtCILibrary/TrieNode.java
	javac CtCILibrary/Trie.java

edu_cs300_MessageJNI.h: edu/cs300/MessageJNI.java
	javac -h . edu/cs300/MessageJNI.java

edu/cs300/PassageProcessor.class: edu/cs300/PassageProcessor.java edu/cs300/Worker.java
	javac edu/cs300/PassageProcessor.java

edu/cs300/TextSamples.class: edu/cs300/TextSamples.java
	javac edu/cs300/TextSamples.java

edu/cs300/Worker.class: edu/cs300/Worker.java
	javac edu/cs300/Worker.java

edu_cs300_MessageJNI.o: system5_msg.c
	export JAVA_HOME=/usr/java/latest
	gcc -c -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux system5_msg.c -o edu_cs300_MessageJNI.o
	gcc -shared -o libsystem5msg.so edu_cs300_MessageJNI.o -lc

msgrcv: msgrcv_lwr.c
	gcc -std=c99 -D_GNU_SOURCE msgrcv_lwr.c -o msgrcv

msgsnd: msgsnd_pr.c
	gcc -std=c99 -D_GNU_SOURCE msgsnd_pr.c -o msgsnd

searchmanager: searchmanager.c $(JObj)
	gcc -std=c99 -D_GNU_SOURCE searchmanager.c -o searchmanager

testp: $(JObj)
	java -cp . -Djava.library.path=. edu.cs300.PassageProcessor

test: all
	./searchmanager 2 con pre wor

clean:
	rm $(JObj)
	rm $(CObj)
