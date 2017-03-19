#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include "Parser.h"

#define BUFSIZE 1024
//#define DEBUG_OPTIONS true;
#define DEBUG_HEADER_INFO true;


/**
 * CLIENT side
 */
int main (int argc, const char * argv[]) {
    int client_socket, int_port, bytestx, bytesrx;
    socklen_t serverlen;
    const char *server_hostname;
    struct hostent *server;
    struct sockaddr_in server_address;
    char buf[BUFSIZE];
    std::string command = "";
    std::string remote_path_unparsed = "";
    std::string remote_path_parsed = "";
    std::string hostname = "";
    std::string local_path = "";
    std::string str_port = "";


    //hending program option //TODO:ASK: local path je povyny pri put command, a ked nie je put zadany ? nemas osetrene
    if (argc < 3) {
        fprintf(stderr, "usage: %s COMMAND REMOTE-PATH [LOCAL-PATH]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    command = argv[1];
    remote_path_unparsed = argv[2];
    if (command.compare("put") == 0) { //TODO:ASK: ma byt command case ssensitive
        if (argc < 4) {
            fprintf(stderr, "usage: %s COMMAND REMOTE-PATH [LOCAL-PATH]\n", argv[0]);
            fprintf(stderr, "[LOCAL-PATH] is required if PUT command is used\n");
            exit(EXIT_FAILURE);
        } else {
            local_path = argv[3];
        }
    }

    //TODO:ASK check if command exists -> mame to kontrolovat ci na to server nieco odpovie zakazdym ze to napr nepozna

    std::string http = "";
    bool is_http_found = false;
    bool is_hostname_founded = false;
    bool is_port_founded = false;

    //go through string and find protocol, paths and port
    for (char &c : remote_path_unparsed) {

        //check if http:// is used
        if (!is_http_found) {
            http += c;
            if (http.compare("http://") == 0) {
                is_http_found = true;
            }
        } else {

            if (!is_hostname_founded) { //find remote path
                if (c == ':') {
                    is_hostname_founded = true;
                    continue;
                }
                hostname += c;
            } else {

                if (!is_port_founded) { //find port
                    if (c == '/') {
                        is_port_founded = true;
                        continue;
                    }
                    str_port += c;
                } else {
                    remote_path_parsed += c;
                }
            }

        }
    }

#ifdef DEBUG_OPTIONS
    std::cerr << "http: " << http << "\n";
    std::cerr << "hostname: " << hostname<< "\n";
    std::cerr << "port: " << str_port<< "\n";
    std::cerr << "remote: " << remote_path_parsed<< "\n";
    std::cerr << "local path: " << local_path<< "\n";
#endif

    //finish if remove path option is not in good format
    if (!is_http_found) {
        fprintf(stderr, "ERROR: bad protocol (use http://)\n");
        exit(EXIT_FAILURE);
    }
    if (!is_hostname_founded) {
         fprintf(stderr, "ERROR:hostname was not found\n");
        exit(EXIT_FAILURE);
    }

    try {
        int_port = std::stoi(str_port);
    } catch (const std::exception &e) {
        fprintf(stderr, "Port number must be integer!\n");
        exit(EXIT_FAILURE);
    }

    if (!is_port_founded){
        int_port = 6677; //default port
    }


    /* 2. ziskani adresy serveru pomoci DNS */
    server_hostname = hostname.c_str();
    if ((server = gethostbyname(server_hostname)) == NULL) {
        fprintf(stderr, "ERROR: no such host as %s\n", server_hostname);
        exit(EXIT_FAILURE);
    }

    /* 3. nalezeni IP adresy serveru a inicializace struktury server_address */
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(int_port);

    /* tiskne informace o vzdalenem soketu */
//    printf("INFO: Server socket: %s : %d \n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

    /* Vytvoreni soketu */
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }

    /* nacteni zpravy od uzivatele */
    bzero(buf, BUFSIZE);
//    printf("Please enter msg: ");
//    fgets(buf, BUFSIZE, stdin);

    HttpHeader client_header;
    client_header.setCommand(command);
    client_header.setRemotePath(remote_path_parsed);
    client_header.setHost(hostname);
    //TODO dorobit ify, doplnit hlavicku pre jednotlive prikazy-> content type, content length
//    if (command.compare("lst") == 0) {
//        //TODO: content type
//    }else if (command.compare("put") == 0){
//        //TODO: content type, content length
//    }
    strcat(buf, client_header.getClientHeader().c_str());
    std::cerr << "Client header: \n"<< client_header.getClientHeader().c_str() << std::endl;

    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
        perror("ERROR: connect");
        exit(EXIT_FAILURE);
    }

    /* odeslani zpravy na server */
    bytestx = send(client_socket, buf, strlen(buf), 0);
    if (bytestx < 0)
        perror("ERROR in sendto");

    /* prijeti odpovedi a jeji vypsani */
    std::string str_respond = "";
    //recive while there is some data
//    char respond_buf[BUFSIZE];
//    while ((bytesrx = recv(client_socket, (void *)respond_buf, BUFSIZE, 0)) > 0){

    bzero(buf, BUFSIZE);
//    if(command.compare("get") != 0) {
        while ((bytesrx = recv(client_socket, buf, BUFSIZE, 0)) > 0) {
            str_respond.append(buf);
            bzero(buf, BUFSIZE);
        }
//    }else{
//
        //if get read only header one time
//        bytesrx = recv(client_socket, buf, BUFSIZE, 0);
//        str_respond.append(buf);
//        bzero(buf, BUFSIZE);
//
//    }
    std::cerr<<"heder from server: "<< str_respond <<"\n";
    Parser parser;
    HttpHeader respond_header = parser.headerParser(str_respond, true);
#ifdef DEBUG_HEADER_INFO
//    std::cerr << "FINAL ECHO: \n" << str_respond << "\n";
//    std::cerr << "respond code: \n" << respond_header.getResposneCode()<< "\n";
//    std::cerr << "Body: \n" << respond_header.getBody()<< "\n\n\n";
#endif

    if (respond_header.getResposneCode() != 200){
        std::cerr << respond_header.getBody()<< "\n";
    }else{
        if (command.compare("lst") == 0){
            std::istringstream iss(respond_header.getBody());
            for (std::string line; std::getline(iss, line); )
            {
                std::cout << line << "\t";
            }
            std::cout << std::endl;
        }else if(command.compare("get") == 0){
//            char buffer[BUFSIZE];
//            int loaded_data = 0;
//            std::string income_file = "";
//            while ((bytesrx = recv(client_socket, buffer, BUFSIZE, 0)) > 0){
//                income_file.append(buffer);
//                bzero(buffer, BUFSIZE);
//
////                if(bytesrx == BUFSIZE)
////                    loaded_data+=BUFSIZE;
//                loaded_data+=BUFSIZE;
//
//                if (std::stoi(respond_header.getBody()) <= loaded_data){
//                    std::cerr << "here stop!!!"<<"\n";
//                    break;
//                }
//
//                if(bytesrx < BUFSIZE-1){
//                    break;
//                }
//            }
//            std::ofstream out("/Users/Marek/Documents/output.txt",std::ios::binary);
//            out << income_file;

            std::ofstream out("/Users/Marek/Documents/output.txt",std::ios::binary);
            std::cerr << "test5\n" ;
            out << respond_header.getBody();
        }
    }
    close(client_socket);
    return 0;
}
