#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include "Parser.h"
#include "Commander.h"

/**
 * SERVER side
 */
int main (int argc, const char * argv[]) {
    int rc;
    int welcome_socket;
    struct sockaddr_in6 sa;
    struct sockaddr_in6 sa_client;
    char str[INET6_ADDRSTRLEN];
    int port_number;
    std::string root_folder;

    //hending program options TODO:ASK osetrit aj viac krat zadanuy rovnaky parameter?
    if (argc > 6) {
        fprintf(stderr,"usage: %s  [-r ROOT-FOLDER] [-p PORT]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(strcmp(argv[1],"-r") == 0) {
        root_folder = argv[2];
    }else if(strcmp(argv[1],"-p") == 0){
        try {
            port_number = std::stoi(argv[2]);
        } catch (const std::exception& e) {
            fprintf(stderr,"Port number must be integer!\n");
            exit(EXIT_FAILURE);
        }
    }else{
        fprintf(stderr,"usage: %s  [-r ROOT-FOLDER] [-p PORT]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[3],"-p") == 0) {
        try {
            port_number = std::stoi(argv[4]);
        } catch (const std::exception& e) {
            fprintf(stderr,"Port number must be integer!\n");
            exit(EXIT_FAILURE);
        }
    }else if(strcmp(argv[3],"-r") == 0){
        root_folder = argv[4];
    }else{
        fprintf(stderr,"usage: %s  [-r ROOT-FOLDER] [-p PORT]\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    socklen_t sa_client_len=sizeof(sa_client);
    if ((welcome_socket = socket(PF_INET6, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }

    memset(&sa,0,sizeof(sa));
    sa.sin6_family = AF_INET6;
    sa.sin6_addr = in6addr_any;
    sa.sin6_port = htons(port_number);



    if ((rc = bind(welcome_socket, (struct sockaddr*)&sa, sizeof(sa))) < 0)
    {
        perror("ERROR: bind");
        exit(EXIT_FAILURE);
    }
    if ((listen(welcome_socket, 1)) < 0)
    {
        perror("ERROR: listen");
        exit(EXIT_FAILURE);
    }

    //fixme: odstranit tieto dva riadky, su tu len nato aby mi nesvietil nekonecny cyklus
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while(1)
    {
        int comm_socket = accept(welcome_socket, (struct sockaddr*)&sa_client, &sa_client_len);
        if (comm_socket > 0)
        {
            char buff[1024];
            int res = 0;
//            for (;;) //TODO:ASK spytat sa co s tymto cyklom
//            {
                //refreah vars
            bzero(buff, 1024);
            current_command.error_found = false;

            res = recv(comm_socket, buff, 1024,0);
            if (res <= 0)
                break;

//            std::cout << "test: " << buff << "\n";
            Parser parser;
            HttpHeader client_header = parser.headerParser(buff, false);
//                std::cerr << "parsed finished: \n" << client_header.getRemotePath() << "\n";
//                std::cerr << "parsed finished2: \n" << client_header.getFileFolderType() << "\n";

            Commander commander;
            commander.do_cmd_from_header(client_header);


            std::string command = client_header.getCommand();
            std::string file_folder_type = client_header.getFileFolderType();

            HttpHeader server_header;
            std::string msg_body = "\r\n\r\n";
            if(current_command.error_found){
//                    std::cerr << "error: "<< current_command.getError() << "\n";
                server_header.setResposneCode(current_command.getResponseCode());
                msg_body += current_command.getError();
            }else{
                std::cerr << "success\n";

//                msg_body += current_command.getResponseBody();
                msg_body.append(current_command.getResponseBody());
            }
            std::cerr << "server 1\n";
            server_header.setContentLength(strlen(msg_body.c_str()));

            std::cerr << "server 2\n";
            std::string final_message = server_header.getServerHeader();
            std::cerr << "server 2.2\n";
            final_message += msg_body;
//                send(comm_socket, buff, strlen(buff), 0);
//            if(send(comm_socket, final_message.c_str(), strlen(final_message.c_str()), 0) < 0){
            std::cerr << "server 3\n";
            if(send(comm_socket, final_message.c_str(), strlen(final_message.c_str()), 0) < 0){
//            if(send(comm_socket, final_message.data(), final_message.size(), 0) < 0){
                std::cerr << "Unknown error.\n";
            };
            std::cerr << "server 4\n";
            //check if command is a get or put
//            if(command.compare("GET") == 0 &&
//                    file_folder_type.compare("file") == 0){                 //get command
//                //TODO: iterativne posielanie suboru
//                char buffer[1024];
//                FILE *ptr;
//                ptr = fopen(client_header.getRemotePath().c_str(), "rb");  // r for read, b for binary
//                int position = 0;
//                std::string file = "";
//                while (position <= Commander::getFileSize(client_header.getRemotePath())){
//                    bzero(buffer,1024);
//                    fread(buffer, sizeof(buffer), 1, ptr);
//                    position+=1024;
//                    file+=buffer;
//
//
//                    send(comm_socket, buffer, strlen(buffer), 0);
//                }
//                fclose(ptr);
////                std::cerr << "output file \n" << file << "\n";
//
//            }else if (command.compare("PUT") == 0 &&
//                        file_folder_type.compare("file") == 0){             //put command
//                //TODO: iterativne prijmanie suboru
//            }

        }
//        }
        else
        {
            printf(".");
        }
        printf("Connection to %s closed\n",str);
        close(comm_socket);
    }
}
