#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <set>
#include <sys/stat.h>
#include <fcntl.h>
#include "logger.hpp"
using namespace std;

#define BUFFER_MAX_LENGTH 1024
#define INFO "INFO"
#define ERROR "ERROR"

Logger logger;
string store;

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

vector<vector<string>> read_from_csv(string path)
{
    vector<vector<string>> data;
    string line;
    ifstream file(path);

    if(!file.is_open())
    {
        logger.log(ERROR, "Failed to open file " + path, store);
        exit(0);
    }

    while(getline(file, line))
    { 
        data.push_back(separate_words(line, ','));
    }

    file.close();

    return data;
}

void separate_input_output_parts(vector<vector<string>> data, vector<string> &unique_parts, vector<string> &in_parts, vector<string> &in_prices, 
                                 vector<string> &in_quantities, vector<string> &out_parts, vector<string> &out_prices, vector<string> &out_quantities)
{
    set<string> unique_set;

    for(int i = 0; i < data.size(); i++)
    {
        string input = data[i][3].substr(0, 5), output = data[i][3].substr(0, 6);

        unique_set.insert(data[i][0]);

        if(input == "input")
        {
            in_parts.push_back(data[i][0]);
            in_prices.push_back(data[i][1]);
            in_quantities.push_back(data[i][2]);
        }
        else if(output == "output")
        {
            out_parts.push_back(data[i][0]);
            out_prices.push_back(data[i][1]);
            out_quantities.push_back(data[i][2]);
        }
    }

    for(int i = 0; i < unique_set.size(); i++)
    {
        unique_parts.push_back(*next(unique_set.begin(), i));
    }
}

vector<string> read_from_parent(int read_fd)
{
    char buff[BUFFER_MAX_LENGTH];

    read(read_fd, buff, BUFFER_MAX_LENGTH);

    close(read_fd);

    return separate_words(string(buff), ':');
}

vector<string> compute_profits(vector<vector<string>> data)
{
    vector<string> profits, unique_parts, in_parts, in_prices, in_quantities, out_parts, out_prices, out_quantities;

    separate_input_output_parts(data, unique_parts, in_parts, in_prices, in_quantities, out_parts, out_prices, out_quantities);

    for(int i = 0; i < unique_parts.size(); i++)
    {
        float profit = 0;

        for(int j = 0; j < out_parts.size(); j++)
        {
            if(out_parts[j] != unique_parts[i])
                continue;

            for(int k = 0; k < in_parts.size(); k++)
            {
                if((out_parts[j] != in_parts[k]) || (in_quantities[k] == "0"))
                    continue;

                if(stoi(in_quantities[k]) < stoi(out_quantities[j]))
                {
                    profit += (stof(out_prices[j]) - stof(in_prices[k])) * stoi(in_quantities[k]);

                    out_quantities[j] = to_string(stoi(out_quantities[j]) - stoi(in_quantities[k]));
                    in_quantities[k] = "0";
                }
                else
                {
                    profit += (stof(out_prices[j]) - stof(in_prices[k])) * stoi(out_quantities[j]);

                    in_quantities[k] = to_string(stoi(in_quantities[k]) - stoi(out_quantities[j]));
                    out_quantities[j] = "0";
                    break;
                }
            }
        }

        profits.push_back(unique_parts[i] + ':' + to_string(profit));
    }

    return profits;
}

vector<string> compute_leftover_prices(vector<vector<string>> data)
{
    vector<string> leftover_prices, unique_parts, in_parts, in_prices, in_quantities, out_parts, out_prices, out_quantities;

    separate_input_output_parts(data, unique_parts, in_parts, in_prices, in_quantities, out_parts, out_prices, out_quantities);

    for(int i = 0; i < out_parts.size(); i++)
    {
        for(int j = 0; j < in_parts.size(); j++)
        {
            if((out_parts[i] != in_parts[j]) || (in_quantities[j] == "0"))
                continue;

            if(stoi(in_quantities[j]) < stoi(out_quantities[i]))
            {
                out_quantities[i] = to_string(stoi(out_quantities[i]) - stoi(in_quantities[j]));
                in_quantities[j] = "0";
            }
            else
            {
                in_quantities[j] = to_string(stoi(in_quantities[j]) - stoi(out_quantities[i]));
                out_quantities[i] = "0";
                break;
            }
        }
    }

    for(int i = 0; i < unique_parts.size(); i++)
    {
        int price = 0;

        for(int j = 0; j < in_parts.size(); j++)
        {
            if(in_parts[j] != unique_parts[i])
                continue;

            price += stof(in_prices[j]) * stoi(in_quantities[j]);
        }

        leftover_prices.push_back(to_string(price));
    }

    return leftover_prices;
}

