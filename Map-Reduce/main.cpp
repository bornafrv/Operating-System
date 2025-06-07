#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "logger.hpp"
using namespace std;

#define READ 0
#define WRITE 1
#define BUFFER_MAX_LENGTH 1024
#define PARTS_FILE "Parts.csv"
#define STORE_EXECUTABLE "./store.out"
#define PART_EXECUTABLE "./part.out"
#define INFO "INFO"
#define ERROR "ERROR"

Logger logger;

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
        logger.log(ERROR, "Failed to open file " + path, "main");
        exit(0);
    }

    while(getline(file, line))
    { 
        data.push_back(separate_words(line, ','));
    }

    file.close();

    return data;
}

void create_named_pipe(vector<string> stores, vector<string> parts)
{
    for(int i = 0; i < stores.size(); i++)
    {
        for(int j = 0; j < parts.size(); j++)
        {
            string name = stores[i] + "_to_" + parts[j];

            mkfifo(name.c_str(), 0666);

            logger.log(INFO, "Named pipe " + name + " has been created successfully", "main");
        }
    }
}

void delete_named_pipe(vector<string> stores, vector<string> parts)
{
    for (int i = 0; i < stores.size(); i++)
    {
        for(int j = 0; j < parts.size(); j++)
        {
            string name = stores[i] + "_to_" + parts[j];

            unlink(name.c_str());
    
            logger.log(INFO, "Named pipe " + name + " has been unlinked successfully", "main");
        }
    }
}

vector<string> show_existing_parts(vector<string> parts)
{
    cout << "We have " << parts.size() << " parts:" << endl;

    for(int i = 0; i < parts.size(); i++)
    {
        cout << " " << i + 1 << "- " << parts[i] << endl;
    }

    cout << endl << "Enter the number of the parts yout want, separated by space: ";

    return parts;
}

vector<string> get_chosen_parts(vector<string> parts)
{
    vector<string> numbers, chosen_parts;
    string line;

    getline(cin, line);

    numbers = separate_words(line,' ');

    for(int i = 0; i < numbers.size(); i++)
    {
        chosen_parts.push_back(parts[stoi(numbers[i]) - 1]);
    }

    return chosen_parts;
}

