#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <limits.h>


int sendall(int , char *, int);
void readfromfile(char*);
int receiveall(int);
void storetofile(char* message,char * filename,int error);



FILE *file;
long lSize;

int lines_sent=0;

enum state{IDLE,INIT,TRANSMIT,RECEIVE};



int main(int argc, char* argv[]){
    int socket_desc,socket_dgram;
     enum state next_state;
    if(argc < 4){
        fprintf(stderr,"Usage: ./client [hostname] [port]\n");
        exit(1);
    }
    char* localAddr =malloc(1024);
    localAddr = argv[3];
    //printf("LOCAL address = %d\n",localAddr);

    // Creates a socket
    socket_desc = socket(AF_INET,SOCK_STREAM,0);
    socket_dgram = socket(AF_INET,SOCK_DGRAM,0);


    if(socket_desc == -1)
        perror("Could not create socket!");

    // Bind the socket to a port
    struct sockaddr_in local_address;

    // Getting the remote address
    char *HOST_ADDRESS = argv[1];
    struct hostent *hp;
    hp = gethostbyname(HOST_ADDRESS);

    memset(&local_address, '\0', sizeof(local_address));
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    local_address.sin_port = htons(atoi(argv[3]));
    int bind_error;
    bind_error = bind(socket_dgram,(struct sockaddr *)&local_address,sizeof(local_address));
    if(bind_error == -1){
        perror("Could not bind address to port!");
        return 0;
    }
    struct sockaddr_in remote_addr;
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(atoi(argv[2]));
    remote_addr.sin_addr = *((struct in_addr*)hp->h_addr);
    memset(&(remote_addr.sin_zero),'\0',8);

    // Connects to the server
    if(connect(socket_desc,(struct sockaddr *)&remote_addr,sizeof(remote_addr))){
        puts("error connecting...");
        return 0;
    }else{receiveall(socket_desc);printf("\n\r");}
///////////////////////////////////////////////////////////////
    file = fopen(argv[4],"r");
    fseek( file , 0L , SEEK_CUR);
    lSize = ftell( file );
    rewind(file);
    if( !file ) perror("blah.txt"),exit(1);
///////////////////////////////////////////////////////////////
    next_state = INIT;
    char *message;
    char *line;
    char *command;
    int boolean;
    char * get;
    char* buffer;
    char* inbuffer;
    ssize_t read;
    int addr_size = sizeof(local_address);
    int received;
    int file_no=1;



     // Sending a message to the remote server
    while(1){




      switch (next_state) {
        case IDLE:
           usleep(1000);
           next_state=RECEIVE;
           break;

        case INIT:
            send(socket_desc,localAddr,sizeof(localAddr),0);
            usleep(1000);
            //free(localAddr);
            next_state = TRANSMIT;
        break;

        case TRANSMIT:
            message = malloc(8080);
            line = malloc(2048);
            command = malloc(1024);
            get = malloc(1024);
            line = "test";
                  //line = NULL;
            buffer=malloc(1024);


                 //buffer = calloc( 1, lSize+1 );
                 if( !buffer ) fclose(file),fputs("memory alloc fails",stderr),exit(1);

                 read = getline(&buffer, &lSize, file);

                 if(strcmp(buffer,"")==0){
                    send(socket_desc,"OK",strlen("OK"),0);
                    fclose(file);
                    next_state=RECEIVE;

                 }

                  //gets(line);
                 strcpy(command,buffer);
                  //strcat(command,"\n.\n");
                  if(lines_sent==10){
                    printf("Waiting..\n");
                     sleep(5);
                     next_state=TRANSMIT;
                  }
                 send(socket_desc,buffer,strlen(buffer)-1,0);
                 lines_sent++;

                 usleep(1000);
                 free(buffer);
            break;

        case RECEIVE:
            inbuffer=malloc(PIPE_BUF);
            char* to_store = malloc(100);
            char filename[1024];
            char port[10];
            sprintf(port,"%d",file_no);
            strcpy(filename,"output.receivePORT.");
            strcat(filename,port);
            while(strcmp(inbuffer,"OK")!=0){

            for(int i=strlen((char*)inbuffer);i>=0;i--) inbuffer[i]='\0';

            received=recvfrom(socket_dgram,inbuffer,PIPE_BUF,0,(struct sockaddr*)&local_address, &addr_size);
            if(received<3){
              storetofile("null",filename,1);
            }
            else{
            storetofile(inbuffer,filename,0);
            }
            if(received==-1){perror("Could not receive");}

            }
            file_no++;

            free(inbuffer);
            next_state = IDLE;

        break;
      }



    }
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
int receiveall(int new_socket){

    int bufsize = 1024;
    char *buffer = malloc(bufsize);
    char *received = malloc(bufsize);
    int num;
    do{
    if((num = recv(new_socket,buffer,bufsize,0)) == -1){
        perror("Could not recv");
    }else{
        buffer[num] = '\0';
        printf("%s",buffer);
	    strcpy(received,buffer);
    }
}while(strpbrk(buffer,".") == NULL);
if(strcmp(received,"200 OK") == 0) return 1;
else return 0;
}

void storetofile(char* message,char* filename,int error){
	FILE *file;

	file = fopen(filename,"a");
	if(!file) perror("can't open file!");
  if(error==0){
  fprintf(file,message);
	//fwrite(message,sizeof(char*),strlen(message),file);
	fclose(file);
  }
  else{
    fprintf(file, " \n");
    fclose(file);

  }

	return;
}
