all: 
	javac edu/cs300/*.java
	javac -h . edu/cs300/MessageJNI.java
	javac CtCILibrary/*.java
	export JAVA_HOME=/usr/java/latest
	gcc -c -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux system5_msg.c -o edu_cs300_MessageJNI.o
	gcc -shared -o libsystem5msg.so edu_cs300_MessageJNI.o -lc
	gcc -std=c99 -D_GNU_SOURCE msgrcv_lwr.c -o msgrcv
	gcc -std=c99 -D_GNU_SOURCE msgsnd_pr.c -o msgsnd
	gcc -std=c99 -D_GNU_SOURCE -lpthread -lrt -Wall searchmanager.c -o searchmanager

pull:
	git pull origin master

testp: all
	java -cp . -Djava.library.path=. edu.cs300.PassageProcessor

test: all
	./searchmanager 2 pr 02 345 p0w[, "iln" ottffssentettffssentt

testsig: all
	./searchmanager 0 con pre wor

clean:
	rm -f edu/cs300/*.class
	rm -f edu_cs300_MessageJNI.h
	rm -f CtCILibrary/*.class
	rm -f searchmanager
	rm -f msgrcv
	rm -f msgsnd