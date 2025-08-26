#include "server.hpp"

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

Server::Server(bool debug, std::string portno):debug{debug}, portno{portno}{
    initServer();
}

void Server::initServer()
{
    serverLog = Log{};
    logging = std::thread{loggingThread, std::ref(serverLog)};
    running = false;
    serverSocket = INVALID_SOCKET;
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(std::stoi(DEFAULT_PORT));
}

void Server::run()
{
    std::cout << "Starting server...\nOS: " << getOsName() << "\n";
    running = true;

    int methodResult;
    methodResult = gethostname(hostname, sizeof(hostname));
    if (methodResult == 0){
        std::cout << "Host: " << hostname << "\n";
    }    
    
    methodResult = bind(serverSocket, reinterpret_cast<struct sockaddr *>(&address), sizeof(address));

    if (methodResult == 0) {
        std::cout << "Binding to port " << DEFAULT_PORT << "...\n";

        methodResult = listen(serverSocket, 5);
        if (methodResult == 0){
            int cnt = 0;
            while (running) {
                std::string waiting = "Waiting for connections...\n";
                std::cout << waiting;// << std::string(cnt,'.') << std::string(3-cnt, ' ') << std::flush;

                const SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
                if (clientSocket != INVALID_SOCKET) {
                    std::cout << "\n**Server** Connection established... Using Socket: " << clientSocket << "\n";
                    tpool.emplace_back(std::thread{serverThread, std::ref(serverLog), clientSocket});
                }
            //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
            //     std::cout << "\r" << std::string(waiting.length() + cnt, ' ') << "\r";
            //     cnt = (cnt == 3) ? 0 : cnt + 1;
            }
        } else {
            std::cerr << errno << ": Error while listening.\n";
        }
    } else {
        std::cerr << "**Server** Error: Failed to bind to port." << std::endl;
    }
}

void Server::serverThread(const SOCKET& socket)
{

}