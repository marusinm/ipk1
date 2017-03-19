#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <fstream>

class CurrentCommand{
private:
    std::string command;// string command (mkd, lst ...)
    std::string error;  //error message
    int response_code;  //response code
    std::string response_body; //filled by command (lst ...)

public:
    bool error_found = false;

    void setCommand(std::string command){
        this->command = command;
    }
    void setError(std::string error){
        this->error = error;
        this->error_found = true;
    }
    void setResponseCode(int response_code){
        this->response_code = response_code;
    }
    void setResponseBody(std::string response_body){
        this->response_body = response_body;
    }

    std::string getCommand(){
        return this->command;
    }
    std::string getError(){
        return this->error;
    }
    int getResponseCode(){
        return this->response_code;
    }
    std::string getResponseBody(){
        return this->response_body;
    }
    bool getErrorFound(){
        return this->error_found;
    }
}current_command;



class Commander{
    HttpHeader httpHeader;
    std::string root_folder = "";

    bool checkIfDirectoryExists(std::string remote_path){
        DIR *dir = opendir(remote_path.c_str());
        if (dir == NULL) {
            return false;
        }
        return true;
    }

    bool checkIfUserAccountExists(std::string remote_path){
        if (root_folder.compare(".") == 0){
            return true;
        }
        std::string str_user = "/";
        int slashcount = 0;
        for(char& c : remote_path) {

            if (slashcount == 2){//user is finded
                break;
            }

            if(c == '/') {
                slashcount++;
                continue;
            }
            str_user+=c;
        }
        DIR *dir = opendir(str_user.c_str());
        if (dir == NULL) {
            return false;
        }
        return true;
    }

    /**
     * checks if remote path is just path to user
     * @return true if remote path is path only to user
     */
    bool checkIfPathIsOnlyUser(std::string remote_path){
        std::string str_user = "/";
        int slashcount = 0;
        for(char& c : remote_path) {

            if (slashcount == 2){//user is finded
                break;
            }

            if(c == '/') {
                slashcount++;
                continue;
            }
            str_user+=c;
        }
        if (str_user.compare(remote_path) == 0){
            return true;
        }
        return false;
    }

    int checkIfPathIsFileOrDirectory(std::string remote_path){
        struct stat s;
        if( stat(remote_path.c_str(),&s) == 0 )
        {
            if( s.st_mode & S_IFDIR )
            {
                //it's a directory
                return 1;
            }
            else if( s.st_mode & S_IFREG )
            {
                //it's a file
                return 2;
            }
            else
            {
                //something else
                return 0;
            }
        }
        else
        {
            //error
            return 0;
        }
    }


    std::string getFile(std::string remote_path){
//                std::cerr << "file size: " << getFileSize(remote_path) << "\n";


        std::ifstream input( remote_path, std::ios::binary );
        std::string str((std::istreambuf_iterator<char>(input)),
                        std::istreambuf_iterator<char>());

        return str;

//        unsigned char buffer[getFileSize(remote_path)];
//        FILE *ptr;
//        ptr = fopen(remote_path.c_str(),"rb");  // r for read, b for binary
//        fread(buffer,sizeof(buffer),1,ptr); // read 10 bytes to our buffer
//        std::cerr << "output file \n" << buffer << "\n";
//
//        char buffer[1024];
//        FILE *ptr;
//        ptr = fopen(remote_path.c_str(), "rb");  // r for read, b for binary
//        int position = 0;
//        std::string file = "";
//        while (position<=this->getFileSize(remote_path)){
//            fread(buffer, sizeof(buffer), 1, ptr);
//            position+=1024;
//            file+=buffer;
//        }
//        fclose(ptr);
//        std::cerr << "output file \n" << file << "\n";
    }


    /**
     * checks and execute mkdir
     *
     * @result false if error ocured (error info in current_command)
     */
    bool mkd(std::string remote_path){
        //check if user exist
        if(false == checkIfUserAccountExists(remote_path)){
            current_command.setError("User Account Not Found");
            current_command.setResponseCode(404);
            return false;
        }
        //check if derectory exist
        if(true == checkIfDirectoryExists(remote_path)){
            current_command.setError("Already exists.");
            current_command.setResponseCode(404);
            return false;
        }
        // create dir and chek if was created
        if (0 != mkdir(remote_path.c_str(),0777)) {
//            perror("mkdir");
            current_command.setError("Directory not found.");
            current_command.setResponseCode(404);
            return false;
        }

        return true;
    }

    /**
     * checks and execute rmdir
     *
     * @result false if error ocured (error info in current_command)
     */
    bool rmd(std::string remote_path){
        //check if user exist
        if(false == checkIfUserAccountExists(remote_path)){
            current_command.setError("User Account Not Found");
            current_command.setResponseCode(404);
            return false;
        }
        //check if we try to delete user account folder
        if (true == checkIfPathIsOnlyUser(remote_path)){
            current_command.setError("Unknown error.");
            current_command.setResponseCode(404);
            return false;
        }
        //check if path is pointer to directory
        if (2 == checkIfPathIsFileOrDirectory(remote_path)){
            current_command.setError("Not a directory.");
            current_command.setResponseCode(400);
            return false;
        }
        //check if derectory exist
        if(false == checkIfDirectoryExists(remote_path)){
            current_command.setError("Directory not found.");
            current_command.setResponseCode(404);
            return false;
        }
        // remove dir and chek if was removed
        if (0 != rmdir(remote_path.c_str())) {
//            perror("rmdir");
            current_command.setError("Unknown error.");
            current_command.setResponseCode(404);
            return false;
        }
        return true;
    }

