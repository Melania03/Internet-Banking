#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <unistd.h>
using namespace std;

#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}


int main(int argc, char *argv[])
{
    int sockfd, n;
    ofstream filelog;
    char numelog[10];
    sprintf(numelog, "client-%d.log", getpid() );
    filelog.open(numelog);
    struct sockaddr_in serv_addr;
    struct hostent *server;
    fd_set read_fds;
    fd_set tmp_fds;
    int fdmax, j;
    char buffer[BUFLEN];
    if (argc < 3) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    FD_SET(0, &read_fds);
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;
    
    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting"); 
    while(1){
      tmp_fds = read_fds;
      select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
      for (j = 0; j <= fdmax; j++) {
        if (FD_ISSET(j, &tmp_fds)) {
          if (j == 0) { //trimit la server
            memset(buffer, 0, BUFLEN);

            fgets(buffer, BUFLEN-1, stdin);
            filelog << buffer;
             //scriu in fisier ce introduce clientul de la tastatura
              if (strcmp (buffer, "quit\n") == 0) {
                filelog.close();
                close(sockfd); //daca se introduce "quit", se inchide conexiunea
                return 0;         
            }
            
            n = send(sockfd, buffer, strlen(buffer), 0);
          
          }
          else if (j == sockfd) { //primesc de la server
            memset(buffer, 0, BUFLEN);
            n = recv(sockfd, buffer, sizeof(buffer), 0);
            if (n == 0)
              return 1;
            else {              
              printf("%s", buffer);
              //scriu in fisier ce primesc de la server
              filelog << buffer;
              
            }
          }
        }
      }
    }
}