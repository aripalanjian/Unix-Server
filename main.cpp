/***************************************************************
  Student Name: Ari Palanjian
  Class Name: Systems and Networks II
  Project 1

  Driver Program for HTTP Socket Server
***************************************************************/

#include "server.hpp"
#include <string.h>
#include <ostream>

int main(int argC, char** argV){
    bool debug = false;
    std::string portno = "51001";
    if (argC > 0){
        for (int i = 1; i < argC; i++){
            if (strcmp(argV[i], "-d") == 0){
                debug = true;
                std::cout << "Debug Mode\n";
            } else {
                portno = argV[i];
            }
        }
    }
    Server server(debug, portno);
    server.run();

    return 0;

}

//TODO: Add functionality to accept cli options