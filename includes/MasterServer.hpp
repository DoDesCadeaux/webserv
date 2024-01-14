#pragma once

#include "./Server.hpp"
#include "./Client.hpp"
#include "./CGI.hpp"
#include <list>

class Server;
class Client;

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
    void 						run();
    bool 						recvAll(const int &fd);
    bool 						sendAll(const int &fd);
    void 						saveFile(const int &fd, const std::string &fileData, const std::string &mimeType);

    void 						addFd(int fd);
    void 						newConnection(const int &fd);
    void 						killConnection(const int &fd);
    void 						removeFd(int fd);

    std::string 				getResourceContent(const std::string &uri, int fd);

	bool						isCgiRequest(const Request &request);
};