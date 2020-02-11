#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>


# define MSGSIZE 1024
# define BUFFER_FULL 1
# define BUFFER_EMPTY 0

#define ERROR_MSG "COMMAND ERROR"


int sendall(int, char*, int );
void decode(char*, char*);
void upper(char*, char*);
void storetofile(char*);
void readfromfile(char*);
void receiveall(int,int,char*,char*);
char*  evaluate(char*);
int p[2] , i=0 , rsize=0;
int PIPE_SIG = 0;
enum state{IDLE,INIT,TRANSMIT,RECEIVE};




int main(int argc, char* argv[]){

    enum state next_state;

    int numOfProcesses=atoi(argv[2]);

    int v = getpid();
    // Pipe Initialization
    pid_t pid ;
    if ( pipe(p) == -1) { perror ( " pipe call " ) ; exit (1) ;}

    signal(SIGPIPE,SIG_IGN);
    // Socket Initialization
    struct sockaddr_in local_addr,udp_addr;
    if(argc < 2){
        fprintf(stderr,"Usage: ./server [port]\n");
        exit(1);
    }
    int socket_des;

    // Creating the socket
    socket_des = socket(AF_INET,SOCK_STREAM,0);
    if (socket_des == -1){
        perror("Could not connect to socket!");
    }

    // Setting up the local address
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(atoi(argv[1]));

    int socket_udp;

    // Creating the socket UDP
    socket_udp = socket(AF_INET,SOCK_DGRAM,0);
    if (socket_udp == -1){
        perror("Could not connect to socket!");
    }



    // Binding the local address to the socket
    int bind_error;
    bind_error = bind(socket_des,(struct sockaddr *)&local_addr,sizeof(local_addr));
    if(bind_error == -1){
        perror("Could not bind address to port!");
        return 0;
    }

    int addrlen;
    int new_socket;
// Receive data
    int bufsize = MSGSIZE;
// Connection
char* localAddr=malloc(MSGSIZE);

if(listen(socket_des,3) == 0){
  // Accepting new connections

  struct sockaddr_in remote_addr;
  char* client_ipaddress = malloc(1024);
  addrlen=sizeof(remote_addr);
  new_socket = accept(socket_des, (struct sockaddr *)&remote_addr, &addrlen);
  if(new_socket == -1){
      perror("Could not accept the connection!");
  }else{strcpy(client_ipaddress,inet_ntoa(remote_addr.sin_addr));
      send(new_socket,"Server is ready...",18,0);
      printf("server: got connection from client %s \n", client_ipaddress);
  }
  printf("New Socket: %d from thread %d\n",new_socket,getpid());
// Read local address
       char* localAddr=malloc(1024);
       int num0 = recv(new_socket,localAddr,MSGSIZE,0);
       printf("LOCAL_ADDRESS=%d\n",atoi(localAddr));

// Setting up the local address UDP
  memset(&udp_addr, '\0', sizeof(udp_addr));
  udp_addr.sin_family = AF_INET;
  udp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  udp_addr.sin_port = htons(atoi(localAddr));


  close(socket_des);
}

// Transmit DATA
char c;
FILE *pipe_fp;
int ix;
int flag;
int j=0;
int pid1=0;
    while(1)
    {
      if(getpid()==v && j<numOfProcesses){
      j++;
      printf("j= %d\n",j);
      pid1=fork();
      }
      if(pid1 == 0) //Child code
      {
        char *inbuf;
        char *outbuf;
        char *command;
        int sent,sentok,sentsmall=0;
        inbuf=malloc(PIPE_BUF);
        while(read(p[0],inbuf, PIPE_BUF)!=0){
          outbuf=malloc(512);
          ix=0;
          flag=0;
          command=malloc(MSGSIZE);
          strcpy(command,inbuf);
          command=evaluate(command);

          if(command=="OK"){
            printf("evaluated..\n");
             /* Invoke ls through popen */
             if ((pipe_fp = popen(inbuf,"r")) == NULL){
               perror("popen");
             }
             c = getc(pipe_fp);
             while (c != EOF ){
               if(ix==512){
                 sent=sendto(socket_udp,outbuf, strlen(outbuf),0,(struct sockaddr*)&udp_addr,sizeof(udp_addr));
                 if(sent==-1){perror("Could not send");}
                  memset(outbuf,'\0', sizeof(outbuf));
                  ix=0;
                  c=getc(pipe_fp);
                  continue;
               }
               outbuf[ix]=c;
               c=getc(pipe_fp);
               if(c==EOF){sentok=sendto(socket_udp,outbuf, strlen(outbuf),0,(struct sockaddr*)&udp_addr,sizeof(udp_addr));}
                ix++;
             }
             sentsmall=sendto(socket_udp,"OK", strlen("OK"),0,(struct sockaddr*)&udp_addr,sizeof(udp_addr));
         }
        else{
          outbuf=" ";
          sentsmall=sendto(socket_udp,"OK", strlen("OK"),0,(struct sockaddr*)&udp_addr,sizeof(udp_addr));
        }
        pclose(pipe_fp);
        }

           exit(0);

        }

        else if(pid1 > 0){  // Parent code
          if(j==numOfProcesses){
            char *buffer;
            char *received;
            int num;
            bufsize = MSGSIZE;
            buffer = malloc(bufsize);
            received = malloc(bufsize);
            num = 0;

          //Receives the command
          if((num = recv(new_socket,buffer,bufsize,0)) == -1){
            perror("Could not recv");}
            strcat(received,buffer);
            strcpy(buffer,strtok(received,"."));
          if(strcmp(received,"OK")==0){
            free(buffer);
            free(received);
            wait(0);
          }
          else{
            write(p[1],received, 100);
          }

        }
      }

    }
    for(int i=0;i<numOfProcesses;i++) // loop will run n times (n=5)
    wait(NULL);

}
char * evaluate(char* inbuf){
   char delim[]= " ";
   char * buffer,* command = malloc(MSGSIZE);
   strcpy(command,inbuf);
   char *ptr = strtok(command, delim);

    if (strcmp(command, "ls") == 0)
    {
     return "OK";
    }
    else if (strcmp(command, "cut") == 0)
    {
    return "OK";
    }
    else if (strcmp(command, "grep") == 0)
    {
    return "OK";
    }
    else if (strcmp(command, "cat") == 0)
    {
    return "OK";
    }
    else if (strcmp(command, "rm") == 0)
    {
    return "OK";
    }
    else{
      return "error";
    }
}
void receiveall(int socket,int packetsize,char* buffer,char* rval){
	int num = 0;
    int total = 0;
	do{
	    if((num = recv(socket,buffer,strlen(buffer),0)) == -1){
		perror("Could not recv");
	    }
        total += num;
        if(packetsize > total)
	        strncat(rval,buffer,num);
        else
            strncat(rval,buffer,(num - (total -packetsize)));
	}while(strlen(rval) < packetsize);
    return;
}
void upper(char* rval,char* buffer){
    int i = 0;
    for(i = 0; i < strlen(buffer); i++){
        buffer[i] = toupper(buffer[i]);
    }
    strcpy(rval,buffer);
    return;
}
int sendall(int sockfd, char *buf, int len){
	int total = 0; // how many bytes we've sent
	int bytesleft = len; // how many we have left to send
	int n;
	while(total < len) {
		n = send(sockfd, buf+total, bytesleft, 0);
	if (n == -1) { break; }
		total += n; bytesleft -= n;
	}
	len = total; // return number actually sent here
	return n==-1?-1:0; // return -1 on failure, 0 on success
}

void readfromfile(char*rval){
	FILE *file;
	long lSize;
	char* buffer;
	file = fopen("store.txt","r");

    if( !file ) perror("blah.txt"),exit(1);

    fseek( file , 0L , SEEK_END);
    lSize = ftell( file );
    rewind( file );

    buffer = calloc( 1, lSize+1 );
    if( !buffer ) fclose(file),fputs("memory alloc fails",stderr),exit(1);

    if( 1!=fread( buffer , lSize, 1 , file) )
        fclose(file),free(buffer),fputs("entire read fails",stderr),exit(1);

    strcpy(rval,buffer);
    fclose(file);
    free(buffer);
	return;
}
