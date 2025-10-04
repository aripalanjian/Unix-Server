/*
*Create threaded server debug displays log otherwise only serious alerts shown in non-debug modes
*/
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <vector>
#include <thread>
#include <fstream>
#include <format>
#include <chrono>
#include <concepts>
#include <termios.h>
#include <unistd.h>

namespace logger
{

constexpr char nl = '\n';

enum class Code {
    //GENERAL EVENT CODES
    START, INIT, RUNNING, TERMINATE, DEFUALT, SERVERMSG,SERIALIZE,
    //CONNECTION AND SERVER EVENT CODES
    CONNACCEPT, CONNFAIL, SERVERBIND,
    //HTTP REQUEST METHODS
    CONNECT, DELETE, GET, HEAD, OPTIONS, 
    PATCH, POST, PUT, TRACE
};

constexpr std::string_view getCodeString(const Code& code)
{
    switch (code)
    {
        case Code::START: return "START";
        case Code::INIT: return "INIT";
        case Code::RUNNING: return "RUNNING";
        case Code::TERMINATE: return "TERMINATE";
        case Code::DEFUALT: return "DEFUALT";
        case Code::SERVERMSG: return "SERVERMSG";
        case Code::SERIALIZE: return "SERIALIZE";
        case Code::CONNACCEPT: return "CONNACCEPT";
        case Code::CONNFAIL: return "CONNFAIL";
        case Code::SERVERBIND: return "SERVERBIND";
        case Code::CONNECT: return "CONNECT";
        case Code::DELETE: return "DELETE";
        case Code::GET: return "GET";
        case Code::HEAD: return "HEAD";
        case Code::OPTIONS: return "OPTIONS";
        case Code::PATCH: return "PATCH";
        case Code::POST: return "POST";
        case Code::PUT: return "PUT";
        case Code::TRACE: return "TRACE";
        default: return "UNKNOWN";
    }
}

struct Event{
    /*
        To add:
            *Pid/tid
            *Severity levels
            *IP Address
            *Client ID
    */
    Code code;
    std::string description;
    bool toPrint;
    std::chrono::time_point<std::chrono::system_clock> time; // = std::chrono::system_clock::now()
    Event(): 
        code(Code::DEFUALT), description("DEFAULT"), toPrint(false), time(std::chrono::system_clock::now())
    {
    }
    Event(Code code, const std::string&& description): 
        code(code), description(std::move(description)), toPrint(false), time(std::chrono::system_clock::now())
    {
    }
    Event(Code code, const std::string&& description, bool toPrint): 
        code(code), description(std::move(description)), toPrint(toPrint), time(std::chrono::system_clock::now())
    {
    }
};

class Log{
    //Consider transforming into a linked list fifo queue rather than vector
    size_t lastEventIndex;
    size_t lastEventSerializedIndex;
    std::vector<Event> events;

    bool debug;

    void setStdinEcho(bool enable)
    {
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

public:
    Log():lastEventIndex(0), lastEventSerializedIndex(0), events(), debug(false)
    {
    }
    Log(const bool debug):lastEventIndex(0), lastEventSerializedIndex(0), events(), debug(debug)
    {
    }
    ~Log()
    {
        close();
    }

    int logLoop()
    {   
        size_t durationBetweenSerailizations = 0;
        while(1)
        {
            /*To implement:
                *Throttle thread resource access when incoming # of Events are low
                *Print only on debug otherwise store
            */
            if (lastEventIndex != events.size()) 
            {
                //Get Event 
                Event event = events.at(lastEventIndex++);
                //Check Termination first
                std::string logEvent;
                if (event.code == Code::TERMINATE)
                {
                    int serializeStatus = serialize();
                    if (serializeStatus != 0)
                    {
                        std::puts(std::format("[Log] ERROR: Failed to serialize log with status code {}", serializeStatus).data());
                        return serializeStatus;
                    }
                    std::puts("Goodbye!");
                    return 0;
                }                
                else
                {    
                    //Handle All other events
                    handleEvent(logEvent, event);
                }

                //Print All Events in debug mode
                if(debug || event.toPrint) std::puts(logEvent.data());
            }
            else
            {
                //No new events, sleep for 1 second
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            //Currently serializes every ~10 seconds or if there are more than 100 new events
            if (durationBetweenSerailizations >= 10 || lastEventIndex - lastEventSerializedIndex > 100)
            {
                int serializeStatus = serialize();
                if (serializeStatus != 0)
                {
                    std::puts(std::format("[Log] ERROR: Failed to serialize log with status code {}", serializeStatus).data());
                    return serializeStatus;
                }
                durationBetweenSerailizations = 0;
            }
            else
            {
                durationBetweenSerailizations++;
            }
        }
        return -1;
    }

    void handleEvent(std::string& logEvent, const Event& event)
    {
        logEvent = std::format("[{}] {} {}",
            getCodeString(event.code), 
            event.description, 
            event.time);
    }

    //variadic function?
    // template <typename... Events>
    void addEvent(const Event&& event)//, Events&&... args)
    {
        events.emplace_back(std::move(event));
        // addEvent(std::forward<Events>(args)...);
    }

    int serialize()
    {
        std::ofstream ofs;

        using namespace std::chrono_literals;
        //Get current time in time_t format
        std::time_t tp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        //Convert to local calandar time
        std::tm c_tp = *std::localtime(&tp);

        std::string file = debug ? 
            std::format("../logs/debug-log-{}-{}-{}.csv", c_tp.tm_mday, c_tp.tm_mon + 1, c_tp.tm_year + 1900):
            std::format("../logs/log-{}-{}-{}.csv", c_tp.tm_mday, c_tp.tm_mon + 1, c_tp.tm_year + 1900);

        ofs.open(file,
                std::ios::out |   // output file stream
                std::ios::app |   // can append to a existing file
                std::ios::ate );  // set file cursor at the end
        
        if(ofs)
        {
            if (ofs.tellp() == 0)
            {
                //New file, add header row
                ofs << "EventCode,EventString,Description,Day,Month,Year\n";
            }
            //Serialize all new events
            size_t i = lastEventSerializedIndex;
            for (; i < lastEventIndex; i++)
            {
                Event event = events.at(i);
                std::time_t event_tp = std::chrono::system_clock::to_time_t(event.time);
                std::tm event_c_tp = *std::localtime(&event_tp);
                ofs << std::format("{},{},{},{},{},{}\n", 
                    std::__to_underlying(event.code), //Event code as integer
                    getCodeString(event.code), //Event code as string
                    event.description,
                    event_c_tp.tm_mday, //Day of the month [1,31] 
                    event_c_tp.tm_mon + 1, //Month of the year [1,12]
                    event_c_tp.tm_year + 1900); //Year since 1900
            }
            lastEventSerializedIndex = i;
    
            ofs.close();
            return 0;
        }
        else 
        {
            std::puts("Unable to open file");
            return -1;
        }
    }

    int close(){
        addEvent({Code::TERMINATE, "Server Terminating", false});
        int serializeStatus = serialize();
        if (serializeStatus != 0)
        {
            std::puts(std::format("[Log] ERROR: Failed to serialize log with status code {}", serializeStatus).data());
            return serializeStatus;
        }
        return 0;
    }
};

}
#endif