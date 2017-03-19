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
    int client_socket, int_port, bytesrx;
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

    /* Vytvoreni soketu */
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }

    /* nacteni zpravy od uzivatele */
    bzero(buf, BUFSIZE);
    HttpHeader client_header;
    client_header.setCommand(command);
    client_header.setRemotePath(remote_path_parsed);
    client_header.setHost(hostname);
    //TODO dorobit ify, doplnit hlavicku pre jednotlive prikazy-> content type, content length
    if (command.compare("put") == 0){
        std::ifstream input( local_path, std::ios::binary );
        std::string str((std::istreambuf_iterator<char>(input)),
                        std::istreambuf_iterator<char>());
        client_header.setBody(str);
        client_header.setContentLength(str.size());
    }

    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
        perror("ERROR: connect");
        exit(EXIT_FAILURE);
    }

    /* odeslani zpravy na server */
    std::string final_message = client_header.getClientHeader() + "\r\n\r\n" + client_header.getBody();
//    std::cerr << "Client header: \n"<< final_message << std::endl;
    if(send(client_socket, final_message.data(), final_message.size(), 0) < 0){
        std::cerr << "Unknown error.\n";
    };

    /* prijeti odpovedi a jeji vypsani */
    std::string str_respond = "";
    bzero(buf, BUFSIZE);
    while ((bytesrx = recv(client_socket, buf, BUFSIZE, 0)) > 0) {
        str_respond.append(buf);
        bzero(buf, BUFSIZE);
        if (bytesrx <= 0)
            break;

        if(bytesrx < BUFSIZE-1){
            break;
        }
    }
    Parser parser;
    HttpHeader respond_header = parser.headerParser(str_respond, true);

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
            std::string path = "./";
            std::string output_file_name = parser.parseFileFromPath(client_header.getRemotePath());
            path+=output_file_name;
//            std::cerr<<"path: "<<path<<"\n";
            std::ofstream out(path,std::ios::binary);
            out << respond_header.getBody();
        }
    }
    close(client_socket);
    return 0;
}
