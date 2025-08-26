/***************************************************************
  Student Name: Ari Palanjian
  Class Name: Systems and Networks II
  Project 2

  Server Header file to define Server class
***************************************************************/

#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <time.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.hpp"
typedef int SOCKET;


struct Header{
  std::string method;
  std::string uri;
  std::string protocol;
  Header(): method{""}, uri{"/"}, protocol{"HTTP/1.1"}
  {
  };
  Header(const std::string& m,
         const std::string& u,
         const std::string& p): method{m}, uri{u}, protocol{p}
  {
  };
  Header(const Header& copy): method{copy.method}, uri{copy.uri}, protocol{copy.protocol}
  {
  };
};

struct Request{
  Header header;
  std::unordered_map<std::string,std::string> payload;
  Request():header{},payload{}
  {
  };
  Request(const Header& h,
          const std::unordered_map<std::string,std::string>& p):
          header{h}, payload{p}
  {
  };
};

struct Response{
  std::string protocol;
  std::string content;
  std::string size;
  std::string status;
  std::string contentType;
  Response(): protocol{"HTTP/1.1"},content{},size{},status{},contentType{}
  {
  };
  Response(const std::string& p, const std::string& c, const std::string& sz, 
           const std::string& s, const std::string& cT):
           protocol{p},content{c},size{sz},status{s},contentType{cT}
  {
  };
  const std::string message()
  {
    return protocol + status + "\nContent-Type: " + contentType + "\nContent-Length: " + size + "\n\n" + content;
  }
};

const std::string statusFactory(const int& status){
  switch(status)
  {
    case 200:
      return " 200 OK";
    case 404:
      return " 404 Not Found";
    default:
      return " 501 Not Implemented";
  }
}

class Server {
  //Default Values
  const std::string DEFAULT_PORT{"6993"};
  const int MAX_PORT = 65536;
  const int INVALID_SOCKET = 0;

  //Newly Added Member Values
  std::string portno;
  Log serverLog;
  std::thread logging;

  //In the process of deprecating
  char hostname[256];
  SOCKET serverSocket;
  sockaddr_in address{};
  bool running;

  bool debug;
  void loggingThread(Log& sLog); //new

  void initServer(); // New

  std::vector<std::thread> tpool;

  void serverThread(const SOCKET& socket);

  void parseRequest(Request& request);
  void formResponse(Request& request, Response& response);

  std::vector<std::string> parseRequest(const std::string&); //Deprecated
  std::vector<std::string> parseHeader(const std::string&); //Deprecated
  std::unordered_map<std::string,std::string> requestToMap(const std::vector<std::string>& request);

  static const std::string formGetResponse(std::string uri, std::string protocol);
  bool formPostResponse(std::unordered_map<std::string,std::string> &);
public:
  
  Server(bool debug, std::string portno);
  ~Server();

  void run();
};



#endif //SERVER_HPP