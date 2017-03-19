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
int main (int argc, char **argv) {
    int rc;
    int welcome_socket;
    struct sockaddr_in6 sa;
    struct sockaddr_in6 sa_client;
    char str[INET6_ADDRSTRLEN];
    int port_number;
    std::string root_folder = ".";

    if (argc > 1) {
        int opt;
        while ((opt = getopt(argc, argv, "r:p:")) != EOF) {
            switch (opt) {
                case 'r':
                    root_folder = optarg;
                    break;
                case 'p':
                    port_number = strtol(optarg, &optarg, 0);
                    break;
            }
        }
    }

    if (root_folder.back() == '/'){
        root_folder = root_folder.substr(0, root_folder.size()-1);
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
#pragma clang diagnostic ignored "-Wmissing-noreturn"os
    while(1)
    {
        int comm_socket = accept(welcome_socket, (struct sockaddr*)&sa_client, &sa_client_len);
        if (comm_socket > 0)
        {
            char buff[1024];
            int res = 0;

            bzero(buff, 1024);
            current_command.error_found = false;

            std::string str_income_message = "";
            while ((res = recv(comm_socket, buff, 1024,0)) > 0) {
                str_income_message.append(buff);
                bzero(buff, 1024);
                if (res <= 0)
                    break;

                if(res < 1024-1){
                    break;
                }
            }

            Parser parser;
            HttpHeader client_header = parser.headerParser(str_income_message, false);
//            client_header.setRemotePath(root_folder+=client_header.getRemotePath());

            Commander commander(root_folder);
            commander.do_cmd_from_header(client_header);

            HttpHeader server_header;
            std::string msg_body = "\r\n\r\n";
            if(current_command.error_found){
                server_header.setResposneCode(current_command.getResponseCode());
                msg_body += current_command.getError();
            }else{
//                std::cerr << "success\n";
                msg_body.append(current_command.getResponseBody());
            }
            server_header.setContentLength(strlen(msg_body.c_str()));

            std::string final_message = server_header.getServerHeader();
            final_message += msg_body;
            if(send(comm_socket, final_message.data(), final_message.size(), 0) < 0){
                std::cerr << "Unknown error.\n";
            };
        }
        else
        {
            printf(".");
        }
//        printf("Connection to %s closed\n",str);
        close(comm_socket);
    }
}
