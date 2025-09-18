/***************************************************************
  Student Name: Ari Palanjian
  Class Name: Systems and Networks II
  Project 2

  Server Method definitions file: Creates and binds a socket to
  then listen for a browser connection. The program then reacts
  to browser request using the HTTP/1.1 protocol then sends an 
  appropriate response.
***************************************************************/

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

Server::Server(bool debug, std::string portno): debug{debug}, portno{portno}
{
    running = false;
    serverSocket = INVALID_SOCKET;
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(std::stoi(DEFAULT_PORT));
}

Server::~Server()
{
    for (auto& thread : tpool) 
    {
        thread.join();
    }

    close(serverSocket);
}

void Server::run() {
    std::cout << "Starting server...\nOS: " << getOsName() << "\n";
    running = true;

    int methodResult;
    methodResult = gethostname(hostname, sizeof(hostname));
    if (methodResult == 0){
        std::cout << "Host: " << hostname << "\n";
    }    
    
    methodResult = bind(serverSocket, reinterpret_cast<struct sockaddr *>(&address), sizeof(address));

    if (methodResult == 0) 
    {
        std::cout << "Binding to port " << DEFAULT_PORT << "...\n";
        int cnt = 0;

        methodResult = listen(serverSocket, 5);
        if (methodResult == 0)
        {
            while (running)
            {
                std::string waiting = "Waiting for connections...\n";
                std::cout << waiting;// << std::string(cnt,'.') << std::string(3-cnt, ' ') << std::flush;

                const SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
                if (clientSocket != INVALID_SOCKET) {
                    std::cout << "\n**Server** Connection established... Using Socket: " << clientSocket << "\n";
                    clientThread(clientSocket);
                    // threads.emplace_back(serverThread, clientSocket);
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

void Server::clientThread(const SOCKET& socket) {
    bool connected = true;
    int methodResult;

    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    struct timespec now;

    //Reduce recv() timeout to 30s
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    while (connected) {
        std::string buffer(1024,'\0');
        std::vector<std::string> request;
        std::vector<std::string> header;
        std::string response;
        #ifdef _WIN32
        methodResult = recv(socket, buffer.data(), buffer.size(), 0);
        #elif __APPLE__ || __MACH__
        methodResult = recv(socket, (void*)buffer.data(), buffer.size(), 0);
        #endif
        if (methodResult == -1) {
            const auto duration = now.tv_sec - start.tv_sec; // Time since last recv() call with data
            clock_gettime(CLOCK_REALTIME, &now);
            if (debug) {
                std::cerr << "\nError: " << errno << "\n";
                std::cout << duration  << "\n";
            }
            if (duration >= 200){
                connected = false;
                std::cout << "Client Connection Timeout...\n";
            }
        } else if (methodResult > 0){

            request = parseRequest(buffer);
            header = parseHeader(request.at(0));
            auto requestMap = requestToMap(request); //Load Remaining contents of request into map

            //Print Request contents
            if (debug) {
                for (const std::pair<const std::string, std::string>& n : requestMap){
                    std::cout << "Key:[" << n.first << "] Value:[" << n.second.substr(0, n.second.rfind('\r')) << "]\n";
                }
                
            }

            //Formulate response based on HTTP Method
            std::string method = header.at(0);
            std::string uri = header.at(1);
            std::string protocol = header.at(2).substr(0,header.at(2).find('\r'));
            std::cout << "**Server** Socket " << socket << " Request: [" 
                << method << " " << uri << " " << protocol
                << "]\n";
            
            if (requestMap.find("quit") != requestMap.end()) {
                connected = false;
            } else if (requestMap.find("quit-server") != requestMap.end()) {
                connected = false;
                running = false;
                std::cout << "Server Closed.\n";
            } else if(method.compare("POST") == 0){
                if(formPostResponse(requestMap)){
                    response = formGetResponse("/", protocol);
                    methodResult = send(socket, response.data(), response.size(), 0);
                    if (methodResult == -1){
                        std::cout << errno << ": Error sending to browser\n";
                    }
                } else {
                    std::cerr <<"Error: Missing POST request value.\n";
                }
            } else if (method.compare("GET") == 0) {
                response = formGetResponse(uri, protocol);
                if (response.size() > 0) {
                    methodResult = send(socket, response.data(), response.size(), 0);
                    if (methodResult == -1){
                        std::cout << errno << ": Error sending to browser\n";
                    }
                }
            } else {
                std::cerr << "Unknown request.\n";
            }

            std::cout << "**Server** Response: [" << response.substr(0, response.find("\n")) << "]\n";

            clock_gettime(CLOCK_REALTIME, &start);
        } else {
            connected = false;
            std::cout << "Client Connection Timeout...\n";
        }
    }

    close(socket);

    std::cout << "\tClient Connection Closed.\n";
}

std::vector<std::string> Server::parseRequest(const std::string & buffer){
    std::stringstream ss{buffer};
    std::vector<std::string> request;
    //Split Message By Line
    for (std::string tmp{}; std::getline(ss, tmp, '\n'); request.push_back(tmp)) {}
    //Clear stringstream contents
    ss.clear();

    return request;
}

std::vector<std::string> Server::parseHeader(const std::string & buffer){
    //Load Header into ss and split by whitespace
    std::stringstream ss{buffer};
    std::vector<std::string> header;
    for (std::string tmp{}; std::getline(ss, tmp, ' '); header.push_back(tmp)) {}

    ss.clear();

    return header;
}

std::unordered_map<std::string,std::string> Server::requestToMap(const std::vector<std::string>& request){
    std::unordered_map<std::string, std::string> map;
    for (int i = 1; i < request.size(); i++){
        std::string key, value, tmp = request.at(i);
        if(tmp.find(' ') != -1){
            key = tmp.substr(0,tmp.find(':'));
            value = tmp.substr(tmp.find(' ') + 1, tmp.find('\r'));
            map.insert({key,value});
        } else if (tmp.find('=') != -1) {
            key = tmp.substr(0,tmp.find('='));
            value = tmp.substr(tmp.find('=') + 1, tmp.find('\r'));
            map.insert({key,value});
        }
        
    }

    return map;
} 

const std::string Server::formGetResponse(std::string uri, std::string protocol){
    std::string response = protocol;
    std::string htmlContent = "";
    std::ifstream file;
    
    if (uri.compare("/testPresence.html") == 0){
        response += " 200 OK\nContent-Type: text/html\nContent-Length: ";
        file.open("templates/testPresence.html");
        htmlContent.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    } else if (uri.compare("/") == 0 || uri.compare("/index.html") == 0){
        response += " 200 OK\nContent-Type: text/html\nContent-Length: ";
        file.open("templates/index.html");
        htmlContent.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    } else if (uri.compare("/img.jpg") == 0){
        response += " 200 OK\nContent-Type: image/*\nContent-Length: ";
        file.open("assets/img.jpg");
        htmlContent.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    } else if (uri.compare("/favicon.ico") == 0){
        response += " 404 Not Found\n";
    } else {
        response += " 404 Not Found\nContent-Type: text/html\nConnection: keep-alive\nContent-Length: ";
        htmlContent = std::string("<HTML><HEAD><TITLE>WHOOPS</TITLE><link rel=\"icon\" href=\"data:,\"></HEAD>")
            +"<BODY BGCOLOR=\"#99cc99\" TEXT=\"#000000\" LINK=\"#2020ff\" VLINK=\"#4040cc\">"
            +"<h1>404 Not Found</h1></BODY></HTML>";
    }
    if (htmlContent.size() != 0) {
        response += std::to_string(htmlContent.size()) + "\n\n" + htmlContent;
    }


    file.close();
    return response;
}

bool Server::formPostResponse(std::unordered_map<std::string,std::string> &request) {
    auto it = request.find("client-message");
    if (it != request.end()){
        std::cout << "**Server** Message From Client: " << it->second << '\n';
        return true;
    }

    return false;
}