/***************************************************************
  Student Name: Ari Palanjian
  Class Name: Systems and Networks II
  Project 2

  Client class method definitions
***************************************************************/
#include "client.hpp"

Client::Client(bool debug, std::string portno){
    running = false;
    tcp_client_socket = INVALID_SOCKET;
    this->debug = debug;
    DEFAULT_PORT = portno;
    DEFAULT_PROTOCOL = "HTTP/1.1";

    tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0);

    tcp_server_address.sin_family = AF_INET;
    tcp_server_address.sin_addr.s_addr = INADDR_ANY;
    tcp_server_address.sin_port = htons(std::stoi(DEFAULT_PORT));
}

Client::~Client(){
    close(tcp_client_socket);
    int status;
    wait(&status);
}

void Client::run(){

    int connection_status = connect(tcp_client_socket, reinterpret_cast<struct sockaddr *>(&tcp_server_address), sizeof(tcp_server_address));     //params: which socket, cast for address to the specific structure type, size of address
    if (connection_status == -1) {                                                                                         //return value of 0 means all okay, -1 means a problem
        std::cout << "Error connecting to the socket: " << errno << '\n';
        std::cout << "Server may not be running...\n";
        exit(1);
    }

    std::cout << "  Options: \n    ls\t\t\t  list of html files found in templates"
                << "\n    GET /<file_name>\t  show HTML code for a given file"
                << "\n    POST <message>\t  send message to server"
                << "\n    exit\t          close client\n\n";
    

    //start client loop
    running = true;
    std::string request;
    std::string input;

    while (running) {
        std::cout << "client$ ";
        getline(std::cin, input);

        if (input.compare("quit") == 0 || input.compare("exit") == 0){ //Immediately exit before executing any more instructions
            running = false;
            std::string request = "POST / HTTP/1.1\r\nquit: exit\r\n";
            send(tcp_client_socket, request.data(), request.size(), 0);
            break;
        }

        
        std::vector<std::string> request;
        parseRequest(input, request);
        

        //React Based on user input
        if (request.at(0).compare("ls") == 0){
            if (debug) std::cout << "Attempting to display templates contents...\n";
            int child = fork();
            if (debug) std::cout << "Pid: " << child << '\n';
            if (child == 0) {
                std::string relativeTemplatesPath = "templates";
                char cmd[] = "ls";
                char* args[] = {cmd, relativeTemplatesPath.data()};
                
                if (debug) std::cout << " **child** searching path: " << relativeTemplatesPath <<'\n';
                execvp(cmd, args);
                exit(0);
            }
            sleep(1); //ensure input is on same line as prompt
        } else {
            if (request.at(0).compare("GET") == 0){
                clientGetResponse(input);
            } else if (request.at(0).compare("POST") == 0){
                clientPostResponse(input.substr(input.find(' ')));
            } else {
                std::cout << request.at(0) << ": command not recognized\n";
            }
        }
    }
}

void Client::parseRequest(const std::string & input, std::vector<std::string> &request){
    //Split input into request options
    std::stringstream ss{input};
    for (std::string tmp{}; std::getline(ss, tmp, ' '); request.push_back(tmp)) {}
    if (debug) {
        std::cout << "**Client Log** User Input: ";
        for(std::string s: request){
            std::cout << s << " ";
        }
        std::cout << '\n';
    }
}

bool Client::clientGetResponse(std::string input){
    input += std::string(" ") + DEFAULT_PROTOCOL;

    ssize_t conn = send(tcp_client_socket, input.data(), input.size(), 0);

    if (debug) std::cout << "**Client Log** Send size: " << conn << '\n';
    
    if (conn == -1){
        std::cout << "Error: Couldn't maintain server connection.";
        return false;
    }

    std::cout << "**Client**\n  Server Response: [\n    " ;
    std::string msg;

    //Loop to receive full message from server
    while (true) {
        std::string buffer(1024,'\0');
        auto size = recv(tcp_client_socket, buffer.data(), buffer.size(), 0);
        if (debug) std::cout << "\n**Client Log** Msg size: " << size << "\n";

        msg += buffer;
        
        if (size < buffer.size()) {
            break;
        } else if (size == -1) {
            std::cout << "Error: " << strerror(errno);
        }
    }
    std::cout << msg;
    std::cout << "\n  ]\n";

    return true;
}

void Client::clientPostResponse(std::string message){
    std::string request = "POST / HTTP/1.1\r\nclient-message: "+ message + "\r\n";
    if (debug) std::cout << "**Client Log** Request format: \n" << request << '\n';
    send(tcp_client_socket, request.data(), request.size(), 0);

    //Loop to receive full message from server
    while (true) {
        std::string buffer(1024,'\0');
        auto size = recv(tcp_client_socket, buffer.data(), buffer.size(), 0);
        if (debug) std::cout << "\n**Client Log** Msg size: " << size << "\n";

        if (size < buffer.size()) {
            break;
        } else if (size == -1) {
            std::cout << "Error: " << strerror(errno);
        }
    }

}

int main(int argC, char** argV) {
    bool debug = false;
    std::string portno = "51002";
    if (argC > 1){
        for (int i = 1; i < argC; i++){
            if (strcmp(argV[i], "-d") == 0){
                debug = true;
                std::cout << "Debug Mode\n";
            } else {
                portno = argV[i];
            }
        }
    }
    Client c(debug , portno);
    c.run();
}