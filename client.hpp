/***************************************************************
  Student Name: Ari Palanjian
  Class Name: Systems and Networks II
  Project 2

  Client Header class definition
***************************************************************/

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include <cerrno>
#include <cstring>

#include <sys/wait.h>
#include <errno.h>
// #include <stdlib.h>


#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET ~0

class Client{
    SOCKET tcp_client_socket;
    struct sockaddr_in tcp_server_address;
    std::string DEFAULT_PORT;
    std::string DEFAULT_PROTOCOL;
    bool debug;
    bool running;

public:
    Client(bool debug, std::string portno);
    ~Client();

    void run();

    void parseRequest(const std::string &input, std::vector<std::string> &);
    bool clientGetResponse(std::string input);
    void clientPostResponse(std::string message);
    void clientLsExec();
};

#endif