vector<string> find_stores(string path)
{
    vector<string> stores;
    string name;
    struct dirent *en;
    DIR *dir;

    dir = opendir(path.c_str());

    if(!dir)
    {
        logger.log(ERROR, "Failed to open directory " + path, "main");
        exit(0);
    }

    en = readdir(dir);
    while(en)
    {
        name = en->d_name;

        if(name != "." && name != ".." && name != PARTS_FILE)
        {
            stores.push_back(name);
        }

        en = readdir(dir);
    }
    closedir(dir);

    return stores;
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

void send_message_to_child(int write_fd, string message)
{
    write(write_fd, message.c_str(), BUFFER_MAX_LENGTH);

    logger.log(INFO, "Message has been written in a pipe successfully", "main");

    close(write_fd);
    
    logger.log(INFO, "A pipe fd has been closed successfully", "main");
}

int create_process(string message, string executable)
{
    int pid;
    int pipe_main_to_child[2];
    int pipe_child_to_main[2];

    if(pipe(pipe_main_to_child) == -1 || pipe(pipe_child_to_main) == -1)
    {
        logger.log(ERROR, "Failed to create a pipe", "main");
        exit(0);
    }

    logger.log(INFO, "Pipe has been created successfully", "main");

    pid = fork();

    if(pid < 0)
    {
        logger.log(ERROR, "Failed to create a child process", "main");
        exit(0);
    }
    else if(pid == 0)
    {        
        char child_read_fd[20];
        char child_write_fd[20];

        close(pipe_main_to_child[WRITE]);
        close(pipe_child_to_main[READ]);

        logger.log(INFO, "A pipe fd has been closed successfully", "main");

        sprintf(child_read_fd, "%d", pipe_main_to_child[READ]);
        sprintf(child_write_fd, "%d", pipe_child_to_main[WRITE]);

        logger.log(INFO, "Executable file " + executable + " is running", "main");
       
        execl(executable.c_str(), child_read_fd, child_write_fd, NULL);

        logger.log(ERROR, "Failed to execute " + executable + " file", "main");
        exit(0);
    }
    else
    {
        close(pipe_main_to_child[READ]);
        close(pipe_child_to_main[WRITE]);

        logger.log(INFO, "A pipe fd has been closed successfully", "main");

        send_message_to_child(pipe_main_to_child[WRITE], message);
    }

    return pipe_child_to_main[READ];
}

vector<int> create_stores_processes(vector<string> parts, vector<string> stores, string path)
{
    string all_parts;
    vector<int> fds_to_read;

    all_parts = make_message(parts);
    
    for(int i = 0; i < stores.size(); i++)
    {
        int main_read_fd;

        main_read_fd = create_process(path + '/' + stores[i] + ':' + all_parts, STORE_EXECUTABLE);
        
        logger.log(INFO, "A store process has been created successfully", "main");

        fds_to_read.push_back(main_read_fd);
    }

    return fds_to_read;
}

vector<int> create_parts_processes(vector<string> parts, vector<string> stores)
{
    string all_stores;
    vector<int> fds_to_read;

    all_stores = make_message(stores);

    for(int i = 0; i < parts.size(); i++)
    {
        int main_read_fd;

        main_read_fd = create_process(parts[i] + ':' + all_stores, PART_EXECUTABLE);
        
        logger.log(INFO, "A part process has been created successfully", "main");

        fds_to_read.push_back(main_read_fd);
    }

    return fds_to_read;
}

void push_back_information_from_stores(string buff, vector<string> &messages, vector<string> chosen_parts)
{
    vector<string> separated_parts, part_amount;

    separated_parts = separate_words(buff, '-');

    for(int i = 0; i < separated_parts.size(); i++)
    {
        part_amount = separate_words(separated_parts[i], ':');

        for(int j = 0; j < chosen_parts.size(); j++)
        {
            if(chosen_parts[j] == part_amount[0])
            {
                messages.push_back(part_amount[1]);
                break;
            }
        }
    }
}

void push_back_information_from_parts(string buff, vector<string> &messages, vector<string> chosen_parts)
{
    vector<string> part_price_quantity = separate_words(buff, '-');

    for(int i = 0; i < chosen_parts.size(); i++)
    {
        if(chosen_parts[i] == part_price_quantity[0])
        {
            messages.push_back(part_price_quantity[0]);
            messages.push_back(part_price_quantity[1]);
            messages.push_back(part_price_quantity[2]);

            break;
        }
    }
}

vector<string> read_from_child(vector<int> fds_to_read, vector<string> chosen_parts, string type)
{
    vector<string> messages, separated_parts, part_number;

    for(int i = 0; i < fds_to_read.size(); i++)
    {
        char buff[BUFFER_MAX_LENGTH];
        
        read(fds_to_read[i], buff, BUFFER_MAX_LENGTH);
        
        logger.log(INFO, "Message has been read from " + type + " successfully", "main");

        close(fds_to_read[i]);
        
        logger.log(INFO, "A pipe fd has been closed successfully", "main");

        if(type == "store")
        {
            push_back_information_from_stores(buff, messages, chosen_parts);
        }
        else if(type == "part")
        {
            push_back_information_from_parts(buff, messages, chosen_parts);

        }
    }

    return messages;
}

float compute_total_profit(vector<string> profits)
{
    float total_profit = 0;

    for(int i = 0; i < profits.size(); i++)
    {
        total_profit += stof(profits[i]);
    }

    return total_profit;
}

void show_results(vector<int> fds_to_read_from_stores, vector<int> fds_to_read_from_parts, vector<string> chosen_parts)
{
    float total_profit;
    vector<string> messages_from_stores, messages_from_parts;

    messages_from_stores = read_from_child(fds_to_read_from_stores, chosen_parts, "store");
    messages_from_parts = read_from_child(fds_to_read_from_parts, chosen_parts, "part");

    total_profit = compute_total_profit(messages_from_stores);

    for(int i = 0; i < messages_from_parts.size(); i += 3)
    {
        cout << messages_from_parts[i] << endl << '\t' << "Total leftover quantity = " << stoi(messages_from_parts[i + 2]) << endl 
             << '\t' << "Total leftover price = " << stoi(messages_from_parts[i + 1]) << endl;
    }

    cout << "Total profits of all stores = " << total_profit << endl;
}

int main(int argc, char *argv[])
{
    string path = "./" + string(argv[1]);
    vector<string> parts, stores, chosen_parts;
    vector<int> fds_to_read_from_stores, fds_to_read_from_parts;

    parts = read_from_csv(path + '/' + PARTS_FILE)[0];
    
    logger.log(INFO, "Parts have been found successfully", "main");

    stores = find_stores(path);
    
    logger.log(INFO, "Stores have been found successfully", "main");

    create_named_pipe(stores, parts);
    
    show_existing_parts(parts);

    chosen_parts = get_chosen_parts(parts);

    fds_to_read_from_stores = create_stores_processes(parts, stores, path);
    fds_to_read_from_parts = create_parts_processes(parts, stores);

    wait(NULL);

    show_results(fds_to_read_from_stores, fds_to_read_from_parts, chosen_parts);
    
    logger.log(INFO, "All child processes have been terminated successfully", "main");

    delete_named_pipe(stores, parts);

    return 0;
}