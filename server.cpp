#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream> 
#include <vector>
using namespace std;
#define MAX_CLIENTS 5
#define BUFLEN 256


void error(char * msg) {
  perror(msg);
  exit(1);
}

struct InfoClient { //structura pentru informatiile unui client
  char nume[13];
  char prenume[13];
  int numar_card;
  int pin;
  char parola_secreta[9];
  double sold;
};

vector < InfoClient > v;
int nr;
int arr[100];

void read_input(char argv[]) { //citirea din fisierul users_data_file
  ifstream fin(argv);
  fin >> nr;
  for (int i = 0; i < nr; i++) {
    struct InfoClient info;
    fin >> info.nume >> info.prenume >> info.numar_card >> info.pin >> info.parola_secreta >> info.sold;
    v.push_back(info);
  }

  fin.close();
}

int main(int argc, char * argv[]) {
  int sockfd, newsockfd, portno, clilen;
  char buffer[BUFLEN];
  struct sockaddr_in serv_addr, cli_addr;
  int n, i, j;
  ofstream filelog("log");
  bool conectare = false;
  bool transfer = false;
  int x, y;
  double sum;
  read_input(argv[2]);
  for (i = 0; i < nr; i++)
    arr[i] = 0;

  fd_set read_fds; //multimea de citire folosita in select()
  fd_set tmp_fds; //multime folosita temporar 
  int fdmax; //valoare maxima file descriptor din multimea read_fds

  if (argc < 2) {
    fprintf(stderr, "Usage : %s port\n", argv[0]);
    exit(1);
  }

  //golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
  FD_ZERO( & read_fds);
  FD_ZERO( & tmp_fds);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  portno = atoi(argv[1]);

  memset((char * ) & serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY; // foloseste adresa IP a masinii
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr * ) & serv_addr, sizeof(struct sockaddr)) < 0)
    error("ERROR on binding");

  listen(sockfd, MAX_CLIENTS);

  //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
  FD_SET(0, & read_fds);
  FD_SET(sockfd, & read_fds);
  fdmax = sockfd;

  // main loop
  while (1) {
    tmp_fds = read_fds;

    if (select(fdmax + 1, & tmp_fds, NULL, NULL, NULL) == -1)
      error("ERROR in select");

    for (i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, & tmp_fds)) {

        if (i == 0) {
          memset(buffer, 0, BUFLEN);
          fgets(buffer, BUFLEN - 1, stdin);
          if (strcmp(buffer, "quit\n") == 0) {
            close(sockfd);
            return 0;
          }
        }

        if (i == sockfd) {
          // a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
          // actiunea serverului: accept()
          clilen = sizeof(cli_addr);
          if ((newsockfd = accept(sockfd, (struct sockaddr * ) & cli_addr, & clilen)) == -1) {
            error("ERROR in accept");
          } else {
            //adaug noul socket intors de accept() la multimea descriptorilor de citire
            FD_SET(newsockfd, & read_fds);
            if (newsockfd > fdmax) {
              fdmax = newsockfd;
            }
          }
          printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
        } else {
          // am primit date pe unul din socketii cu care vorbesc cu clientii
          //actiunea serverului: recv()
          memset(buffer, 0, BUFLEN);
          if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
            if (n == 0) {
              //conexiunea s-a inchis
              printf("selectserver: socket %d hung up\n", i);
              conectare = false;
              //filelog.close();
            } else {
              error("ERROR in recv");
            }
            close(i);
            FD_CLR(i, & read_fds); // scoatem din multimea de citire socketul pe care 
          } else { //recv intoarce >0
            //Am primit de la clientul de pe socketul i mesajul buffer
            char * p, * arg2, * arg3;
            char * buffsend;
            p = strtok(buffer, " \n");
            arg2 = strtok(NULL, " ");
            arg3 = strtok(NULL, " \n");

            if (strcmp(p, "login") == 0) { //cazul in care primesc login de la client                      
              if (conectare == false) { //daca nu e nimeni conectat pe server
                for (int j = 0; j < nr; j++) {
                  if (v[j].numar_card == atoi(arg2)) {
                    if ((v[j].pin == atoi(arg3)) && (arr[j] < 3)) {
                      sprintf(buffsend, "IBANK> Welcome %s %s\n\n", v[j].nume, v[j].prenume);
                      x = j; //memorez clientul
                      conectare = true; //activez conexiunea
                      break;

                    } else {
                      if (arr[j] < 2) {
                        strcpy(buffsend, "IBANK> -3: Pin gresit\n\n");
                        arr[j]++;
                        break;
                      } else { //daca se introduc 3 pinuri gresite consecutive pt un card se blocheaza 
                        strcpy(buffsend, "IBANK> -5: Card blocat\n\n");
                        break;
                      }
                    }
                  } else
                    strcpy(buffsend, "IBANK> -4 : Numar card inexistent\n\n");
                }
                send(i, buffsend, strlen(buffsend), 0);
              } else { //daca exista conexiunea
                strcpy(buffsend, "IBANK> -2: Sesiune deja deschisa\n\n");
                send(i, buffsend, strlen(buffsend), 0);
              }

            } else if (strcmp(p, "logout") == 0) { //cazul in care se primeste logout de la client
              if (conectare == true) {
                strcpy(buffsend, "IBANK> Clientul a fost deconectat\n\n");
                send(i, buffsend, strlen(buffsend), 0);
                conectare = false;
                arr[x] = 0;
              } else {
                strcpy(buffsend, "IBANK> -1: Clientul nu este autentificat\n\n");
                send(i, buffsend, strlen(buffsend), 0);
              }

            } else if (strcmp(p, "listsold") == 0 && conectare == true) { //listsold, doar daca exista conexiune
              sprintf(buffsend, "%.2f\n\n", v[x].sold);
              send(i, buffsend, strlen(buffsend), 0);

            } else if (strcmp(p, "transfer") == 0) { //cazul transfer
              for (int j = 0; j < nr; j++) {
                if (v[j].numar_card == atoi(arg2)) {
                  if (v[x].sold >= atof(arg3)) { //verific fondul clientului
                    sprintf(buffsend, "IBANK> Transfer %.2f catre %s %s? [y/n]\n", atof(arg3), v[j].nume, v[j].prenume);
                    y = j; //memorez clientul in contul caruia voi transfera banii
                    transfer = true;
                    sum = atof(arg3);
                  } else
                    strcpy(buffsend, "IBANK> -8: Fonduri insuficiente\n\n");
                  break;
                } else
                  strcpy(buffsend, "IBANK> -4: Numar card inexistent\n\n");
              }
              send(i, buffsend, strlen(buffsend), 0);

            } else if (strcmp(p, "y") != 0 && transfer == true) { //daca la transfer raspunsul e orice altceva decat y
              transfer = false;
              strcpy(buffsend, "IBANK> -9 Operatie anulata\n\n");
              send(i, buffsend, strlen(buffsend), 0);

            } else if (strcmp(p, "y") == 0 && transfer == true) { //daca se confirma transferul cu y
              transfer = false;
              strcpy(buffsend, "IBANK> Transfer realizat cu succes\n\n");
              send(i, buffsend, strlen(buffsend), 0);
              v[x].sold -= sum; //se scad banii din soldul clientului curent
              v[y].sold += sum; //se adauga in contului clientului solicitat

            } else if (strcmp(p, "quit") == 0) { //la quit se inchide sochetul si se deconecteaza clientul
              printf("QUIT");
              conectare = false;
              close(i);
              FD_CLR(i, & tmp_fds);
            }
          }
        }
      }
    }
  }
  return 0;
}