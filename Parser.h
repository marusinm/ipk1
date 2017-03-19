#include <sstream>
#include "HttpHeader.h"

//
// Created by Marek Marusin on 15/03/2017.
//
class Parser{

    /**
     * replace char in string
     *
     * @param s string to replace
     * @param ch1 char to be removed
     * @param ch2 char to be added
     */
//    std::string replace_char(std::string s, char ch1, char ch2) {
//        std::replace( s.begin(), s.end(), ch1, ch2);
//        return s;
//    }

//    std::string getNextToken(std::string header, int current_pointer){
//        std::string token = "";
//
//        return token;
//    }
    bool replace(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }


public:
    //TODO upravit aj na parsovanie server header + okomentovat
    HttpHeader headerParser(std::string str_header, bool isHeaderFromServer){
        HttpHeader header;

        //parse line by line
        std::istringstream f(str_header);
        std::string line;

        while (std::getline(f, line)) {
            int location_tab_char = line.find_first_of(" ");
            int loacation_End_chars = line.find_first_of("\r\n");

            if ("Host:" == line.substr(0, location_tab_char))
            {
                replace(line, "Host:", "");
                header.setHost(line);
                continue;
            }
            else if ("Date:" == line.substr(0, location_tab_char))
            {
                replace(line, "Date:", "");
                header.setTimeStamp(line);
                continue;
            }
            else if ("Accept:" == line.substr(0, location_tab_char))
            {
                replace(line, "Accept:", "");
                header.setAccept(line);
                continue;
            }
            else if ("Accept-Encoding:" == line.substr(0, location_tab_char))
            {
                //do nothing
                continue;
            }
            else if ("Content-Type:" == line.substr(0, location_tab_char))
            {
                replace(line, "Content-Type:", "");
                header.setContentType(line);
                continue;
            }
            else if ("Content-Length:" == line.substr(0, location_tab_char))
            {
                replace(line, "Content-Length:", "");
                std::cerr << "test2.1\n" ;
                header.setContentLength(stoi(line));
                std::cerr << "test2.1.1\n" ;
                continue;
            }
            else if(line.find("HTTP/1.1") != std::string::npos) {

                if (!isHeaderFromServer) {
                    //this is parsing of first line
                    std::string command = line.substr(0, location_tab_char);

                    int position = location_tab_char;
                    //fixme why +2 and -5 maybe tho location is not founded right, maybe write my own algorithm
//                std::string remote_path = line.substr(location_tab_char + 1, line.find_last_of(" ") - 4);
//                std::string remote_path = line.substr(location_tab_char + 1, line.find_first_of("?") - 4)
                    std::string remote_path = "";
                    for (int i = position + 1; i < line.size(); ++i) {
                        if (line.at(i) == '?') {
                            break;
                        }
                        remote_path += line.at(i);
                    }

                    //check if file or folder come in rest api
                    std::string file_folder_type = "";
                    position = line.find_first_of("?") + 6;
                    for (int i = position; i < position + 4; ++i) {
                        file_folder_type += line.at(i);
                    }
                    if (!file_folder_type.compare("file") == 0) {
                        file_folder_type = "folder";
                    }

                    header.setCommand(command);
                    header.setRemotePath(remote_path);
                    header.setFileFolderType(file_folder_type);
                } else {
                    std::string response_code = "";
                    for (int i = location_tab_char + 1; i < location_tab_char + 4; ++i) {
                        response_code += line.at(i);
                    }
                    std::cerr << "test2.2\n" ;
                    header.setResposneCode(stoi(response_code));
                    std::cerr << "test2.2.1\n" ;
                };
                continue;
            }

        }
        size_t pointer = str_header.find("\r\n\r\n");
        std::string body = str_header.substr(pointer+4,pointer+header.getContentLength());
        header.setBody(body);

        return header;
    }
};
