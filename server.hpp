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
#include <chrono>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include ".tools/logger.hpp"
using SOCKET = int;


struct Header{
  std::string method;
  std::string uri;
  std::string protocol;
  Header(): method{""}, uri{"/"}, protocol{"HTTP/1.1"}
  {
  }
  Header(const std::string& m,
         const std::string& u,
         const std::string& p): method{m}, uri{u}, protocol{p}
  {
  }
  Header(const Header& copy): method{copy.method}, uri{copy.uri}, protocol{copy.protocol}
  {
  }
};

struct Request{
  Header header;
  std::unordered_map<std::string,std::string> payload;
  Request():header{},payload{}
  {
  }
  Request(const Header& h,
          const std::unordered_map<std::string,std::string>& p):
          header{h}, payload{p}
  {
  }
};

struct Response{
  std::string protocol;
  std::string content;
  std::string size;
  std::string status;
  std::string contentType;
  Response(): protocol{"HTTP/1.1"},content{},size{},status{},contentType{}
  {
  }
  Response(const std::string& p, const std::string& c, const std::string& sz, 
           const std::string& s, const std::string& cT):
           protocol{p},content{c},size{sz},status{s},contentType{cT}
  {
  }
  const std::string message()
  {
    return protocol + status + "\nContent-Type: " + contentType + "\nContent-Length: " + size + "\n\n" + content;
  }
};

using namespace toolsAPUSC;
class Server {
  //Default Values
  const std::string DEFAULT_PORT{"6993"};
  const int MAX_PORT = 65536;
  const int INVALID_SOCKET = 0;

  //Current Member Values
  std::string portno;
  char hostname[256];

  Log serverLog;

  bool debug;
  

  //In the process of deprecating // May not be deprecating these things
  SOCKET serverSocket;
  sockaddr_in address{};
  bool running;


  void initServer(); // Server Initialization
  void welcomeMsg(); // Customizable Server Message on startup

  std::thread userInterface; //Thread controlling cli messages for critical server events
  std::vector<std::thread> tpool; //All threads connected to server, should consider periodically trying to join threads
  void uiThread(); //Fleshing out console message class
  void clientThread(const SOCKET& socket); //Updating previous serverThread from server.cpp


  void parseRequest(Request& request); //handles parsing request from client
  void formResponse(Request& request, Response& response); //forms response based on request

  //In the process of deprecating
  std::vector<std::string> parseRequest(const std::string&);
  std::vector<std::string> parseHeader(const std::string&);
  std::unordered_map<std::string,std::string> requestToMap(const std::vector<std::string>& request);

  static const std::string formGetResponse(std::string uri, std::string protocol);
  bool formPostResponse(std::unordered_map<std::string,std::string> &);
public:
  
  Server();
  Server(bool debug, std::string portno);
  ~Server();

  void run();
};



#endif //SERVER_HPP