vector<string> compute_leftover_quantities(vector<vector<string>> data)
{
    vector<string> leftover_quantities, unique_parts, in_parts, in_prices, in_quantities, out_parts, out_prices, out_quantities;

    separate_input_output_parts(data, unique_parts, in_parts, in_prices, in_quantities, out_parts, out_prices, out_quantities);

    for(int i = 0; i < unique_parts.size(); i++)
    {
        int quantity = 0;

        for(int j = 0; j < data.size(); j++)
        {
            if(data[j][0] != unique_parts[i])
                continue;

            string input = data[j][3].substr(0, 5), output = data[j][3].substr(0, 6);

            if(input == "input")
            {
                quantity += stoi(data[j][2]);
            }   
            else if(output == "output")
            {
                quantity -= stoi(data[j][2]);
            }    
        }

        leftover_quantities.push_back(to_string(quantity));
    }

    return leftover_quantities;
}

string make_message(vector<string> messages)
{
    string message = "";

    for(int i = 0; i < messages.size(); i++)
    {
        if(i != messages.size() - 1)
        {
            message += messages[i] + '-';
        }
        else
        {
            message += messages[i];
        }
    }

    return message;
}

void send_message_to_parent(int write_fd, vector<string> messages)
{
    string message;

    message = make_message(messages);
    
    write(write_fd, message.c_str(), BUFFER_MAX_LENGTH);
    
    logger.log(INFO, "Message has been written in a pipe successfully", store);

    close(write_fd);
    
    logger.log(INFO, "A pipe fd has been closed successfully", store);
}

void send_messages_to_named_pipes(vector<vector<string>> data, vector<string> prices, vector<string> quantities, vector<string> parts)
{
    vector<string> unique_parts, in_parts, in_prices, in_quantities, out_parts, out_prices, out_quantities;

    separate_input_output_parts(data, unique_parts, in_parts, in_prices, in_quantities, out_parts, out_prices, out_quantities);

    for(int i = 0; i < parts.size(); i++)
    {
        string name = store + "_to_" + parts[i];

        int named_pipe_fd = open(name.c_str(), O_WRONLY);

        if(named_pipe_fd < 0)
        {
            logger.log(ERROR, "Failed to open named pipe " + name, store);
            exit(0);
        }

        for(int j = 0; j < unique_parts.size(); j++)
        {
            if(unique_parts[j] != parts[i])
                continue;

            write(named_pipe_fd, (prices[j] + '-' + quantities[j]).c_str(), BUFFER_MAX_LENGTH);
            
            logger.log(INFO, "Message has been written in named pipe" + name + " successfully", store);

            close(named_pipe_fd);
            
            logger.log(INFO, "Named pipe " + name + " fd  has been closed successfully", store);

            break;
        }

        write(named_pipe_fd, "0", BUFFER_MAX_LENGTH);
        
        logger.log(INFO, "Message has been written in named pipe" + name + " successfully", store);

        close(named_pipe_fd);

        logger.log(INFO, "Named pipe " + name + " fd  has been closed successfully", store);
    }
}

int main(int argc, char *argv[])
{
    int read_fd = stoi(argv[0]), write_fd = stoi(argv[1]);
    string path, profit;
    vector<string> profits, leftover_prices, leftover_quantities, temp_read, parts;
    vector<vector<string>> data;

    temp_read = read_from_parent(read_fd);

    path = temp_read[0];

    store = path.substr(9, path.size() - 9);

    parts = separate_words(temp_read[1], '-');

    data = read_from_csv(path);

    profits = compute_profits(data);
    leftover_prices = compute_leftover_prices(data);
    leftover_quantities = compute_leftover_quantities(data);

    send_message_to_parent(write_fd, profits);

    send_messages_to_named_pipes(data, leftover_prices, leftover_quantities , parts);

    return 0;
}