#include "server.hpp"
#include "messages/clitext.hpp"

using toolsAPUSC::nl;

std::string getOsName()
{
    //Used to display OS name
    //https://stackoverflow.com/questions/15580179/how-do-i-find-the-name-of-an-operating-system
    #ifdef _WIN32
    return "Windows 32-bit";
    #elif _WIN64
    return "Windows 64-bit";
    #elif __APPLE__ || __MACH__
    return "Mac OSX";
    #elif __linux__
    return "Linux";
    #elif __FreeBSD__
    return "FreeBSD";
    #elif __unix || __unix__
    return "Unix";
    #else
    return "Other";
    #endif
}

const std::string statusFactory(const int& status)
{
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

Server::Server():debug(true), portno(DEFAULT_PORT), running(false)
{
    initServer();
}

Server::Server(bool debug, std::string portno):debug(debug), portno(portno), running(false)
{
    initServer();
}

Server::~Server()
{
    for (auto& thread : tpool) 
    {
        thread.join();
    }
    userInterface.join();

    close(serverSocket);
}

void Server::initServer()
{
    //Initialize Log
    serverLog = (debug) ? Log{debug} : Log{};

    //initialize ui thread
    userInterface = std::thread{uiThread};
    serverLog.addEvent({Code::START, "New Log Created:"});
    serverLog.addEvent({Code::INIT, "Server Initializing"});

    running = false;
    serverSocket = INVALID_SOCKET;
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(std::stoi(portno));
}

void Server::welcomeMsg(){
    std::cout << "Starting server..." << getOsName() << nl;

    std::cout << "Running on: ";
    int getHost = gethostname(hostname, sizeof(hostname));
    if (getHost == 0){
        std::cout << hostname << nl;
    } else {
        std::cout << "Unkown Host Name\n";
    }
}

void Server::run()
{
    running = true;
    
    int bindSocket = bind(serverSocket, reinterpret_cast<struct sockaddr *>(&address), sizeof(address));

    if (bindSocket == 0)
    {
        serverLog.addEvent({Code::INIT, "Binding to port " + DEFAULT_PORT, true});

        int listenSocket = listen(serverSocket, 5);
        if (listenSocket == 0)
        {
            int cnt = 0;
            while (running)
            {

                int clientSocket = accept(serverSocket, nullptr, nullptr);
                if (clientSocket != INVALID_SOCKET)
                {
                    serverLog.addEvent({Code::SERVERMSG, "Connection established... Using Socket: " + clientSocket, true});
                    tpool.emplace_back(std::thread{clientThread, std::ref(serverLog), clientSocket});
                }
            }
        } else {
            std::cerr << errno << ": Error while listening.\n";
        }
    } else {
        std::cerr << "[Server] Error: Failed to bind to port." << nl;
    }
}

void Server::uiThread()
{
    welcomeMsg();
    while (tpool.size() < 1)
    {   
        //Flesh out console message class
        cmsg::loopMsgMod("Waiting for connections",'.',3);
    }

    //log loop will print critical server events, all other events will be serialized
    int exitStatus = serverLog.logLoop();
    if (exitStatus !=0)
    {
        std::cerr << "[Log] ERROR: Server exited unexpectedly with status code " << exitStatus << nl;
    }
    std::cout << "[Log] Server Log exited normally.\n";
}

void Server::clientThread(const SOCKET& socket)
{

}

/*
    Future:
        Add functionality for creating user defined functions to handle HTTP requests
*/