    /**
     * implementation of ls command
     * return empty string if error ocured, string of files otherwise
     */
    std::string lst(std::string remote_path){
        std::string result = "";
        //check if user exist
        if(false == checkIfUserAccountExists(remote_path)){
            current_command.setError("User Account Not Found");
            current_command.setResponseCode(404);
//            return std::string("");
            return result;
        }
        //check if path is pointer to directory
        if (2 == checkIfPathIsFileOrDirectory(remote_path)){
            current_command.setError("Not a directory.");
            current_command.setResponseCode(400);
            return result;
        }
        //check if derectory exist
        if(false == checkIfDirectoryExists(remote_path)){
            current_command.setError("Directory not found.");
            current_command.setResponseCode(404);
            return result;
        }
        DIR *mydir;
        struct dirent *myfile;
        struct stat mystat;

        mydir = opendir(remote_path.c_str());
        while((myfile = readdir(mydir)) != NULL)
        {
            stat(myfile->d_name, &mystat);
//            printf(" %s\n", myfile->d_name);
            result += myfile->d_name;
            result += "\n";
        }
        closedir(mydir);
        current_command.setResponseBody(result);
        return result;
    }

    bool del(std::string remote_path){
        //check if user exist
        if(false == checkIfUserAccountExists(remote_path)){
            current_command.setError("User Account Not Found");
            current_command.setResponseCode(404);
            return false;
        }
//        //check if we try to delete user account folder
//        if (true == checkIfPathIsOnlyUser(remote_path)){
//            current_command.setError("Unknown error.");
//            current_command.setResponseCode(404);
//            return false;
//        }
        //check if path is pointer to file
        if (1 == checkIfPathIsFileOrDirectory(remote_path)){
            current_command.setError("Not a file.");
            current_command.setResponseCode(400);
            return false;
        }
        // remove file and chek if was removed
        if (0 != remove(remote_path.c_str())) {
//            perror("rmdir");
            current_command.setError("Unknown error.");
            current_command.setResponseCode(404);
            return false;
        }
        return true;
    }

    bool get(std::string remote_path) {
        //check if user exist
        if (false == checkIfUserAccountExists(remote_path)) {
            current_command.setError("User Account Not Found");
            current_command.setResponseCode(404);
            return false;
        }
        //check if path is pointer to file
        if (1 == checkIfPathIsFileOrDirectory(remote_path)) {
            current_command.setError("Not a file.");
            current_command.setResponseCode(400);
            return false;
        }
//        int size = getFileSize(remote_path);
//        current_command.setResponseBody(std::to_string(size));
        current_command.setResponseBody(getFile(remote_path));
        return true;
    }

    bool put(std::string remote_path) {
        //check if user exist
        if (false == checkIfUserAccountExists(remote_path)) {
            current_command.setError("User Account Not Found");
            current_command.setResponseCode(404);
            return false;
        }
        //check if path is pointer to file
        if (1 == checkIfPathIsFileOrDirectory(remote_path)) {
            current_command.setError("Not a file.");
            current_command.setResponseCode(400);
            return false;
        }
//        std::cerr<<"path: "<<remo<<"\n";
        std::ofstream out(remote_path,std::ios::binary);
        out << httpHeader.getBody();
        return true;
    }


public:
//    long getFileSize(std::string filename)
//    {
//        struct stat stat_buf;
//        int rc = stat(filename.c_str(), &stat_buf);
//        return rc == 0 ? stat_buf.st_size : -1;
//    }
//    static int getFileSize(std::string filename) // path to file
//    {
//        FILE *p_file = NULL;
//        p_file = fopen(filename.c_str(),"rb");
//        fseek(p_file,0,SEEK_END);
//        int size = ftell(p_file);
//        fclose(p_file);
//        return size;
//    }

    Commander(std::string root){
        this->root_folder = root;
    }

    bool do_cmd_from_header(HttpHeader httpHeader){
        this->httpHeader = httpHeader;
//        std::cerr << "parsed header on server: \n" << httpHeader.getClientHeader() << "\n";

        bool isExecuted = true;
        std::string command = httpHeader.getCommand();
        std::string file_folder_type = httpHeader.getFileFolderType();
        std::string path = root_folder + httpHeader.getRemotePath();
        current_command.setCommand(command);
        if (command.compare("PUT") == 0 &&
            file_folder_type.compare("folder") == 0){                   //mkd command
            isExecuted = mkd(path);

        }else if(command.compare("DELETE") == 0 &&
                 file_folder_type.compare("folder") == 0){              //rmd command
            isExecuted = rmd(path);

        }else if(command.compare("GET") == 0 &&
                 file_folder_type.compare("folder") == 0){              //lst command
            std::string result = lst(httpHeader.getRemotePath());
            if (result.compare("") == 0)
                isExecuted = true;
            else
                isExecuted = false;

        }else if(command.compare("PUT") == 0 &&
                 file_folder_type.compare("file") == 0){                 //put command
            isExecuted = put(path);

        }else if(command.compare("DELETE") == 0 &&
                 file_folder_type.compare("file") == 0) {                //del command
            isExecuted = del(path);

        }else if(command.compare("GET") == 0 &&
                 file_folder_type.compare("file") == 0){                 //get command
            isExecuted = get(path);
        }

        if (!isExecuted){
            return false;
        }
        return true;
    }
};
