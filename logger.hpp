/*
*Create threaded server debug displays log otherwise only serious alerts shown in non-debug modes
*/
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <vector>
#include <chrono>
#include <termios.h>
#include <unistd.h>

struct Event{
    int code;
    std::string message;
    std::chrono::time_point<std::chrono::system_clock> time;// = std::chrono::system_clock::now()
};

class Log{
    std::vector<Event> events;
    void setStdinEcho(bool enable);
public:
    Log(){};
};

#endif

void Log::setStdinEcho(bool enable = true){
    //Disables key strokes being echoed to current terminal
    //Cross-plastform version definied in `messages/textloop.cpp`
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}