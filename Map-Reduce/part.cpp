#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include "logger.hpp"
using namespace std;

#define BUFFER_MAX_LENGTH 1024
#define INFO "INFO"
#define ERROR "ERROR"

Logger logger;
string part;

vector<string> separate_words(string line, char separator)
{
    vector<string> words;
    string word; 
    stringstream ss(line); 

    while(getline(ss, word, separator))
    {    
        words.push_back(word);
    }

    return words; 
}

vector<string> read_from_parent(int read_fd)
{
    char buff[BUFFER_MAX_LENGTH];

    read(read_fd, buff, BUFFER_MAX_LENGTH);

    close(read_fd);

    return separate_words(string(buff), ':');
}

vector<string> read_message_from_named_pipe(vector<string> stores)
{
    vector<string> messages;

    for(int i = 0; i < stores.size(); i++)
    {
        char buff[BUFFER_MAX_LENGTH];
        string name = stores[i] + "_to_" + part;

        int named_pipe_fd = open(name.c_str(), O_RDONLY);

        if(named_pipe_fd < 0)
        {
            logger.log(ERROR, "Failed to open named pipe " + name, part);
            exit(0);
        }

        read(named_pipe_fd, buff, BUFFER_MAX_LENGTH);
        
        logger.log(INFO, "Message has been written in named pipe" + name + " successfully", part);

        close(named_pipe_fd);
        
        logger.log(INFO, "Named pipe " + name + " fd  has been closed successfully", part);

        messages.push_back(string(buff));
    }

    return messages;
}

string compute_total_leftovers(vector<string> messages, int index)
{
    float result = 0;

    for(int i = 0; i < messages.size(); i++)
    {
        result += stof(separate_words(messages[i], '-')[index]);
    }

    return to_string(result);
}

void send_message_to_parrent(int write_fd, string price, string quantity)
{
    write(write_fd, (part + '-' + price + '-' + quantity).c_str(), BUFFER_MAX_LENGTH);
    
    logger.log(INFO, "Message has been written in a pipe successfully", part);

    close(write_fd);
    
    logger.log(INFO, "A pipe fd has been closed successfully", part);
}

int main(int argc, char *argv[])
{
    int read_fd = stoi(argv[0]), write_fd = stoi(argv[1]);
    string leftover_price, leftover_quantity;
    vector<string> temp_read, stores, messages;

    temp_read = read_from_parent(read_fd);

    part = temp_read[0];

    stores = separate_words(temp_read[1], '-');

    messages = read_message_from_named_pipe(stores);

    leftover_price = compute_total_leftovers(messages, 0);
    leftover_quantity = compute_total_leftovers(messages, 1);

    send_message_to_parrent(write_fd, leftover_price, leftover_quantity);

    return 0;
}