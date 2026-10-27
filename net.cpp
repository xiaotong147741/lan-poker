#include "net.h"

//服务器监听
int hostListen(int port){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        std::cout<<"create socket failed"<<std::endl;
        return -1;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(port);

    if(bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0){
        std::cout<<"bind socket failed"<<std::endl;
        close(fd);
        return -1;
    }

    if(listen(fd, 9) < 0){
        std::cout<<"listen failed"<<std::endl;
        close(fd);
        return -1;
    }
    return fd;
}

//等待客户端连接
int hostAccept(int fd){
    int cfd = accept(fd, NULL, NULL);
    if(cfd < 0){
        std::cout<<"accept failed"<<std::endl;
        return -1;
    }
    return cfd;
}

//客户端链接服务器
int clientConnection(const char *ip, int port){
    int cli = socket(AF_INET, SOCK_STREAM, 0);
    if(cli < 0){
        std::cout << "create socker failed" <<std::endl;
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    if(connect(cli, (sockaddr *)&addr, sizeof(addr)) < 0){
        std::cout<< "connect failed" <<std::endl;
        close(cli);
        return -1;
    }
    return cli;

}

//大厅等待
int waitLobby(int fd){
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    FD_SET(STDIN_FILENO, &set);

    int max_fd = fd;
    if(STDIN_FILENO > fd)
        max_fd = STDIN_FILENO;
    if(select(max_fd + 1, &set, NULL, NULL, NULL) < 0)
        return -1;
    if(FD_ISSET(fd, &set))
        return 1;
    return 2;
}
/*
//发送整型数据
int sendInt(int fd, int &x){
    if(send(fd, &x, sizeof(x), 0) != (ssize_t)sizeof(x))
        return -1;
    return 0;
}

//接收整型数据
int recvInt(int fd, int &x){
    if(recv(fd, &x, sizeof(x), 0) != (ssize_t)sizeof(x))
        return -1;
    return 0;
}
*/

//发送整型数据：循环直到 4 字节全部发完
int sendInt(int fd, const int &x){
    int sent = 0;
    while(sent < 4){
        ssize_t n = send(fd, (char*)&x + sent, 4 - sent, 0);
        if(n <= 0)
            return -1;
        sent += n;
    }
    return 0;
}

//接收整型数据：循环直到读满 4 字节
int recvInt(int fd, int &x){
    int got = 0;
    while(got < 4){
        ssize_t n = recv(fd, (char*)&x + got, 4 - got, 0);
        if(n <= 0)
            return -1;
        got += n;
    }
    return 0;
}

//发送字符串数据
int sendStr(int fd, const std::string &s){
    int len = s.size();
    if(sendInt(fd, len) < 0)
        return -1;
    int sent = 0;
    while(sent < len){
        ssize_t n = send(fd, s.c_str() + sent, len - sent, 0);
        if(n <= 0)
            return -1;
        sent += n;
    }
    return 0;
}

//接收字符串数据
int recvStr(int fd, std::string &s){
    int len = 0;
    if(recvInt(fd, len) < 0)
        return -1;
    if(len < 0 || len > 1024)
        return -1;
    char buf[1024];
    int got = 0;
    while(got < len){
        ssize_t n = recv(fd, buf + got, len - got, 0);
        if(n <= 0)
            return -1;
        got += n;
    }
    s.assign(buf, len);
    return 0;
}

void sendRoster(int cfd[], std::string nicks[], int ready[], int nclient){
    for(int i = 0; i < nclient; i++){
        int step = 31;
        if(sendInt(cfd[i], step) < 0)
            std::cout << "send failed" << std::endl;
        if(sendInt(cfd[i], nclient) < 0)
            std::cout << "send failed" << std::endl;
        for(int j = 0; j < nclient; j++){
            if(sendStr(cfd[i], nicks[j]) < 0)
                std::cout << "send failed" << std::endl;
            if(sendInt(cfd[i], ready[j]) < 0)
                std::cout << "send failed" << std::endl;
        }
        if(sendInt(cfd[i], 0) < 0)
            std::cout << "send failed" << std::endl;
    }
}