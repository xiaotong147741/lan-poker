#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/select.h>
#include <csignal>

int hostListen(int port);
int hostAccept(int fd);
int clientConnection(const char *ip, int port);

int waitLobby(int fd);

int sendInt(int fd, const int &x);
int recvInt(int fd, int &x);

int sendStr(int fd, const std::string &s);
int recvStr(int fd, std::string &s);

int runServer(int port, int startChips, int bigBlind, int maxPlayers, int handNum);

void sendRoster(int cfd[], std::string nicks[], int ready[], int nclient);