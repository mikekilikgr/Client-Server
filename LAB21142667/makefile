all: remoteServer remoteClient

remoteServer: remoteServer.o
	cc remoteServer.o -o remoteServer

remoteClient: remoteClient.o
	cc remoteClient.o -o remoteClient

remoteServer.o: remoteServer.c
	cc -c -Wall remoteServer.c -o remoteServer.o

remoteClient.o: remoteClient.c
	cc -c -Wall remoteClient.c -o remoteClient.o

clean:
	rm -f remoteServer.o remoteServer
	rm -f remoteClient.o remoteClient
	rm -f output.receivePORT.*
