all: serverM.cpp serverC.cpp serverEE.cpp serverCS.cpp client.cpp
	g++ -o serverM serverM.cpp
	g++ -o serverC serverC.cpp
	g++ -o serverEE serverEE.cpp
	g++ -o serverCS serverCS.cpp
	g++ -o client client.cpp

serverM:serverM.o
	./serverM

serverC:serverC.o
	./serverC

serverEE:serverEE.o
	./serverEE

serverCS:serverCS.o
	./serverCS

client:client.o
	./client

clean:
	$(RM) serverM
	$(RM) serverC
	$(RM) serverEE
	$(RM) serverCS
	$(RM) client
	$(RM) serverM.o
	$(RM) serverC.o
	$(RM) serverEE.o
	$(RM) serverCS.o
	$(RM) client.o