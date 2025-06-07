#pragma once

#include <iostream>
#include <string>
using namespace std;

class Logger
{
    public:
        Logger();
        void log(string type, string message, string process_name);
};