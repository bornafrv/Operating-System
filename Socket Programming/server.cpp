#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
using namespace std;

typedef struct pollfd pollfd;

#define STDIN 0
#define STDOUT 1
#define BUFFER_SIZE 512

const char* SERVER_LAUNCHED = "Server Launched!\n";
const char* ROOM_LAUNCHED = "Room Launched!\n";
const char* NEW_CONNECTION = "New Connection!\n";
const char* WELCOMED = "Hello! Welcome to our game. what is your name?\n";
const char* CORRECT_ROOM = "You successfully joined the room!\n";
const char* WAITING = "Please wait for another player to join this room.\n";
const char* STARTING = "Your game is in starting process...\n";
const char* ROCK_PAPER_SCISSORS = "Enter your choice from: rock, paper, scissors\n";
const char* ROCK = "rock";
const char* PAPER = "paper";
const char* SCISSORS = "scissors";
const char* TIME_OUT = "Your time is out!\n";
const char* END_GAME = "end_game\n";
const char* FINAL_RESULTS = "- Final game results:\n";
const char* NOTHING = "nothing";
const char* NOTHING2 = "nothing\n";

struct User {
    char name[BUFFER_SIZE];
    int fd;
    int room_fd;
    int number_of_wins;
    bool has_name;
};

struct Room {
    User user1;
    User user2;
    char choice1[BUFFER_SIZE];
    char choice2[BUFFER_SIZE];
    int number_of_players;
    int fd;
    int port;
};

int connect_server(char* ipaddr, int opt, int port)
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

    if(bind(new_fd, (const struct sockaddr*)(&addr), sizeof(addr)) == -1)
        perror("FAILED: Bind unsuccessfull");

    if(listen(new_fd, 20) == -1)
        perror("FAILED: Listen unsuccessfull");

    return new_fd;
}

