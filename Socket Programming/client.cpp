#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
using namespace std;

typedef struct pollfd pollfd;

#define STDIN 0
#define STDOUT 1
#define BUFFER_SIZE 512

const char* CORRECT_ROOM = "You successfully joined the room!\n";
const char* ROCK_PAPER_SCISSORS = "Enter your choice from: rock, paper, scissors\n";
const char* TIME_OUT = "Your time is out!\n";
const char* FINAL_RESULTS = "- Final game results:\n";

int connect_user(char* ipaddr, int opt, int port)
{
    int new_fd;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, ipaddr, &(addr.sin_addr)) == -1)
        perror("FAILED: Input ipv4 address invalid");

    if((new_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        perror("FAILED: Socket was not created");

    if(setsockopt(new_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable failed");

    if(setsockopt(new_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable port failed");

    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    
    addr.sin_port = port;

    if(connect(new_fd, (sockaddr*)(&addr), sizeof(addr)))
        perror("FAILED: Connect"); 

    return new_fd;
}

int connect_broadcast(char* ipaddr, int opt, int broadcast, int port)
{
    int new_fd;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET; 
    if((new_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        perror("FAILED: Socket was not created");

    if(setsockopt(new_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1)
        perror("FAILED: Making socket reusable failed");

    if(setsockopt(new_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        perror("FAILED: Making socket reusable port failed");

    addr.sin_port = port; 
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    if(bind(new_fd, (struct sockaddr*)(&addr), sizeof(addr)) == -1)
        perror("FAILED: Bind unsuccessfull");

    return new_fd;
}

void alarm_handler(int sig)
{
    write(STDOUT, TIME_OUT, strlen(TIME_OUT));
}

int main(int argc, char* argv[])
{
    if(argc != 3)
        perror("Invalid Arguments");

    char buffer[BUFFER_SIZE]; 
    char* ipaddr = argv[1];
    int server_fd, broadcast_fd, opt = 1, broadcast = 1, room_fd = -1, room_port, correct = 0;
    vector<pollfd> pfds;

    server_fd = connect_user(ipaddr, opt, htons(strtol(argv[2], NULL, 10)));
    broadcast_fd = connect_broadcast(ipaddr, opt, broadcast, htons(8080));

    pfds.push_back(pollfd{STDIN, POLLIN, 0});
    pfds.push_back(pollfd{server_fd, POLLIN, 0});
    pfds.push_back(pollfd{broadcast_fd, POLLIN, 0});

    while(1)
    {
        if(poll(pfds.data(), (nfds_t)(pfds.size()), -1) == -1)
            perror("FAILED: Poll");

        for(size_t i = 0; i < pfds.size(); ++i)
        {
            if(pfds[i].revents & POLLIN)
            {
                int fd = pfds[i].fd;

                if(fd == STDIN) // recieve message from standard input
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    read(STDIN, buffer, BUFFER_SIZE); 
                    send(server_fd, buffer, strlen(buffer), 0);
                }
                else if(fd == server_fd)  // recieve message from server
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(server_fd, buffer, BUFFER_SIZE, 0); 

                    if(((string)buffer).substr(0, strlen(CORRECT_ROOM)) == CORRECT_ROOM)
                    {
                        correct = 1;
                        room_port = stoi(((string)buffer).substr(strlen(CORRECT_ROOM)).c_str());

                        strcpy(buffer, CORRECT_ROOM);
                    }

                    write(STDOUT, buffer, strlen(buffer));

                    if(correct) 
                    {
                        correct = 0;
                        room_fd = connect_user(ipaddr, opt, room_port);

                        pfds.push_back(pollfd{room_fd, POLLIN, 0});
                    }
                }
                else if(fd == room_fd) // recieve message from room
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(room_fd, buffer, BUFFER_SIZE, 0); 
                    write(STDOUT, buffer, strlen(buffer));

                    if(((string)buffer).substr(0, strlen(ROCK_PAPER_SCISSORS)) == ROCK_PAPER_SCISSORS)
                    {
                        signal(SIGALRM, alarm_handler);
                        siginterrupt(SIGALRM, 1);

                        alarm(10);

                        memset(buffer, 0, BUFFER_SIZE);
                        int read_ret = read(STDIN, buffer, BUFFER_SIZE);

                        alarm(0);

                        if(read_ret == -1)
                        {
                            send(room_fd, TIME_OUT, strlen(TIME_OUT), 0);
                        }
                        else
                        {
                            send(room_fd, buffer, strlen(buffer), 0);
                        }
                    }
                }
                else if(fd == broadcast_fd) // recieve message from broadcast
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    recvfrom(broadcast_fd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
                    write(STDOUT, buffer, strlen(buffer));

                    if(((string)buffer).substr(0, strlen(FINAL_RESULTS)) == FINAL_RESULTS)
                    {
                        exit(0);
                    }
                }
            }
        }
    }
}