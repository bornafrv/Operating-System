#include "logger.hpp"

Logger::Logger(){};

void Logger::log(string type, string message, string process_name)
{
    string color, code;

    if(type == "INFO")
    {
        color = "\033[32m";
        code = "\033[32m[INFO]\033[0m";
    }
    else if(type == "ERROR")
    {
        color = "\033[31m";   
        code = "\033[31m[ERROR]\033[0m"; 
    }

    cout << code << " " << color << "[" << process_name << "]\033[0m " << message << endl;

}