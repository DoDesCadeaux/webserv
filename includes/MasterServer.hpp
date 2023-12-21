#pragma once

#include "./Server.hpp"
#include <list>

class Server;

class MasterServer
{
private:
    fd_set 						_allfds;
    fd_set 						_readfds;
    fd_set 						_writefds;
    std::list<int> 				_listfds;
    int 						_maxfd;
    std::map<int, Client *>		_clients;
    std::vector<Server>			_servers;
    std::map<std::string, int>	_ports;

public:
    MasterServer();
    MasterServer &operator=(MasterServer const &other);
    ~MasterServer();

    // Setter
    void setServer(std::vector<Server> &servers);

    // Getter
    std::map<std::string, int>  &getPorts();
    std::vector<Server>         &getServers();
	std::map<int, Client *>		&getClients();
    Server 						&getServerBySocketPort(int port);
    Server 						&getServerByClientSocket(int port);

    // Methods
    void run();
    bool recvAll(const int &fd);
    bool sendAll(const int &fd);
    void saveFile(const int &fd, const std::string &fileData, const std::string &directoryPath, const std::string &mimeType);
    void saveImage(const std::string& imageData, const std::string& filePath);

    void addFd(int fd);
    void newConnection(const int &fd);
    void killConnection(const int &fd);
    void removeFd(int fd);

    //Perdu dans la bataille
        //saveImage
        //generateRandomFileName

};