int connect_broadcast(int opt, int broadcast, int port, sockaddr_in &addr)
{
    int new_fd;

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

void send_rooms_to_user(vector<Room> rooms, int total_rooms, int user_fd, bool invalid, bool full)
{
    string rooms_list;

    if(invalid)
        rooms_list = "You chose an invalid room. please choose a correct room again:\n";
    else if(full)
        rooms_list = "You chose a full room. please choose another room again:\n";
    else
        rooms_list = "Please choose a room from these available rooms to join:\n";

    for(int i = 0; i < total_rooms; i++)
    {
        if(rooms[i].number_of_players == 2)
            continue;

        string temp = "- Room number " + to_string(i + 1) + ": ";

        if(rooms[i].number_of_players == 0)
            temp += " Emplty.\n";
        else if(rooms[i].number_of_players == 1)
            temp += " A player is waiting.\n";
        else if(rooms[i].number_of_players == 2)
            temp += " Full.\n";

        rooms_list += temp;
    }

    const char* CHOOSE_ROOM = rooms_list.c_str();

    send(user_fd, CHOOSE_ROOM, strlen(CHOOSE_ROOM), 0);
}

int is_message_to_server(int fd, vector<User> users)
{
    for(int i = 0; i < users.size(); i++)
    {
        if(fd == users[i].fd)
            return i;
    }
 
    return -1;
}

int is_message_to_room(int fd, vector<Room> rooms)
{
    for(int i = 0; i < rooms.size(); i++)
    {
        if(fd == rooms[i].fd)
            return rooms[i].fd;
    }
 
    return -1;
}

int find_user_index_by_name(char name[BUFFER_SIZE], vector<User> users)
{
    for(int i = 0; i < users.size(); i++)
    {
        if(!strcmp(users[i].name, name))
            return i;
    }

    return -1;
}

void judging(Room &room, vector<User> &users, vector<Room> &rooms, int broadcast_fd, sockaddr_in &addr)
{
    int index_user1 = find_user_index_by_name(room.user1.name, users);
    int index_user2 = find_user_index_by_name(room.user2.name, users);
    string choice1(room.choice1), choice2(room.choice2), name1(room.user1.name), name2(room.user2.name);

    if(choice1 == TIME_OUT)
    {
        choice1 = NOTHING2;
    }
    if(choice2 == TIME_OUT)
    {
        choice2 = NOTHING2;
    }

    choice1 = choice1.substr(0, strlen(choice1.c_str()) - 1);
    choice2 = choice2.substr(0, strlen(choice2.c_str()) - 1);
    name1 = name1.substr(0, strlen(name1.c_str()) - 1);
    name2 = name2.substr(0, strlen(name2.c_str()) - 1);

    string result = name1 + " chose " + choice1 + " AND " + name2 + " chose " + choice2 + ".\n" + "- Match result:\n\t";

    if((choice1 == ROCK && choice2 == ROCK) || (choice1 == PAPER && choice2 == PAPER) || (choice1 == SCISSORS && choice2 == SCISSORS) ||
       (choice1 == NOTHING && choice2 == NOTHING))
    {
        result += "Draw!\n";
    }
    else if((choice1 == ROCK && choice2 == SCISSORS) || (choice1 == PAPER && choice2 == ROCK) || (choice1 == SCISSORS && choice2 == PAPER) ||
            (choice2 == NOTHING))
    {
        result += name1 + " wins!\n\t" + name2 + " looses!\n";

        users[index_user1].number_of_wins ++;
    }
    else if((choice1 == ROCK && choice2 == PAPER) || (choice1 == PAPER && choice2 == SCISSORS) || (choice1 == SCISSORS && choice2 == ROCK) ||
            (choice1 == NOTHING))
    {
        result += name2 + " wins!\n\t" + name1 + " looses!\n";

        users[index_user2].number_of_wins ++;
    }

    room.number_of_players = 0;

    const char* MATCH_RESULT = result.c_str();

    sendto(broadcast_fd, MATCH_RESULT, strlen(MATCH_RESULT), 0, (struct sockaddr*)(&addr), sizeof(addr));
    sleep(1);
    send_rooms_to_user(rooms, rooms.size(), users[index_user1].fd, false, false);
    sleep(1);
    send_rooms_to_user(rooms, rooms.size(), users[index_user2].fd, false, false);
}

void print_game_results(vector<User> users, int broadcast_fd, sockaddr_in &addr)
{
    string result = FINAL_RESULTS;

    for(int j = 0; j < users.size(); j++)
    {
        string name(users[j].name);
        name = name.substr(0, strlen(name.c_str()) - 1);

        result += "\t" +  name + ": " + to_string(users[j].number_of_wins) + " wins!\n";
    }

    const char* GAME_RESULT = result.c_str();

    sendto(broadcast_fd, GAME_RESULT, strlen(GAME_RESULT), 0, (struct sockaddr*)(&addr), sizeof(addr));
}

int main(int argc, char* argv[])
{
    if(argc != 4)
        perror("Invalid Arguments");

    char buffer[BUFFER_SIZE], caller_user_name[BUFFER_SIZE];
    char* ipaddr = argv[1];
    struct sockaddr_in bc_addr;
    int server_fd, room_fd, broadcast_fd, user_index, opt = 1, broadcast = 1, total_rooms = stoi(argv[3]), count = 0;
    vector<pollfd> pfds;
    vector<User> users, users_in_room;
    vector<Room> rooms;

    server_fd = connect_server(ipaddr, opt, htons(strtol(argv[2], NULL, 10)));

    write(STDOUT, SERVER_LAUNCHED, strlen(SERVER_LAUNCHED));

    pfds.push_back(pollfd{STDIN, POLLIN, 0});
    pfds.push_back(pollfd{server_fd, POLLIN, 0});

    for(int i = 0; i < total_rooms; i++)
    {
        Room new_room;

        room_fd = connect_server(ipaddr, opt, 2000 + i);

        write(STDOUT, ROOM_LAUNCHED, strlen(ROOM_LAUNCHED));

        new_room.fd = room_fd;
        new_room.number_of_players = 0;
        new_room.port = 2000 + i;

        pfds.push_back(pollfd{room_fd, POLLIN, 0});
        rooms.push_back(new_room);
    }

    broadcast_fd = connect_broadcast(opt, broadcast, htons(8080), bc_addr);

    pfds.push_back(pollfd{broadcast_fd, POLLIN, 0});

    while(1)
    {
        if(poll(pfds.data(), (nfds_t)(pfds.size()), -1) == -1)
            perror("FAILED: Poll");
        
        for(size_t i = 0; i < pfds.size(); ++i)
        {
            if(pfds[i].revents & POLLIN)
            {
                int fd = pfds[i].fd, room_fd;

                if(fd == STDIN) // recieve message from standard input
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    read(STDIN, buffer, BUFFER_SIZE);

                    if(!strcmp(buffer, END_GAME))
                    {
                        print_game_results(users, broadcast_fd, bc_addr);

                        exit(0);
                    }
                }
                else if(fd == broadcast_fd) // recieve message from broadcast
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    recvfrom(broadcast_fd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
                }
                else if(fd == server_fd) // new user connects to server
                {
                    struct sockaddr_in new_addr;
                    User new_user;
                    socklen_t new_size = sizeof(new_addr);
                    int new_fd = accept(server_fd, (struct sockaddr*)(&new_addr), &new_size);

                    write(STDOUT, NEW_CONNECTION, strlen(NEW_CONNECTION));

                    new_user.has_name = false;
                    new_user.fd = new_fd;
                    new_user.room_fd = -1;
                    new_user.number_of_wins = 0;

                    users.push_back(new_user);
                    pfds.push_back(pollfd{new_fd, POLLIN, 0});

                    send(new_fd, WELCOMED, strlen(WELCOMED), 0);
                }
                else if((room_fd = is_message_to_room(fd, rooms)) != -1) // new user connects to room
                {
                    struct sockaddr_in new_addr;
                    socklen_t new_size = sizeof(new_addr);
                    int new_fd = accept(room_fd, (struct sockaddr*)(&new_addr), &new_size);

                    if(rooms[fd - 4].number_of_players == 0)
                    {
                        rooms[fd - 4].user1.fd = new_fd;
                        rooms[fd - 4].user1.room_fd = room_fd;
                        strcpy(rooms[fd - 4].user1.name, caller_user_name);

                        users_in_room.push_back(rooms[fd - 4].user1);

                        send(new_fd, WAITING, strlen(WAITING), 0);
                    }
                    else if(rooms[fd - 4].number_of_players == 1)
                    {
                        rooms[fd - 4].user2.fd = new_fd;
                        rooms[fd - 4].user2.room_fd = room_fd;
                        strcpy(rooms[fd - 4].user2.name, caller_user_name);

                        users_in_room.push_back(rooms[fd - 4].user2);

                        send(new_fd, STARTING, strlen(STARTING), 0);
                        sleep(1);
                        send(rooms[fd - 4].user1.fd, ROCK_PAPER_SCISSORS, strlen(ROCK_PAPER_SCISSORS), 0);
                        sleep(1);
                        send(rooms[fd - 4].user2.fd, ROCK_PAPER_SCISSORS, strlen(ROCK_PAPER_SCISSORS), 0);
                    }

                    rooms[fd - 4].number_of_players ++;

                    pfds.push_back(pollfd{new_fd, POLLIN, 0});
                }
                else
                {   
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(fd, buffer, BUFFER_SIZE, 0);

                    if((user_index = is_message_to_server(fd, users)) != -1) // message from user to server
                    {
                        if(users[user_index].has_name == false)
                        {
                            users[user_index].has_name = true;

                            strcpy(users[user_index].name, buffer);

                            send_rooms_to_user(rooms, total_rooms, users[user_index].fd, false, false);
                        }
                        else
                        {
                            int room_number = stoi(buffer);

                            if((room_number <= 0) || (room_number > total_rooms))
                            {
                                send_rooms_to_user(rooms, total_rooms, users[user_index].fd, true, false);
                            }
                            else if (rooms[room_number - 1].number_of_players == 2)
                            {
                                send_rooms_to_user(rooms, total_rooms, users[user_index].fd, false, true);
                            }
                            else
                            {
                                strcpy(caller_user_name, users[user_index].name);

                                const char* room_port = to_string(rooms[room_number - 1].port).c_str();

                                send(users[user_index].fd, CORRECT_ROOM, strlen(CORRECT_ROOM), 0);
                                send(users[user_index].fd, room_port, strlen(room_port), 0);
                            }
                        }
                    }
                    else if((user_index = is_message_to_server(fd, users_in_room)) != -1) // message from user to room
                    {
                        int room_index = users_in_room[user_index].room_fd - 4;

                        if(users_in_room[user_index].fd == rooms[room_index].user1.fd)
                        {
                            strcpy(rooms[room_index].choice1, buffer);

                            count ++;
                        }
                        else if(users_in_room[user_index].fd == rooms[room_index].user2.fd)
                        {
                            strcpy(rooms[room_index].choice2, buffer);

                            count ++;
                        }

                        if(count == 2)
                        {
                            judging(rooms[room_index], users, rooms, broadcast_fd, bc_addr);

                            count = 0;
                        }
                    }
                }
            }
        }
    }
}