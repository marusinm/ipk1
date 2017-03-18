//
// Created by Marek Marusin on 15/03/2017.
//

#include <iostream>
#include <string>

/**
 * defines header for cliet/server
 */
class HttpHeader {
    std::string command;
    std::string remote_path;
    std::string host;
    int content_length = 0; //default is zero
    int response_code = 200; //default return code
    std::string timestamp;
    std::string accept;
    std::string content_type;
    std::string file_folder_type;
    std::string body; //it can be string or file


    /**
     * helper function to generate time in format fo HTTP header
     *
     * @return string time
     */
    std::string generate_time(){
        char buf[1000];
        time_t now = time(0);
        struct tm tm = *gmtime(&now);
        strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
//        printf("Time is: [%s]\n", buf);

        return std::string(buf);
    }

    /**
     * helper function, capitalize all letters in string
     *
     * @param str input string
     * @return capitalize string
     */
//    std::string capitalizeWord(std::string str){
//        std::transform(str.begin(), str.end(),str.begin(), ::toupper);
//        return str;
//    }

    /**
     * heper function
     *
     * @return string of 'Accept' for HTTP header
     */
    std::string getBackContentAcception(){
        if (this->command.compare("put") == 0 ||
            this->command.compare("lst") == 0 ||
            this->command.compare("mkd") == 0 ||
            this->command.compare("rmd") == 0 ||
            this->command.compare("del") == 0 )
            return std::string("application/json");

        //TODO: pridat else do ifu, to by malo vracat nieco ako file

        return std::string("application/json");
    }

    std::string getRestApiCommand(){
        if(this->command.compare("get") == 0 ||
           this->command.compare("lst") == 0){
            return std::string("GET");

        }else if(this->command.compare("put") == 0 ||
                 this->command.compare("mkd") == 0){
            return std::string("PUT");
        }else{
            return std::string("DELETE");
        }
    }

    std::string findFileFolderType(){
        if(this->command.compare("get") == 0 ||
           this->command.compare("del") == 0 ||
           this->command.compare("put") == 0){
            return std::string("file");

        }else {
            return std::string("folder");
        }
    }

    //getters and setters for private vars
public:
    void setCommand(std::string command){
        this->command = command;
    }
    void setRemotePath(std::string remote_path){
        this->remote_path = remote_path;
    }
    void setHost(std::string host){
        this->host = host;
    }
    void setContentLength(int content_length){
        this->content_length = content_length;
    }
    void setResposneCode(int response_code){
        this->response_code = response_code;
    }
    void setTimeStamp(std::string timestamp){
        this->timestamp = timestamp;
    }
    void setAccept(std::string accept){
        this->accept = accept;
    }
    void setContentType(std::string content_type){
        this->content_type = content_type;
    }
    void setFileFolderType(std::string file_folder_type){
        this->file_folder_type = file_folder_type;
    }
    void setBody(std::string body){
        this->body = body;
    }

    std::string getCommand(){
        return this->command;
    }
    std::string getRemotePath(){
        return this->remote_path;
    }
    std::string getHost(){
        return this->host;
    }
    int getContentLength(){
        return this->content_length;
    }
    int getResposneCode(){
        return this->response_code;
    }
    std::string getTimeStamp(){
        return this->timestamp;
    }
    std::string getAccept(){
        return this->accept;
    }
    std::string getContentType(){
        return this->content_type;
    }
    std::string getFileFolderType(){
        return this->file_folder_type;
    }
    std::string getBody(){
        return this->body;
    }

    /**
     * @return string client header
     */
    std::string getClientHeader(){
        std::string http_client_header = "";
        http_client_header+= getRestApiCommand()+ " /" +this->remote_path +"?type="+findFileFolderType()+ " HTTP/1.1\n";
        http_client_header+= "Host: " + this->host + "\n";
        http_client_header+= "Date: " + generate_time() + "\n";
        http_client_header+= "Accept: " + getBackContentAcception() + "\n";
        http_client_header+= "Accept-Encoding: identity\n";
        http_client_header+= "Content-Type: application/octet-stream\n"; //FIXME: not constant
        http_client_header+= "Content-Length: " + std::to_string(getContentLength()) + "\n";

        return http_client_header;
    }

    /**
     * @return string server header
     */
    std::string getServerHeader(){
        std::string http_server_header = "";
        if (getResposneCode() == 200)
            http_server_header+= "HTTP/1.1 "+ std::to_string(getResposneCode()) +" OK\n";
        else
            http_server_header+= "HTTP/1.1 "+ std::to_string(getResposneCode()) +" ERROR\n";
        http_server_header+= "Date: " + generate_time() + "\n";
        http_server_header+= "Content-Type: text/plain\n"; //FIXME: not constant
        http_server_header+= "Content-Length: " + std::to_string(getContentLength()) + "\n";
        http_server_header+= "Accept-Encoding: identity\n";

        return http_server_header;
    }

};

