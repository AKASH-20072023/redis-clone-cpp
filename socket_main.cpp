#include<iostream>
#include<cerrno>
#include<cstring>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sstream>
#include<chrono>
#include<unordered_map>
#include<thread>
#include<mutex>



//lets update our unordered map so that it can also implement TTL
struct Entry{
    std:: string value;
    std:: chrono:: steady_clock:: time_point expiry;
};

std::unordered_map<std::string,Entry>database;

std:: mutex db_mutex;   //will treat db_mutex as a lock for the database.

//lets create functions 
void sendResponse(int client_fd,const std:: string &response)
{
    write(client_fd,response.c_str(),response.size());
}


void handleSet(std:: stringstream &ss,int client_fd)
{
    std:: string key;
    ss>>key;  

    std:: string value;
    std:: string extra;
    ss>>value;
    std:: string word;
    while(ss>>word)
    {
        if(word=="EX")         //checking for the TTL
        {
            break;
        } 
        if(!value.empty())
            {
                value+=" ";
            }            
         value+=word;
    }

    Entry entry;
    entry.value=value;
                    
    if(word=="EX")   //MEans TTL
    {
        int seconds;
        ss>>seconds;
        entry.expiry=std:: chrono:: steady_clock:: now()+std:: chrono:: seconds(seconds);
    }
    else //-> NO TTL
    {
        entry.expiry=std:: chrono:: steady_clock:: time_point:: max();
    }

    {
        std:: lock_guard<std:: mutex>lock(db_mutex);
        database[key]=entry;
    }
    

    sendResponse(client_fd,"OK\n");
}

void handleGet(std:: stringstream &ss,int client_fd)
{
    std:: string key;
    ss>>key;
    std:: lock_guard<std:: mutex>lock(db_mutex);
    auto it=database.find(key);
    if(it!=database.end())      //Find the key as it points on that key 
    {
                    
        if(it->second.expiry!=std:: chrono :: steady_clock:: time_point:: max())
        {
            if(std:: chrono:: steady_clock:: now()>=it->second.expiry)
            {
                database.erase(it);
                sendResponse(client_fd,"(nil)\n");
                return;
            }
        }
        std:: string response=it->second.value+ "\n";
        sendResponse(client_fd,response);
    }
    else 
    {
        sendResponse(client_fd,"(nil)\n");
    }
}

void handelDel(std::stringstream &ss,int client_fd)
{
    std:: string key;
    int count=0;
    std:: lock_guard<std:: mutex> lock(db_mutex); 
    while(ss>>key)
    {
        count+=database.erase(key);
    }

    std :: string response=std:: to_string(count)+"\n";
    sendResponse(client_fd,response);
}

void handleClient(int client_fd)   // Handle each client in a separate thread 
{
    std:: cout<<"Client Connected! FD : "<<client_fd<<std:: endl;

    while(true)     //--> As after each iteration my data is getting vanished from the database so to make permanent 
    {
        //Read from the client 
        char buffer[1024];
        ssize_t bytesRead=read(client_fd,buffer,sizeof(buffer)-1);   //preserving the pos of \0 so that it doesn't go out of bound
        if(bytesRead>0)
        {
            buffer[bytesRead]='\0';   //as read() gives us raw bytes & doesnt add \0 -> THE C STRING TERMINATOR 
            std :: string command(buffer);
                
            //std::cout<<"Client Sent : "<<command<<std::endl;
                
            //std::string response ="Hello from Redis Server";
            //write(client_fd,response.c_str(),response.size());
            
            std:: stringstream ss(command);

            std:: string operation;
            // std:: string key;    --> NEEDS Modification for Deleting multi keys at one go 
            // std:: string value;   //lets dont take it like the value can be like -> Akash Chatterjee

            ss>>operation;
            // ss>>key;
            // ss>>value;

            // std:: cout<<"Operation is : "<<operation<<std :: endl;
            // std:: cout<<"Key is : "<<key<<std::endl;
            // std:: cout<<"Value is : "<<value<<std::endl; 

            //storing the value 
                
            if(operation=="SET")
            {
                handleSet(ss,client_fd);
            }
            else if(operation=="GET")
            {
                handleGet(ss,client_fd);
            }
            else if(operation=="DEL")
            {
                handelDel(ss,client_fd);         
            }
            else  //Means client have sent unkonwn command 
            {
                std:: string response="unknown Command\n";
                sendResponse(client_fd,response);
            }
        }
        else if(bytesRead==0)
        {
            std::cout<<"Client Disconnected\n";
            break;
        }
        else 
        {
            std:: cout<<"Read Failed"<<strerror(errno)<<std::endl;
            break;
        }
    }
    close(client_fd);
}


int main(){
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd==-1)
    {
        std::cout<<"SOcket Creation Failed"<<strerror(errno)<<std::endl;
        close(server_fd);
        return 1;
    }
    std::cout<<"Server Socket fd is : "<<server_fd<<std::endl;
    sockaddr_in server_addr{};
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(6379);
    server_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);

    //so whenever we close the server the linux still kept some information in it so whenever we try to on the server agin on the same port 
    //Its stats its already in use -> so lets use SO_REUSEADDR

    int opt=1;
    if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))==-1)
    {
        std:: cout<<"SetSockOpt Failed: "<<strerror(errno)<<std::endl;

        close(server_fd);
        return 1;
    }



    int result=bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    if(result==-1)
    {
        std::cout<<"Bind Failed"<<strerror(errno)<<std::endl;
        close(server_fd);
        return -1;
    }

    result=listen(server_fd,10);
    if(result==-1)
    {
        std::cout<<"Listen Failed"<<strerror(errno)<<std::endl;
        close(server_fd);
        return 1;
    }
    std::cout<<"Server is Listening\n";

    while(true)   //-> As without this our code runs for only one clients but with while-> we can deal with multi clients 
    {
        int client_fd=accept(server_fd,nullptr,nullptr);
        if(client_fd==-1)
        {
            std::cout<<"Accept Faled"<<strerror(errno)<<std::endl;
            close(server_fd);
            return 1;
        }
        
        //now lets add threading here 

        std:: thread(handleClient,client_fd).detach();
    }

    close(server_fd);
    return 0;
}